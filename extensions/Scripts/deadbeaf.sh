#!/bin/sh
# now playing - DeaDBeeF
SONG=$(deadbeef --nowplaying \
        "%a - %t (%b) | %l (@%@:BITRATE@kbps)" \
        2> /dev/null)

OUTPUT=$OUTPUT+"/me is listening to " ;
OUTPUT=$OUTPUT+"$SONG";
echo "$OUTPUT";
