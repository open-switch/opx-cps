#!/usr/bin/python

import struct
import sys
import binascii

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


def to_ba(val,datatype):

    """
    Converts a numeric value(uint8_t, uint16_t, uint32_t, uint64_t) to a byte array of
    appropriate data type.

    val is the numeric value
    datatype is the type of val
    return bytearray of the value
    """
    if datatype not in pack_len_map:
        raise TypeError("Invalid data type - " + str(datatype))
        return

    length = pack_len_map[datatype]
    s = bytearray(length)
    s[0:length] = struct.pack(pack_type_map[datatype],val)
    return s

def from_ba(ba,datatype):
    """
    Converts from byte array to a numeric value(uint8_t, uint16_t, uint32_t, uint64_t)

    ba - bytearray containing value
    datatype - the type of bytearray
    return numeric value of bytearray
    """
    if datatype not in pack_len_map:
        raise TypeError("Invalid data type - " + str(datatype))
        return

    length = pack_len_map[datatype]
    s = struct.unpack(pack_type_map[datatype],ba[0:length])[0]
    return s

def str_to_ba(str,length):
    """
    Converts a string to a bytearray.

    str - string to be converted
    length - length of string
    return bytearray of the string
    """
    s = bytearray(length)
    s[0:length] = struct.pack('<'+str(length)+'s',val)
    return s

def ba_to_str(ba,length):
    """
    Converts a bytearray to string

    ba - bytearray of the string
    length - length of bytearray
    return string of the bytearray
    """
    s = struct.unpack('<'+str(length)+'s',val[0:length])[0]
    return s

def ba_to_str_wr(t,val):
    return ba_to_str(ba,len(ba)-1)

def ba_to_int_type(t,val):
    return from_ba(val,t)

def ba_to_ba(t,val):
    return val

def hex_from_data(t,val):
    return binascii.hexlify(val)

ba_to_type={
    'string' : ba_to_str_wr,
    'uint8_t': ba_to_int_type,
    'uint16_t': ba_to_int_type,
    'uint32_t': ba_to_int_type,
    'uint64_t': ba_to_int_type,
    'hex' : hex_from_data,
}

def ba_to_value(typ, val):
    if typ in ba_to_type:
        return ba_to_type[typ](typ,val)
    return val

def wr_str_to_ba(t,val):
    return bytearray(val+'\0')

def wr_int_type_to_ba(t,val):
    return to_ba(val,t)

def hex_to_ba(t,val):
    return binascii.unhexlify(val)

type_to_ba={
    'string' : wr_str_to_ba,
    'uint8_t': wr_int_type_to_ba,
    'uint16_t': wr_int_type_to_ba,
    'uint32_t': wr_int_type_to_ba,
    'uint64_t': wr_int_type_to_ba,
    'hex'     : hex_to_ba,
}

def value_to_ba(typ,val):
    if typ in type_to_ba:
        return type_to_ba[typ](typ,val)
    return bytearray(val)


