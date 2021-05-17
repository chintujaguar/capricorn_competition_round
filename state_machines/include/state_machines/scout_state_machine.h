#pragma once

#include <iostream>
#include <actionlib/client/simple_action_client.h>
#include <actionlib/server/simple_action_server.h>
#include <utils/common_names.h>
#include <state_machines/ScoutStateMachineTaskAction.h>
#include <operations/Spiral.h>
#include <operations/ResourceLocaliserAction.h>

using namespace COMMON_NAMES;

typedef actionlib::SimpleActionServer<state_machines::ScoutStateMachineTaskAction> SM_SERVER;

const std::set<STATE_MACHINE_TASK> SCOUT_TASKS = {
    STATE_MACHINE_TASK::SCOUT_SEARCH_VOLATILE,
    STATE_MACHINE_TASK::SCOUT_STOP_SEARCH,
    STATE_MACHINE_TASK::SCOUT_LOCATE_VOLATILE,
    STATE_MACHINE_TASK::SCOUT_UNDOCK,
};

class ScoutStateMachine
{
private:
  ros::NodeHandle nh_;

  std::string robot_name_;

  ros::ServiceClient spiralClient_;
  
  typedef actionlib::SimpleActionClient<operations::ResourceLocaliserAction> ResourceLocaliserClient_;
  ResourceLocaliserClient_* resource_localiser_client_;

  /**
   * @brief Start the searching algorithm
   * 
   */
  bool startSearchingVolatile();

  /**
   * @brief Stop the searching algorithm
   * 
   */
  bool stopSearchingVolatile();

  /**
   * @brief Sets the status of searching algorithm according to argument resume
   * 
   */
  bool resumeSearchingVolatile(const bool resume);

  /**
   * @brief Pinpoint the location of the volatile, and stand on it
   * 
   */
  bool locateVolatile();

  /**
   * @brief Move away from the excavator to continue on the trajectory
   *        CURRENTLY NOT IMPLEMENTED
   * 
   */
  bool undockRobot();

public:
  ScoutStateMachine(ros::NodeHandle nh, const std::string &robot_name);

  ~ScoutStateMachine();

  friend void cancelGoal(ScoutStateMachine *sm);
  friend void execute(const state_machines::ScoutStateMachineTaskGoalConstPtr &goal, SM_SERVER *as, ScoutStateMachine *sm);
};

// #endif // SCOUT_STATE_MACHINE_H