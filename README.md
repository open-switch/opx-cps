# opx-cps
This repository contains the CPS object library files. The OPX CPS provides a micro-service data-centric API allowing applications to communicate with each other between threads, processes, or diverse locations.

The data model of OPX CPS is described through Yang or other constructs.
Applications can use CPS objects with Python, C, C++, and REST services in the opx-cps-REST service.
Applications/threads register for ownership of CPS objects, while other applications/threads operate and receive events of the registered CPS objects. Applications can also publish objects through the event service.

High-level list of CPS features include:
- Distributed framework for application interaction
- Database-like API (Get, Commit[add,delete,create,modify])
- Publish/subscribe semantics supported

Lookup and binary to text translation and object introspection is available.

Applications define objects through (optionally Yang-based) object models. These object models are converted into binary (C accessable) object keys and object attributes that can be used in conjunction with the C-based CPS APIs. There are adaptions on top of the CPS that allows these objects and APIs to be converted to different languages like Python.

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

opx-yang-utils-dev\_*version*\_*arch*.deb — Tools to parse Yang files

See [Architecture](https://github.com/open-switch/opx-docs/wiki/Architecture) for more information on the CPS module.

(c) 2017 Dell
