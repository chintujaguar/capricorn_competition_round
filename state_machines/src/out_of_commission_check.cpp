/*
Author: Chris DeMaio
Email: cjdemaio@wpi.edu
TEAM CAPRICORN
NASA SPACE ROBOTICS CHALLENGE
*/

#include <ros/ros.h>
#include <utils/common_names.h>
#include "std_msgs/String.h"
#include <nav_msgs/Odometry.h>
#include <geometry_msgs/PoseStamped.h>
#include <sensor_msgs/Imu.h>
#include <rtabmap_ros/ResetPose.h>
#include <tf2/LinearMath/Matrix3x3.h>
#include <state_machines/robot_state_status.h>

bool scout_1_odom_msg_received = false;
bool scout_2_odom_msg_received = false;
bool hauler_1_odom_msg_received = false;
bool hauler_2_odom_msg_received = false;
bool excavator_1_odom_msg_received = false;
bool excavator_2_odom_msg_received = false;
bool state_msg_received = false;
bool wait_till_reset = false;

double scout_1_x;
double scout_1_y;
double scout_1_z;
double scout_2_x;
double scout_2_y;
double scout_2_z;
double hauler_1_x;
double hauler_1_y;
double hauler_1_z;
double hauler_2_x;
double hauler_2_y;
double hauler_2_z;
double excavator_1_x;
double excavator_1_y;
double excavator_1_z;
double excavator_2_x;
double excavator_2_y;
double excavator_2_z;

bool scout_1_idle = false;
bool scout_2_idle = false;
bool hauler_1_idle = false;
bool hauler_2_idle = false;
bool excavator_1_idle = false;
bool excavator_2_idle = false;

std::string curr_bot;
int curr_state;
bool curr_state_done;
bool last_state_succeeded;

void scout_1_odom_callback(nav_msgs::Odometry odom_data) 
{   
    scout_1_x = odom_data.pose.pose.position.x;
    scout_1_y = odom_data.pose.pose.position.y;
    scout_1_z = odom_data.pose.pose.position.z;
    scout_1_odom_msg_received = true;
}

void scout_2_odom_callback(nav_msgs::Odometry odom_data) 
{   
    scout_2_x = odom_data.pose.pose.position.x;
    scout_2_y = odom_data.pose.pose.position.y;
    scout_2_z = odom_data.pose.pose.position.z;
    scout_2_odom_msg_received = true;
}

void hauler_1_odom_callback(nav_msgs::Odometry odom_data) 
{   
    hauler_1_x = odom_data.pose.pose.position.x;
    hauler_1_y = odom_data.pose.pose.position.y;
    hauler_1_z = odom_data.pose.pose.position.z;
    hauler_1_odom_msg_received = true;
}

void hauler_2_odom_callback(nav_msgs::Odometry odom_data) 
{   
    hauler_2_x = odom_data.pose.pose.position.x;
    hauler_2_y = odom_data.pose.pose.position.y;
    hauler_2_z = odom_data.pose.pose.position.z;
    hauler_2_odom_msg_received = true;
}

void excavator_1_odom_callback(nav_msgs::Odometry odom_data) 
{   
    excavator_1_x = odom_data.pose.pose.position.x;
    excavator_1_y = odom_data.pose.pose.position.y;
    excavator_1_z = odom_data.pose.pose.position.z;
    excavator_1_odom_msg_received = true;
}

void excavator_2_odom_callback(nav_msgs::Odometry odom_data) 
{   
    excavator_2_x = odom_data.pose.pose.position.x;
    excavator_2_y = odom_data.pose.pose.position.y;
    excavator_2_z = odom_data.pose.pose.position.z;
    excavator_2_odom_msg_received = true;
}

void robot_state_callback(state_machines::robot_state_status robot_state_info) 
{   
    curr_bot = robot_state_info.robot_name;
    curr_state = robot_state_info.robot_current_state;
    
    scout_1_idle = ((curr_bot == "small_scout_1") && (curr_state == COMMON_NAMES::ROBOT_IDLE_STATE));
    scout_2_idle = ((curr_bot == "small_scout_2") && (curr_state == COMMON_NAMES::ROBOT_IDLE_STATE));
    hauler_1_idle = ((curr_bot == "small_hauler_1") && (curr_state == COMMON_NAMES::ROBOT_IDLE_STATE));
    hauler_2_idle = ((curr_bot == "small_hauler_2") && (curr_state == COMMON_NAMES::ROBOT_IDLE_STATE));
    excavator_1_idle = ((curr_bot == "small_excavator_1") && (curr_state == COMMON_NAMES::ROBOT_IDLE_STATE));
    excavator_2_idle = ((curr_bot == "small_excavator_2") && (curr_state == COMMON_NAMES::ROBOT_IDLE_STATE));

    state_msg_received = true;
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "out_of_commission_checker");
    ros::NodeHandle nh;
    
    ros::Subscriber camera_odom_sub_scout_1 = nh.subscribe("/small_scout_1" + COMMON_NAMES::RTAB_ODOM_TOPIC, 223, scout_1_odom_callback);
    ros::Subscriber camera_odom_sub_scout_2 = nh.subscribe("/small_scout_2" + COMMON_NAMES::RTAB_ODOM_TOPIC, 223, scout_2_odom_callback);
    ros::Subscriber camera_odom_sub_hauler_1 = nh.subscribe("/small_hauler_1" + COMMON_NAMES::RTAB_ODOM_TOPIC, 223, hauler_1_odom_callback);
    ros::Subscriber camera_odom_sub_hauler_2 = nh.subscribe("/small_hauler_2" + COMMON_NAMES::RTAB_ODOM_TOPIC, 223, hauler_2_odom_callback);
    ros::Subscriber camera_odom_sub_excavator_1 = nh.subscribe("/small_excavator_1" + COMMON_NAMES::RTAB_ODOM_TOPIC, 223, excavator_1_odom_callback);
    ros::Subscriber camera_odom_sub_excavator_2 = nh.subscribe("/small_excavator_2"+ COMMON_NAMES::RTAB_ODOM_TOPIC, 223, excavator_2_odom_callback);

    ros::Subscriber robot_state_sub = nh.subscribe(COMMON_NAMES::CAPRICORN_TOPIC + COMMON_NAMES::ROBOTS_CURRENT_STATE_TOPIC, 223, robot_state_callback);

    ros::Publisher out_of_commission_pub = nh.advertise<std_msgs::String>("/capricorn/" + COMMON_NAMES::ROBOTS_OUT_OF_COMMISSION_TOPIC, 223);

    // Wait until we have received a message from all odoms
    bool all_odoms_received = scout_1_odom_msg_received/* && scout_2_odom_msg_received && hauler_1_odom_msg_received && hauler_2_odom_msg_received && excavator_1_odom_msg_received && excavator_2_odom_msg_received*/;
    
    while(ros::ok() && !(state_msg_received && all_odoms_received))
    {
        all_odoms_received = scout_1_odom_msg_received/* && scout_2_odom_msg_received && hauler_1_odom_msg_received && hauler_2_odom_msg_received && excavator_1_odom_msg_received && excavator_2_odom_msg_received*/;
        ros::Duration(0.1).sleep();
        ros::spinOnce();
    }

    ROS_WARN_STREAM("MADE IT HERE");

    ros::Time curr_time = ros::Time::now();

    double scout_1_start_x = scout_1_x;
    double scout_1_start_y = scout_1_y;
    double scout_1_start_z = scout_1_z;
    ros::Time scout_1_start_time = curr_time;

    double scout_2_start_x = scout_2_x;
    double scout_2_start_y = scout_2_y;
    double scout_2_start_z = scout_2_z;
    ros::Time scout_2_start_time = curr_time;

    double hauler_1_start_x = hauler_1_x;
    double hauler_1_start_y = hauler_1_y;
    double hauler_1_start_z = hauler_1_z;
    ros::Time hauler_1_start_time = curr_time;

    double hauler_2_start_x = hauler_2_x;
    double hauler_2_start_y = hauler_2_y;
    double hauler_2_start_z = hauler_2_z;
    ros::Time hauler_2_start_time = curr_time;

    double excavator_1_start_x = excavator_1_x;
    double excavator_1_start_y = excavator_1_y;
    double excavator_1_start_z = excavator_1_z;
    ros::Time excavator_1_start_time = curr_time;

    double excavator_2_start_x = excavator_2_x;
    double excavator_2_start_y = excavator_2_y;
    double excavator_2_start_z = excavator_2_z;
    ros::Time excavator_2_start_time = curr_time;

    ROS_WARN_STREAM("[STATE_MACHINES | out_of_commission_check.cpp | Checking if Robot is Out of Commission");
    
    ros::Duration(0.5).sleep();
    ros::spinOnce();
    
    // Inf loop
    while(ros::ok())
    {

        ros::Time curr_time = ros::Time::now();

        if (sqrt(pow(scout_1_x - scout_1_start_x, 2) + pow(scout_1_y - scout_1_start_y, 2) + pow(scout_1_z - scout_1_start_z, 2)) > 1)
        {
            
            ROS_WARN_STREAM("SCOUT 1 DISTANCE OK");
            
            scout_1_start_x = scout_1_x;
            scout_1_start_y = scout_1_y;
            scout_1_start_z = scout_1_z;

            scout_1_start_time = curr_time;
        }
        else if (curr_time.sec - scout_1_start_time.sec > 30 && !scout_1_idle)
        {
            std_msgs::String pub_data;
            pub_data.data = "small_scout_1";
            out_of_commission_pub.publish(pub_data);
            ROS_WARN_STREAM("small scout 1 bad");
        }

        if (sqrt(pow(scout_2_x - scout_2_start_x, 2) + pow(scout_2_y - scout_2_start_y, 2) + pow(scout_2_z - scout_2_start_z, 2)) > 1)
        {
            
            ROS_WARN_STREAM("SCOUT 2 DISTANCE OK");
            
            scout_2_start_x = scout_2_x;
            scout_2_start_y = scout_2_y;
            scout_2_start_z = scout_2_z;

            scout_2_start_time = curr_time;
        }
        else if (curr_time.sec - scout_2_start_time.sec > 30 && !scout_2_idle)
        {
            std_msgs::String pub_data;
            pub_data.data = "small_scout_2";
            out_of_commission_pub.publish(pub_data);
            ROS_WARN_STREAM("small scout 2 bad");
        }

        if (sqrt(pow(hauler_1_x - hauler_1_start_x, 2) + pow(hauler_1_y - hauler_1_start_y, 2) + pow(hauler_1_z - hauler_1_start_z, 2)) > 1)
        {
            
            ROS_WARN_STREAM("HAULER 1 DISTANCE OK");
            
            hauler_1_start_x = hauler_1_x;
            hauler_1_start_y = hauler_1_y;
            hauler_1_start_z = hauler_1_z;

            hauler_1_start_time = curr_time;
        }
        else if (curr_time.sec - hauler_1_start_time.sec > 30 && !hauler_1_idle)
        {
            std_msgs::String pub_data;
            pub_data.data = "small_hauler_1";
            out_of_commission_pub.publish(pub_data);
            ROS_WARN_STREAM("small hauler 1 bad");
        }

        if (sqrt(pow(hauler_2_x - hauler_2_start_x, 2) + pow(hauler_2_y - hauler_2_start_y, 2) + pow(hauler_2_z - hauler_2_start_z, 2)) > 1)
        {
            
            ROS_WARN_STREAM("HAULER 2 DISTANCE OK");
            
            hauler_2_start_x = hauler_2_x;
            hauler_2_start_y = hauler_2_y;
            hauler_2_start_z = hauler_2_z;

            hauler_2_start_time = curr_time;
        }
        else if (curr_time.sec - hauler_2_start_time.sec > 30 && !hauler_2_idle)
        {
            std_msgs::String pub_data;
            pub_data.data = "small_hauler_2";
            out_of_commission_pub.publish(pub_data);
            ROS_WARN_STREAM("small hauler 2 bad");
        }

        if (sqrt(pow(excavator_1_x - excavator_1_start_x, 2) + pow(excavator_1_y - excavator_1_start_y, 2) + pow(excavator_1_z - excavator_1_start_z, 2)) > 1)
        {
            
            ROS_WARN_STREAM("EXCAVATOR 1 DISTANCE OK");
            
            excavator_1_start_x = excavator_1_x;
            excavator_1_start_y = excavator_1_y;
            excavator_1_start_z = excavator_1_z;

            excavator_1_start_time = curr_time;
        }
        else if (curr_time.sec - excavator_1_start_time.sec > 30 && !excavator_1_idle)
        {
            std_msgs::String pub_data;
            pub_data.data = "small_excavator_1";
            out_of_commission_pub.publish(pub_data);
            ROS_WARN_STREAM("small excavator 1 bad");
        }

        if (sqrt(pow(excavator_2_x - excavator_2_start_x, 2) + pow(excavator_2_y - excavator_2_start_y, 2) + pow(excavator_2_z - excavator_2_start_z, 2)) > 1)
        {
            
            ROS_WARN_STREAM("EXCAVATOR 2 DISTANCE OK");
            
            excavator_2_start_x = excavator_2_x;
            excavator_2_start_y = excavator_2_y;
            excavator_2_start_z = excavator_2_z;

            excavator_2_start_time = curr_time;
        }
        else if (curr_time.sec - excavator_2_start_time.sec > 30 && !excavator_2_idle)
        {
            std_msgs::String pub_data;
            pub_data.data = "small_excavator_2";
            out_of_commission_pub.publish(pub_data);
            ROS_WARN_STREAM("small excavator 2 bad");
        }
        
        
        
        // if (curr_bot == "small_scout_1")
        // {
        //     if (curr_state != COMMON_NAMES::ROBOT_IDLE_STATE)
        //     {
        //         double curr_x = scout_1_odometry.pose.pose.position.x;
        //         double curr_y = scout_1_odometry.pose.pose.position.y;
        //         double curr_z = scout_1_odometry.pose.pose.position.z;

        //         ROS_WARN_STREAM("Curr bot: " << curr_bot << " x " << curr_x << " y " << curr_y << " z " << curr_z);

        //         if (sqrt(pow(curr_x - scout_1_start_x, 2) + pow(curr_y - scout_1_start_y, 2) + pow(curr_z - scout_1_start_z, 2)) > 1)
        //         {
                    
        //             ROS_WARN_STREAM("DISTANCE OK");
                    
        //             scout_1_start_x = curr_x;
        //             scout_1_start_y = curr_y;
        //             scout_1_start_z = curr_z;

        //             scout_1_start_time = curr_time;
        //         }
        //         else if (curr_time.sec - scout_1_start_time.sec > 30)
        //         {
        //             std_msgs::String pub_data;
        //             pub_data.data = "small_scout_1";
        //             out_of_commission_pub.publish(pub_data);
        //             ROS_WARN_STREAM("Chris is a genius " << curr_bot);
        //         }
        //     }
        // }        
        // else if (curr_bot == "small_scout_2")
        // {
        //     if (curr_state != COMMON_NAMES::ROBOT_IDLE_STATE)
        //     {
        //         double curr_x = scout_2_odometry.pose.pose.position.x;
        //         double curr_y = scout_2_odometry.pose.pose.position.y;
        //         double curr_z = scout_2_odometry.pose.pose.position.z;

        //         if (sqrt(pow(curr_x - scout_2_start_x, 2) + pow(curr_y - scout_2_start_y, 2) + pow(curr_z - scout_2_start_z, 2)) > 1)
        //         {
        //             scout_2_start_x = curr_x;
        //             scout_2_start_y = curr_y;
        //             scout_2_start_z = curr_z;

        //             scout_2_start_time = curr_time;
        //         }
        //         else if (curr_time.sec - scout_2_start_time.sec > 30)
        //         {
        //             std_msgs::String pub_data;
        //             pub_data.data = "small_scout_2";
        //             out_of_commission_pub.publish(pub_data);
        //             ROS_WARN_STREAM("Chris is a genius " << curr_bot);
        //         }
        //     }
        // }
        // else if (curr_bot == "small_hauler_1")
        // {
        //     if (curr_state != COMMON_NAMES::ROBOT_IDLE_STATE)
        //     {
        //         double curr_x = hauler_1_odometry.pose.pose.position.x;
        //         double curr_y = hauler_1_odometry.pose.pose.position.y;
        //         double curr_z = hauler_1_odometry.pose.pose.position.z;

        //         if (sqrt(pow(curr_x - hauler_1_start_x, 2) + pow(curr_y - hauler_1_start_y, 2) + pow(curr_z - hauler_1_start_z, 2)) > 1)
        //         {
        //             hauler_1_start_x = curr_x;
        //             hauler_1_start_y = curr_y;
        //             hauler_1_start_z = curr_z;

        //             hauler_1_start_time = curr_time;
        //         }
        //         else if (curr_time.sec - hauler_1_start_time.sec > 30)
        //         {
        //             std_msgs::String pub_data;
        //             pub_data.data = "small_hauler_1";
        //             out_of_commission_pub.publish(pub_data);
        //             ROS_WARN_STREAM("Chris is a genius " << curr_bot);
        //         }
        //     }
        // }
        // else if (curr_bot == "small_hauler_2")
        // {
        //     if (curr_state != COMMON_NAMES::ROBOT_IDLE_STATE)
        //     {
        //         double curr_x = hauler_2_odometry.pose.pose.position.x;
        //         double curr_y = hauler_2_odometry.pose.pose.position.y;
        //         double curr_z = hauler_2_odometry.pose.pose.position.z;

        //         if (sqrt(pow(curr_x - hauler_2_start_x, 2) + pow(curr_y - hauler_2_start_y, 2) + pow(curr_z - hauler_2_start_z, 2)) > 1)
        //         {
        //             hauler_2_start_x = curr_x;
        //             hauler_2_start_y = curr_y;
        //             hauler_2_start_z = curr_z;

        //             hauler_2_start_time = curr_time;
        //         }
        //         else if (curr_time.sec - hauler_2_start_time.sec > 30)
        //         {
        //             std_msgs::String pub_data;
        //             pub_data.data = "small_hauler_2";
        //             out_of_commission_pub.publish(pub_data);
        //             ROS_WARN_STREAM("Chris is a genius " << curr_bot);
        //         }
        //     }
        // }
        // else if (curr_bot == "small_excavator_1")
        // {
        //     if (curr_state != COMMON_NAMES::ROBOT_IDLE_STATE)
        //     {
        //         double curr_x = excavator_1_odometry.pose.pose.position.x;
        //         double curr_y = excavator_1_odometry.pose.pose.position.y;
        //         double curr_z = excavator_1_odometry.pose.pose.position.z;

        //         if (sqrt(pow(curr_x - excavator_1_start_x, 2) + pow(curr_y - excavator_1_start_y, 2) + pow(curr_z - excavator_1_start_z, 2)) > 1)
        //         {
        //             excavator_1_start_x = curr_x;
        //             excavator_1_start_y = curr_y;
        //             excavator_1_start_z = curr_z;

        //             excavator_1_start_time = curr_time;
        //         }
        //         else if (curr_time.sec - excavator_1_start_time.sec > 30)
        //         {
        //             std_msgs::String pub_data;
        //             pub_data.data = "small_excavator_1";
        //             out_of_commission_pub.publish(pub_data);
        //             ROS_WARN_STREAM("Chris is a genius " << curr_bot);
        //         }
        //     }
        // }
        // else if (curr_bot == "small_excavator_2")
        // {
        //     if (curr_state != COMMON_NAMES::ROBOT_IDLE_STATE)
        //     {
        //         double curr_x = excavator_2_odometry.pose.pose.position.x;
        //         double curr_y = excavator_2_odometry.pose.pose.position.y;
        //         double curr_z = excavator_2_odometry.pose.pose.position.z;

        //         if (sqrt(pow(curr_x - excavator_2_start_x, 2) + pow(curr_y - excavator_2_start_y, 2) + pow(curr_z - excavator_2_start_z, 2)) > 1)
        //         {
        //             excavator_2_start_x = curr_x;
        //             excavator_2_start_y = curr_y;
        //             excavator_2_start_z = curr_z;

        //             excavator_2_start_time = curr_time;
        //         }
        //         else if (curr_time.sec - excavator_2_start_time.sec > 30)
        //         {
        //             std_msgs::String pub_data;
        //             pub_data.data = "small_excavator_2";
        //             out_of_commission_pub.publish(pub_data);
        //             ROS_WARN_STREAM("Chris is a genius " << curr_bot);
        //         }
        //     }
        // }
        // else
        // {
        //     ROS_ERROR_STREAM("CHRIS IS AN IDIOT");
        // }
        
        // Wait 10 seconds and then do it all again
        ros::Duration(0.05).sleep();
        ros::spinOnce();
    }

    ros::spin();
    return 0;
}