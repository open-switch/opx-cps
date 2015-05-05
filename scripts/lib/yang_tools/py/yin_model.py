import os
import yin_cps
import yin_utils
import tempfile
import cps_c_lang
import cms_lang
import sys

class CPSYinFiles:
    yin_map = dict()
    yin_parsed_map = dict()

    def __init__(self,context):
        self.tmpdir = tempfile.mkdtemp();
        self.context = context


    def get_yin_file(self,filename):
        yin_file = os.path.join(self.tmpdir,os.path.splitext(os.path.basename(filename))[0]+".yin")
        if not os.path.exists(yin_file):
            yin_utils.create_yin_file(filename,yin_file)
        return yin_file

    def get_parsed_yin(self,filename):
        key_name = os.path.splitext(filename)[0]
        key_name = os.path.split(key_name)[1]
        if key_name not in self.yin_map:
            f = self.get_yin_file(filename)
            self.yin_map[key_name] = yin_cps.CPSParser(self.context,f)
            self.yin_map[key_name].load()
            self.yin_map[key_name].walk()

            self.context['model-names'][self.yin_map[key_name].module.name()] = key_name
        return self.yin_map[key_name]

    def check_deps_loaded(self,module,context):
        entry = self.yin_map[module]
        for i in entry.imports:
            i = os.path.splitext(i)[0]
            if not i in context['current_depends']: return False
        return True

    def load_depends(self,module,context):
        entry = self.yin_map[module]
        for i in entry.imports:
            i = os.path.splitext(i)[0]
            if i in context['current_depends']: continue
            if not self.check_deps_loaded(i,context):
                self.load_depends(i,context)
            if i in context['current_depends']:
                raise Exception ("")
            context['current_depends'].append(i)

        if module in context['current_depends']:
            return

        context['current_depends'].append(module)

    def load(self,yang_file):
        return self.get_parsed_yin(yang_file)

    def seed(self,filename):
        s = set()
        l = list()
        l.append(filename)
        while len(l) > 0:
            f = l.pop()
            p = self.get_parsed_yin(f)
            for n in p.imports:
                if n not in l:
                    l.append(n)
        #parse based on dependencies
        context = dict()
        context['current_depends'] = list()
        for i in self.yin_map.keys():
            self.load_depends(i,context)

        print context['current_depends']

        for i in context['current_depends']:
            self.yin_map[i].parse()

    def __exit__(self, type, value, traceback):
         os.rmdir(self.tmpdir)
        #self.yin_map[key_name].parse()

class CPSYangModel:
    model = None
    coutput = None
    def __init__(self,args):
        self.args = args
        self.filename = self.args['file']
        self.context = dict()
        self.context['args'] = args
        self.context['output']={}
        self.context['output']['header'] = {}
        self.context['output']['src'] = {}
        self.context['output']['language']= {}
        self.context['output']['language']['cps'] = cps_c_lang.Language(self.context)
        self.context['output']['language']['cms'] = cms_lang.Language(self.context)

        self.context['types'] = {}
        self.context['enum'] = {}
        self.context['union'] = {}
        self.context['model-names'] = {}

        self.context['loader'] = CPSYinFiles(self.context)

        self.model = self.context['loader'].load(self.filename)
        for i in self.context['output']['language']:
            self.context['output']['language'][i].setup(self.model)


        for i in self.context['args']['output'].split(','):
            self.context['output']['language'][i].write()

        #self.context['output']['language'].setup(self.model)

        #self.write_details('header')
        #self.write_details('src')


    def write_details(self,key):
        class_type = self.context['output'][key]
        if key in self.args:
            old_stdout = sys.stdout
            with open(self.args[key],"w") as sys.stdout:
                class_type.COutputFormat(self.context).show(self.model)
            sys.stdout = old_stdout


    def close(self):
        for i in self.context['output']['language']:
            self.context['output']['language'][i].close()

