#pragma once
#include <utils/common_names.h>
#include "ros/ros.h"
#include <state_machines/set_robot_state.h>
#include <team_level/robot_status.h>

// #include <team_level/team_scheduler.h>

// /**
//  * The macrostate is a state machine for team-level coordination.
//  *
//  * A state in the macrostate is a collection of individual robot states.
//  */
// class MacroState {
//   public:
// };

enum TEAM_MACRO_STATE{
   STANDBY, // If No robot in the team;
   IDLE,    // If Robots in the team are idle
   SEARCH,  // Team has a busy scout
   SCOUT_WAITING, // Team has a scout waiting for Excavator to take over
   EXCAVATING,    // No or idle scout in the team
                  // Excavator and Hauler busy digging and collecting
   DUMPING, // Hauler going to dump the volatile
            // No or idle Excavator
};

enum TEAM_MICRO_STATE{
   // SEARCH
   SEARCH_FOR_VOLATILE,

   // SCOUT_WAITING
   ROBOTS_TO_GOAL,
   UNDOCK_SCOUT,
   PARK_EXCAVATOR_AT_SCOUT,

   // EXCAVATING
   WAIT_FOR_HAULER,
   PRE_PARK_MANEUVER_EXCAVATOR,
   PARK_HAULER,
   DIG_AND_DUMP,

   // DUMPING
   DUMP_COLLECTION,

   // IDLE
   IDLE_MICRO_STATE
};

using namespace COMMON_NAMES;

class Team;

class TeamState {
   
public:

   TeamState(uint32_t un_id, const std::string& str_name, ros::NodeHandle &nh);

   virtual ~TeamState() {}

   uint32_t getId() const { return m_unId; }

   const std::string& getName() const { return m_strName; }
   
   virtual bool entryPoint(ROBOTS_ENUM scout = NONE, ROBOTS_ENUM excavator = NONE, ROBOTS_ENUM hauler = NONE) = 0;
   
   virtual void exitPoint() = 0;
   
   virtual void step() = 0;
   
   virtual bool isDone() = 0;

   virtual TeamState& transition() = 0;

   TeamState& getState(uint32_t un_state);

   void setTeam(Team& c_robot_scheduler);
   
   virtual TEAM_MICRO_STATE getMicroState() = 0;

protected:

   Team* m_pcTeam;
   uint32_t m_unId;
   std::string m_strName;
   ROBOTS_ENUM scout_in_team, excavator_in_team, hauler_in_team;
   RobotStatus *robot_status;
};

// // All the states as per the diagram
class Standby: public TeamState{
public:
   Standby(ros::NodeHandle &nh):TeamState(STANDBY, "Standby", nh){}
   bool isDone() override ;

   TeamState& transition() override {return *this;}
   TEAM_MICRO_STATE getMicroState(){return IDLE_MICRO_STATE;}
   
   bool entryPoint(ROBOTS_ENUM scout = NONE, ROBOTS_ENUM excavator = NONE, ROBOTS_ENUM hauler = NONE) override;
   void step() override;
   void exitPoint() override;
};

class Idle: public TeamState{
public:
   Idle(ros::NodeHandle &nh):TeamState(IDLE, "Idle", nh){}
   bool isDone() override ;
   
   TeamState& transition() override {return *this;}
   TEAM_MICRO_STATE getMicroState(){return IDLE_MICRO_STATE;}
   
   bool entryPoint(ROBOTS_ENUM scout = NONE, ROBOTS_ENUM excavator = NONE, ROBOTS_ENUM hauler = NONE) override;
   void step() override;
   void exitPoint() override;
};

class Search: public TeamState{
public:
   Search(ros::NodeHandle &nh):TeamState(SEARCH, "Search", nh){}
   bool isDone() override ;

   TeamState& transition() override;
   TEAM_MICRO_STATE getMicroState(){return IDLE_MICRO_STATE;}
   
   bool entryPoint(ROBOTS_ENUM scout = NONE, ROBOTS_ENUM excavator = NONE, ROBOTS_ENUM hauler = NONE) override;
   void step() override;
   void exitPoint() override;
};

class ScoutWaiting: public TeamState{
public:
   ScoutWaiting(ros::NodeHandle &nh):TeamState(SCOUT_WAITING, "ScoutWaiting", nh){}
   bool isDone() override ;
   
   TeamState& transition() override;
   TEAM_MICRO_STATE getMicroState();
   
   bool entryPoint(ROBOTS_ENUM scout = NONE, ROBOTS_ENUM excavator = NONE, ROBOTS_ENUM hauler = NONE) override;
   void step() override;
   void exitPoint() override;

private:
   TEAM_MICRO_STATE micro_state;
   void stepRobotsToGoal();
   void stepUndockScout();
   void stepParkExcavatorAtScout();
};

class Excavating: public TeamState{
public:
   Excavating(ros::NodeHandle &nh):TeamState(EXCAVATING, "Excavating", nh){}
   bool isDone() override ;
   
   TeamState& transition() override;
   TEAM_MICRO_STATE getMicroState(){return IDLE_MICRO_STATE;}
   
   bool entryPoint(ROBOTS_ENUM scout = NONE, ROBOTS_ENUM excavator = NONE, ROBOTS_ENUM hauler = NONE) override;
   void step() override;
   void exitPoint() override;
};

class Dumping: public TeamState{
public:
   Dumping(ros::NodeHandle &nh):TeamState(DUMPING, "Dumping", nh){}
   bool isDone() override ;
   
   TeamState& transition() override;
   TEAM_MICRO_STATE getMicroState(){return IDLE_MICRO_STATE;}
   
   bool entryPoint(ROBOTS_ENUM scout = NONE, ROBOTS_ENUM excavator = NONE, ROBOTS_ENUM hauler = NONE) override;
   void step() override;
   void exitPoint() override;
};