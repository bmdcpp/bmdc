#!/bin/sh
# announce the song currently playing in audacious, in the format:
# audacious(1.1.0) playing (Artist - Title - Bitrate) - (0:02/4:19 [|---------])
# Optionally uses mp3info from http://www.ibiblio.org/mp3info on VBR files

PROG="audacious"

VER=`${PROG} -v | cut -d " " -f 2`

STATUS=`audtool playback-status`

if [ "$STATUS" = "audtool: audacious server is not running!" ] ; then
	echo "Audacious is not running."
	exit 10
fi

MP3FILE=`audtool current-song-filename`
POS=`audtool current-song-output-length`
LENGTH=`audtool current-song-length`

SCUR=`audtool current-song-output-length-seconds`
STOT=`audtool current-song-length-seconds`

KBPS=`audtool current-song-bitrate-kbps `

#BITRATE=`mp3info -p "%r" "$MP3FILE"`
#if [ "$BITRATE" = "Variable" ] ; then
#	BITRATE=`mp3info -r a -p "%r" "$MP3FILE"`
#	KBPS=`printf "VBR avg. %.0f" $BITRATE`
#fi

TITLE=`audtool current-song`

PERDECA=`expr 10 \* ${SCUR}`
PERDECA=`expr ${PERDECA} \/ ${STOT}`

SLIDER="|---------"

case "$PERDECA" in
	0  ) SLIDER="|---------" ;;
	1  ) SLIDER="-|--------" ;;
	2  ) SLIDER="--|-------" ;;
	3  ) SLIDER="---|------" ;;
	4  ) SLIDER="----|-----" ;;
	5  ) SLIDER="-----|----" ;;
	6  ) SLIDER="------|---" ;;
	7  ) SLIDER="-------|--" ;;
	8  ) SLIDER="--------|-" ;;
	9  ) SLIDER="---------|" ;;
	10 ) SLIDER="---------|" ;;
esac

echo -n "${PROG}(${VER}) ${STATUS} (${TITLE} - ${KBPS}kbps) - (${POS}/${LENGTH} [${SLIDER}])"
