from distutils.core import setup, Extension

setup(name="cps", version="0.0",
	ext_modules = [Extension("cps", ["cps_api_python.cpp"],
    libraries=['cps-api-common'],
    )])
