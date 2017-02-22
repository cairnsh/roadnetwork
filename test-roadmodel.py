import unittest
import roadmodel

class TestRoad(unittest.TestCase):
    def test_connecting_two_points(self):
        pass

def not_really_a_test():
        # Ok, I know this isn't really a test. I was too lazy to figure out what order stuff should be in
        x = roadmodel.RoadModel()
        print "This is a simple test of the road model."
        print "\nFirst, connect (0, 0) and (1, 0)\n"
        print "*** Lines created:", x.connect((0, 0), (1, 0))
        print "\nThen connect (2, 0) and (3, 0)\n"
        print "*** Lines created:", x.connect((2, 0), (3, 0))
        print \
"""\nThen connect (-1, 0) and (6, 0). There should be three lines,
one from (-1, 0) to (0, 0), another one from (1, 0) to (2, 0),
and another one from (3, 0) to (6, 0).\n"""
        print "*** Lines created:", x.connect((-1, 0), (6, 0))
        print \
"""\nFinally, connect (-2, 0) to (7, 0). There should be two lines,
one from -2 to -1, and the other one from 6 to 7.\n"""
        print "*** Lines created:", x.connect((-2, 0), (7, 0))

if __name__ == '__main__':
    not_really_a_test()
