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
# Global Constants
################################################################################

from twython import Twython, TwythonStreamer
import pygame.camera
import pygame.image
import threading
import signal
import sys
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
# Global Variables
################################################################################

g_mainloop = False
g_listen_streamer = None

################################################################################
# Classes
################################################################################

# Streamer class. Use this to look for commands on Twitter.
class ListenStreamer(TwythonStreamer):
    def on_success(self, data):
        if 'text' in data:
            print data['text'].encode('utf-8')
                
    def on_error(self, status_code, data):
        print status_code, data
        
    def stop(self):
        self.disconnect()

################################################################################
# Functions
################################################################################

# Create a Twitter streamer in a thread
def createStreamer():
    global g_listen_streamer
    g_listen_streamer = ListenStreamer( APP_KEY, \
                                        APP_SECRET, \
                                        OAUTH_TOKEN, \
                                        OAUTH_TOKEN_SECRET)
    g_listen_streamer.statuses.filter(track='@SFE_Fellowship')

# Handle ctrl-C events
def signalHandler(signal, frame):
    global g_mainloop
    if DEBUG > 0:
        print 'Ctrl-C pressed. Exiting nicely.'
    g_mainloop = False
  
################################################################################
# Main
################################################################################

def main():

    global g_mainloop
    global g_listen_streamer

    # Initialize GPIO
    in_success = mraa.Gpio(SUCCESS_PIN)
    in_failure = mraa.Gpio(FAILURE_PIN)
    in_status_0 = mraa.Gpio(STATUS_PIN_0)
    in_status_1 = mraa.Gpio(STATUS_PIN_1)
    
    # Connect to Twitter
    tw = Twython(APP_KEY, APP_SECRET, OAUTH_TOKEN, OAUTH_TOKEN_SECRET)
    listen_thread = threading.Thread(target=createStreamer)
    listen_thread.daemon = True
    listen_thread.start()
    
    # Register 'ctrl+C' signal handler
    signal.signal(signal.SIGINT, signalHandler)

    # Main loop
    g_mainloop = True
    if DEBUG > 0:
        print 'Starting door'
    while g_mainloop:
        
        # Poll pins for success or failure
        if in_success.read() == 0:
            person_ind = 3 - ((2 * in_status_1.read()) + in_status_0.read())
            if DEBUG > 0:
                print 'Success!'
                print 'Person = ' + NAMES[person_ind]
        elif in_failure.read() == 0:
            if DEBUG > 0:
                print 'Fail.'

                
    # Outside of main loop. Cleanup and cuddles.
    if DEBUG > 0:
        print 'Cleaning up.'
    g_listen_streamer.stop()
    listen_thread.join(None)
    del listen_thread
                
# Run main
main()
