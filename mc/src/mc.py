#!/usr/bin/env python

###############################################################################
# Turtlebot Mission Control (mc) Node                                         #
#                                                                             #
# Simple three-step hybrid archtecture, using 'belief, desire,                #
# intention' (BDI) deliberation.                                              #
#                                                                             #
# 0. Create a set of beliefs e.g. 'has the turtlebot crashed?' (y/n).         #
# 1. Continuously listen for changes is the beliefs.                          #
# 2. Decide what to do next (deliberate) using current belief values.         #
# 3. Perform appropriate task.                                                #
# 4. Repeat steps 1-3 until mission is complete (or error)                    #
#                                                                             #
# The mission control (MC) node does not deal with low-level functionality    #
# of the turtlebot. This is done by the 'tasks'. The mc will decide which     #
# to run when certain preconditions are met. It does not care how tasks are   #
# imnplemented.                                                               #
#                                                                             #
# TASKS:                                                                      #
# Each task is a class derived from the 'tasks.task' base class. They should  #
# be located in the 'tasks' sub-directory. This ensures that all tasks have   #
# a common interface. Tasks use services provided by the hardware controller  #
# nodes. For example: the 'gotoTarget' task uses the low-level services       #
# provided by the 'motionControl' node in order to move the turtlebot.        #
#                                                                             #
# CONTROLLERS:                                                                #
# All hardware control is performed by 'controllers'. All controllers provide #
# services which allow tasks to uses their capabilities. Some tasks may need  #
# to access multiple pieces of hardware. Controllers are just ROS nodes.      #
#                                                                             #
# E.g. motionController --> move the roomba, provide access to odometry etc.  #
# E.g. smart_arm_server --> move the arm, provide torque information etc.     #
# E.g. VISPcontroller (?) --> provide image processing capabilites etc.       #
#                                                                             #
# Controller nodes also publish messages which the MC will listen for e.g.    #
# 'target located' or 'roomba crashed into object'.                           #
###############################################################################

# Python modules

# ROS modules
import roslib
roslib.load_manifest('mc')
import rospy
from turtlebot_node.msg import BatteryState
from turtlebot_node.msg import LaptopChargeStatus

# Custom modules
from tasks.fixNodes import fixNodes
from tasks.calibrateCamera import calibrateCamera
from tasks.handleCrash import handleCrash
from tasks.locateTarget import locateTarget
from tasks.gotoTarget import gotoTarget
from tasks.graspTarget import graspTarget
from mc.srv import mc_updateBelief

class mc:
    """Mission control class, based on hybrid BDI architecture."""

###############################################################################

    def __init__(self):
        """Initialize local varaiables etc."""
        rospy.loginfo("MC: Initializing mission control node...")

        self.beliefs = {
                        'nodesOnline': 1,         # Assume nodes are OK.
                        'cameraCalibrated': 1,    # Assume camera is calibrated.
                        'crashed': 0,             # Roomba crashed into something
                        'targetLocated': 0,       # Camera can see the object from far away
                        'atTarget': 0,            # Object near the robot
                        'targetReachable': 1,     # Arm can grab the object
                        'targetGrasped': 0,       # Object in hand 
                        'roombaBatteryPerc': 100, # Assume fully charged.
                        'laptopBatteryPerc': 100  # Assume fully charged.
                        }

        self.currentTask = 0

        # Let tuertlebot_node know that we're using a 'roomba' not a 'create'.
        rospy.set_param('~robot_type', 'roomba')

        #Linux USB device from which to read the data. Default = /dev/ttyUSB0
        rospy.set_param('~port', '/dev/ttyUSB0')

        # Play Mario Bros. theme song.
        rospy.set_param('~bonus', True)

###############################################################################

    def launch(self):
        """Launch the mission control logic."""
        rospy.loginfo("MC: Launching mission control logic...")

        # 1. Provide service for updating beliefs.
        rospy.Service('mc_updateBelief', mc_updateBelief, self.updateBeliefHandler)

        # Loop until the mission is completed or until an error occurs.
        while(not rospy.is_shutdown()):

            # Check the status of the battery.
            rospy.Subscriber('/turtlebot_node/LaptopChargeStatus', LaptopChargeStatus, self.LaptopChargeHandler)

            # Check the roomba battery.
            rospy.Subscriber('/turtlebot_node/BatteryState', BatteryState, self.BatteryStateHandler)

            # 2. Deliberate / decide what to do next.
            self.deliberate()

            # 3. Act.
            self.act()

            # Is the mission completed?
            if(self.beliefs['targetGrasped'] == 1):
                break

        rospy.loginfo("MC: Mission complete!")

###############################################################################

    def updateBeliefHandler(self, msg):
        """Callback function for the updateBelief service."""
        belief = msg.belief
        value = msg.value
        # Has the belief changed?
        if self.beliefs[belief] != value:
            # Update our records.
            self.beliefs[belief] = value
            rospy.loginfo("MC: Updated belief: '" + str(belief) + "' = " + str(value))
        return []

###############################################################################

    def LaptopChargeHandler(self, msg):
        self.beliefs['laptopBatteryPerc'] = msg.percentage

###############################################################################

    def BatteryStateHandler(self, msg):
        percentage = (msg.charge / msg.capacity) * 100.0
        self.beliefs['roombaBatteryPerc'] = percentage

###############################################################################

    def deliberate(self):
        """ Use the current values in self.beliefs to decide which
            task to perform next."""
        rospy.loginfo("MC: Deliberating...")

        # The following list of rules are ordered by importance.
        # I.e. for any rule to be applied, all of the rules above
        # it must be applied first. For example, before trying to locate the target,
        # we must check that all nodes are running. Before grasping the
        # target, we must check that we are at the target.

        #######################################################################
        # Rule: Is the laptop battery sufficiently charged?
        if(self.beliefs['latptopBatteryPerc'] < 10): # Nominal choice of 10% charge.
            rospy.logwarn("MC: Laptop battery has insufficient charge! ("
                          + str(self.beliefs['laptopBatteryPerc']) + "%)")
            return

        #######################################################################
        # Rule: Is the roomba battery sufficiently charged?
        if(self.beliefs['roombaBatteryPerc'] < 10): # Nominal choice of 10% charge.
            rospy.logwarn("MC: Roomba battery has insufficient charge! ("
                          + str(self.beliefs['roombaBatteryPerc']) + "%)")
            return

        #######################################################################
        # Rule: Are all of the ROS nodes online?
        ##if(self.beliefs['nodesOnline'] == 0):
        ##    # Try to fix downed nodes.
        ##    rospy.logwarn("MC: One or more of my nodes are down!")
        ##    self.currentTask = fixNodes()
        ##    return

        #######################################################################
        # Rule: Are the nodes online?
        if(self.beliefs['nodesOnline'] == 0):
            # Calibrate the camera.
            rospy.logwarn("MC: One of the nodes is down!")
            self.currentTask = fixNodes()
            return

        #######################################################################
        # Rule: Is the camera calibrated?
        if(self.beliefs['cameraCalibrated'] == 0):
            # Calibrate the camera.
            rospy.logwarn("MC: My camera is not calibrated!")
            self.currentTask = calibrateCamera()
            return

        #######################################################################
        # Rule: Has the turtlebot crashed into something?
        if(self.beliefs['crashed'] == 1):
            # Try to reorient the turtlebot.
            rospy.logwarn("MC: I crashed into something!")
            self.currentTask = handleCrash()
            return

        #######################################################################
        # Rule: Has the target been located?
        if(self.beliefs['targetLocated'] == 0):
            # Calibrate the camera.
            rospy.logwarn("MC: I can't see the target!")
            self.currentTask = locateTarget()
            return

        #######################################################################
        # Rule: Is the turtlebot at the target?
        if(self.beliefs['atTarget'] == 0):
            # Drive to the target.
            rospy.logwarn("MC: I'm not at the target yet!")
            self.currentTask = gotoTarget()
            return

        #######################################################################
        # Rule: Is the target reachable?
        if(self.beliefs['targetReachable'] == 0):
            rospy.loginfo("MC: I can't reach the target from here!")
            self.currentTask = makeReachable()

        #######################################################################
        # Rule: If everything is OK then grasp the target.
        if(self.beliefs['targetGrasped'] == 0):
            rospy.loginfo("MC: I'm going to try to grasp the target!")
            self.currentTask = graspTarget()



###############################################################################

    def act(self):
        """Carry out the next task."""
        rospy.loginfo("MC: Starting task '" + str(self.currentTask.name) + "'...")
        self.currentTask.start()

###############################################################################

# Module entry point.
if __name__ == "__main__":

    # Initialize ROS node and set the logger verbosity.
    #rospy.init_node("mc", log_level=rospy.DEBUG) # <--- Uncomment/comment to show/hide debug message.
    rospy.init_node("mc", log_level=rospy.INFO) # <--- Comment/uncomment to show/hide debug message.

    # Instantiate mc class.
    m = mc()

    # Launch mc logic.
    m.launch()
