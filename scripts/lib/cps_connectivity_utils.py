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

import os
import sys
import socket
import subprocess
import event_log as ev

default_cert = "/etc/stunnel/redis-stunnel.pem"
default_server_port = "41000"
stunnel_config_path = "/tmp/"
stunnel_path = '/usr/bin/stunnel4 '
config_file_map = {}
default_timeout_connect = "2"
default_timeout_busy = "4"
default_timeout_idle = "-1"
default_retry = "yes"
default_keepalive_enable = "1"
default_keepalive_count = "4"
default_keepalive_interval = "1"
default_keepalive_idle = "2"
default_log_level = "3"
log_filename = sys._getframe().f_code.co_filename

def log_msg(level,msg,func_name, line_num):
    ev.logging("CPS_CONNECTIVITY",level,"CONNECTIVITY_UTILS",log_filename,func_name,line_num,msg)

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


class TunnelConfigManager():
    def __init__(self,port,node,ip,client = True):
        log_funcname = sys._getframe().f_code.co_name

        self.node = node
        self.ip_str = get_ip_from_string(ip)
        self.fname = stunnel_config_path + node + "_" + self.ip_str + ".conf"
        if os.path.exists(self.fname):
            log_msg(4,"Tunnel Conf File "+self.fname+" Already exists", log_funcname, sys._getframe().f_lineno)
            self.valid = False
            return
        try:
            f = open(self.fname,'w')

            # Setting the default log level to 3 (Emergency/Alert/Critical/Error messages will be logged)
            f.write("debug = "+default_log_level+"\n")

            # Global socket options
            f.write("socket = r:SO_KEEPALIVE="+default_keepalive_enable+"\n")
            f.write("socket = r:TCP_KEEPCNT="+default_keepalive_count+"\n")
            f.write("socket = r:TCP_KEEPINTVL="+default_keepalive_interval+"\n")
            f.write("socket = r:TCP_KEEPIDLE="+default_keepalive_idle+"\n")

            f.write("socket = a:SO_KEEPALIVE="+default_keepalive_enable+"\n")
            f.write("socket = a:TCP_KEEPCNT="+default_keepalive_count+"\n")
            f.write("socket = a:TCP_KEEPINTVL="+default_keepalive_interval+"\n")
            f.write("socket = a:TCP_KEEPIDLE="+default_keepalive_idle+"\n")
            f.write("pid = /tmp/stunnel_"+node+"_"+self.ip_str+".pid\n")

            # Service specific socket options
            f.write("[redis - "+node+" ]\n")
            f.write("client = yes \n")
            f.write("accept = 127.0.0.1:"+port+" \n")
            f.write("connect = "+self.ip_str+":"+default_server_port+" \n")
            f.write("cert = "+default_cert+"\n")
            f.write("TIMEOUTconnect = "+default_timeout_connect+"\n")
            f.write("TIMEOUTbusy = "+default_timeout_busy+"\n")
            f.write("TIMEOUTidle = "+default_timeout_idle+"\n")
            f.write("retry = "+default_retry+"\n")

        except Exception as e:
            if os.path.exists(self.fname):
                os.remove(self.fname)
            log_msg(4,str(e), log_funcname, sys._getframe().f_lineno)
            log_msg(4,"Failed to create new Tunnel config file for node "+node, log_funcname, sys._getframe().f_lineno)
            self.valid = False
            return

        f.close()
        self.valid = True
        config_file_map[self.node+"_"+self.ip_str] = self.fname

    def is_valid(self):
        return self.valid

    def get_config_file(self):
        return config_file_map[self.node+"_"+self.ip_str]


    def close(self):
        log_funcname = sys._getframe().f_code.co_name
        try:
            os.remove(self.fname)
        except Exception as e:
            log_msg(4,str(e), log_funcname, sys._getframe().f_lineno)
            return False

        log_msg(6,"Deleted Tunnel Conf file for Node"+self.node+" and IP" + self.ip_str, log_funcname, sys._getframe().f_lineno)
        del config_file_map[self.node+"_"+self.ip_str]
        return True


class CPSTunnelProcessManager():

    def __init__(self,node,ip):
        """
         Constructor to create a CPS Tunnel instance manager object.
        @node - node name
        @ip - ip address of the node

        """
        self.port = str(get_free_port())
        self.obj = TunnelConfigManager(self.port,node,ip)
        if not self.obj.is_valid():
            self.valid = False
            return

        log_funcname = sys._getframe().f_code.co_name
        try:
            self.p = subprocess.Popen(stunnel_path+ self.obj.get_config_file(),shell=True)
        except Exception as e:
            log_msg(4,str(e), log_funcname, sys._getframe().f_lineno)
            log_msg(4,"Failed to create new Tunnel Instance for node "+node, log_funcname, sys._getframe().f_lineno)
            self.valid = False
            self.obj.close()
            del self.obj
            return

        self.process_id = self.p.pid
        self.valid = True

        log_msg(6,"Created new Tunnel instance with process id "+str(self.p.pid), log_funcname, sys._getframe().f_lineno)

    def is_valid(self):
        return self.valid

    def get_port(self):
        return self.port


    def close(self):
        log_funcname = sys._getframe().f_code.co_name
        try:
            self.p.terminate()
            self.p.wait()
            subprocess.call(["pkill","-f",self.obj.get_config_file()])
            self.obj.close()

        except Exception as e:
            log_msg(4,str(e), log_funcname, sys._getframe().f_lineno)
            return False

        log_msg(6,"Deleted Tunnel instance with process id "+str(self.process_id), log_funcname, sys._getframe().f_lineno)
        return True

