#!/usr/bin/env python

# Python modules

# ROS modules
import roslib
#roslib.load_manifest('mc')
import rospy

# Custom modules
from task import task

###############################################################################

class fixNodes(task):

    def __init__(self):
        pass

    def task(self, statusServices=[]):
        """Make sure all nodes are up and running."""
        rospy.loginfo("* Fixing nodes...")
