#ifndef NAVIGATION_ALGO_H
#define NAVIGATION_ALGO_H

#include <vector>
#include <math.h>
#include <tf/transform_datatypes.h>
#include <geometry_msgs/Point.h>
#include <geometry_msgs/PointStamped.h>
#include <geometry_msgs/PoseStamped.h>

#include <tf2_ros/transform_listener.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>

#include <utils/common_names.h>

using namespace COMMON_NAMES;

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
  static constexpr float wheel_sep_length_ = 1.076;
  static constexpr float wheel_rad_ = 0.17;

  /**
   * @brief **DEPRICATED** Get the Steering Angles for Making Radial Turn 
   *        Use override with geometry_msgs::Point instead
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
  static std::vector<double> getSteeringAnglesRadialTurn(const float radius);

  /**
   * @brief Get the Steering Angles for Making Radial Turn 
   *        Use override with geometry_msgs::Point instead
   * 
   * @param center_of_rotation    The center of rotation from the center of the robot.                                
   *                              The Center of Rotation follows right hand rule from top
   *                              i.e. Positive X is to the front of the robot 
   *                                   Positive Y is to the left of the robot
   * 
   * @return std::vector<float>   output steering angles
   *                              The vector will be in order:
   *                              Clockwise from top, starting with FRONT_LEFT
   *             
   *                      element 0: Front Left Wheel
   *                      element 1: Front Right Wheel
   *                      element 2: Back Right Wheel
   *                      element 3: Back Left Wheel
   */
  static std::vector<double> getSteeringAnglesRadialTurn(const geometry_msgs::Point center_of_rotation);

  /**
   * @brief Get the Driving Velocities for Making Radial Turn 
   * 
   * @param center_of_rotation    The center of rotation from the center of the robot.                                
   *                              The Center of Rotation follows right hand rule from top
   *                              i.e. Positive X is to the front of the robot 
   *                                   Positive Y is to the left of the robot
   * 
   * @param velocity              Desired Velocity at the center of the robot
   * 
   * @return std::vector<float>   Output steering angles
   *                              The vector will be in order:
   *                              Clockwise from top, starting with FRONT_LEFT
   * 
   *                      element 0: Front Left Wheel
   *                      element 1: Front Right Wheel
   *                      element 2: Back Right Wheel
   *                      element 3: Back Left Wheel
   */
  static std::vector<double> getDrivingVelocitiesRadialTurn(const geometry_msgs::Point center_of_rotation, const float velocity);

  /**
   * @brief **DEPRICATED** Get the Driving Efforts for Making Radial Turn 
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
  static std::vector<double> getDrivingVelocitiesRadialTurn(const float radius, const float effort);

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
   * @brief Converts a desired linear velocity to an angular velocity for the wheels
   * 
   * @param linear_vel Desired linear velocity
   * @return double Angular velocity, in rad/s
   */
  static double linearToAngularVelocity(double linear_vel);

  /**
   * @brief Returns a set of Euler angles
   * 
   * @param pose            Current robot pose
   * @return std::vector<double> 
   */
  static std::vector<double> fromQuatToEuler(const geometry_msgs::PoseStamped& pose);

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
  static double changeInHeading(const geometry_msgs::PoseStamped& current_robot_pose, const geometry_msgs::PoseStamped& current_waypoint, const std::string& robot_name, const tf2_ros::Buffer& tf_buffer);
  
  /**
   * @brief Calculates the distance between two poses in XY.
   * 
   * @param current_robot_pose The current robot pose (map frame)
   * @param target_robot_pose  The next pose (map frame)
   * @return double            Distance formula between the x and y components of each pose.
   */
  static double changeInPosition(const geometry_msgs::PoseStamped& current_robot_pose, const geometry_msgs::PoseStamped& target_robot_pose);

  /**
   * @brief Calculates the yaw of the desired_pose relative to the robot.
   * 
   * @param desired_pose The pose that we want the yaw of.
   * @param robot_name   The name of the robot we want the orientation relative to.
   * @param tf_buffer    The transform buffer, used to transform the desired_pose into the robot's frame
   * @return double      The yaw in radians. A positive rotation is CCW about the z axis.
   */
  static double changeInOrientation(const geometry_msgs::PoseStamped& desired_pose, const std::string& robot_name, const tf2_ros::Buffer& tf_buffer);

  /**
   * @brief Calls tf_buffer.transform, but handles tf2::ExtrapolationException
   * 
   * @param pose The pose to transform. Passed as a reference, and is changed in place.
   * @param frame The frame to transform the pose into.
   * @param tf_buffer A transform buffer that is populated with up-to-date transforms.
   * @param duration The longest time any one iteration can take to try and transform
   * @param tries How many iterations this function can try to transform. Defaults to 1
   * @return true Transform succeeded
   * @return false Transform failed
   */
  static bool transformPose(geometry_msgs::PoseStamped& pose, const std::string& frame, const tf2_ros::Buffer& tf_buffer, float duration, int tries = 1);

  /**
   * @brief Same as transformPose, but for a point. Passes through to transformPose.
   * 
   * @param point The point to transform. Passed as a reference, and is changed in place.
   * @param frame See transformPose
   * @param tf_buffer See transformPose
   * @param duration See transformPose
   * @param tries See transformPose
   * @return true See transformPose
   * @return false See transformPose
   */
  static bool transformPoint(geometry_msgs::PointStamped& point, const std::string& frame, const tf2_ros::Buffer& tf_buffer, float duration, int tries = 1);

};

#endif