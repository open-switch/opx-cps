#!/usr/bin/python
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

