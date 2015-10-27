from distutils.core import setup, Extension

setup(name="cps", version="0.0",
      ext_modules=[Extension("cps", [
                             "cps_api_python.cpp",
                             "cps_api_python_utils.cpp",
                             "cps_api_python_events.cpp",
                             "cps_api_python_operation.cpp"],
                             libraries=[
                             'cps-api-common',
                             "cps-class-map-util"],
                             )])
