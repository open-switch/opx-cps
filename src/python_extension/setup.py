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

from distutils.core import setup, Extension

setup(name="cps", version="0.0",
      ext_modules=[Extension("cps", [
                             "cps_api_python.cpp",
                             "cps_api_python_utils.cpp",
                             "cps_api_python_events.cpp",
                             "cps_api_python_operation.cpp",
                             "cps_api_python_db_extension.cpp"],
                             libraries=[
                             'cps-api-common',
                             "cps-class-map-util"],
                             )])
