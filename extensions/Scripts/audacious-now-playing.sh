#!/bin/sh
# Print out the song currently playing in Rhythmbox

PROG="rhythmbox"
#ARTIST=`rhythmbox-client --print-playing-format '%aa - %at - %tt'`
ARTIST=`rhythmbox-client --print-playing-format '%st %tt'`
echo "/me listening ( ${ARTIST} ) with RB "
