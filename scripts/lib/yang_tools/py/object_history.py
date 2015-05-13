
import sys
import yin_utils
from os import walk
import os

class enum_tracker_int:

    def __init__(self,indexer,name):
        self.the_dict = dict()
        self.indexer = indexer
        self.the_name = name

    def add_enum(self, name, index):
        if index > int('ffff',16):
            raise Exception("Invalid id detected "+str(index))

        self.indexer.use_enum(index)
        self.the_dict[name] = index

    def get_name(self):
        return self.the_name

    def get_value(self,name, requested):
        if not name in self.the_dict.keys():
            if requested!=None:
                ix = int(requested)
            else:
                ix = self.indexer.get_free_enum()
            self.add_enum(name, ix)
        if requested!=None:
            self.the_dict[name] = int(requested)
        return self.the_dict[name]

    def setup(self,dic):
        for en in dic:
            self.add_enum(en,int(dic[en]) & int("ffff", 16))


class IndexFile:
    def __init__(self,file):
        self.file = file

    def write(self,enum):
        file = self.file
        file.write("++obj "+enum.the_name+"\n")
        for k in enum.the_dict.keys():
            file.write(k+"="+str(enum.the_dict[k])+"\n")
        file.write("--obj "+enum.the_name+" end\n");

    def get(self):
        file = self.file
        for l in file:
            if l.find("#")==0:
                continue
            if l.find("++obj ") == -1:
                continue
            line = l.split();
            if len(line) < 2:
                print("Invalid line in file "+l);
                sys.exit(1)

            the_name = line[1];
            d = {'name':line[1] }
            d['list'] = {}
            for el in file:
                if el.find("--obj ")!=-1:
                    break
                if el.find("=")==-1:
                    continue
                line = el.split("=");
                if len(line) < 2:
                    print("Badly formatted line "+line)
                    sys.exit(1)
                d['list'][line[0]] = line[1]

            return d
        return {}

class IndexTracker :
    def __init__(self,start):
        self.last_index = 1
        if self!=None:
                self.last_index = start

    def get_free_enum(self):
        return self.last_index

    def use_enum(self, index):
        if int(self.last_index) < index+1:
            self.last_index = index+1
        return index;

class history:
    the_file = None
    the_name = ""
    the_dict = None
    object_cat = []
    parsed_files = set()

    GLOBAL_SECTION="global"
    MODULE_SECTION="module"

    GLOBAL_enums = enum_tracker_int(IndexTracker(10),"global")

    def get_global_enums(self,filename):
        with open(filename,"r") as f:
            ix_reader = IndexFile(f)
            while True:
                    d = ix_reader.get()
                    if 'name' not in d: break
                    if d['name'] != self.GLOBAL_SECTION: continue
                    self.GLOBAL_enums.setup(d['list'])
                    break;

    def load_all_globals(self):
        l = []
        path = os.getenv('YANG_PATH','')
        f = []
        found_names = []
        for i in path.split(':'):
            for (dirpath, dirnames, filenames) in os.walk(i):
                for filename in filenames:
                    if filename.find('.yhist')==-1: continue

                    if filename in found_names:
                        continue
                        #print("Error.. found two instances of " + filename + " at "+dirpath)
                        #print("other files found are:")
                        #print f
                        #sys.exit(1)

                    f.append(os.path.join(dirpath,filename))
                    found_names.append(filename)
        for i in f:
            self.get_global_enums(i)

    staticmethod (get_global_enums)
    staticmethod (load_all_globals)

    def __init__(self, filename, category):
        self.the_name = filename
        self.the_dict = dict()
        self.module = IndexTracker(1)

        self.load_all_globals()
        self.category = category
        self.the_dict[self.MODULE_SECTION] = enum_tracker_int(self.module,category)
        self.the_dict[self.GLOBAL_SECTION] = enum_tracker_int(self.GLOBAL_enums.indexer,self.GLOBAL_SECTION)

        try:
            the_file = open(self.the_name,"r")
            ix_reader = IndexFile(the_file)
            while True:
                d = ix_reader.get()
                if not 'name' in d : break
                if not d['name'] in self.the_dict: continue
                self.the_dict[d['name']].setup(d['list'])

            the_file.close()

        except IOError:
            the_file = open(self.the_name,"w")
            the_file.close()


    def get_global(self,name):
        return self.the_dict[self.GLOBAL_SECTION].get_value(name, None)

    def get_enum(self,name, requested):
        return (self.the_dict[self.MODULE_SECTION].get_value(name,requested) +
            (self.get_global(self.category) << 16))

    def write(self):
        print "Writing history to "+self.the_name
        with open(self.the_name,"w") as f:
            f.write("# writing "+self.the_name+"\n")
            writer = IndexFile(f)
            for k in self.the_dict.keys():
                writer.write(self.the_dict[k])

def init(file,category):
    h = history(file,category)
    return h

def close(hist):
    hist.write()

if __name__ == '__main__':
    hf = history(sys.argv[1])


