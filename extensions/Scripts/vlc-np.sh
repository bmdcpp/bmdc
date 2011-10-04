#!/bin/sh
# written by mids with assistance from Rampage. Edited by curse for BMDC/CrZDC

TRACK=$(dbus-send --print-reply --type=method_call --dest=org.mpris.vlc /Player org.freedesktop.MediaPlayer.GetMetadata | grep variant | grep string)

export TRACK

TITLE=`perl -e '$a=$ENV{TRACK}; $a =~ m/.*\/(.*)/i; print $1'`

LENGTHMILLI=$(dbus-send --print-reply --type=method_call --dest=org.mpris.vlc /Player org.freedesktop.MediaPlayer.GetMetadata | grep variant | grep int64 | cut -c 36-)

POSITIONMILLI=$(dbus-send --print-reply --type=method_call --dest=org.mpris.vlc /Player org.freedesktop.MediaPlayer.PositionGet | grep int32 | cut -c 10-)


LEN_S=`expr ${LENGTHMILLI} \/ 1000`

LEN_H=`expr ${LEN_S} \/ 3600`
LEN_S=`expr ${LEN_S} \% 3600`

LEN_M=`expr ${LEN_S} \/ 60`
LEN_S=`expr ${LEN_S} \% 60`

POS_S=`expr ${POSITIONMILLI} \/ 1000`

POS_H=`expr ${POS_S} \/ 3600`
POS_S=`expr ${POS_S} \% 3600`

POS_M=`expr ${POS_S} \/ 60`
POS_S=`expr ${POS_S} \% 60`

if [ $POS_M -lt 10 ]; then 
	POS_M=0${POS_M}
fi

if [ $POS_S -lt 10 ]; then
	POS_S=0${POS_S}
fi

if [ $LEN_M -lt 10 ]; then 
	LEN_M=0${LEN_M}
fi

if [ $LEN_S -lt 10 ]; then
	LEN_S=0${LEN_S}
fi

TIMER="(${POS_H}:${POS_M}:${POS_S} of ${LEN_H}:${LEN_M}:${LEN_S})"

OUTPUT=$(echo VLC Spams: '-=["'${TITLE} ${TIMER})']=-'

echo ${OUTPUT}

# match filename with (?!.*\/).*
