#!/usr/bin/python
import sys
import cps
if __name__=='__main__':
	if len(sys.argv)==1:
		print "Missing args.  Please enter a CPS key path in the format of a.b.c"
		print "Example to wait for all interface events it is 1.3.1"
		print "Example wait for all TARGET events is 1"
		exit(1)
	l = []
	cps.get(sys.argv[1:],l)
	for entry in l:
		print entry['key']
		print entry['data']
