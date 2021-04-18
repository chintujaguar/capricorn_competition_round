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
  auto pt1 = std::make_pair<int, int>(ind1 % width, (int) floor(ind1 / width));
  auto pt2 = std::make_pair<int, int>(ind2 % width, (int) floor(ind2 / width));
  return sqrt((pt1.first - pt2.first) * (pt1.first - pt2.first) + (pt1.second - pt2.second) * (pt1.second - pt2.second));
}

std::array<int, 8> AStar::getNeighborsIndiciesArray(int pt, int widthOfGrid, int sizeOfGrid)
{
  // Get the neighbors around any given index (ternary operators to ensure the points remain within bounds)
  std::array<int, 8> neighbors;

  neighbors[0] = ((pt + 1) < 0)               || ((pt + 1) > sizeOfGrid)                ? -1 : pt + 1;
  neighbors[1] = ((pt - 1) < 0)               || ((pt - 1) > sizeOfGrid)                ? -1 : pt - 1;
  neighbors[2] = ((pt + widthOfGrid) < 0)     || ((pt + widthOfGrid) > sizeOfGrid)      ? -1 : pt + widthOfGrid;
  neighbors[3] = ((pt + widthOfGrid + 1) < 0) || !((pt + widthOfGrid + 1) > sizeOfGrid) ? -1 : pt + widthOfGrid + 1;
  neighbors[4] = ((pt + widthOfGrid - 1) < 0) || !((pt + widthOfGrid - 1) > sizeOfGrid) ? -1 : pt + widthOfGrid - 1;
  neighbors[5] = ((pt - widthOfGrid) < 0)     || !((pt - widthOfGrid) > sizeOfGrid)     ? -1 : pt - widthOfGrid;
  neighbors[6] = ((pt - widthOfGrid + 1) < 0) || !((pt - widthOfGrid + 1) > sizeOfGrid) ? -1 : pt - widthOfGrid + 1;
  neighbors[7] = ((pt - widthOfGrid - 1) < 0) || !((pt - widthOfGrid - 1) > sizeOfGrid) ? -1 : pt - widthOfGrid - 1;

  return neighbors;
}

inline bool AStar::collinear(int pt1, int pt2, int pt3, int width)
{
  // Checks if three points lie on the same line.
  int pt1x = pt1 % width;
  int pt1y = (int) floor(pt1 / width);

  int pt2x = pt2 % width;
  int pt2y = (int) floor(pt2 / width);

  int pt3x = pt3 % width;
  int pt3y = (int) floor(pt3 / width);

  // Compare slopes of the points and check if they are equal
  return (pt2y - pt1y) * (pt3x - pt2x) == (pt3y - pt2y) * (pt2x - pt1x);
}

PoseStamped AStar::poseStampedFromIndex(int ind, const nav_msgs::OccupancyGrid &oGrid)
{
  // Helper function to turn a grid index into a posedstamped point.
  double indx = ind % oGrid.info.width;
  double indy = floor(ind / oGrid.info.width);

  PoseStamped ps;

  ps.pose.position.x = indx * oGrid.info.resolution;
  ps.pose.position.y = indy * oGrid.info.resolution;

  ps.header = oGrid.header;
  return ps;
}

Path AStar::reconstructPath(int current, int last, std::unordered_map<int, int> &reverse_list, const nav_msgs::OccupancyGrid &oGrid)
{
  // This function takes the list of nodes generated by A* and converts it into a list of waypoints.
  // It does this by taking the last point and retracing its steps back to the starting point
  // It also removes any collinear points.

  Path p;
  PoseStamped lastPs = poseStampedFromIndex(last, oGrid);
  PoseStamped firstPs = poseStampedFromIndex(current, oGrid);
  p.poses.push_back(firstPs);
  int lastPt = current;
  current = reverse_list[current];

  p.header = oGrid.header;

  // Loop through the list of node associations backwards. If a node is not collinear with the nodes before and after it, add it to the path.
  while (current != -1)
  {
    if (!collinear(lastPt, current, reverse_list[current], oGrid.info.width))
      p.poses.push_back(poseStampedFromIndex(current, oGrid));

    lastPt = current;
    current = reverse_list[current];
  }

  p.poses.push_back(lastPs);
  return p;
}

Path AStar::findPathOccGrid(const nav_msgs::OccupancyGrid &oGrid, const Point target, const Point start, int threshold)
{
  if(oGrid.data.size() == 0) {
    ROS_WARN("Occupancy Grid is Empty.");
    return Path();
  }
  // A Star Implementation based off https://en.wikipedia.org/wiki/A*_search_algorithm

  // Set up the open and closed sets and heuristic scores
  std::vector<double> gScores(oGrid.data.size(), INFINITY);

  int endIndex = target.y * oGrid.info.width + target.x;
  int startIndex = start.y * oGrid.info.width + start.x;

  auto origin = std::make_pair<double, int>(0, std::move(startIndex));

  std::set<std::pair<double, int>> open_set;
  open_set.insert(origin);

  std::unordered_map<int, int> came_from;
  came_from[startIndex] = -1;

  gScores[start.y * oGrid.info.width + start.x] = 0;

  // Loop through the open set
  while (!open_set.empty())
  {
    auto iter = open_set.lower_bound(std::make_pair<double, int>(0, 0));
    auto current = *iter;
    open_set.erase(iter);

    // Check if we hit the target
    if (current.second == endIndex)
    {
      return reconstructPath(current.second, startIndex, came_from, oGrid);
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
      if (tentative_gscore < gScores[neighbor])
      {
        gScores[neighbor] = tentative_gscore;
        came_from[neighbor] = current.second;
        open_set.insert(std::make_pair<double, int>((tentative_gscore + distance(neighbor, endIndex, oGrid.info.width)), std::move(neighbor)));
      }
    }
  }

  ROS_WARN("[WARNING] Call to navigation failed to find valid path.\n");
  return Path();
}