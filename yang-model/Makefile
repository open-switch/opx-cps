include ${MAKE_INC}/buildpaths.mak

install : ../inc/dell-cps.h ../src/metadata/dell-cps.cpp

#build issue still exists. Need to remove config type from .h manually for now
../inc/dell-cps.h ../src/metadata/dell-cps.cpp: dell-cps.yang
	(  cd .. ; YANG_PATH=${PWD}/yang-model scripts/lib/yang_tools/ar_parse_yang.sh dell-cps.yang inc src/metadata )
	( cd .. ; sed -i -e 's/module_init/cps_api_yang_module_init/g' src/metadata/dell-cps.cpp)


include ${MAKE_INC}/workspace.mak


