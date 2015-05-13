import cps
import os

class CPSKeys:
    def find_last_piece_of_key(self,key):
        if key[-1:] == '.':
            key = key[:-1]
        pos = key.rfind('.')
        if pos==-1: return key
        return key[pos+1:]

    def dict(self,type):
        key = self.find_last_piece_of_key(cps.key_from_name('target',type))
        print key
        reg = cps.info(key)
        d = {}
        for i in reg.keys():
            key = self.find_last_piece_of_key(i)
            val = reg[i]
            d[key] = val
            d[val] = key
        return d

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

