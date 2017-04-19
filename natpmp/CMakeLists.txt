set(SOURCES
getgateway.c
natpmp.c
wingettimeofday.c)
set(HEADERS getgateway.h declspec.h
natpmp.h)

add_library(natpmp STATIC ${SOURCES})
