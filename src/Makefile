PROGRAMS= cps_api_service cps_api_example
LIBRARIES= cps-api-common cps-class-map-util
UNIT_TEST=cps_api_key_unittest cps_api_object_unittest cps_api_operation_unittest cps_api_events_unittest

CXXFLAGS+=-std=c++11


cps-class-map-util_SRCS=cps_class_map_query.cpp
cps-class-map-util_CXXFLAGS=-std=c++11
cps-class-map-util_LDFLAGS=-ldn_common -levent_log -lcps-api-common -lpthread

cps-api-common_CXXFLAGS+=-I${TOP_SRCDIR}/inc/private -I${TOP_SRCDIR}/inc/private/db

cps-api-common_SRCS+=cps_api_object.cpp
cps-api-common_SRCS+=cps_api_events.cpp
cps-api-common_SRCS+=cps_api_operation_common.cpp
cps-api-common_SRCS+=cps_api_key.cpp
cps-api-common_SRCS+=cps_api_object_attr.cpp
cps-api-common_SRCS+= cps_ns.cpp
cps-api-common_SRCS+=cps_api_operation_service.cpp
cps-api-common_SRCS+=cps_api_client_ipc.cpp
cps-api-common_SRCS+=cps_dictionary.cpp
cps-api-common_SRCS+=cps_dictionary_loader.cpp
cps-api-common_SRCS+=cps_api_object_tools.cpp
cps-api-common_SRCS+=cps_api_operation_tools.cpp
cps-api-common_SRCS+=cps_api_utils.cpp
cps-api-common_SRCS+=cps_string_utils.cpp
cps-api-common_SRCS+=cps_api_operation_debug.cpp
cps-api-common_SRCS+=db/cps_api_db_connection.cpp
cps-api-common_SRCS+=db/cps_api_db_response.cpp
cps-api-common_SRCS+=db/cps_api_db_object_key.cpp
cps-api-common_SRCS+=db/cps_redis.cpp
cps-api-common_SRCS+=cps_api_vector_utils.cpp
cps-api-common_SRCS+=cps_comm_utils.cpp
cps-api-common_SRCS+=dell-cps.cpp
cps-api-common_SRCS+=cps_api_node.cpp db/cps_api_db_init.cpp db/cps_api_db_node_set.cpp
cps-api-common_SRCS+=db/cps_api_db_event_adapter.cpp db/cps_api_db_operations.cpp


cps-api-common_LDFLAGS = -levent_log -ldn_common -lpthread -lhiredis


cps_api_service_SRCS=cps_api_service.cpp
cps_api_service_LDFLAGS = -lcps-api-common -ldn_common -levent_log

cps_api_db_unittest_CXXFLAGS+=-I$(TOP_SRCDIR)/inc/private/db -I$(TOP_SRCDIR)/inc/private
cps_api_db_unittest_SRCS=unit_test/cps_api_db_unittest.cpp
cps_api_db_unittest_LDFLAGS=-lcps-api-common
UNIT_TEST+= cps_api_db_unittest

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

UNIT_TEST+=cps_api_timing_unittest
cps_api_timing_unittest_SRCS=unit_test/cps_api_timings.cpp
cps_api_timing_unittest_LDFLAGS=-lcps-api-common -lcps-class-map-util

UNIT_TEST+=cps_api_key_cache_unittest
cps_api_key_cache_unittest_SRCS=unit_test/cps_api_key_cache_unittest.cpp
cps_api_key_cache_unittest_LDFLAGS=-lcps-api-common -lcps-class-map-util

UNIT_TEST+=cps_api_registration_unittest
cps_api_registration_unittest_SRCS=unit_test/cps_registration_unittest.cpp
cps_api_registration_unittest_LDFLAGS=-lcps-api-common -lcps-class-map-util -ldn_common

UNIT_TEST+=cps_api_db_event_unittest
cps_api_db_event_unittest_SRCS=unit_test/cps_api_db_event_unittest.cpp
cps_api_db_event_unittest_LDFLAGS=-lcps-api-common -lcps-class-map-util -ldn_common

UNIT_TEST+=cps_api_libevent_unittest
cps_api_libevent_unittest_SRCS=unit_test/cps_api_libevnent_unittest.cpp
cps_api_libevent_unittest_LDFLAGS=-lcps-api-common -lcps-class-map-util -ldn_common -levent 
cps_api_libevent_unittest_CXXFLAGS+=-I$(TOP_SRCDIR)/inc/private/db -I$(TOP_SRCDIR)/inc/private

PY_OBJ_PATH=${BUILD_CURDIR}/pybuild

ADDED_FOLDERS+=${PY_OBJ_PATH}

include ${MAKE_INC}/workspace.mak

${BUILD_CURDIR_sos}/libcps-class-map-util.so : ${BUILD_CURDIR_sos}/libcps-api-common.so

cps_python_CPPFLAGS=-DMAJOR_VERSION=1 -DMINOR_VERSION=0 -I${TOP_SRCDIR}/inc -Ipython2.7 -I.
cps_python_CXXFLAGS=-std=c++11

${BUILD_DN_SYSROOT}/lib/cps.so: $(wildcard python_extension/*) ${BUILD_CURDIR_sos}/libcps-class-map-util.so ${BUILD_CURDIR_sos}/libcps-api-common.so | ${PY_OBJ_PATH} ${BUILD_DN_SYSROOT}/lib
	cp -f $(wildcard python_extension/*) ${PY_OBJ_PATH}
	mkdir -p ${PY_OBJ_PATH}/private && cp -f $(wildcard ../inc/private/*.h) ${PY_OBJ_PATH}/private
	cd ${PY_OBJ_PATH} && DISTUTILS_DEBUG=1 LDFLAGS="${ADDED_LDFLAGS}" CFLAGS="${ADDED_CPPFLAGS} ${ADDED_CFLAGS} ${cps_python_CPPFLAGS} ${cps_python_CXXFLAGS}" CC="${CC}" python setup.py build -v --build-lib ${PY_OBJ_PATH}
	cp ${PY_OBJ_PATH}/cps.so ${BUILD_DN_SYSROOT}/lib/cps.so

install-local: ${BUILD_DN_SYSROOT}/lib/cps.so
