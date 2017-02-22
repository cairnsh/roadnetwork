from Tkinter import *
import shortestline
import math, random

class RoadsCanvas:
    TEXT_POSITION = (20, 20)
    CANVAS_SIZE = (768, 768)

    def __init__(self, c, text=""):
        self.ca = c
        #x, y = RoadsCanvas.TEXT_POSITION
        #self.textidx = c.create_text(x, y, anchor=NW, text=text)

        import roadmodel
        self.roadmodel = roadmodel.RoadModel()

    def change_text(self, text):
        pass
        #self.ca.itemconfig(self.textidx, text=text)

    def randompot(self):
        x = random.random()*self.CANVAS_SIZE[0]
        y = random.random()*self.CANVAS_SIZE[1]
        return (x, y)

    def add_line(self, li):
        self.ca.create_line(li, cap=ROUND)

    def add_random_pts(self):
        p1 = self.randompot()
        p2 = self.randompot()
        lines = self.roadmodel.connect(p1, p2)
        #print lines
        for l in lines:
            self.add_line(l)

    def idlelp(self):
        self.add_random_pts()

    def snapshot(self):
        import datetime ##
        time = datetime.datetime.today().strftime("%Y-%m-%d-%H%M%S")
        self.ca.postscript(file="roadnetwork-snapshot-" + time + ".ps")
