import subprocess
import cps
import event_log as ev
import sys
import socket
import time
import cps_object
import os
import bytearray_utils as ba

default_cert = "/etc/stunnel/redis-stunnel.pem"
default_server_port = "41000"
config_file_map = {}
tunnel_config_map = {}
tunnel_group_mapping = {}
group_node_ip_map = {}

def get_free_port():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(('', 0))
    addr, port = sock.getsockname()
    sock.close()
    return port

def get_ip_from_string(ip):
    count = 0
    index = 0
    for i in ip:
        if i == ":":
            index = count
        count +=1

    if index:
        return ip[0:index]
    else:
        return ip

def log_msg(level,msg):
    ev.logging("DSAPI",level,"TUNNEL-INSTANCE-MANAGER","","",0,msg)


class TunnelConfigManager():
    def __init__(self,port,group,node,ip,client = True):
        self.group = group
        self.node = node
        self.fname = "/etc/stunnel/"+group +"_"+ node + ".conf"
        if os.path.exists(self.fname):
            log_msg(4,"File "+self.fname+" Already exists")
            self.valid = False
            return
        try:
            f = open(self.fname,'w')
            f.write("[redis - "+group+" - "+node+" ]\n")
            f.write("client = yes \n")
            f.write("accept = :::"+port+" \n")
            f.write("connect = "+get_ip_from_string(ip)+":"+default_server_port+" \n")
            f.write("cert = "+default_cert+"\n")
        except Exception as e:
            if os.path.exists(self.fname):
                os.remove(self.fname)
            log_msg(4,str(e))
            log_msg(4,"Failed to create new config file for group "+group+" and node "+node)
            self.valid = False
            return

        f.close()
        self.valid = True
        config_file_map[group+"_"+node] = self.fname

    def is_valid(self):
        return self.valid

    def get_config_file(self):
        return config_file_map[self.group+"_"+self.node]

    def close(self):
        try:
            os.remove(self.fname)
        except Exception as e:
            log_msg(4,str(e))
            return False

        log_msg(6,"Deleted file for group"+self.group+" and node" + self.node)
        del config_file_map[self.group+"_"+self.node]
        return True


class CPSTunnelProcessManager():

    def __init__(self,group,node,ip):
        """
         Constructor to create a CPS Tunnel instance manager object.
        @group - group name
        @node - node name
        @ip - ip address of the node

        """
        self.port = str(get_free_port())
        self.obj = TunnelConfigManager(self.port,group,node,ip)
        if not self.obj.is_valid():
            self.valid = False
            return

        try:
            self.p = subprocess.Popen('/usr/bin/stunnel4 '+ self.obj.get_config_file(),shell=True)
        except Exception as e:
            log_msg(4,str(e))
            log_msg(4,"Failed to create new Stunnel Instance for group "+group+" and node "+node)
            self.valid = False
            self.obj.close()
            del self.obj
            return

        self.process_id = self.p.pid
        self.valid = True
        log_msg(6,"Created new stunnel instance with process id "+str(self.p.pid))

    def is_valid(self):
        return self.valid

    def get_port(self):
        return self.port

    def close(self):

        try:
            self.p.terminate()
            self.p.wait()
            subprocess.call(["pkill","-f",self.obj.get_config_file()])
            self.obj.close()

        except Exception as e:
            log_msg(4,str(e))
            return False

        log_msg(6,"Deleted stunnel instance with process id "+str(self.process_id))
        return True



def handle_create(group,node,ip):
    instance_key = group+"_"+node
    if instance_key in group_node_ip_map:
        if group_node_ip_map[instance_key] != ip.split(":6379"):
            handle_delete(group, node)

    if instance_key in tunnel_group_mapping:
        log_msg(6, "Already a tunnel instance for group "+group+" and node "+node+" is running")
        return tunnel_group_mapping[instance_key].get_port()

    p = CPSTunnelProcessManager(group,node,ip)
    if p.is_valid():
        tunnel_group_mapping[instance_key]=p
        group_node_ip_map[instance_key] = ip.split(":6379")
        return p.get_port()

    return False

def handle_delete(group,node):
    instance_key = group+"_"+node
    if instance_key in tunnel_group_mapping:
        p = tunnel_group_mapping[instance_key]
        if p.close():
            del tunnel_group_mapping[instance_key]
            del p
            return True
    return False

def set_tunnel_cb(methods, params):
    obj = cps_object.CPSObject(obj=params['change'])
    try:
        group_name = obj.get_attr_data('group')
        node = obj.get_attr_data('node-id')
    except Exception as e:
        log_msg(4,str(e))
        return False
    if params['operation'] == 'create':
        try:
            ip = obj.get_attr_data('ip')
        except Exception as e:
            log_msg(4,str(e))
            return False
        port = handle_create(group_name,node,ip)
        if port:
            params['change']['data']['cps/tunnel/port'] = ba.str_to_ba(port,len(port))
            return True
        else:
            return False

    if params['operation'] == 'delete':
        return handle_delete(group_name,node)

    return False

def get_tunnel_cb(methods, params):
    return False

if __name__ == '__main__':

    handle = cps.obj_init()

    d = {}
    d['get'] = get_tunnel_cb
    d['transaction'] = set_tunnel_cb

    # Should create a new objet in cps model or use something internal
    cps.obj_register(handle, cps.key_from_name("target","cps/tunnel"), d)


    while True:
        time.sleep(1)
