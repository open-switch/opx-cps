__author__ = 'root'


from cps_utils import *



#Dictionay to be filled in the cps object
d = { "id" : "1", "vlan" : "5"}

# takes qualifier as target by default and operation as create, to change that
# pass CPSObject("xxx",qual="yyy",op="zzz",data=d)
# Passing Data Dictionary is also optional
obj = CPSObject("stg",data=d)

#Change Operation
obj.set_operation("set")

# Embedded Attribute list
el = [ "intf","0","state"]
obj.add_embed_attr(el,"10")

# Another Dictionary to add it to object
fd = { "viraj" : "127.0.0.1"}

# Add data type of attribute
obj.add_attr_type("viraj","ipv4")
obj.fill_data(fd)

#Get the object content
print obj.get()
print "\n\n"
#Print the object in nicer format
obj.print_obj()
print "\n\n"
#Compare if dictioanry attributes are present in the obj with same values
bd = { "id" : '\x01\x00\x00\x00', "vlan" : '\x05\x00\x00\x00'}
print obj.key_compare(bd)
print "\n\n"

# Create get object, it is created with "target" qualifier by default,to change
# pass qual="xxx" in below constructor
get_obj = CPSGetObject("stg",filter=d)

#Pass a dictionary which contains filter attributes and values
get_obj.fill_filter(fd)

#Get the object
print get_obj.get()
print "\n\n"

#Print the get object in nicer format
get_obj.print_obj()
print "\n\n"

#Compare if dictionary attributes are present in the obj with same values
print get_obj.key_compare(bd)
