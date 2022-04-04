"""
.. module:: WaypointQueue.py
	:platform: MacOS, Unix, Windows,
	:synopsis: Waypoint queue for path planning 
.. moduleauthor:: Pavlo Vlastos <pvlastos@ucsc.edu>
"""
import numpy as np


class WaypointQueue():
    def __init__(self, waypoint_queue=np.array([]), threshold=2.5):
        """
        :param waypoint_queue: A queue of waypoints, with East and North 
        elements. For example: wp_0 = np.array([[0.0, 0.0],
                                                [0.0, 1.0],
                                                [1.0, 2.0],
                                                [4.0, 1.0]])
        :param threshold: The minimum distance in meters between two points 
        used to decide whether get the next available waypoint from the
        waypoint queue
        """
        self.index = 0
        self.added_zeros_flag = False

        # Handle the case where the queue is empty, just add a point
        if len(waypoint_queue.shape) == 1:
            self.waypoint_queue = np.zeros((1,2))
            self.next_waypoint = np.zeros((1,2))
            self.added_zeros_flag = True
        else:
            self.waypoint_queue = waypoint_queue
            self.next_waypoint = self.waypoint_queue[[self.index], :]

        self.threshold = threshold

    def getNext(self):
        """
        :return: The next waypoint from the queue if the queue is not empty
        """

        self.next_waypoint = self.waypoint_queue[[self.index], :]
        
        if self.index+1 < self.waypoint_queue.shape[0]:
            self.index += 1

        return self.next_waypoint

    def add(self, new_waypoint=np.zeros((1, 2))):
        """
        :param new_waypoint: A new waypoint to add onto the waypoint queue
        :return: None
        """

        if self.added_zeros_flag:
            self.added_zeros_flag = False
            self.waypoint_queue = new_waypoint
        else:
            self.waypoint_queue = np.concatenate(
            (self.waypoint_queue, new_waypoint), axis=0)

        return None

    def isNearNext(self, point=np.zeros((1, 2))):
        """
        :param point: Any point to compare with the next waypoint. Usually the
        projected closest point of the vehicle onto the path makes the most 
        sense to use.
        :return: True or False
        """
        return (np.linalg.norm(point - self.next_waypoint) <= self.threshold)

    def getTotal(self):
        """
        :return: The total number of waypoints
        """
        return self.waypoint_queue.shape[0]

    def get_waypoints(self):
        return self.waypoint_queue

    def peak_next_waypoint(self):
        return self.waypoint_queue[[self.index], :]

    def peak_last_waypoint(self):
        return self.waypoint_queue[[-1], :]

    def get_index(self):
        return self.index

    def move_waypoints(self, angle, ox, oy):
        """
        :param angle: The orientation of the grid with respect to the LTP
        :param ox: The desired 'x' offset for all points
        :param oy: The desired 'y' offset for all points
        """

        m, _ = self.waypoint_queue.shape

        # Shift in x and y
        for i in range(m):
            self.waypoint_queue[i][0] += ox
            self.waypoint_queue[i][1] += oy

        # Rotate grid
        R = np.array([[np.cos(angle), -np.sin(angle)],
                        [np.sin(angle), np.cos(angle)]])

        self.waypoint_queue = self.waypoint_queue @ R  

###############################################################################
# MODULE TEST EXAMPLE
###############################################################################
if __name__ == '__main__':
    waypoints = np.array([[0.0, 2.0],
                          [0.0, 1.0],
                          [5.0, 4.0]])
    threshold = 2.1

    wpq = WaypointQueue(threshold=threshold)

    print("adding waypoints to the waypoint queue:")

    for row in waypoints:
        row = np.array([row])
        print("adding wp: {}, shape: {}".format(row, row.shape))
        wpq.add(row)

    print("Waypoint Queue contents:\r\n{}".format(wpq.get_waypoints()))

    print("\r\nTest getting waypoints from queue")
    n = wpq.getTotal() + 1
    for i in range(0, n):
        print("getting wp: {}".format(wpq.getNext()))

    print("\r\nTest Proximity for True")
    test_point = np.array([[5.1, 4.1]])
    print("test point: {}".format(test_point))
    print("next waypoint: {}".format(wpq.getNext()))
    print("result: {}".format(wpq.isNearNext(test_point)))
    if wpq.isNearNext(test_point) == True:
        print("Test successul!")
    else:
        print("Test failed!")

    print("\r\nTest Proximity for False")
    test_point = np.array([[15.1, 4.1]])
    print("test point: {}".format(test_point))
    print("next waypoint: {}".format(wpq.getNext()))
    print("result: {}".format(wpq.isNearNext(test_point)))
    if wpq.isNearNext(test_point) == False:
        print("Test successul!")
    else:
        print("Test failed!")