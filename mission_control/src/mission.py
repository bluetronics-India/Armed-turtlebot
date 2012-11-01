#!/usr/bin/env python

# Python Libraries
import ConfigParser
import pkgutil

# ROS Libraries
import roslib
roslib.load_manifest('mission_control')
import rospy

# Mission Control Librariesi
import tasks
# from mission_control.srv import emergencyStop
# from mission_control.srv import generatePlan
# from mission_control.srv import executePlan
# from smart_arm_node.srv import SmartArmService

###############################################################################


class mission:

    def __init__(self):
        self.currentTask = 0  # Keep track of how many tasks have completed.
        self.totalTasks = 0  # Keep track of how many tasks there are in the mission.
        self.exitStatus = 0  # Keep track of how the last task exited.

        # Think of some way to populate the task dictionary using
        # a scan of the 'task' package.
        # I.e. {class_name; class}
        self.task_dictionary = {}

        # Scan task package for task classes and populate
        # the 'task_dictionary' dicitonary.
        self.getTasks()

        # Mission arry / task list
        self.mission = []

        ### Setup a parser to read the text file for the load() function.
        self.config = ConfigParser.ConfigParser()

    def getTasks(self):
        package = tasks
        prefix = package.__name__ + "."
        for importer, modname, ispkg in pkgutil.iter_modules(package.__path__, prefix):
            task_module = __import__(modname, fromlist="dummy")
            task_class = getattr(task_module, task_module.__name__.split('.')[-1])
            self.task_dictionary[str(task_class.__name__)] = task_class

    def getExitStatus(self):
        """
        Return the exit status of the LAST task that executed.
        This is for the benefit of mission_control.
        It may need to change the mission.
        """
        return self.exitStatus

    def load(self, filename='mission.txt'):
        """
        Load data from a text file
        and generate the mission using the python parser class
        and a dictionary of 'taskName: class' pairs.
        """
        self.config.read(filename)
        sections = self.onfig.sections()

        # Get the 'name' variable from each section
        # this corresponds to the task name.
        for section in sections:
            taskName = Config.get(section, 'name')
            self.mission += task_dictionary[taskName]
            print("'" + str(taskName) + "' added to mission.")

    def reset(self):
       """
       === Not essential ===
       Removes all entries from the mission
       """
       pass

    def remove(self, taskNumber):
       """
       === Not essential ===
       Remove specific task from mission
       """
       pass

    def insert(self, tasknNumber, taskName):
       """
       === Not essential ===
       Insert taskName after taskNumber
       """
       pass

    def list(self):
       """
       Print an ordered list of the mission tasks
       """
       pass

    def StartNextTask(self):
       """
       Executes the next task.
       Increments the task counter on complete
       """
       pass