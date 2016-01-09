In order to generate customer facing CPS API documentation, go to the 'top' ngos directory, and

1. Create a 'workspace' directory, if not already created as a result of a build
2. Execute the command:

doxygen cps-api/doc/doxygen.cfg

The results are in the directory:

workspace/cps_api_doc

3. Copy the Dell footer GIF file to the html directory

cp cps-api/doc/dell-footer-logo.gif workspace/cps_api_doc/html

NOTE. The process can be further automated.
