# door.py
# IoT Video Series Airlock Door Controller
# Shawn Hymel @ SparkFun Electronics
# May 20, 2015
# https://github.com/sparkfun/IoT_Airlock
#
# Runs on an Edison to receive input from an Arduino, which controls the
# security sensors. The Edison posts photos of uncleared individuals to Twitter
# and opens doors for cleared individuals.
# This code is beerware; if you see me (or any other SparkFun 
# employee) at the local, and you've found our code helpful, please
# buy us a round!
#
# Distributed as-is; no warranty is given.

################################################################################
# Globals
################################################################################

from twython import Twython, TwythonStreamer
import pygame.camera
import pygame.image
import threading
import mraa
import time

# Parameters
DEBUG = 1
IMG_NAME = 'intruder.jpeg'
INTRUDER_MSG = 'Someone is at the door.'
NAMES = ['@ShawnHymel', '@NorthAllenPoole', '@Sarah_Al_Mutlaq']
SUCCESS_PIN = 31    # GP44
FAILURE_PIN = 45    # GP45
STATUS_PIN_0 = 32   # GP46
STATUS_PIN_1 = 46   # GP476

# Twitter credentials
APP_KEY = 'sppWdEK9E3RVjo70zOKr6p7y1'
APP_SECRET = 'cRUKs3no6JSYubr6QqalgehhYfqIJYkNcvBLQMUIxfoUx9eQjB'
OAUTH_TOKEN = '3220604167-IvvyfABekUBIhP1twLlgko4ufvwqG8HTYAg5m3F'
OAUTH_TOKEN_SECRET = 'BoKwdbHc0tO4dQ15UZfutBrkmOkwL6J9DABA3YiBlsAH1'

################################################################################
# Classes
################################################################################

################################################################################
# Methods
################################################################################

################################################################################
# Main
################################################################################

def main():

    # Initialize GPIO
    in_success = mraa.Gpio(SUCCESS_PIN)
    in_failure = mraa.Gpio(FAILURE_PIN)
    in_status_0 = mraa.Gpio(STATUS_PIN_0)
    in_status_1 = mraa.Gpio(STATUS_PIN_1)
    
    # Main loop
    mainloop = True
    if DEBUG > 0:
        print 'Starting door'
    while mainloop:
        
        # Poll pins for success or failure
        if in_success.read() == 0:
            person_ind = 3 - ((2 * in_status_1.read()) + in_status_0.read())
            if DEBUG > 0:
                print 'Success!'
                print 'Person = ' + NAMES[person_ind]
        elif in_failure.read() == 0:
            if DEBUG > 0:
                print 'Fail.'
                
    
# Run main
main()
