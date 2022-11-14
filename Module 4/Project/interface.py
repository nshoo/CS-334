from gpiozero import Button, LED
from time import sleep
from signal import pause
from os import system
import RPi.GPIO as GPIO
from threading import Thread
from subprocess import check_call

class Stepper:
    stages = [[1,0,0,1],
              [1,0,0,0],
              [1,1,0,0],
              [0,1,0,0],
              [0,1,1,0],
              [0,0,1,0],
              [0,0,1,1],
              [0,0,0,1]]

    def __init__(self, in1, in2, in3, in4):
        self.pins = [in1, in2, in3, in4]
        GPIO.setwarnings(False)
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(self.pins[0], GPIO.OUT)
        GPIO.setup(self.pins[1], GPIO.OUT)
        GPIO.setup(self.pins[2], GPIO.OUT)
        GPIO.setup(self.pins[3], GPIO.OUT)
        self.pos = 0
        self.thread = Thread(target=self.control_thread)
        self.stopped = True
        self.thread.start()


    def apply_stage(self, stage):
        for i, val in enumerate(stage):
            GPIO.output(self.pins[i], GPIO.HIGH if val == 1 else GPIO.LOW)

    def advance(self):
        stage = self.stages[self.pos]
        self.apply_stage(stage)
        self.pos = (self.pos + 1) % len(self.stages)

    def control_thread(self):
        while True:
            if not self.stopped:
                self.advance()
                sleep(0.001)

    def turning(self, state):
        if state:
            self.stopped = False
        else:
            self.stopped = True

class SalvadorInterface:
    def __init__(self):
        self.start_btn = Button(12, pull_up=True)
        self.status_led = LED(13)
        self.shutdown_btn = Button(26, pull_up=True, hold_time=5.0)
        self.shutdown_btn.when_held = self.shutdown
        self.stepper = Stepper(27, 22, 5, 6)

    def status_light(self, state):
        if state:
            self.status_led.on()
        else:
            self.status_led.off()

    def wait_for_start(self):
        count = 0
        total = 0
        while not self.start_btn.is_pressed:
            if count == 0:
                self.status_led.toggle()
            if total > 200:
                self.status_led.off()
                return False
            count = (count + 1) % 5
            total += 1
            sleep(0.05)
        self.status_led.off()
        return True

    def wait_for_capture(self):
        self.status_led.on()
        self.start_btn.wait_for_press()
        self.status_led.off()

    def loading(self, state):
        self.stepper.turning(state)

    def shutdown(self):
        check_call(['sudo', 'poweroff'])

    def error(self):
        for i in range(10):
            self.status_led.toggle()
            sleep(0.05)
        self.status_led.off()
