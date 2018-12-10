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
#!/usr/bin/python
import sys
import time
import cps
import cps_object
import cps_unit_test_data

def sync_err_cb(methods, params, err):
    print "Reconcile Error Params: ", params
    print "Error: ", err
    return True

def sync_cb(methods, params, res):
    print "Reconcile Params: ", params
    print "Response: ", res
    #res['change'] = "no_change"
    #res['change_notify'] = "raise_no_event"
    return True


def reconcile():
    l = cps_unit_test_data.cps_gen_ut_data(1)
    dest_obj = cps_object.CPSObject("base-ip/ipv6")

    callback_dict = {'sync': sync_cb, 'error': sync_err_cb}

    print cps.reconcile(l, dest_obj.get(), callback_dict)

if __name__ == "__main__":
  reconcile()
  sys.exit(0)
