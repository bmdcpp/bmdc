#!/bin/sh
# now playing - DeaDBeeF
SONG=$(deadbeef --nowplaying \
        "%a - %t (%b) | %l (@%@:BITRATE@kbps)" \
        2> /dev/null)

echo "$SONG";
