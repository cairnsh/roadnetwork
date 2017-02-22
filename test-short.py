import unittest
import numpy as np
import shortestline

class TestClosest(unittest.TestCase):
    def test_when_lines_parallel(self):
        lines = (0, 0, 1, 1), (0, 1, 1, 2)
        cl = shortestline.to_line(*lines)
        np.testing.assert_almost_equal(cl, (0.5, 0.5, 0.0, 1.0))
    def test_when_lines_midpoint(self):
        lines = (0, 1, 1, 2), (0, 2, 2, 3)
        cl = shortestline.to_line(*lines)
        np.testing.assert_almost_equal(cl, (1, 2, 0.8, 2.4))
    def test_when_lines_overlap(self):
        lines = (0, 1, 1, 2), (0.5, 1.5, 1.5, 2.5)
        cl = shortestline.to_line(*lines)
        np.testing.assert_almost_equal(cl, (0.5, 1.5, 0.5, 1.5))
    def test_pt_to_lines(self):
        lines = [(0, 0, 1, 0)]
        cl = shortestline.pt_to_lines((2, 0), lines)
        self.assertEqual(cl, (2, 0, 1, 0))

if __name__ == '__main__':
    unittest.main()
