#!/usr/bin/python
#
# Copyright (c) 2016 Dell Inc.
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
import sys
import os
import subprocess
import cps


def run_cmd_get_output(cmdline) :
    """
    Execute any command from python file
    @params[in] cmdline - command line to execute
    @returns Output from the command line execution
    """

    try:
        p = subprocess.Popen(cmdline,stdout=subprocess.PIPE)

        result = p.wait();
        details = p.stdout.read()

        if result != 0:
            exit(1)

        return details.split('\n')
    except :
        #print( sys.exc_info())
        pass
    return [""]




def get_cps_components(input_proc_name):
    """
    Get CPS component registration information and object ownership information from syslog
    May lose CPS registration information due to log rotation
    @params[in] input_proc_name - Optional input Process name to get the CPS objects it owns
    @returns None
    """

    #Find syslog portion of interest (from last restart)
    strs_to_find = ["rsyslogd", "start"]
    #For component re-registrations check for CPS crash and CPS registrations following the crash
    cps_crash_strs1 = ["systemd[1]",  "dn_cps_api.service", "main process exited"]
    cps_crash_strs2 = ["systemd[1]",  "Stopped", "The CPS service"]

    line_offset = []
    offset = 0
    with open('/var/log/syslog',  'r') as inF:
        for line in inF:
            if all(x in line for x in strs_to_find) or all(x in line for x in cps_crash_strs1) or all(x in line for x in cps_crash_strs2):
                line_offset.append(offset)
            offset += len(line)

    #ps_list = {pid: process name}
    ps_list = {}
    found = False
    if input_proc_name != '':
        ret = run_cmd_get_output(['pidof', '-x', input_proc_name])
        if len(ret) > 1:
            ps_list[ret[0]] = input_proc_name
            found = True

    if found == False:
        #Get all running processes from the system
        ret = run_cmd_get_output(['ps', '-ef'])
        for proc in ret:
            pr = proc.split()
            if len(pr) > 1:
                pid = pr[1]
                if pr[7] == "/usr/bin/python":
                    proc_name = pr[8].split('/')[-1]
                else:
                    proc_name = pr[7].split('/')[-1]
            if input_proc_name == '' or input_proc_name == proc_name:
                ps_list[pid] = proc_name

    if len(ps_list) == 0:
       print "Cannot find pid for process %s" %input_proc_name
       return

    #Get CPS components and their objects
    #cps_obj_info = {process name: [CPS objects]}
    reg_strs_to_find = ["Added", "registration", "for"]
    cps_obj_info = {}

    with open('/var/log/syslog',  'r') as inF:
        inF.seek(line_offset[-1], 0)
        for line in inF:
            if all(x in line for x in reg_strs_to_find):
                e = line.split()
                pid = e[-1].split('-')[-2]  # -2 gives the pid registered with CPS
                obj = line.split("Added registration for")[1].split()[0]  # Get the object key
                if str(pid) in ps_list.keys():
                    if ps_list[pid] in cps_obj_info.keys():
                        cps_obj_info[ps_list[pid]].append(obj)
                    else:
                        cps_obj_info[ps_list[pid]] = [obj]



    #Check if a specific component is registered to CPS and if registered print the CPS objects it owns
    comp_qualifier = {'1': 'target', '2': 'observed', '3': 'proposed', '4': 'realtime', '5': 'registration'}
    cps_process = cps_obj_info.keys()
    if input_proc_name != '':
        for elem in cps_process:
            print "Registered to CPS\nCPS Objects"
            ob = cps_obj_info[elem]
            for obj_key in ob:
                key_list = obj_key.split('.')
                k = key_list[-2]  # key_list[-2] is the innermost key
                if k in cps.info("", False)['ids']:
                    obj = cps.info("", False)['ids'][k]

                    # key_list[0]: The first field of the CPS key is the Component Qualifier
                    print "%15s" %comp_qualifier[key_list[0]], obj
            return

        print "Not registered to CPS"
    else:
        #Print all Process names that have registered with CPS
        for proc in cps_process:
            print proc




if __name__ == "__main__":

    # Usage: python cps_obj_ownership.py [process name]
    if len(sys.argv) >= 2:
        proc_name = sys.argv[1]
    else:
        proc_name = ''

    get_cps_components(proc_name)
    sys.exit(0)




