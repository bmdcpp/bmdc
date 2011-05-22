#!/bin/sh
# Print out the song currently playing in Rhythmbox

PROG="rhythmbox"
ARTIST=`rhythmbox-client --print-playing-format '%aa - %at - %tt'`
echo "/me listenig ( ${ARTIST} ) with RB "
