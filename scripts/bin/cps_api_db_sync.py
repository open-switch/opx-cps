#!/usr/bin/python
import time
import cps
import cps_object

def sync_err_cb(params, err):
    print "Sync Params: ", params
    print "Error: ", err

def sync_cb(params, res):
    print "Sync Params: ", params
    print "Response: ", res


cps.node_set_update("TestGroup", "nodal", [("NODE1", "127.0.0.1:6379"), ("NODE2", "10.11.63.118:6379")])
time.sleep(30)

cps.init_ut()
src_obj = cps_object.CPSObject("base-ip/ipv6")
src_obj.add_attr("cps/object-group/node", "NODE2")

dest_obj = cps_object.CPSObject("base-ip/ipv6")
dest_obj.add_attr("cps/object-group/node", "NODE1")

callback_dict = {'sync': sync_cb, 'error': sync_err_cb}

print cps.sync(dest_obj.get(), src_obj.get(), callback_dict)

