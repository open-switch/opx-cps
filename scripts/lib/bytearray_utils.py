#!/usr/bin/python

import struct
import sys
import binascii
import socket

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

def macstr_to_ba(t, macstr):
    """
    Converts a MAC address string representation to bytearray

    macstr - MAC address octet string with ':' separated hexadecimal octets
             Each octet should be a hexadecimal number
             eg: '01:02:03:BC:05:06'
    return bytearray
    """
    return binascii.unhexlify(macstr.replace(':', ''))

def ba_to_macstr(t, ba):
    """
    Converts a bytearray to MAC address string representation

    ba - bytearray of the MAC
    return MAC address hexadecimal octet string with each octet separated by ':'
    """
    macstr = binascii.hexlify(ba)
    it=iter(macstr)
    return ':'.join(a+b for a,b in zip(it, it))

def ipv4str_to_ba(t, ipv4str):
    """
    Converts a IPv4 address string representation to bytearray

    ipv4str - IP address decimal string with '.' separeted decimal octets.
              Each octet should be a decimal number - Hex not allowed
              eg: '23.0.0.1'
    return bytearray
    """
    return socket.inet_pton(socket.AF_INET, ipv4str)

def ba_to_ipv4str(t, ba):
    """
    Converts a bytearray to IPv4 address string representation

    ba - bytearray of the IPv4 address
    return IPv4 address decimal string with each decimal octet seperated by '.'
    """
    return socket.inet_ntop(socket.AF_INET, ba)

def ipv6str_to_ba(t, ipv6str):
    """
    Converts a IPv6 address string representation to bytearray

    ipv4str - IPv6 address hexadecimal octet string with ':' separating every 2 octets
             Each octet should be a hexadecimal number
             eg: '2001:0db8:85a3:0000:0000:8a2e:0370:7334'
             or  '2001:0db8:85a3::8a2e:0370:7334'
    return bytearray
    """
    return socket.inet_pton(socket.AF_INET6, ipv6str)

def ba_to_ipv6str(t, ba):
    """
    Converts a bytearray to IPv6 address string representation

    ba - bytearray of the IPv6 address
    return IPv6 address string with ':' separating every 2 octets - leading zeroes will be omitted
    """
    return socket.inet_ntop(socket.AF_INET6, ba)

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
    'mac'     : ba_to_macstr,
    'ipv4'    : ba_to_ipv4str,
    'ipv6'    : ba_to_ipv6str,
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
    'mac'     : macstr_to_ba,
    'ipv4'    : ipv4str_to_ba,
    'ipv6'    : ipv6str_to_ba,
}

def value_to_ba(typ,val):
    if typ in type_to_ba:
        return type_to_ba[typ](typ,val)
    return bytearray(val)

