import subprocess
import cps
import event_log as ev
import sys
import socket
import time
import cps_object
import bytearray_utils as ba

default_ip = "0.0.0.0"


def get_free_port():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(('', 0))
    addr, port = sock.getsockname()
    sock.close()
    return port

def log_msg(level,msg):
    ev.logging("DSAPI",level,"DB-INSTANCE-MANAGER","","",0,msg)

class CPSDbProcessManager():

    def __init__(self,port,ip=default_ip):
        """
         Constructor to create a CPS DB instance manager object.
        @port - port number for db instance
        @ip - ip address of the new db instance in string, its optional otherwise default ip will be used

        """

        try:
            self.p = subprocess.Popen(['/usr/bin/redis-server', '--port',str(port),'--bind',ip])
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

db_group_mapping = {}


def handle_create(obj,group):
    port = get_free_port()
    p = CPSDbProcessManager(port)
    if(p.is_valid()) and group not in db_group_mapping:
        db_group_mapping[group]=p
        return port

    return False

def handle_delete(obj,group):
    if group in db_group_mapping:
        p = db_group_mapping[group]
        if p.close():
            del db_group_mapping[group]
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
        port = handle_create(obj,group_name)
        if port:
            params['change']['data']['cps/db-instance/port'] = ba.str_to_ba(str(port),len(str(port)))
            return True
        else:
            return False

    if params['operation'] == 'delete':
        return handle_delete(obj,group_name)

    return False

def get_db_cb(methods, params):
    return False

if __name__ == '__main__':

    handle = cps.obj_init()

    d = {}
    d['get'] = get_db_cb
    d['transaction'] = set_db_cb

    # Should create a new objet in cps model or use something internal
    cps.obj_register(handle, cps.key_from_name("target","cps/db-instance"), d)


    while True:
        time.sleep(1)
