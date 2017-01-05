set(SOURCES
connecthostport.c
igd_desc_parse.c
minihttptestserver.c
minisoap.c
minissdpc.c
miniupnpc.c
miniwget.c
minixml.c
minixmlvalid.c
portlistingparse.c
receivedata.c
testigddescparse.c
upnpc.c
upnpcommands.c
upnpdev.c
upnperrors.c
upnpreplyparse.c
)
set(HEADERS upnpcommands.h receivedata.h miniupnpctypes.h miniupnpc_declspec.h
miniupnpc.h igd_desc_parse.h
miniupnpcstrings.h minissdpc.h connecthostport.h
declspec.h)

add_library(miniupnpc STATIC ${SOURCES})
