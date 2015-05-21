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
HANDLE = '@SFE_Fellowship'
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

    # [Constructor] Inherits a Twython streamer
     def __init__(self, parent, app_key, app_secret, oauth_token, 
                    oauth_token_secret, timeout=300, retry_count=None, 
                    retry_in=10, client_args=None, handlers=None, 
                    chunk_size=1):
        TwythonStreamer.__init__(self, app_key, app_secret, oauth_token, 
                    oauth_token_secret, timeout=300, retry_count=None, 
                    retry_in=10, client_args=None, handlers=None, 
                    chunk_size=1)
        self.parent = parent

    # Callback from streamer when tweet matches the search term(s)
    def on_success(self, data):
        if 'text' in data:
            print data['text'].encode('utf-8')
                
    # Callback from streamer if error occurs
    def on_error(self, status_code, data):
        print status_code, data
        
    # Called from main thread to stop the streamer
    def stop(self):
        self.disconnect()

# TweetFeed class sets up the streamer and provides access to the tweets
class TweetFeed:

    # [Constructor] Set up streamer thread
    def __init__(self, twitter_auth):
    
        # Extract authentication tokens
        app_key = twitter_auth['app_key']
        app_secret = twitter_auth['app_secret']
        oauth_token = twitter_auth['oauth_token']
        oauth_token_secret = twitter_auth['oauth_token_secret']
        self.auth_args = (  app_key, 
                            app_secret, 
                            oauth_token, 
                            oauth_token_secret)
                            
        # Setup Twitter object to send tweets
        self.twitter = Twython( app_key, 
                                app_secret, 
                                oauth_token, 
                                oauth_token_secret)
 
    # [Private] Set up streamer and filter(s)
    def __createStreamer(   self, 
                            app_key, 
                            app_secret, 
                            oauth_token, 
                            oauth_token_secret ):
        self.track_stream = ListenStreamer( self, 
                                            app_key, 
                                            app_secret,
                                            oauth_token,
                                            oauth_token_secret )
        self.track_stream.statuses.filter(track=self.track_terms)
        
    # [Public] Start streamer in a thread
    def startStreamer(self, search_terms):
        self.track_terms = [''.join([x, ' ']) for x in search_terms
        self.listen_thread = threading.Thread( \
                target=self.__createStreamer, args=self.auth_args)
        self.listen_thread.daemon = True
        self.listen_thread.start()
        
    # [Public] Stop streamer
    def stopStreamer(self, timeout=None):
        self.track_stream.stop()
        self.listen_thread.join(timeout)
        del self.listen_thread
 
################################################################################
# Functions
################################################################################

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
    
    # Connect to Twitter and start listening
    tf = TweetFeed({APP_KEY, APP_SECRET, OAUTH_TOKEN, OAUTH_TOKEN_SECRET})
    tf.startStreamer([HANDLE])
    
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
    tf.stopStreamer()
    del tf
    pygame.quit()
                
# Run main
main()
