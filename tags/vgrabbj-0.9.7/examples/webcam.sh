#!/bin/bash
# Amaya Rodrigo sastre <amaya@debian.org>, 15 Jan 2001
# This is free software, under the terms of the GPL v2 or later
# See http://www.gnu.org/copyleft/gpl.html
# Comments on this script are highly appreciated.

pid=`fuser /dev/video | cut -b 22-30`

# If it's not being used, we take a snapshot:
if [ "pid" != "" ]; then
	/usr/bin/vgrabbj
# If it's in use:
else
	# Find out who is using the device:
	program=`ps ax | grep $pid | grep -v grep | cut -d \/ -f 4`

	# If it's a crashed vgrabbj, we kill it and take a snapshot.
	if [ "program" = "vgrabbj" ]; then
		kill -9 $pid
		/usr/bin/vgrabbj
	# If it's another program, leave it alone:
	else
		exit 0
	fi
fi  	

