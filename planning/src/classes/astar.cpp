#include <astar.h>
#include <queue>
#include <ros/ros.h>

#include <algorithm>
#include <math.h>
#include <string>

using geometry_msgs::Point;
using geometry_msgs::PoseStamped;
using nav_msgs::Path;

geometry_msgs::Point point;

#define RVIZ_COMPATABILITY

Point AStar::getPoint(double x, double y)
{
  // ROS should really add constructors...
  Point p;
  p.x = x;
  p.y = y;
  return p;
}

double AStar::distance(int ind1, int ind2, int width)
{
  // Return Euclidean Distance.
  auto pt1 = std::make_pair<int, int>(ind1 % width, (int)floor(ind1 / width));
  auto pt2 = std::make_pair<int, int>(ind2 % width, (int)floor(ind2 / width));
  return sqrt((pt1.first - pt2.first) * (pt1.first - pt2.first) + (pt1.second - pt2.second) * (pt1.second - pt2.second));
}

std::array<int, 8> AStar::getNeighborsIndiciesArray(int pt, int widthOfGrid, int sizeOfGrid)
{
  // Get the neighbors around any given index (ternary operators to ensure the points remain within bounds)
  std::array<int, 8> neighbors;

  neighbors[0] = ((pt + 1) < 0)               || ((pt + 1) > sizeOfGrid) ? -1               : pt + 1;
  neighbors[1] = ((pt - 1) < 0)               || ((pt - 1) > sizeOfGrid) ? -1               : pt - 1;
  neighbors[2] = ((pt + widthOfGrid) < 0)     || ((pt + widthOfGrid) > sizeOfGrid) ? -1     : pt + widthOfGrid;
  neighbors[3] = ((pt + widthOfGrid + 1) < 0) || ((pt + widthOfGrid + 1) > sizeOfGrid) ? -1 : pt + widthOfGrid + 1;
  neighbors[4] = ((pt + widthOfGrid - 1) < 0) || ((pt + widthOfGrid - 1) > sizeOfGrid) ? -1 : pt + widthOfGrid - 1;
  neighbors[5] = ((pt - widthOfGrid) < 0)     || ((pt - widthOfGrid) > sizeOfGrid) ? -1     : pt - widthOfGrid;
  neighbors[6] = ((pt - widthOfGrid + 1) < 0) || ((pt - widthOfGrid + 1) > sizeOfGrid) ? -1 : pt - widthOfGrid + 1;
  neighbors[7] = ((pt - widthOfGrid - 1) < 0) || ((pt - widthOfGrid - 1) > sizeOfGrid) ? -1 : pt - widthOfGrid - 1;

  return neighbors;
}

inline bool AStar::collinear(int pt1, int pt2, int pt3, int width)
{
  // Checks if three points lie on the same line.
  int pt1x = pt1 % width;
  int pt1y = (int)floor(pt1 / width);

  int pt2x = pt2 % width;
  int pt2y = (int)floor(pt2 / width);

  int pt3x = pt3 % width;
  int pt3y = (int)floor(pt3 / width);

  // Compare slopes of the points and check if they are equal
  return (pt2y - pt1y) * (pt3x - pt2x) == (pt3y - pt2y) * (pt2x - pt1x);
}

PoseStamped AStar::poseStampedFromIndex(int ind, const nav_msgs::OccupancyGrid &oGrid, std::string & robot_name)
{
  // Helper function to turn a grid index into a posedstamped point.
  double indx = ind % oGrid.info.width;
  double indy = floor(ind / oGrid.info.width);
  PoseStamped ps;

  ps.pose.position.x = (double)((indx - oGrid.info.width / 2) * oGrid.info.resolution);
  ps.pose.position.y = (double)((indy - oGrid.info.height / 2) * oGrid.info.resolution);
  ps.pose.orientation.w = 1.0;

  ps.header = oGrid.header;
  // TODO: Tell albert to properly set the frame id in map generation.
  ps.header.frame_id = robot_name + "_small_chassis";
  return ps;
}


Path AStar::reconstructPath(int current, int last, std::unordered_map<int, int> &reverse_list, const nav_msgs::OccupancyGrid &oGrid, std::string & robot_name)
{
  // This function takes the list of nodes generated by A* and converts it into a list of waypoints.
  // It does this by taking the last point and retracing its steps back to the starting point
  // It also removes any collinear points.

  Path p;
  PoseStamped lastPs = poseStampedFromIndex(last, oGrid, robot_name);
  PoseStamped firstPs = poseStampedFromIndex(current, oGrid, robot_name);
  p.poses.push_back(firstPs);
  int lastPt = current;
  current = reverse_list[current];

  p.header = oGrid.header;
  // TODO: Tell albert to properly set the frame id in map generation.
  p.header.frame_id = robot_name + "_small_chassis";

  // Loop through the list of node associations backwards. If a node is not collinear with the nodes before and after it, add it to the path.
  while (current != -1)
  {
    if (!collinear(lastPt, current, reverse_list[current], oGrid.info.width))
      p.poses.push_back(poseStampedFromIndex(current, oGrid, robot_name));

    lastPt = current;
    current = reverse_list[current];
  }

  p.poses.push_back(lastPs);
  return p;
}

float AStar::distGridToPoint(int index, Point p1, int width, int height)
{
  Point p2;
  p2.x = (index % width) - width / 2;
  p2.y = floor(index / width) - height / 2;

  return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

Path AStar::findPathOccGrid(const nav_msgs::OccupancyGrid &oGrid, Point target, int threshold, std::string & robot_name)
{
  // Convert meters -> grid units
  target.x = target.x/oGrid.info.resolution;
  target.y = std::round(target.y/oGrid.info.resolution); //ROUNDING OPERATION NECESSARY - DO NOT CHANGE

  if (oGrid.data.size() == 0)
  {
    ROS_WARN("[planning | astar | %s]: Occupancy Grid is Empty.", robot_name);
    return Path();
  }
  // A Star Implementation based off https://en.wikipedia.org/wiki/A*_search_algorithm

  // Set up the open and closed sets and heuristic scores
  std::vector<double> gScores(oGrid.data.size(), INFINITY);

  int endIndex = 0;
  int centerIndex = (oGrid.info.height / 2) * oGrid.info.width + oGrid.info.width / 2;
  

  ROS_WARN("centerIndex calculated");


  //this code takes care of the case when the starting pose is in the padding area
  if(oGrid.data[centerIndex] > threshold)
  {
    ROS_WARN("Start pose in the padding...");
   
    PoseStamped firstPoint;
    firstPoint.header.frame_id = robot_name + "_small_chassis";
    firstPoint.pose.position.x = 0.0;
    firstPoint.pose.position.y  = -1.6;

    PoseStamped secondPoint;
    secondPoint.header.frame_id = robot_name + "_small_chassis";
    secondPoint.pose.position.x = 0.0;
    secondPoint.pose.position.y  = 0.0;

    PoseStamped thirdPoint;
    thirdPoint.header.frame_id = robot_name + "_small_chassis";
    thirdPoint.pose.position.x = 0.0;
    thirdPoint.pose.position.y  = 0.0;


    //new path if the robot's index in
    Path path;

    path.header = oGrid.header;
  // TODO: Tell albert to properly set the frame id in map generation.
    path.header.frame_id = robot_name + "_small_chassis";
    path.poses.push_back(firstPoint);
    path.poses.push_back(secondPoint);
    path.poses.push_back(thirdPoint);


    ROS_WARN("Path sent...");
    ROS_WARN("SEND THE GOAL AGAIN...");
    return path;

  }

  auto origin = std::make_pair<double, int>(0, std::move(centerIndex));
  int bestDistance = INFINITY;

  Point robotLocation;
  robotLocation.y = 0;
  robotLocation.x = 0;

  // Check if the target is outside of the current occupancy grid
  // If it is, we need to find the closest point on the edge of the occupancy grid to the target.
  if ((int)target.x > (int)oGrid.info.width / 2 || (int)target.x < (int)-(oGrid.info.width / 2) || (int)target.y > (int)oGrid.info.height / 2 || (int)target.y < (int)-(oGrid.info.height / 2))
  {
    ROS_WARN("[planning | astar | %s]: Finding New index...", robot_name);
    float distFromRobot = INFINITY;
    float minDist = INFINITY;
    float optmlDist = INFINITY;
    std::vector<int> localBestIndexList;
    int bestIndex = centerIndex;
    for (int i = 0; i < oGrid.info.width; ++i)
    {
      // Loop through the bottom edge
      if ((distGridToPoint(i, target, oGrid.info.width, oGrid.info.height) < minDist))
      {
        if(oGrid.data[i] > threshold) continue;
        minDist = distGridToPoint(i, target, oGrid.info.width, oGrid.info.height);
        distFromRobot = distGridToPoint(i, robotLocation, oGrid.info.width, oGrid.info.height);
        optmlDist = minDist + distFromRobot;
        if(optmlDist < bestDistance)
        {
          bestIndex = i;
          bestDistance = optmlDist;
        }
      }
      // Loop through the right edge
      if ((distGridToPoint(oGrid.info.width * i, target, oGrid.info.width, oGrid.info.height) < minDist))
      {
        if(oGrid.data[i * oGrid.info.width] > threshold) continue;
        minDist = distGridToPoint(oGrid.info.width * i, target, oGrid.info.width, oGrid.info.height);
        distFromRobot = distGridToPoint(oGrid.info.width * i, robotLocation, oGrid.info.width, oGrid.info.height);
        optmlDist = minDist + distFromRobot;
        if(optmlDist < bestDistance)
        {
          bestIndex = oGrid.info.width * i;
          bestDistance = optmlDist;
        }
      }
      // Loop through the left edge
      if ((distGridToPoint((oGrid.info.width * (i + 1)) - 1, target, oGrid.info.width, oGrid.info.height) < minDist))
      {
        if(oGrid.data[oGrid.info.width * (i + 1)] > threshold) continue;
        minDist = distGridToPoint((oGrid.info.width * (i + 1)) - 1, target, oGrid.info.width, oGrid.info.height);
        distFromRobot = distGridToPoint((oGrid.info.width * (i + 1)) - 1, robotLocation, oGrid.info.width, oGrid.info.height);
        optmlDist = minDist + distFromRobot;
        if(optmlDist < bestDistance)
        {
          bestIndex = (oGrid.info.width * (i + 1)) - 1;
          bestDistance = optmlDist;
        }
      }
      // Loop through the top
      if ((distGridToPoint(oGrid.data.size() - 1 - i, target, oGrid.info.width, oGrid.info.height) < minDist))
      {
        if(oGrid.data[oGrid.data.size() - 1 - i] > threshold) continue;
        minDist = distGridToPoint(oGrid.data.size() - 1 - i, target, oGrid.info.width, oGrid.info.height);
        distFromRobot = distGridToPoint(oGrid.data.size() - 1 - i, robotLocation, oGrid.info.width, oGrid.info.height);
        optmlDist = minDist + distFromRobot;
        if(optmlDist < bestDistance)
        {
          bestIndex = oGrid.data.size() - 1 - i;
          bestDistance = optmlDist;
        }
      }
    }
    // Set end to the new closest point
    endIndex = bestIndex;
  }
  else
  {
    // If the target point was on the grid, just calculate the index of the point (with the center being 0,0)
    endIndex = (target.y + (oGrid.info.height / 2)) * oGrid.info.width + (target.x + (oGrid.info.width / 2));
    ROS_WARN("[planning | astar | %s]: Calculated Index: %d", robot_name, endIndex);
  }
  // Check if the final destination is occupied.
  if (oGrid.data[endIndex] > threshold)
  {
    ROS_WARN("[planning | astar | %s]: TARGET IN OCCUPIED SPACE, UNREACHABLE", robot_name);
    return Path();
  }

  std::set<std::pair<double, int>> open_set;
  open_set.insert(origin);

  std::unordered_map<int, int> came_from;
  came_from[centerIndex] = -1;

  gScores[centerIndex] = 0;

  // Loop through the open set
  while (!open_set.empty())
  {
    auto iter = open_set.lower_bound(std::make_pair<double, int>(0, 0));
    auto current = *iter;
    open_set.erase(iter);

    // Check if we hit the target
    if (current.second == endIndex)
    {
      return reconstructPath(current.second, centerIndex, came_from, oGrid, robot_name);
    }

    // If the node is occupied, we can't travel through it so skip it
    if (oGrid.data[current.second] >= threshold)
      continue;

    // Search the neighbors, and set the heuristic scores
    for (int neighbor : getNeighborsIndiciesArray(current.second, oGrid.info.width, oGrid.data.size()))
    {
      if (neighbor == -1)
        continue;

      double tentative_gscore = gScores[current.second] + distance(current.second, neighbor, oGrid.info.width);
      if (collinear(neighbor, current.second, came_from[current.second], oGrid.info.width)) tentative_gscore -= .5; // bias towards straight lines init value = 0.95
      if (tentative_gscore < gScores[neighbor])
      {
        gScores[neighbor] = tentative_gscore;
        came_from[neighbor] = current.second;
        open_set.insert(std::make_pair<double, int>((tentative_gscore + distance(neighbor, endIndex, oGrid.info.width)), std::move(neighbor)));
      }
    }
  }

  ROS_WARN("[planning | astar | %s]: Call to navigation failed to find valid path.\n", robot_name);
  return Path();
}