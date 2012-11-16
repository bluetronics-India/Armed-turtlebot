#!/usr/bin/env python

# Python modules

# ROS modules
import rospy

# Custom modules
from task import task
from mc.srv import updateBelief

###############################################################################


class fixNodes(task):

    name = "fixNodes"

    def __init__(self):
        pass

    def task(self, statusServices=[]):
        """Make sure all nodes are up and running."""
        self.requestService(updateBelief, ("nodesOnline", 1))
