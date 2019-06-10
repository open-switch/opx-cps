#!/usr/bin/python
#
# Copyright (c) 2019 Dell Inc.
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

import cps_connectivity_agent
import cps_object
import cps

def _print_connection_objs(qualifier, mod):
    obj = cps_object.CPSObject(qual=qualifier, module=mod)
    res = []
    cps.db_get(obj.get(), res)
    for r in res:
        print r


def test_cps_cluster_connections():
    cps_connectivity_agent._sync()

    # Get all cps/connectivity-group objects
    _print_connection_objs("observed", "cps/connectivity-group")

    # Get all cps/connection-object
    _print_connection_objs("observed", "cps/connection-object")

