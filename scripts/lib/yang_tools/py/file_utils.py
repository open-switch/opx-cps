


import os
import yin_cps
import yin_utils
import tempfile
import shutil

def search_path_for_file(filename, path):
    for i in path.split(':'):
        f = os.path.join(i, filename)
        if os.path.exists(f):
            return f
    return None


class Locator:

    def __init__(self, context, dirs_as_string):
        self.tmpdir = tempfile.mkdtemp()
        self.context = context

    def get_yin_file(self, filename):
        yin_file = os.path.join(
            self.tmpdir,
            os.path.splitext(os.path.basename(filename))[0] + ".yin")
        if not os.path.exists(yin_file):
            yin_utils.create_yin_file(filename, yin_file)
        return yin_file

    def get_parsed_yin(self, filename, prefix):
        key_name = os.path.splitext(filename)[0]
        key_name = os.path.split(key_name)[1]

        yin_key = key_name
        if prefix is not None:
            yin_key += ":" + prefix

        if yin_key not in self.yin_map:
            f = self.get_yin_file(filename)
            self.yin_map[yin_key] = yin_cps.CPSParser(self.context, f)
            _cps_parser = self.yin_map[yin_key]
            _cps_parser.load(prefix=prefix)
            _cps_parser.walk()

            self.context['model-names'][_cps_parser.module.name()] = yin_key
            self.context['model-names'][_cps_parser.module.name()+'_model_'] = _cps_parser

        return self.yin_map[yin_key]

    def check_deps_loaded(self, module, context):
        entry = self.yin_map[module]
        for i in entry.imports:
            i = os.path.splitext(i)[0]
            if not i in context['current_depends']:
                return False
        return True

    def load_depends(self, module, context):
        entry = self.yin_map[module]
        for i in entry.imports:
            i = os.path.splitext(i)[0]
            if i in context['current_depends']:
                continue
            if not self.check_deps_loaded(i, context):
                self.load_depends(i, context)
            if i in context['current_depends']:
                raise Exception("")
            context['current_depends'].append(i)

        if module in context['current_depends']:
            return

        context['current_depends'].append(module)

    def load(self, yang_file, prefix=None):
        """Convert the yang file to a yin file and load the model"""
        return self.get_parsed_yin(yang_file, prefix)

    def cleanup(self):
        shutil.rmtree(self.tmpdir)
