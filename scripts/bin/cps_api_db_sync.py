#!/usr/bin/python
import time
import cps
import cps_object
import cps_unit_test_data

def sync_err_cb(methods, params, err):
    print "Methods: ", methods
    print "Sync Params: ", params
    print "Error: ", err
    return True

def sync_cb(methods, params, res):
    print "Methods: ", methods
    print "Sync Params: ", params
    print "Response: ", res
    #res['change'] = "no_change"
    #res['change_notify'] = "raise_no_event"
    return True


cps.node_set_update("TestGroup", "nodal", [("NODE1", "127.0.0.1:6379"), ("NODE2", "10.11.63.118:6379")])
time.sleep(30)

cps_unit_test_data.cps_gen_ut_data(2)
src_obj = cps_object.CPSObject("base-ip/ipv6")
src_obj.add_attr("cps/object-group/node", "NODE2")

dest_obj = cps_object.CPSObject("base-ip/ipv6")
dest_obj.add_attr("cps/object-group/node", "NODE1")

callback_dict = {'sync': sync_cb, 'error': sync_err_cb}

print cps.sync(dest_obj.get(), src_obj.get(), callback_dict)

