from Tkinter import *
from roadscanvas import RoadsCanvas
#from roadmodel import RoadModel

class window:
    def __init__(self):
        self.ix = 0

    def run(self):
        self.create_window()
        self.root.mainloop()

    def create_window(self):
        root = Tk()
        root.title("Road network")
        w, h = RoadsCanvas.CANVAS_SIZE

        frame = Frame(root)
        frame.pack(side=TOP, fill=BOTH, expand=YES)
        buttonsFrame = Frame(frame)
        buttonsFrame.pack(side=TOP, expand=NO)
        startButton = Button(buttonsFrame, takefocus=YES, text="Start")
        startButton.pack(expand=NO, side=LEFT)
        startButton.bind("<Button-1>", self.start)
        stopButton = Button(buttonsFrame, takefocus=YES, text="Stop")
        stopButton.pack(expand=NO, side=LEFT)
        stopButton.bind("<Button-1>", self.stop)
        snapshotButton = Button(buttonsFrame, takefocus=YES, text="Snapshot")
        snapshotButton.pack(expand=NO, side=LEFT)
        snapshotButton.bind("<Button-1>", self.snapshot)
        self.buttons = {'start': startButton,
                        'stop': stopButton,
                        'snapshot': snapshotButton}

        stopButton['state'] = 'disabled'
        snapshotButton['state'] = 'disabled'
        self.started = False

        c = Canvas(frame, width=w, height=h)
        c.pack(side=TOP, expand=YES)
        self.rc = RoadsCanvas(c, "Dogs")

        root.after(10, self.idle)

        self.root = root

    def start(self, e):
        if not self.started:
            self.started = True
            self.buttons['start']['state'] = 'disabled'
            self.buttons['stop']['state'] = 'normal'
            self.buttons['snapshot']['state'] = 'normal'

    def stop(self, e):
        if self.started:
            self.started = False
            self.buttons['start']['state'] = 'normal'
            self.buttons['stop']['state'] = 'disabled'

    def snapshot(self, e):
        self.rc.snapshot()

    def idle(self):
        self.root.after(100, self.idle)
        if not self.started:
            return
        self.ix += 1
        self.rc.change_text("Dogs: %d" % self.ix)
        try:
            #print "doing thing: %d" % self.ix
            for i in range(100):
                self.rc.idlelp()
            self.rc.ca.update_idletasks()
        except:
            "stop the lines first"
            self.stop(None)
            raise

if __name__ == "__main__":
    window().run()
