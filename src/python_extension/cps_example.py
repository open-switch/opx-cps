import cps
import sys
import os

if __name__ == '__main__':
    #load the mapping file
    cps.init('/localdisk/cwichmann/cps-api/workspace/debian/jessie/x86_64/sysroot/opt/ngos/lib/','libcpsclass-')

    #Get the cps objects
    result = cps.get(['1.2.3.4','2.3.4.5'])

    #print discovered map
    print result
    #walk through the map and print the contents
    for d in result.keys():
        print d
        #convert to displayable info
        print cps.array_to_dict(d,result[d])
