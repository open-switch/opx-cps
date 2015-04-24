import cps
import sys
import os

if __name__ == '__main__':
    #load the mapping file
    cps.init('/localdisk/cwichmann/cps-api/workspace/debian/jessie/x86_64/sysroot/opt/ngos/lib/','libcpsclass-')

    data = cps.info('19')
    print data
    data = cps.info('20')
    print data
    print cps.convdict({'1':'dasra'})
    #Get the cps objects
    result = cps.get(['1.2.3.4'])

    #print discovered map
    print result
    #walk through the map and print the contents
    for d in result.keys():
        print d
        #convert to displayable info
        di = cps.arrayconv(d,result[d])
        cps.dictconv(d)

