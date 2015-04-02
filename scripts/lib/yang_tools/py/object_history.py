
import sys
import yin_utils


class enum_tracker:
    the_name = "";
    the_dict = None
    last_index = None

    def __init__(self,name):
        self.the_dict = dict()
        self.the_name = name
        self.last_index = 1

    def add_enum(self, name, index):
        name = yin_utils.string_to_c_formatted_name(name)

        if self.last_index < (index+1):
            self.last_index = index+1
        self.the_dict[name] = index

    def get_name(self):
        return self.the_name

    def get_next_enum_value(self):
        return self.last_index

    def get_value(self,name, requested):
        if not name in self.the_dict.keys():
            if requested!=None:
                ix = int(requested)
            else:
                ix = self.get_next_enum_value()
            self.add_enum(name, ix)
        if requested!=None:
            self.the_dict[name] = int(requested)
        return self.the_dict[name]

    def write(self,the_file):
        the_file.write("++obj "+self.the_name+"\n")
        for k in self.the_dict.keys():
            the_file.write(k+"="+str(self.the_dict[k])+"\n")
        the_file.write("--obj "+self.the_name+" end\n");

    def create(self,the_file):
        for l in the_file:
            if l.find("#")==0:
                continue
            if l.find("++obj ") == -1:
                continue
            line = l.split();
            if len(line) < 2:
                print("Invalid line in file "+l);
                sys.exit(1)
            self.the_name = line[1];
            for el in the_file:
                if el.find("--obj ")!=-1:
                    break
                if el.find("=")==-1:
                    continue
                line = el.split("=");
                if len(line) < 2:
                    print("Badly formatted line "+line)
                    sys.exit(1)
                self.add_enum(line[0], int(line[1]))

            return True

        return False

        the_file.write("obj "+self.the_name+"\n")
        for k in self.the_dict.keys():
            the_file.write(k+"="+str(self.the_dict[k])+"\n")
        the_file.write("obj "+self.the_name+" end\n");
        return True

class history:
    the_file = None
    the_name = ""
    the_dict = None

    def __init__(self, filename):
        self.the_name = filename
        self.the_dict = dict()

        try:
            the_file = open(self.the_name,"r")
            while True:
                en = enum_tracker("") # track a single object
                if en.create(the_file):
                    self.the_dict[en.get_name()] = en
                else:
                    break

            the_file.close()

        except IOError:
            the_file = open(self.the_name,"w")
            the_file.close()

    def get_enum(self, container, name, requested):
        if not self.the_dict.has_key(container):
            self.the_dict[container] = enum_tracker(container)
        return self.the_dict[container].get_value(name,requested)

    def write(self):
        f = open(self.the_name,"w");
        f.write("# writing "+self.the_name+"\n")
        for k in self.the_dict.keys():
            en = self.the_dict[k]
            en.write(f);

        f.close()

def init(file):
    return history(file)

def close(hist):
    hist.write()

if __name__ == '__main__':
    hf = history(sys.argv[1])


