#!/bin/bash

CPS_DOC_FILE="OPX-cps-api-doc.zip"
CPS_DOC_DIR="workspace/cps-api-doc"
DOC_DIR="opx-cps/doc"
# File used to generate doxygen documentation for CPS C bindings
PSEUDO_CPS_FILE=workspace/cps.py
PYTHON_CPS_CBINDINGS=cps-api/src/python_extension/cps_api_python.cpp

if [ ! -d cps-api ]; then
   echo "Cannot find cps_api directory"
   exit 1
fi

rm -rf $CPS_DOC_DIR
mkdir -p $CPS_DOC_DIR


### Generate documentation for the C CPS API
doxygen ${DOC_DIR}/doxygen_c.cfg

### Generate documentation for the Python CPS API
echo '"""@package cps' > $PSEUDO_CPS_FILE
echo '' >> $PSEUDO_CPS_FILE
echo 'Python API for the OPX Control Plane Services' >>  $PSEUDO_CPS_FILE
echo '' >> $PSEUDO_CPS_FILE
echo '"""' >> $PSEUDO_CPS_FILE
### extract documentation from C Python bindings
cat $PYTHON_CPS_CBINDINGS | cps-api/doc/cps_py.awk | sed  's#\\n##g' >> $PSEUDO_CPS_FILE
doxygen ${DOC_DIR}/doxygen_python.cfg


### Copy the Dell footer GIF file to the html directories
cp ${DOC_DIR}/dell-footer-logo.gif $CPS_DOC_DIR/c-cpp-doc/html
cp ${DOC_DIR}/dell-footer-logo.gif $CPS_DOC_DIR/python-doc/html
cp ${DOC_DIR}/User_README.txt $CPS_DOC_DIR/README.txt

rm -f $PSEUDO_CPS_FILE

cd workspace

rm $CPS_DOC_FILE
zip -r $CPS_DOC_FILE cps-api-doc

echo ""
echo ""
echo "***"
echo "CPS API Documentation is in workspace/$CPS_DOC_FILE"
echo "***"

