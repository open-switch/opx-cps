PROGRAMS= cps_api_service cps_api_example
SUBDIRS=
LIBRARIES= cps-api-common cps-class-map-util
ARCHIVES=
UNIT_TEST=cps_api_key_unittest cps_api_object_unittest cps_api_operation_unittest cps_api_events_unittest

CXXFLAGS+=-std=c++11

cps-class-map-util_SRCS=cps_class_map.cpp
cps-class-map-util_CFLAGS=-std=c++11
cps-class-map-util_LDFLAGS=-ldn_common -levent_log -lcps-api-common -lpthread

cps-api-common_SRCS=cps_api_object.cpp \
        cps_api_events.cpp \
        cps_api_base_event_adapter.cpp \
        cps_api_operation_common.cpp \
        cps_api_key.c

cps-api-common_SRCS+=cps_api_object_attr.cpp


cps-api-common_SRCS+= cps_ns.cpp \
					cps_api_operation_service.cpp \
					cps_api_client_ipc.cpp

cps-api-common_LDFLAGS = -levent_log -ldn_common -lpthread

cps_api_service_SRCS=cps_api_service.cpp
cps_api_service_LDFLAGS = -lcps-api-common -ldn_common -levent_log


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


UNIT_TEST+=cps_class_map_unittest
cps_class_map_unittest_SRCS=unit_test/cps_class_map_unittest.cpp
cps_class_map_unittest_LDFLAGS=-lcps-api-common -lcps-class-map-util

UNIT_TEST+=cps_api_receiver
cps_api_receiver_SRCS=unit_test/cps_api_receiver.cpp
cps_api_receiver_LDFLAGS=-lcps-api-common -lcps-class-map-util


include ${PROJROOT}/tools/workspace.mak


sos: ${BUILD_DN_SYSROOT}/lib/cps.so

${BUILD_CURDIR_sos}/libcps-class-map-util.so : ${SW_SYSROOT_LIB}/libcps-api-common.so

PY_OBJ_PATH=${BUILD_CURDIR}/pybuild

ADDED_FOLDERS+=${PY_OBJ_PATH}
include ${MAKE_INC}/additional_directories.mak

cps_python_CFLAGS=-DMAJOR_VERSION=1 -DMINOR_VERSION=0 -Ipython2.7 -std=c++11

${BUILD_DN_SYSROOT}/lib/cps.so: $(wildcard python_extension/*) | ${PY_OBJ_PATH}
	cp -f $(wildcard python_extension/*) ${PY_OBJ_PATH}
	cp -f $(wildcard ../inc/private/*.h) ${PY_OBJ_PATH}
	cd ${PY_OBJ_PATH} && DISTUTILS_DEBUG=1 LDFLAGS="${ADDED_LDFLAGS}" CFLAGS="${ADDED_CFLAGS} ${cps_python_CFLAGS}" CC="${CC}" python setup.py build -v --build-lib ${PY_OBJ_PATH}
	cp ${PY_OBJ_PATH}/cps.so ${BUILD_DN_SYSROOT}/lib/cps.so && cp ${PY_OBJ_PATH}/cps.so ${SW_SYSROOT_LIB}



