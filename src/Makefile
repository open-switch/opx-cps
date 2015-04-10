PROGRAMS= cps_api_service cps_api_example
SUBDIRS=
LIBRARIES= cps-api-common
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

include ${PROJROOT}/tools/workspace.mak

sos: ${BUILD_DN_SYSROOT}/lib/cps.so

PY_OBJ_PATH=${BUILD_CURDIR}/pybuild

ADDED_FOLDERS+=${PY_OBJ_PATH}
include ${MAKE_INC}/additional_directories.mak

cps_python_CFLAGS=-DMAJOR_VERSION=1 -DMINOR_VERSION=0 -Ipython2.7 -std=c++11

${BUILD_DN_SYSROOT}/lib/cps.so: $(wildcard python_extension/*) | ${PY_OBJ_PATH}
	cp -f $(wildcard python_extension/*) ${PY_OBJ_PATH}
	cd ${PY_OBJ_PATH} && DISTUTILS_DEBUG=1 CFLAGS="${ADDED_CFLAGS} ${cps_python_CFLAGS}" CC="${CC}" python setup.py build -v --build-lib ${PY_OBJ_PATH}



