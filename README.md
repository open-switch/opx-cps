#opx-cps

##Description The OPX CPS provides a data-centric API allowing applications to communicate with each other between threads, processes or diverse locations.

The data model of OPX CPS is described through Yang or other constructs.
Applications can use CPS objects using Python, C, C++ and REST services in the opx-cps-REST service.
Applications/threads will register for ownership of CPS objects while other applications/threads will operate and receive events of the registered CPS objects. Applications can also publish objects through the event service.

A high level list of CPS features are:
1) Distributed framework for application interaction
2) DB-like API (Get, Commit[add,delete,create,modify])
3) Publish/subscribe semantics supported

Lookup and binary to text translation and object introspection available

Applications define objects through (optionally yang based) object models. These object models are converted into binary (C accessable) object keys and object attributes that can be used in conjunction with the C-based CPS APIs. There are adaptions on top of CPS that allows these objects and APIs to be converted to different languages like Python.

With the object keys and attributes applications can:
1) Get a single or get multiple objects 
2) Perform transactions consisting of:
   a) Create
   b) Delete
   c) Set
   d) Action
3) Register and publish object messages

