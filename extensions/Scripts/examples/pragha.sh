#!/bin/sh
# Simple script to show output from Pragha
PLAYERB=`qdbus org.mpris.MediaPlayer2.pragha /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Metadata|grep album|cut -c 7- -`
PLAYERA=`qdbus org.mpris.MediaPlayer2.pragha /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Metadata|grep artist|cut -c 7- -`
PLAYERT=`qdbus org.mpris.MediaPlayer2.pragha /org/mpris/MediaPlayer2 org.mpris.MediaPlayer2.Player.Metadata|grep title|cut -c 7- -`
echo "Pragha ${PLAYERA} - ${PLAYERB} - ${PLAYERT}"
