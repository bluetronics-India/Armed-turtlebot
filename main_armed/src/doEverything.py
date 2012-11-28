#!/usr/bin/env python

import os, sys, time, ipdb

# ROS init
import roslib
roslib.load_manifest('main_armed')
import rospy

# Messages and services
from std_msgs.msg import Float64, String
from geometry_msgs.msg import Twist
from smart_arm_node.srv import SmartArmService
import tf


def service_handler(kind, data=[0,]):
    rospy.wait_for_service('/arm_server/services')
    try:
        rospy.logdebug(("= Service request\n    kind: %s   data: %s" % (kind, data)))
        use_service = rospy.ServiceProxy('/arm_server/services', SmartArmService)
        resp = use_service(kind, data)
        ans = list(resp.values)
        rospy.logdebug(("    resp: %s\n" % ans))
        return ans
    except rospy.ServiceException, e:
        rospy.logerr(("Service call (%s) failed: %s" % (kind, e)))
        
        
def ballpos_handler(msg):
    global moveto
    # Read message
    line = msg.data
    rospy.logdebug(("ballpos_handler receives : %s" % line))
    line = line.split(' ')
    x = int(line[0])
    y = int(line[1])
    r = float(line[2])
    # image = 640 x 480, write to global variable
    if r > 50:
        if x < 200:
            moveto = -0.05
            rospy.logdebug("ballpos_handler move : positive")
        elif x > 340:
            moveto = +0.05
            rospy.logdebug("ballpos_handler move : negative")
        else:
            moveto = 0
            rospy.logdebug("ballpos_handler move : found!!!")
        rospy.loginfo(("x = %s" % x))
        rospy.loginfo(("moveto = %s" % moveto))
    rospy.loginfo(("r = %s" % r))
        

def points_handler(msg):
    global initpt
    #if not initpt:
    # Read message
    line = msg.data
    rospy.loginfo(("points_handler receives : %s" % line))
    # Set visual servoing parameter
    rospy.set_param('/visualservoing/circleInit', line)
    initpt = True


if __name__ == "__main__":
    # Global variables
    global moveto, initpt
    moveto = 0.05
    initpt = False
    # Init node
#    rospy.init_node('smart_arm_server', log_level=rospy.INFO)
    rospy.init_node('main_control', log_level=rospy.DEBUG)
    rospy.logdebug('>>> Node main_arm started')
    
    # Set arm to home position
    rospy.logdebug('>>> Set arm to home position')
    service_handler(10)
        
    # Subscribe to ball detector for ball position
    ballsub = rospy.Subscriber('/detection/posrad', String, ballpos_handler)
    
    # Rotate until ball is in the center
    rospy.logdebug('>>> Set ball to center of camera')
    cmd_vel = rospy.Publisher('cmd_vel', Twist)
    while not moveto == 0:
#        cmd_move = Twist()
#        cmd_move.linear.x = 0.0
#        cmd_move.linear.y = 0.0
#        cmd_move.linear.z = 0.0
#        cmd_move.angular.x = 0.0
#        cmd_move.angular.y = 0.0
#        cmd_move.angular.z = moveto
#        cmd_vel.publish(cmd_move)
        service_handler(11, [moveto,])
        time.sleep(0.2)
        
    # Obtain the 5 point initialization for visual servoing
    rospy.logdebug('>>> Obtain 5 point initialization')
    poitnsub = rospy.Subscriber('/detection/5points', String, points_handler)
    while not initpt:
        time.sleep(0.2)
        
    # Kill camera_server to free the camera
    rospy.logdebug('>>> Kill camera server')
    os.system('kill -s INT `pidof camera_node`')
    time.sleep(5)
    
    # Start the visual servoing node
    rospy.logdebug('>>> Launch VISP node')
    os.system("xterm -geometry 100x50 -hold -T 'VISP' -e 'roslaunch tracking tracker_real.launch'")
    time.sleep(5)
    
    # Listen to transformations to know the ball position
    rospy.logdebug('>>> Listen to tf')
    x = 0.0
    y = 0.0
    z = 0.0
    tflis = tf.TransformListener()
    for i in range(10):
#        ipdb.set_trace()
        t = tflis.getLatestCommonTime("/joint0", "/obj_pos")
        trans, rot = tflis.lookupTransform("/joint0", "/obj_pos", t)
        x += trans[0]
        y += trans[1]
        z += trans[2]
        time.sleep(0.3)
    x /= 10.0
    y /= 10.0
    z /= 10.0
    rospy.logdebug(('tf last trans : %s' % trans))
    rospy.logdebug(('tf position   : %s, %s, %s' % (x, y, z)))
    
    # Move arm to the position with security
    rospy.logdebug('>>> Move arm with security')
    service_handler(7, [x, y, 0.15])
    
    # Move arm to the position
    rospy.logdebug('>>> Move arm to ball')
    service_handler(7, [x, y, 0.10])
    
    # Grab the ball
    rospy.logdebug('>>> Grab the ball')
    service_handler(8)
    
    # Return home position
    rospy.logdebug('>>> Go back home')
    service_handler(10)
    
    # Celebrate
    rospy.logdebug('>>>>>>>>>>><<<<<<<<<<')
    rospy.logdebug('>>>>>  FINISHED <<<<<')
    rospy.logdebug('>>>>>>>>>>><<<<<<<<<<')