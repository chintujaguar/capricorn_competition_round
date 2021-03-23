#ifndef NAVIGATION_ALGO_H
#define NAVIGATION_ALGO_H

#include <vector>
#include <math.h>
#include <tf/transform_datatypes.h>
#include <geometry_msgs/Point.h>
#include <geometry_msgs/PoseStamped.h>

#include <tf2_ros/transform_listener.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>

class NavigationAlgo
{
private:
  static constexpr float arc_spiral_a = 0;   // Inner radius (starting radius of the spiral)
  static constexpr float arc_spiral_b = 2.7; // Incerement per rev
  static constexpr float arc_spiral_multi = 2;
  static constexpr float arc_spiral_incr = 3.9;

public:
  NavigationAlgo(/* args */);
  ~NavigationAlgo();

  static constexpr float wheel_sep_width_ = 1.0;
  static constexpr float wheel_sep_length_ = 1.0;

  /**
   * @brief Get the Steering Angles for Making Radial Turn 
   * 
   * @param radius    Radius follows right hand rule from top
   *                  i.e. radius to the left is +ve
   *                                     right is -ve
   * 
   * @return std::vector<float>    output steering angles
   *                  The vector will be in order:
   *                  Clockwise from top, starting with FRONT_LEFT
   * 
   *          element 0: Front Left Wheel
   *          element 1: Front Right Wheel
   *          element 2: Back Right Wheel
   *          element 3: Back Left Wheel
   */
  static std::vector<float> getSteeringAnglesRadialTurn(const float radius);

  /**
   * @brief Get the Driving Efforts for Making Radial Turn 
   * 
   * @param radius    Radius follows right hand rule from top
   *                  i.e. radius to the left is +ve
   *                                     right is -ve
   * 
   * @param effort    Effort at the center of the robot
   * 
   * @return std::vector<float>    output steering angles
   *                  The vector will be in order:
   *                  Clockwise from top, starting with FRONT_LEFT
   * 
   *          element 0: Front Left Wheel
   *          element 1: Front Right Wheel
   *          element 2: Back Right Wheel
   *          element 3: Back Left Wheel
   */
  static std::vector<float> getDrivingVelocitiessRadialTurn(const float radius, const float effort);

  /**
   * @brief Get the radius of Osculating circle for Archimedean Spiral
   * 
   * @param time      How further are we into the spiral
   * 
   * @return float   outputs the radius at Arcimedean Spiral
   */
  static float getRadiusInArchimedeanSpiral(const float time);

  /**
   * @brief Retruns the N points lying in spiral path
   * 
   * @param init_location   Initial location of the robot in an environment
   * @param N               Number of points desired
   * @param init_theta      Pick up where left off
   * @return std::vector<geometry_msgs::Point> 
   */
  static std::vector<geometry_msgs::Point> getNArchimedeasSpiralPoints(const geometry_msgs::Point &init_location, const int N, int init_theta = 0);

  /**
   * @brief Get the Kinetic Energy object
   * 
   * @param vel           Velociry of the robot
   * @param robot_mass    Mass of the robot
   * @return float 
   */
  inline static float getKineticEnergy(const float vel, const float robot_mass)
  {
    return 0.5 * (robot_mass) * (std::pow(vel, 2));
  }

  /**
   * @brief Returns a set of Euler angles
   * 
   * @param pose            Current robot pose
   * @return std::vector<double> 
   */
  static std::vector<double> fromQuatToEuler(geometry_msgs::PoseStamped* pose);

  /**
   * @brief Calculates the change in heading between two poses. The current pose will be treated as the origin, and the waypoint will be transformed
   *        into the robot's frame. Delta heading is atan2 of the angle from the robot's x axis to the waypoint.
   * @param current_robot_pose The current robot pose
   * @param current_waypoint The waypoint to which the angle will be calculates
   * @param robot_name The name of the robot, used for frame transforms
   * @param tf_buffer The transform buffer, used to do frame transforms. Must be up to date.
   * 
   * @return double The angle between the robot and waypoint.
   */
  static double changeInHeading(geometry_msgs::PoseStamped* current_robot_pose, geometry_msgs::PoseStamped* current_waypoint, std::string robot_name, tf2_ros::Buffer* tf_buffer);
  
  /**
   * @brief Calculates the distance between two poses in XY.
   * 
   * @param current_robot_pose The current robot pose (map frame)
   * @param target_robot_pose The next pose (map frame)
   * @return double Distance formula between the x and y components of each pose.
   */
  static double changeInPosition(geometry_msgs::PoseStamped *current_robot_pose, geometry_msgs::PoseStamped *target_robot_pose);

};

#endif