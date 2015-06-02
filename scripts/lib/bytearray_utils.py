#!/usr/bin/python

import struct
import sys

pack_type_map = { 'uint8_t':'<B',
		  'uint16_t':'<H',
		  'uint32_t':'<I',
		  'uint64_t':'<Q',
		}
	

pack_len_map = {  'uint8_t':1,
		  'uint16_t':2,
		  'uint32_t':4,
		  'uint64_t':8
	       }

def usage():
    print "\nusage: "
    print "\nto_ba(val(numeric),type('uint8_t', 'uint16_t', 'uint32_t', 'uint64_t'))"
    print "to_ba(1234,'uint32_t')\n"

    print "str_to_ba(string, string_length)"
    print "str_to_ba('24fgd',len('24fgd'))\n"

    print "from_ba(bytearray,type('uint8_t', 'uint16_t', 'uint32_t', 'uint64_t'))"
    print "from_ba('\\x01\\x00\\x00\\x00','uint32_t')\n"

    print "ba_to_str(bytearray, bytearray_length)"
    print "ba_to_str('\\x01\\x00\\x00\\x00',4)\n"
    sys.exit()

def to_ba(val,datatype):
    if datatype not in pack_len_map:
        print "\nInvalid type " + datatype
        usage()

    length = pack_len_map[datatype]
    s = bytearray(length)
    s[0:length] = struct.pack(pack_type_map[datatype],val)
    return s

def from_ba(val,datatype):
    if datatype not in pack_len_map:
        print "\nInvalid type " + datatype
        usage()

    length = pack_len_map[datatype]
    s = struct.unpack(pack_type_map[datatype],val[0:length])[0]
    return s

def str_to_ba(val,length):
    s = bytearray(length)
    s[0:length] = struct.pack('<'+str(length)+'s',val)
    return s

def ba_to_str(val,length):
    s = struct.unpack('<'+str(length)+'s',val[0:length])[0]
    return s


