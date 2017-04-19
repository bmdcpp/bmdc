#!/usr/bin/env python
# -*- coding: CP1250 -*-#

# Copyright  2006-2011 <funman at videolanorg>
# Modifed to BMDC by Mank 2013-2014
#
# NOTE: This controller is a SAMPLE, and thus doesn't use all the
# Media Player Remote Interface Specification (MPRIS for short) capabilities
#
# MPRIS: http://www.mpris.org/2.1/spec/
#

# core dbus stuff
import dbus
from dbus.mainloop.glib import DBusGMainLoop
DBusGMainLoop(set_as_default=True)
# file loading
import os
import re
global player
global props
global bus          # Connection to the session bus


mpris='org.mpris.MediaPlayer2'

# If a Media Player connects to the bus, we'll use it
# Note that we forget the previous Media Player we were connected to
def NameOwnerChanged(name, new, old):
    if old != '' and mpris in name:
        Connect(name)

def PropGet(prop):
    global props
    return props.Get(mpris + '.Player', prop)

def PropSet(prop, val):
    global props
    props.Set(mpris + '.Player', prop, val)

# Callback for when 'TrackChange' signal is emitted
def TrackChange(Track):
    try:
        a = Track['xesam:artist']
    except:
        a = ''
    try:
        t = Track['xesam:title']
    except:
        t = Track['xesam:url']
    try:
        length = Track['mpris:length']
    except:
        length = 0
# Connects to the Media Player we detected
def Connect(name):
    global root, player, tracklist, props
    global playing, shuffle

    root_o = bus.get_object(name, '/org/mpris/MediaPlayer2')
    root        = dbus.Interface(root_o, mpris)
    tracklist   = dbus.Interface(root_o, mpris + '.TrackList')
    player      = dbus.Interface(root_o, mpris + '.Player')
    props       = dbus.Interface(root_o, dbus.PROPERTIES_IFACE)
    Track = PropGet('Metadata')
    TrackChange(Track)
    
    try:
        a = Track['xesam:artist']
    except:
        a = ''
    try:
        t = Track['xesam:title']
    except:
        t = Track['xesam:url']
        q = t.count('/')
        t = t.split("/")
        t = t[q]
    try:
        length = Track['mpris:length']
    except:
        length = 0
    # here change to what want pop up in Chat
    print 'Playing %s - %s ' % (a[0].encode(encoding='UTF-8',errors='strict') , t.encode(encoding='UTF-8',errors='strict'))
    return

import sys
# connect to the bus
bus = dbus.SessionBus()
dbus_names = bus.get_object( 'org.freedesktop.DBus', '/org/freedesktop/DBus' )
dbus_names.connect_to_signal('NameOwnerChanged', NameOwnerChanged, dbus_interface='org.freedesktop.DBus') # to detect new Media Players

dbus_o = bus.get_object('org.freedesktop.DBus', '/')
dbus_intf = dbus.Interface(dbus_o, 'org.freedesktop.DBus')

# connect to the first Media Player found
for n in dbus_intf.ListNames():
    if mpris in n:
        Connect(n)
        break

