# opx-cps
This repository contains the CPS object library files. The OPX CPS provides a micro-service data-centric API allowing applications to communicate with each other between threads, processes, or diverse locations.

The data model of OPX CPS is described through Yang or other constructs.
Applications can use CPS objects with Python, C, C++, and REST services in the opx-cps-REST service.
Applications/threads will register for ownership of CPS objects while other applications/threads will operate and receive events of the registered CPS objects. Applications can also publish objects through the event service.

A high-level list of CPS features include:
- Distributed framework for application interaction
- Database-like API (Get, Commit[add,delete,create,modify])
- Publish/subscribe semantics supported

Lookup and binary to text translation and object introspection is available.

Applications define objects through (optionally Yang-based) object models. These object models are converted into binary (C accessible) object keys and object attributes that can be used in conjunction with the C-based CPS APIs. There are adaptions on top of CPS that allows these objects and APIs to be converted to different languages like Python.

With the object keys and attributes applications can:
- Get a single or get multiple objects.
- Perform transactions consisting of:
   - Create
   - Delete
   - Set
   - Action
- Register and publish object messages.

##Packages
libopx-cps1\_*version*\_*arch*.deb — Utility libraries

libopx-cps-dev\_*version*\_*arch*.deb — Exported header files

python-opx-cps\_*version*\_*arch*.deb — Python bindings

opx-cps\_*version*\_*arch*.deb — Service executables, configuration files, tool scripts 

opx-yang-utils-dev\_*version*\_*arch*.deb — Tools to parse yang files

See [Architecture](https://github.com/open-switch/opx-docs/wiki/Architecture) for more information on the CPS module.


##Debugging Tools:

This section covers the following tools.

1) cps\_model\_info
2) cps\_get\_oid
3) cps\_set\_oid
4) cps\_trace\_events
5) cps\_send\_events


###cps\_model\_info: 
This tool is useful to get all information about CPS Objects on the target.
It's used to get the attributes of a specific CPS object or first-level contents of a given YANG path of the CPS Object (as defined in the Yang model).


####Usage:

cps\_model\_info [CPS Object Path as defined in the Yang model]

####Examples:

root@OPX:~# cps\_model\_info
base-packet
base-pas
base-switch
base-acl
if
base-mirror
base-qos
base-if-phy
<snip>


root@OPX:~# cps\_model\_info base-if-phy

base-if-phy/front-panel-port
        Attribute Type =  list
        Description =  This map contains the front panel ports and theNPU ports associated with with the front panel ports.

Registered to CPS with qualifier:  target

Process Owner:  base\_nas\_front\_panel\_ports.py

base-if-phy/physical
        Attribute Type =  list
        Description =

Registered to CPS with qualifier:  target

Process Owner:  base\_nas




root@OPX:~# cps\_model\_info base-if-phy/physical

base-if-phy/physical/phy-mode
        Attribute Type =  leaf
        Data Type =  enum
        Description =  Port PHY mode, Ethernet or FC

Registered to CPS with qualifier:  target

Process Owner:  base\_nas


base-if-phy/physical/hardware-port-id
        Attribute Type =  leaf
        Data Type =  uint32_t
        Description =  This is the physical hardware port

Registered to CPS with qualifier:  target

Process Owner:  base\_nas
<snip>






###cps\_get\_oid.py
This tool is used to get data from a CPS Object Service provider.

####Usage:
cps\_get\_oid.py <qualifier> <CPS Object Path as defined in the Yang model>

CPS Object Path can be determined from cps\_model\_info tool.

####Examples:

root@OPX:~# cps\_get\_oid.py target base-if-phy/physical hardware-port-id=125

Key: 1.17.1114163.1114115.1114116.
base-if-phy/physical/breakout-capabilities = 4,2,4
base-if-phy/physical/fanout-mode = 4
base-if-phy/physical/npu-id = 0
base-if-phy/physical/hardware-port-list = 125,126,127,128
base-if-phy/physical/hardware-port-id = 125
base-if-phy/physical/speed = 6
base-if-phy/physical/supported-speed = 3,4,6
base-if-phy/physical/port-id = 125
base-if-phy/physical/phy-media = 1
base-if-phy/physical/front-panel-number = 25
base-if-phy/physical/loopback = 0
root@OPX:~#                             




###cps\_set\_oid.py
This tool is used to do transactions on a given CPS Object.

####Usage:

cps\_set\_oid.py <qualifier> <operation> <CPS Object Path as defined in the Yang model> <CPS Object attr=value>

CPS Object Path and its attributes can be determined from cps\_model\_info tool.

Qualifier: target/observed/proposed
Operation: create/set/delete

####Examples:

root@OPX:/opt/dell/os10/bin# cps\_set\_oid.py target create base-if-phy/physical hardware-port-id=26 admin-state=2





###cps\_trace\_events.py
This tool is used to subscribe/listen for CPS events on the target.

####Usage:

cps\_trace\_events.py <qualifier> <CPS Object Path as defined in the Yang model>

CPS Object Path can be determined from cps\_model\_info tool.

####Examples:

root@OPX:~# cps\_trace\_events.py observed dell-base-if-cmn/if/interfaces-state/interface
Key : 2.19.44.2883618.2883611.2883586.
 Registering for observed dell-base-if-cmn/if/interfaces-state/interface
1.2.131094.131075.
Key: 1.2.131094.131075.
cps/connection-entry/ip = 127.0.0.1:6379
cps/connection-entry/connection-state = 1
cps/connection-entry/group = 127.0.0.1:6379



2.19.44.2883618.2883611.2883586.
Key: 2.19.44.2883618.2883611.2883586.
if/interfaces-state/interface/name = e101-025-0
if/interfaces-state/interface/if-index = 41
if/interfaces-state/interface/admin-status = 2
                                                        




###cps\_send\_event.py

This tool is used to publish/send CPS events on the target.

####Usage:

cps\_send\_event.py <operation> <qualifier> <CPS Object Path as defined in the Yang model> <CPS Object attr=value>

CPS Object Path and its attributes can be determined from cps\_model\_info tool.

Qualifier: target/observed/proposed
Operation: create/set/delete


####Examples:

root@OPX:/opt/dell/os10/bin# cps\_send\_event.py create observed  dell-base-if-cmn/if/interfaces-state/interface  if/interfaces-state/interface/name=e101-007-0 if/interfaces-state/interface/oper-status=2


(c) 2017 Dell
