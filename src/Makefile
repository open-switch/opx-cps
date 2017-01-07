PROGRAMS= cps_api_service cps_api_example
LIBRARIES= cps-api-common cps-class-map-util cps-ut-data

CXXFLAGS+=-std=c++11 


cps-ut-data_SRCS= unit_test/cps_class_ut_data.cpp
cps-ut-data_CXXFLAGS+=-I${TOP_SRCDIR}/inc/private -I${TOP_SRCDIR}/inc/private/db
cps-ut-data_LDFLAGS = -levent_log -ldn_common -lpthread -lhiredis


cps-class-map-util_SRCS=metadata/cps_class_map_query.cpp
cps-class-map-util_CXXFLAGS=-std=c++11
cps-class-map-util_LDFLAGS=-ldn_common -levent_log -lcps-api-common -lpthread

cps-api-common_CXXFLAGS+=-I${TOP_SRCDIR}/inc/private -I${TOP_SRCDIR}/inc/private/db

cps-api-common_SRCS+=cps_api_object.cpp
cps-api-common_SRCS+=cps_api_events.cpp
cps-api-common_SRCS+=cps_api_operation_common.cpp
cps-api-common_SRCS+=cps_api_key.cpp
cps-api-common_SRCS+=cps_api_object_attr.cpp
cps-api-common_SRCS+=cps_ns.cpp
cps-api-common_SRCS+=cps_api_operation_service.cpp
cps-api-common_SRCS+=cps_api_client_ipc.cpp
cps-api-common_SRCS+=cps_api_utils.cpp
cps-api-common_SRCS+=cps_string_utils.cpp
cps-api-common_SRCS+=cps_api_operation_debug.cpp
cps-api-common_SRCS+=cps_api_vector_utils.cpp
cps-api-common_SRCS+=cps_comm_utils.cpp
cps-api-common_SRCS+=cps_api_key_utils.cpp
cps-api-common_SRCS+=cps_api_select_utils.cpp

cps-api-common_SRCS+=non-core/cps_api_object_tools.cpp
cps-api-common_SRCS+=non-core/cps_api_operation_tools.cpp
cps-api-common_SRCS+=non-core/cps_api_node.cpp

cps-api-common_SRCS+=metadata/dell-cps.cpp
cps-api-common_SRCS+=metadata/cps_api_string_mapping.cpp
cps-api-common_SRCS+=metadata/cps_dictionary.cpp
cps-api-common_SRCS+=metadata/cps_dictionary_loader.cpp
cps-api-common_SRCS+=metadata/cps_api_metadata_import.cpp

cps-api-common_SRCS+=db/cps_api_core_utils.cpp
cps-api-common_SRCS+=db/cps_api_db_direct.cpp
cps-api-common_SRCS+=db/cps_api_db_pipeline.cpp
cps-api-common_SRCS+=db/cps_api_db_events.cpp
cps-api-common_SRCS+=db/cps_redis.cpp
cps-api-common_SRCS+=db/cps_api_db_connection.cpp
cps-api-common_SRCS+=db/cps_api_db_response.cpp
cps-api-common_SRCS+=db/cps_api_db_object_key.cpp
cps-api-common_SRCS+=db/cps_api_db_init.cpp db/cps_api_db_node_set.cpp
cps-api-common_SRCS+=db/cps_api_db_event_adapter.cpp db/cps_api_db_operations.cpp

cps-api-common_LDFLAGS = -levent_log -ldn_common -lpthread -lhiredis

cps_api_service_SRCS=cps_api_service.cpp
cps_api_service_LDFLAGS = -lcps-api-common -ldn_common -levent_log -lsystemd


cps_api_example_SRCS=example/cps_api_operation_example.c
cps_api_example_LDFLAGS=-lcps-api-common

UNIT_TEST=cps_api_key_unittest cps_api_object_unittest cps_api_operation_unittest cps_api_events_unittest

UNIT_TEST+= cps_api_db_unittest cps_api_db_group_tunnel_unittest
UNIT_TEST+= cps_api_db_connection_event_unittest
UNIT_TEST+=cps_class_map_unittest
UNIT_TEST+=cps_api_receiver
UNIT_TEST+=cps_api_timing_unittest
UNIT_TEST+=cps_api_key_cache_unittest
UNIT_TEST+=cps_api_registration_unittest
UNIT_TEST+=cps_api_db_event_unittest
UNIT_TEST+= cps_api_db_1_1_unittest
UNIT_TEST+=cps_api_db_timings
UNIT_TEST+=cps_api_db_direct_unittest


PY_OBJ_PATH=${BUILD_CURDIR}/pybuild

ADDED_FOLDERS+=${PY_OBJ_PATH}

#is this currect?
include ${MAKE_INC}/buildpaths.mak

define add_UT_pieces
$(1)_SRCS+= unit_test/${1}.cpp
$(1)_LDFLAGS+= -lcps-ut-data -lcps-api-common -lcps-class-map-util -ldn_common
$(1)_CXXFLAGS+=-I$(TOP_SRCDIR)/inc/private/db -I$(TOP_SRCDIR)/inc/private
endef

$(foreach UT,${UNIT_TEST},$(eval $(call add_UT_pieces,${UT})))

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
