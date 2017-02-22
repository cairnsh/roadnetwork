from Tkinter import *
import shortestline

DELAY = 1000 #milliseconds

class testbox:
    TEXT_POSITION = (20, 20)

    def __init__(self, c, text=""):
        self.ca = c
        x, y = testbox.TEXT_POSITION
        self.textidx = c.create_text(x, y, anchor=NW, text=text)

        self.lines = None

    def change_text(self, text):
        self.ca.itemconfig(self.textidx, text=text)

    def clear_lines(self):
        if self.lines:
            for i in self.lines:
                self.ca.delete(i)
            self.lines = None

    def random_line(self):
        import random
        self.clear_lines()
        line1 = [random.random()*640 for i in range(4)]
        line2 = [random.random()*640 for i in range(4)]
        line3 = shortestline.to_line(line1, line2)
        lines = [line1, line2, line3]
        self.lines = [self.ca.create_line(*x) for x in lines]
        self.ca.itemconfig(self.lines[0], fill="black", width=4, arrow=LAST)
        self.ca.itemconfig(self.lines[1], fill="blue", arrow=LAST)
        self.ca.itemconfig(self.lines[2], fill="red")

class window:
    def __init__(self):
        self.ix = 0

    def run(self):
        self.create_window()
        self.root.mainloop()

    def create_window(self):
        root = Tk()
        root.title("Test closest line program")
        w, h = 640, 640

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
        self.buttons = [startButton, stopButton]

        stopButton['state'] = 'disabled'
        self.started = False

        c = Canvas(frame, width=w, height=h)
        c.pack(side=TOP, expand=YES)
        self.rc = testbox(c, "Dogs")

        root.after(DELAY, self.idle)

        self.root = root

    def start(self, e):
        if not self.started:
            self.started = True
            startButton, stopButton = self.buttons
            startButton['state'] = 'disabled'
            stopButton['state'] = 'normal'

    def stop(self, e):
        if self.started:
            self.started = False
            startButton, stopButton = self.buttons
            startButton['state'] = 'normal'
            stopButton['state'] = 'disabled'

    def idle(self):
        self.root.after(DELAY, self.idle)
        if not self.started:
            return
        self.ix += 1
        self.rc.change_text("Dogs: %d" % self.ix)
        self.rc.random_line()

if __name__ == "__main__":
    window().run()
