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
import systemd.daemon
import signal
import threading
import cps
import cps_object
import event_log as ev
import datetime
import cps_connectivity_utils as conn_utils

__running = True
signum_caught = -1

default_redis_addrv4 = '127.0.0.1:6379'
tunnel_group_mapping = {}
log_filename = sys._getframe().f_code.co_filename

def log_msg(level,msg,func_name, line_num):
    ev.logging("CPS_CONNECTIVITY",level,"CONNECTIVITY_AGENT",log_filename,func_name,line_num,msg)

def _fill_connectivity_group_obj(group_info):
    cur_obj = cps_object.CPSObject(qual="observed", module="cps/connectivity-group")

    for e in group_info:
        if not isinstance(group_info[e], list):
            cur_obj.add_attr(e, group_info[e])
        else:
            for idx,elem in enumerate(group_info[e]):
                for k in elem:
                    cur_obj.add_embed_attr(['node', str(idx), k], elem[k], 3)
    return cur_obj.get()

def _fill_connection_obj(node_info):
    cur_obj = cps_object.CPSObject(qual="observed", module="cps/connection-object")

    if 'ip' in node_info:
        cur_obj.add_attr('addr', node_info['ip'])
    if 'name' in node_info:
        cur_obj.add_attr('name', node_info['name'])
    if 'tunnel' in node_info:
        cur_obj.add_attr('tunnel', node_info['tunnel'])
        
    cur_obj.add_attr('timestamp', '{:%Y-%b-%d %H:%M:%S:%f}'.format(datetime.datetime.now()))
    return cur_obj.get()


def commit_obj(op, data):
    log_funcname = sys._getframe().f_code.co_name

    ch = {'change': {}, 'prev' : {}}
    ch['change'] = data
    ch['change']['operation'] = op

    if cps.db_commit(ch['change'], ch['prev'], True):
        log_msg(3," Transaction Failure with operation %s for data %s" %(str(op), str(data)), log_funcname, sys._getframe().f_lineno)
    else:
        log_msg(6," Transaction Success with operation %s for data %s " %(str(op), str(data)), log_funcname, sys._getframe().f_lineno )
        return True
    return False

def cps_convert_attr_data( raw_elem ):
    d={}
    obj = cps_object.CPSObject(obj=raw_elem)
    for attr in raw_elem['data']:
        d[attr] = obj.get_attr_data(attr)

    return d

def _node_connect(node):
    log_funcname = sys._getframe().f_code.co_name

    if node['ip'] == default_redis_addrv4:
        log_msg(6," No Tunnel creations for localhost", log_funcname, sys._getframe().f_lineno )
        return True
    instance_key = node['name']+"_"+node['ip']
    p = conn_utils.CPSTunnelProcessManager(node['name'],node['ip'])

    if p.is_valid():
        tunnel_group_mapping[instance_key]=p
        tunnel_port = p.get_port()

        node['tunnel'] = tunnel_port
        data = _fill_connection_obj(node)
        ret = commit_obj('create', data )
        if ret:
            log_msg(6," Tunnel Created successfully for node %s : %s " %(str(node['name']), str(node['ip'])), log_funcname, sys._getframe().f_lineno )
            return tunnel_port
        else:
            # Commit cps/connection-object failed, hence undo/delete the tunnels
            if p.close():
                del tunnel_group_mapping[instance_key]
                del p
                log_msg(3," Connection object commit failed and hence Tunnels deleted for node %s : %s " %(str(node['name']), str(node['ip'])), log_funcname, sys._getframe().f_lineno )


    return False

def _node_disconnect(node):
    log_funcname = sys._getframe().f_code.co_name

    if node['ip'] == default_redis_addrv4:
        log_msg(6," No Tunnel deletions for localhost since it doesn't exist", log_funcname, sys._getframe().f_lineno )
        return True

    instance_key = node['name']+"_"+node['ip']
    if instance_key in tunnel_group_mapping:
        p = tunnel_group_mapping[instance_key]
        if p.close():
            del tunnel_group_mapping[instance_key]
            del p
            log_msg(6," Tunnel deleted for node %s : %s" %(node['name'], node['ip']), log_funcname, sys._getframe().f_lineno )

            # Delete cps/connection-object
            data = _fill_connection_obj(node)
            return commit_obj('delete', data )
    return False
    
    
def _get_connection_objs():
    # Get all cps/connection-object
    obj = cps_object.CPSObject(qual="observed", module="cps/connection-object")
    res = []
    cps.db_get(obj.get(), res)

    conn_objs = {}
    for r in res:
        conn = {}
        conn = cps_convert_attr_data(r)
        conn_objs.update( { conn['cps/connection-object/name']: {"ip": conn['cps/connection-object/addr'], "tunnel": conn['cps/connection-object/tunnel'], "timestamp": conn['cps/connection-object/timestamp']}} )
    
    return conn_objs

def _get_node_group_objs():
    # Get all cps/node-group objects
    obj = cps_object.CPSObject(qual="target", module="cps/node-group")
    node_grp_objs = []
    cps.db_get(obj.get(), node_grp_objs)

    node_group_objs = []
    for r in node_grp_objs:
        grp = {}
        grp = cps_convert_attr_data(r)
        node_group_objs.append(grp)

    return node_group_objs

def _sync():

    log_funcname = sys._getframe().f_code.co_name

    # Delete all cps/connectivity-group objects
    group_info = {}
    data = _fill_connectivity_group_obj(group_info)
    ret = commit_obj("delete", data )
    if ret == False:
        return ret
    log_msg(6," Deleted all Tunnel cps/connectivity-group objects ", log_funcname, sys._getframe().f_lineno )

    # Get all cps/node-group objects
    node_grp_objs = _get_node_group_objs()

    # Get all cps/connection-object and mark them as not seen
    conn_objs = _get_connection_objs()
    for conn in conn_objs:
        conn_objs[conn]['seen'] = "no"

    for grp in node_grp_objs:
        
        # Build group_info dict to fill cps/connectivity-group object
        group_info = {}
        l = {}
        group_info = {'name': grp['cps/node-group/name'], 'node': []}
        if 'cps/node-group/type' in grp:
            group_info['type'] = grp['cps/node-group/type']

        if 'cps/node-group/node' in grp:
            for node_index in grp['cps/node-group/node']:
                
                node = grp['cps/node-group/node'][node_index]
                l = {'name': node['name'], 'ip': node['ip'], 'timestamp':  '{:%Y-%b-%d %H:%M:%S:%f}'.format(datetime.datetime.now())}

                if node['ip'] == default_redis_addrv4:
                    l.update( {'tunnel-ip': node['ip']} )
                elif node['name'] not in conn_objs:
                     # Create tunnels
                    node_info = {'name': node['name'], 'ip': node['ip']}
                    tunn_port = _node_connect(node_info)
                    if tunn_port == False:
                         log_msg(6," Tunnel Node connect returned false for node %s : %s" %(node['name'], node['ip']), log_funcname, sys._getframe().f_lineno )
                         continue
                    conn_objs[node['name']] = {"ip": node['ip'], "tunnel": tunn_port, "timestamp":  '{:%Y-%b-%d %H:%M:%S:%f}'.format(datetime.datetime.now()), "seen": "yes"}
                    
                    l.update( {'tunnel-ip': "127.0.0.1:"+tunn_port} )
                else:
                    conn_objs[node['name']]["seen"] = "yes"
                    l.update( {'tunnel-ip': "127.0.0.1:"+conn_objs[node['name']]['tunnel'], 'timestamp':  conn_objs[node['name']]['timestamp']} )
                
                group_info['node'].append(l)
        
        # Create cps/connectivity-group object in DB
        group_info['timestamp']  = '{:%Y-%b-%d %H:%M:%S:%f}'.format(datetime.datetime.now())

        data = _fill_connectivity_group_obj(group_info)
        commit_obj("create", data )

    # Delete tunnels that is not part of any group
    log_msg(6," Delete Tunnels that is not part of any group" , log_funcname, sys._getframe().f_lineno )
    for conn in conn_objs: 
        if conn_objs[conn]["seen"] == "no":            
            node = {'name': conn, 'ip': conn_objs[conn]['ip']}
            _node_disconnect(node)
    return True
    

def process_node_grp_events(event_data):
    if 'cps/node-group/name' in event_data['data']:
        log_msg(6," Node group event : %s" %(str(event_data['data'])) , sys._getframe().f_code.co_name, sys._getframe().f_lineno )

        _sync()
    return True
 
def _ev_wait_node_grp(handle):
    while True:
        event_data = {}
        event_data = cps.event_wait(handle)
        if 'operation' in event_data:
            process_node_grp_events(event_data)

def _register_ev(qual, obj):
    # Listen for cps/node-group events    
    _key = cps.key_from_name(qual, obj)
    handle = cps.event_connect()
    
    cps.event_register(handle, _key)
    
    t = threading.Thread( target=_ev_wait_node_grp, args=(handle,) ).start()

def delete_conn_obj(node_name):
    node_info = {'name': node_name}
    data = _fill_connection_obj(node_info)
    commit_obj("delete", data )

def _validate_conn_objs():
     # Get all cps/connection-object
    conn_objs = _get_connection_objs()

    for conn in conn_objs:
        tunnel_pid_path = "/tmp/stunnel_"+conn+"_"+(conn_objs[conn]['ip']).split(":6379", -1)[0]+".pid"
        if os.path.isfile(tunnel_pid_path):
            with open(tunnel_pid_path, 'r') as fd:
                pid = fd.read()
            proc_pid_path = "/proc/"+pid
            proc_pid_path = proc_pid_path.rstrip()
            if not os.path.isdir(proc_pid_path):
                # Delete the corresponding cps/connection-object
                delete_conn_obj(conn)
        else:
            # Delete the corresponding cps/connection-object
            delete_conn_obj(conn)

def sig_handler(signum, frame):
    global signum_caught
    global __running

    signum_caught = signum
    if(signum_caught == signum):
        __running = False
        os._exit(os.EX_OK)

if __name__ == '__main__':

    signal.signal(signal.SIGTERM, sig_handler)

    # Sync groups with connectivity objects
    _sync()

    # Audit/Validate cps/connection-object every 30 secs
    threading.Timer(30, _validate_conn_objs).start()

    # Register for cps/node-group events
    _register_ev("target", "cps/node-group")
    
    systemd.daemon.notify("READY=1")

    while(__running):
        signal.pause()

    sys.stdout.write("Signal %d received - Shutting down daemon" % (signum_caught,))
    sys.exit(0)
