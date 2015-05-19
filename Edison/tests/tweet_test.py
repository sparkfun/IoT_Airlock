from twython import Twython, TwythonStreamer
import pygame.camera
import pygame.image
import threading
import time

# Parameters
img_name = 'intruder.jpeg'
intruder_msg = 'Someone is at the door.'
names = ['@ShawnHymel', '@NorthAllenPoole', '@Sarah_Al_Mutlaq']

# Twitter credentials
APP_KEY = 'sppWdEK9E3RVjo70zOKr6p7y1'
APP_SECRET = 'cRUKs3no6JSYubr6QqalgehhYfqIJYkNcvBLQMUIxfoUx9eQjB'
OAUTH_TOKEN = '3220604167-IvvyfABekUBIhP1twLlgko4ufvwqG8HTYAg5m3F'
OAUTH_TOKEN_SECRET = 'BoKwdbHc0tO4dQ15UZfutBrkmOkwL6J9DABA3YiBlsAH1'

# Streamer
class ListenStreamer(TwythonStreamer):
  def on_success(self, data):
    if 'text' in data:
      print data['text'].encode('utf-8')

  def on_error(self, status_code, data):
    print status_code, data

  def stop(self):
    self.disconnect()

# Create streamer in a thread
def create_streamer():
  listen_streamer = ListenStreamer(APP_KEY, APP_SECRET, OAUTH_TOKEN, OAUTH_TOKEN_SECRET)
  listen_streamer.statuses.filter(track='@SFE_Fellowship')

# Connect to Twitter
tw = Twython(APP_KEY, APP_SECRET, OAUTH_TOKEN, OAUTH_TOKEN_SECRET)
thread_listen = threading.Thread(target=create_streamer)
thread_listen.daemon = True
thread_listen.start()

# Take a picture
pygame.camera.init()
cam = pygame.camera.Camera(pygame.camera.list_cameras()[0])
cam.start()
img = cam.get_image()
pygame.image.save(img, img_name)
pygame.camera.quit()

# Construct the message
time_now = time.strftime("%m/%d/%Y (%H:%M:%S)")
msg = time_now + ' ' + intruder_msg
for n in names:
  msg = msg + ' ' + n
print msg

# Post to Twitter
fp = open(img_name, 'rb')
image_id = tw.upload_media(media=fp)
tw.update_status(media_ids=[image_id['media_id']], status=msg)
fp.close()

# Wait 30 seconds
time.sleep(30)
listen_streamer.stop()
thread_listen.join(None)
del thread_listen
