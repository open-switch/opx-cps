# opx-cps
This repository contains the CPS object library files. The OPX CPS provides a micro-service, data-centric API allowing applications to communicate with each other between threads, processes or diverse locations.

The data model of OPX CPS is described through YANG or other constructs:
- Applications can use CPS objects with Python, C, C++, and REST services in the opx-cps-REST service. 
- Applications/threads will register for ownership of CPS objects while other applications/threads will operate and receive events of the registered CPS objects. 
- Applications can also publish objects through the event service.

A high-level list of CPS features include:
- Distributed framework for application interaction
- Database-like API (Get, Commit[add,delete,create,modify])
- Publish/subscribe semantics supported

Lookup and binary to text translation and object introspection is available.

Applications define objects through (optionally YANG-based) object models. These object models are converted into binary (C accessible) object keys, and object attributes that can be used in conjunction with the C-based CPS APIs. There are adaptions on top of CPS that allows these objects and APIs to be converted to different languages (example Python).

With object keys and attributes, applications can:
- Get single or multiple objects
- Perform transactions:
   - Create
   - Delete
   - Set
   - Action
- Register and publish object messages

## API documentation
The CPS API is documented through doxygen. To generate the CPS doxygen content, at the top-level of your source directory (one level underneath `opx-cps`), run the `opx-cps/doc/cps_gen_doc.sh` command.  
    
    git clone git@github.com:open-switch/opx-cps.git
    cd opx-cps
    (cd .. ; sh -x opx-cps/doc/cps_gen_doc.sh )
    firefox workspace/cps-api-doc/c-cpp-doc/html/index.html

## Packages
`libopx-cps1\_*version*\_*arch*.deb` — Utility libraries

`libopx-cps-dev\_*version*\_*arch*.deb` — Exported header files

`python-opx-cps\_*version*\_*arch*.deb` — Python bindings

`opx-cps\_*version*\_*arch*.deb` — Service executables, configuration files, tool scripts 

`opx-yang-utils-dev\_*version*\_*arch*.deb` — Tools to parse YANG files

See [Architecture](https://github.com/open-switch/opx-docs/wiki/Architecture) for more information on the CPS module.

## Debugging tools

#### `cps\_model\_info` 
This tool is useful to get all information about CPS objects on the target. It is used to get the attributes of a specific CPS object, or first-level contents of a given YANG path of the CPS object (as defined in the YANG model).

**Usage**

`cps\_model\_info` [CPS object path as defined in the YANG model]

**Example**

```
root@OPX:~# cps_model_info

base-packet
base-pas
base-acl
if
base-qos
base-if-phy
```

```
root@OPX:~# cps_model_info base-if-phy

base-if-phy/front-panel-port
        Attribute Type =  list
        Description =  This map contains the front panel ports and theNPU ports associated with with the front panel ports.

Registered to CPS with qualifier:  target

Process Owner:  base_nas_front_panel_ports.py

base-if-phy/hardware-port
        Attribute Type =  list
        Description =  This entity holds the details of which front panel port corresponds to a specific NPU/hardware port.  Thislist is not dynamic and therefore will not change for a single instance of a product.

Registered to CPS with qualifier:  target

Process Owner:  base_nas_front_panel_ports.py
```

```
root@OPX:~# cps_model_info base-if-phy/physical

base-if-phy/physical/phy-mode
        Attribute Type =  leaf
        Data Type =  enum
        Description =  Port PHY mode, Ethernet or FC

Registered to CPS with qualifier:  target

Process Owner:  base_nas


base-if-phy/physical/hardware-port-id
        Attribute Type =  leaf
        Data Type =  uint32_t
        Description =  This is the physical hardware port

Registered to CPS with qualifier:  target

Process Owner:  base_nas
```

#### `cps\_get\_oid.py`

This tool is used to get data from a CPS object service provider.

**Usage**

`cps\_get\_oid.py` *qualifier* *CPS object path as defined in the YANG model*

CPS object path can be determined from `cps\_model\_info` tool.

Qualifier: target/observed/proposed

**Example**

```
root@OPX:~# cps_get_oid.py target base-if-phy/physical hardware-port-id=125

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
```

#### `cps\_set\_oid.py`

This tool is used to do transactions on a given CPS Object.

**Usage**

`cps\_set\_oid.py` *qualifier* *operation* *CPS object path as defined in the YANG model* *CPS Object attr=value*

CPS Object Path and its attributes can be determined from `cps\_model\_info` tool.

Qualifier: target/observed/proposed

Operation: create/set/delete

**Example**

```
root@OPX:/opt/dell/os10/bin# cps_set_oid.py target create base-if-phy/physical hardware-port-id=26 admin-state=2
```

#### `cps\_trace\_events.py`

This tool is used to subscribe/listen for CPS events on the target.

**Usage**

`cps\_trace\_events.py` *qualifier* *CPS object path as defined in the YANG model*

CPS object path can be determined from `cps\_model\_info` tool.

**Example**

```
root@OPX:~# cps_trace_events.py observed dell-base-if-cmn/if/interfaces-state/interface
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
```                                                     

#### `cps\_send\_event.py`

This tool is used to publish/send CPS events on the target.

**Usage**

`cps\_send\_event.py` *operation* *qualifier* *CPS object path as defined in the YANG model* *CPS Object attr=value*

CPS object path and its attributes can be determined from `cps\_model\_info` tool.

Qualifier: target/observed/proposed

Operation: create/set/delete

**Example**

```
root@OPX:/opt/dell/os10/bin# cps_send_event.py create observed  dell-base-if-cmn/if/interfaces-state/interface  if/interfaces-state/interface/name=e101-007-0 if/interfaces-state/interface/oper-status=2
```

(c) 2017 Dell
