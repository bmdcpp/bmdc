#!/bin/sh
PROG=exaile
TITLE=`exaile --get-title`
ALBUM=`exaile --get-album`
ARTIST=`exaile --get-artist`

echo "${PROG} playing ${ARTIST} - ${ALBUM} - ${TITLE}"

