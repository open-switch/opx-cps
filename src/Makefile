PROGRAMS= cps_api_service cps_api_example 
SUBDIRS=
LIBRARIES= cps-api-common cps_python
ARCHIVES=


UNIT_TEST=cps_api_key_unittest cps_api_object_unittest cps_api_operation_unittest cps_api_events_unittest

cps_api_key_unittest_SRCS=unit_test/cps_api_key_unittest.cpp
cps_api_key_unittest_LDFLAGS=-lcps-api-common

cps_api_object_unittest_SRCS=unit_test/cps_api_object_unittest.cpp
cps_api_object_unittest_LDFLAGS=-lcps-api-common

cps_api_operation_unittest_SRCS=unit_test/cps_api_operation_unittest.cpp
cps_api_operation_unittest_LDFLAGS=-lcps-api-common

cps_api_events_unittest_SRCS=unit_test/cps_api_events_unittest.cpp
cps_api_events_unittest_LDFLAGS=-lcps-api-common -ldn_common

cps_api_example_SRCS=example/cps_api_operation_example.c
cps_api_example_LDFLAGS=-lcps-api-common -ldn_common

cps-api-common_SRCS=cps_api_object.cpp \
        cps_api_events.cpp \
        cps_api_base_event_adapter.c \
        cps_api_operation_common.cpp \
        cps_api_key.c

cps-api-common_SRCS+= cps_ns.cpp \
					cps_api_operation_service.cpp \
					cps_api_client_ipc.cpp

cps_api_service_SRCS=cps_api_service.cpp
cps_api_service_LDFLAGS = -lcps-api-common -ldn_common -levent_log


cps-api-common_LDFLAGS = -levent_log -ldn_common


cps_python_SRCS=python_extension/cps_api_python.cpp 
cps_python_CFLAGS=-DMAJOR_VERSION=1 -DMINOR_VERSION=0 -Ipython2.7 -std=c++11
cps_python_LDFLAGS=-lX11

include ${PROJROOT}/tools/workspace.mak


