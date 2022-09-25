import RPi.GPIO as GPIO
import time

GPIO.setmode(GPIO.BOARD)

BUTTON = 11
JOYSTICK = 15
SWITCH = 13

GPIO.setup(BUTTON, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
GPIO.setup(JOYSTICK, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(SWITCH, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

oldButton = 0
oldJoystick = 0

msgInd = 0

greetings = [
	"Hello",
	"Hola",
	"Howdy"
]

goodbyes = [
	"Goodbye",
	"Adios",
	"See ya"
]

msgState = GPIO.input(SWITCH)


def button_pressed(channel):
	global oldButton, msgInd
	while GPIO.input(BUTTON):
		newTime = time.time()
		diff = newTime - oldButton
		oldButton = newTime
		if diff > .01:
			if msgState:
				print(greetings[msgInd])
			else:
				print(goodbyes[msgInd])

def switch_activated(channel):
	global msgState
	msgState = GPIO.input(SWITCH)
	print("Switching to {}...".format("greetings" if msgState else "goodbyes"))

def joystick_pressed(channel):
	global oldJoystick, msgInd
	while not GPIO.input(JOYSTICK):
		newTime = time.time()
		diff = newTime - oldJoystick
		oldJoystick = newTime
		if diff > .008:
			msgInd = (msgInd + 1) % 3

GPIO.add_event_detect(BUTTON,GPIO.RISING,callback=button_pressed, bouncetime=150)
GPIO.add_event_detect(SWITCH,GPIO.BOTH,callback=switch_activated, bouncetime=150)
GPIO.add_event_detect(JOYSTICK,GPIO.FALLING,callback=joystick_pressed, bouncetime=150)


input("Press enter to quit!")

GPIO.cleanup()
