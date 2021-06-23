#include <team_level/team.h>

Team::Team(ros::NodeHandle &nh, TEAM_MACRO_STATE team_state, ROBOTS_ENUM scout, 
         ROBOTS_ENUM excavator, ROBOTS_ENUM hauler)
{
    robot_state = new RobotStatus(nh);

    if(scout != NONE)
        setScout(scout);
    if(excavator != NONE)
        setScout(excavator);
    if(hauler != NONE)
        setScout(hauler);

    hired_scout = scout;
    hired_excavator = excavator;
    hired_hauler = hauler;

    macro_state = team_state;

    addStates(nh);
    setInitialState(macro_state);
}

Team::~Team()
{
    delete robot_state;
}

void Team::addStates(ros::NodeHandle &nh)
{
    addState(new Search(nh));
    addState(new Standby(nh));
    addState(new Idle(nh));
}

void Team::addState(TeamState* pc_state) {
   if(MACRO_STATE_PTR_MAP.find(pc_state->getId()) == MACRO_STATE_PTR_MAP.end()) {
      MACRO_STATE_PTR_MAP[pc_state->getId()] = pc_state;
    //   pc_state->setRobotScheduler(*this);
   }
   else {
      ROS_ERROR_STREAM("Duplicated state id " << pc_state->getId());
   }
}

void Team::setInitialState(uint32_t un_state) {
   auto pcState = MACRO_STATE_PTR_MAP.find(un_state);
   // if state exists in map, then set it to the initial state of the scheduler
   if(pcState != MACRO_STATE_PTR_MAP.end()) {
      // acquire value of the state (every map has a key(first) and a value(second))
      current_state_ptr = pcState->second;
      // completes entry point of the initial state
      current_state_ptr->entryPoint();
   }
   else {
      ROS_ERROR_STREAM("Can't set initial state to " << un_state);
   }
}


TeamState& Team::getState(uint32_t un_id) {
   auto pcState = MACRO_STATE_PTR_MAP.find(un_id);
   if(pcState != MACRO_STATE_PTR_MAP.end()) {
      return *(pcState->second);
   }
   else {
      ROS_ERROR_STREAM("Can't get state id " << un_id);
   }
}


void Team::step() {
   /* Only execute if 'current' was initialized */
   if(current_state_ptr) {
       ROS_INFO_STREAM("Robot enum:" << SCOUT_1);
      /* Attempt a transition, every state of every rover has its own transition() */
      TeamState* cNewState = current_state_ptr;
      if (new_state_request)
      {
         cNewState = &getState(macro_state);
         new_state_request = false;
      }

      if(cNewState != current_state_ptr) {
         /* Perform transition */
         current_state_ptr->exitPoint();
         cNewState->entryPoint(SCOUT_1);
         current_state_ptr = cNewState;
      }
      /* Execute current state */
      current_state_ptr->step();
   }
   else {
      ROS_ERROR_STREAM("The Robotscheduler has not been initialized, you must call SetInitialState()");
   }
}

/****************************************/
/****************************************/

//UNDERSTANDING: Each robot has its own done() and this is what is checked to perform step()
void Team::exec() {
      step();
      ros::Duration(.50).sleep();
      setTeamMacroState(IDLE);
      step();
      ros::Duration(.50).sleep();
      setTeamMacroState(SEARCH);
      step();
      ros::Duration(.50).sleep();
      setTeamMacroState(STANDBY);
      step();
    //   ros::spinOnce();
}


int main(int argc, char** argv)
{
   ros::init(argc, argv, "scout_state_machine");
   ros::NodeHandle nh;

   Team team(nh, TEAM_MACRO_STATE::STANDBY, ROBOTS_ENUM::SCOUT_1, ROBOTS_ENUM::NONE, ROBOTS_ENUM::NONE);
    team.exec();

    // ros::Duration(1).sleep();
//     team.setTeamMacroState(SEARCH);
//     ros::Duration(1).sleep();
//     team.setTeamMacroState(SEARCH);
//     ros::Duration(1).sleep();
//     team.setTeamMacroState(SEARCH);
//     ros::spinOnce();
    return 0;
}
