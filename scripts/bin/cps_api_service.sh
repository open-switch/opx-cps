#!/bin/bash

USERNAME=cpsuser
GROUPNAME=cpsusers
GROUPMEMBERS="admin amazon"

HAS_GROUP=$(/bin/cat /etc/group | grep  $GROUPNAME  &> /dev/null; echo $?)

if [ ! $HAS_GROUP = 0 ] ; then
	groupadd  $GROUPNAME
	if [ ! $? = 0 ] ; then
		echo "Failed to add group named $GROUPNAME"		
	fi  	
fi

for i in $GROUPMEMBERS ; do
	usermod -a -G  $GROUPNAME $i
	if [ ! $? = 0 ] ; then
		echo "Failed to add user $i to $GROUPNAME"
		continue
	fi  	
done

HAS_USER=$(id $USERNAME &>/dev/null ; echo $?)
if [ ! $HAS_USER = 0 ] ; then
	useradd -g $GROUPNAME $USERNAME
fi

$(dirname $0)/cps_api_service