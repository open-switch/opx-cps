#!/usr/bin/python
import sys
import cps
import cps_utils

if __name__=='__main__':
	if len(sys.argv)==1:
		print "Missing args.  Please enter a CPS key path in the format of a.b.c"
		print "Example to wait for all interface events it is 1.3.1"
		print "Example wait for all TARGET events is 1"
		exit(1)
	l = []
	k = []
	for e in sys.argv[1:]:
		k.append(cps.key_from_name('target',e))
	t = cps_utils.CPSTypes()
	cps.get(k,l)
	for entry in l:
		print ""
		print "Key: " + entry['key']
		for k in entry['data']:
			print k +" = "+ str(t.from_data(k,entry['data'][k]))
