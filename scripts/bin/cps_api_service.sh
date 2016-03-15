#!/bin/bash
#
# Copyright (c) 2015 Dell Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
#
# THIS CODE IS PROVIDED ON AN #AS IS* BASIS, WITHOUT WARRANTIES OR
# CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
#  LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
# FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
#
# See the Apache Version 2.0 License for specific language governing
# permissions and limitations under the License.
#

USERNAME=cpsuser
GROUPNAME=cpsusers
GROUPMEMBERS="admin linuxadmin"

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
