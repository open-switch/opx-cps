#
# Copyright (c) 2018 Dell Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
#
# THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
# CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
# LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
# FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
#
# See the Apache Version 2.0 License for specific language governing
# permissions and limitations under the License.
#
import subprocess
import cps
import event_log as ev
import sys
import cps_object
import socket
import bytearray_utils as ba

db_group_mapping = {}
group_port_mapping = {}
default_ip = "0.0.0.0"
redis_server_path = "/usr/bin/redis-server"

def get_free_port():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(('', 0))
    addr, port = sock.getsockname()
    sock.close()
    return port

def log_msg(level,msg):
    ev.logging("DSAPI",level,"DB-TUNNEL-MANAGER","","",0,msg)

class CPSDbProcessManager():

    def __init__(self,port,ip=default_ip):
        """
         Constructor to create a CPS DB instance manager object.
        @port - port number for db instance
        @ip - ip address of the new db instance in string, its optional otherwise default ip will be used

        """

        try:
            self.p = subprocess.Popen([redis_server_path, '--port',str(port),'--bind',ip])
        except Exception as e:
            log_msg(4,str(e))
            log_msg(4,"Failed to create new DB Instance for port "+str(port)+" and ip "+str(ip))
            self.valid = False
            return

        self.process_id = self.p.pid
        self.valid = True
        log_msg(6,"Created new db instance with process id "+str(self.p.pid))

    def is_valid(self):
        return self.valid

    def close(self):

        try:
            self.p.terminate()
            self.p.wait()
        except Exception as e:
            log_msg(4,str(e))
            return False

        log_msg(6,"Deleted db instance with process id "+str(self.process_id))
        return True


def handle_db_create(obj,group):
    if group in db_group_mapping:
        return group_port_mapping[group]

    port = get_free_port()
    p = CPSDbProcessManager(port)
    if p.is_valid():
        db_group_mapping[group]=p
        group_port_mapping[group]=port
        return port

    return False

def handle_db_delete(obj,group):
    if group in db_group_mapping:
        p = db_group_mapping[group]
        if p.close():
            del db_group_mapping[group]
            del group_port_mapping[group]
            return True

    return False

def set_db_cb(methods, params):
    obj = cps_object.CPSObject(obj=params['change'])
    try:
        group_name = obj.get_attr_data('group')
    except Exception as e:
        log_msg(4,str(e))
        return False
    if params['operation'] == 'create':
        port = handle_db_create(obj,group_name)
        if port:
            params['change']['data']['cps/db-instance/port'] = ba.str_to_ba(str(port),len(str(port)))
            return True
        else:
            return False

    if params['operation'] == 'delete':
        return handle_db_delete(obj,group_name)

    return False

def get_db_cb(methods, params):
    return False

signum_caught = -1
def sig_handler(signum, frame):
    global signum_caught
    signum_caught = signum

if __name__ == '__main__':

    import signal
    signal.signal(signal.SIGTERM, sig_handler)

    handle = cps.obj_init()

    db_cb = {}
    db_cb['get'] = get_db_cb
    db_cb['transaction'] = set_db_cb

    cps.obj_register(handle, cps.key_from_name("target","cps/db-instance"), db_cb)

    import systemd.daemon
    systemd.daemon.notify("READY=1")

    signal.pause()

    sys.stdout.write("Signal %d received - Shutting down daemon" % (signum_caught,))
    sys.exit(0)

