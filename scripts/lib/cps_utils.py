import cps
import os

import bytearray_utils


class CPSTypes:
    def __init__(self):
        self.types = {}

    def add_type(self, key, typ):
        self.types[key] = typ

    def guess_type_for_len(self,val):
        if type(val) == int:
            return 'uint32_t'
        if type(val) == bytearray:
            if len(val)==4:
                return 'uint32_t'
        return None

    def find_type(self,key,val):
        if key in self.types:
            t = self.types[key]
        else:
            t = self.guess_type_for_len(val)
            if t==None:
                t = 'bytearray'
        return t

    def from_data(self, key, val):
        t = self.find_type(key,val)
        return bytearray_utils.ba_to_value(t,val)

    def to_data(self,key,val):
        t = self.find_type(key,val)
        return bytearray_utils.value_to_ba(t,val)


class CPSLibInit:
    def load_class_details(self):
        libs=[]
        path=os.getenv("LD_LIBRARY_PATH")
        if path==None:
            path='/opt/ngos/lib'
        for i in path.split(':'):
            print "Searching "+i
            files = os.listdir(i)
            for f in files:
                if f.find('cpsclass')==-1:continue
                libs.append(i)
                break;
        for i in libs:
            print "Loading from "+i
            res = cps.init(i,'cpsclass')
            print res

    def __init__(self):
        self.load_class_details()


def init():
    CPSLibInit()

def key_mapper():
    return CPSKeys()

