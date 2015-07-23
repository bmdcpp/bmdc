import fcntl
import os, time

def listening():	# Announce in the current channel
   global idjc					# the track now playing in idjc
   try:
      file = open(idjc + "songtitle")
      fcntl.flock(file.fileno(), fcntl.LOCK_EX)	# File locking
      song = file.read()
      fcntl.flock(file.fileno(), fcntl.LOCK_UN)
      file.close()
   except IOError:
      print "Unable to read the songtitle file"
   else:
      print ("IDJC playing => " +song +" <= The Best Radio")
   return song			# Processing of the command stops here
 
idjc = os.environ.get("HOME") + "/.idjc/"	# The file path of the ~/.idjc directory   
listening()  
