#
# Copyright (c) 2019 Dell Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
#
# THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
# CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
# LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
# FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
#
# See the Apache Version 2.0 License for specific language governing
# permissions and limitations under the License.
#

ACLOCAL_AMFLAGS=-I m4
SUBDIRS = inc

#Script files

bin_SCRIPTS = scripts/bin/*

toolsdir= /usr/lib/opx
tools_SCRIPTS = scripts/lib/yang_tools/*.sh scripts/lib/*.py

yangtoolsdir = /usr/lib/opx/py
yangtools_SCRIPTS = scripts/lib/yang_tools/py/*.py

metadatadir = /usr/lib/opx/cpsmetadata
metadata_SCRIPTS = scripts/lib/cpsmetadata/*

lib_LTLIBRARIES = libopx_cps_api_common.la libopx_cps_class_map.la

libopx_cps_api_common_la_SOURCES = \
          src/non-core/cps_api_operation_tools.cpp	\
          src/cps_api_object_attr.cpp	\
          src/cps_api_operation_common.cpp	\
          src/cps_api_events.cpp	\
          src/cps_api_key.cpp	\
          src/cps_api_key_utils.cpp \
          src/cps_api_operation_debug.cpp	\
          src/cps_api_utils.cpp	\
          src/cps_string_utils.cpp	\
          src/cps_api_operation_service.cpp	\
          src/cps_ns.cpp	\
          src/cps_api_client_ipc.cpp	\
          src/non-core/cps_api_object_tools.cpp	\
          src/cps_api_object.cpp  \
          src/metadata/dell-cps.cpp \
          src/non-core/cps_api_node.cpp \
          src/cps_api_vector_utils.cpp \
          src/metadata/cps_api_metadata_import.cpp \
          src/metadata/cps_api_string_mapping.cpp \
          src/metadata/cps_class_map_query.cpp \
          src/metadata/cps_dictionary.cpp \
          src/metadata/cps_dictionary_loader.cpp \
          src/db/cps_api_db_connection.cpp \
          src/db/cps_api_db_event_adapter.cpp \
          src/db/cps_api_db_events.cpp \
          src/db/cps_api_db_direct.cpp \
          src/db/cps_api_db_init.cpp \
          src/db/cps_api_db_node_set.cpp \
          src/db/cps_api_db_object_key.cpp \
          src/db/cps_api_db_operations.cpp \
          src/db/cps_api_db_response.cpp \
          src/db/cps_api_db_pipeline.cpp \
          src/db/cps_api_core_utils.cpp \
          src/db/cps_redis.cpp \
          src/db/cps_api_db_connection_tools.cpp \
          src/cps_api_select_utils.cpp \
          src/non-core/cps_api_db_interface_tools.cpp


#Unit tests that will be merged into the config file at a later date
#src/unit_test/cps_api_operation_unittest.cpp
#src/unit_test/cps_api_events_unittest.cpp
#src/unit_test/cps_api_object_unittest.cpp
#src/unit_test/cps_api_key_unittest.cpp
#src/unit_test/cps_api_receiver.cpp
#src/unit_test/cps_api_timings.cpp
#src/unit_test/cps_api_key_cache_unittest.cpp
#src/unit_test/cps_class_map_unittest.cpp
#src/unit_test/cps_registration_unittest.cpp
#src/unit_test/cps_api_event_loss_unittest.cpp
#src/unit_test/cps_api_c_client_stub.c
#src/example/cps_api_operation_example.c

#current,revision,age version format - currently age 1 revision 1 (0.1)
libopx_cps_api_common_la_LDFLAGS =  -lpthread -lrt \
    -lopx_logging -lopx_common -lstdc++  -lsystemd -version-info 1:1:0 -lhiredis

libopx_cps_api_common_la_CPPFLAGS = -std=c++11 -I$(top_srcdir)/inc/opx  \
    -I$(top_srcdir)/inc/opx/private/db \
    -I$(top_srcdir)/inc/opx/private \
    -I$(includedir)/opx

libopx_cps_class_map_la_SOURCES = src/metadata/cps_class_map_query.cpp
libopx_cps_class_map_la_CPPFLAGS = -std=c++11 -I$(top_srcdir)/inc/opx \
                                     -I$(includedir)/opx
libopx_cps_class_map_la_LDFLAGS = -lpthread -lopx_common -version-info 1:1:0
libopx_cps_class_map_la_LIBADD=libopx_cps_api_common.la

#The CLI used to change levels at runtime
bin_PROGRAMS = opx_cps_service

opx_cps_service_SOURCES=src/cps_api_service.cpp
opx_cps_service_CPPFLAGS=-I$(top_srcdir)/inc/opx -I$(includedir)/opx -std=c++11
opx_cps_service_LDFLAGS= -lpthread -lopx_logging -lsystemd
opx_cps_service_LDADD=libopx_cps_api_common.la

#cps module
pyexec_LTLIBRARIES = cps.la
cps_la_LDFLAGS = -module -avoid-version -export-symbols-regex initpoppler -lopx_common -lopx_logging -lpython2.7 -lstdc++
cps_la_SOURCES = src/python_extension/cps_api_python.cpp \
    src/python_extension/cps_api_python_db_extension.cpp \
    src/python_extension/cps_api_python_operation.cpp \
  src/python_extension/cps_api_python_utils.cpp \
  src/python_extension/cps_api_python_events.cpp
cps_la_CXXFLAGS=-I$(top_srcdir)/src/python_extension -I$(top_srcdir)/inc/opx -I$(includedir)/opx -I$(includedir)/python2.7  -std=c++11
cps_la_LIBADD=libopx_cps_api_common.la  libopx_cps_class_map.la

systemdconfdir=/lib/systemd/system
systemdconf_DATA = $(top_srcdir)/scripts/init/opx-cps.service \
	$(top_srcdir)/scripts/init/opx-cps-db.service \
	$(top_srcdir)/scripts/init/opx-cps-connectivity.service

man1_MANS = man/man1/cps_set_oid.py.1 man/man1/cps_trace_events.py.1 man/man1/cps_get_oid.py.1 man/man1/cps_model_info.1
