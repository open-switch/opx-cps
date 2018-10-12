

import temp_elements

import os
import yin_utils


class context:
    __log_levels = {'ERR': 0, 'INFO': 1, 'DEB': 2}

    def __log_level_to_number(self, level):
        if level in __log_levels:
            return __log_levels[level]
        return __log_levels['DEB']

    def __init__(self, args):
        self.__details = {}

        context = self.__details

        # setup the initial context structure
        context['args'] = args
        context['output'] = {}
        context['output']['header'] = {}
        context['output']['src'] = {}

        context['history'] = {}
        context['identity'] = {}

        context['grouping'] = {}
        context['types'] = {}
        context['enum'] = {}
        context['union'] = {}

        # All model categories based on the key
        context['model-category'] = {}
        context['model-history-name'] = {}

        # from the filename get the model key name
        context['file-to-key'] = {}

        # from the file get the module name
        context['file-to-module'] = {}

        # model given the key
        context['models-by-key'] = {}
        context['model-name-by-key'] = {}

        # provide a list of imports for a given model
        context['key-to-imports'] = {}

        context['models'] = {}  # list of all files (nodes) and module mappings
        context['model-names'] = {}

        _parser_cache = self.get_arg('cache-folder')
        if len(_parser_cache) == 0:
            _parser_cache = os.getenv('CPS_PARSER_CACHE', '')

        if len(_parser_cache) > 0:
            class Directory:

                def __init__(self, dir):
                    self.__tmpdir = dir

                def clean(self):
                    pass

                def get_path(self):
                    return self.__tmpdir

                def make_path(self, filename):
                    return os.path.join(self.__tmpdir, filename)

            context['temp-dir-obj'] = Directory(_parser_cache)
        else:
            context['temp-dir-obj'] = temp_elements.Directory()

        context['temp-dir'] = context['temp-dir-obj'].get_path()
        context['yin-loader'] = yin_utils.Locator(context, context['temp-dir'])

        context['debug'] = 0
        if len(self.get_arg('debug')) > 0:
            context['debug'] = self.__log_level_to_number(
                self.get_arg('debug'))

        context['temp-setting'] = {}
        context['temp-setting']['remove-files'] = len(
            self.get_arg('cleanup')) == 0

        context['history']['sources'] = self.get_arg('history')

        __standard_copyright = True
        if self.get_arg('opensource-header').capitalize() == 'TRUE':
            __standard_copyright = False

        context['standard-copyright'] = __standard_copyright

    def name_is_a_type(self, name):
        return  name in self.__details['types'] or \
            name in self.__details['enum'] or \
            name in self.__details['union']

    def get_tyoe_node(self, name):
        if name in self.__details['types']:
            return self.__details['types'][name]
        elif name in self.__details['enum']:
            return self.__details['enum'][name]
        return self.__details['union'][name]

    def resolve_type(self, type_elem, get_type_name=True):

        if type_elem is None:
            return None

        _type_name = type_elem.get('name')

        _types = self.__details['types']
        _enums = self.__details['enum']
        _unions = self.__details['union']

        _type = type_elem

        while _type_name in _types or _type_name in _enums or _type_name in _unions:
            if _type_name in _enums:
                _elem = _enums[_type_name]
            elif _type_name in _types:
                _elem = _types[_type_name]
            else:
                _elem = _unions[_type_name]
            _type = yin_utils.get_type(_elem)
            _type_name = _type.get('name')

        if get_type_name:
            return _type_name
        return _type

    def get_yang_nodes(self, yang_file):
        _yin_file = self.__details['yin-loader'].get_yin_file(yang_file)
        return self.__details['yin-loader'].get_yin_nodes(_yin_file)

    def log(self, msg_type, msg, *args):
        __lvl = self.__log_level_to_number(msg_type)

        if __lvl <= context['debug']:
            print(msg, args)

    def __check_and_add_to_model(self, key, sub_key, value):
        if key not in self.__details['models-by-key']:
            self.__details['models-by-key'][key] = {}
        self.__details['models-by-key'][key][sub_key] = value

    def __get_model_attr(self, key, sub_key):
        try:
            return self.__details['models-by-key'][key][sub_key]
        except:
            pass
        return None

    def get_model_for_key(self, key):
        _key_prefix = key.split('/', 1)
        _key_prefix = _key_prefix[0]
        if len(_key_prefix) < 2:
            raise Exception(
                'Missing prefix in key %s - model can\'t be located' %
                key)
        if _key_prefix not in self.__details['model-names']:
            raise Exception('Prefix unknown for key %s' % key)
        _model_full_name = self.__details['model-names'][_key_prefix]
        if _model_full_name not in self.__details['models-by-key']:
            raise Exception('Full model not found %s' % _model_full_name)
        return self.__details['models-by-key'][_model_full_name]['model']

    def get_model_name(self, key):
        return self.__get_model_attr(key, 'filename')

    def add_model_name(self, key, model_name):
        self.__check_and_add_to_model(key, 'filename', model_name)

    def set_nodes(self, key, nodes):
        self.__check_and_add_to_model(key, 'nodes', nodes)

    def get_nodes(self, key):
        return self.__get_model_attr(key, 'nodes')

    def set_hist_name(self, key, name):
        self.__check_and_add_to_model(key, 'hist', name)

    def get_hist_name(self, key):
        return self.__get_model_attr(key, 'hist')

    def add_cat(self, key, category):
        self.__check_and_add_to_model(key, 'category', category)

    def get_cat(self, key):
        return self.__get_model_attr(key, 'category')

    def add_ns(self, key, ns):
        self.__check_and_add_to_model(key, 'ns', ns)

    def get_ns(self, key):
        return self.__get_model_attr(key, 'ns')

    def add_model(self, key, model):
        self.__check_and_add_to_model(key, 'model', model)

    def get_model(self, key):
        return self.__get_model_attr(key, 'model')

    def add_import(self, key_name, module, prefix):
        __imports = self.__get_model_attr(key_name, 'import')
        if __imports is None:
            self.__check_and_add_to_model(key_name, 'import', {})
            __imports = self.__get_model_attr(key_name, 'import')
        __imports[module] = prefix

    def get_arg(self, var_name):
        if var_name not in self.__details['args']:
            return ""
        return self.__details['args'][var_name]

    def __getitem__(self, key):
        return self.__details[key]

    def __iter__(self):
        return self.__details.__iter__()

    def __setitem__(self, key, value):
        self.__details[key] = value
        return self.__details[key]

    def get_config_path(self, relative_name):
        return os.path.join(self.__details['history']['sources'], relative_name)

    def get_tmp_filename(self, relative_filename):
        return os.path.join(self.__details['temp-dir'], relative_filename)

    def __enter__(self):
        pass

    def __exit__(self, exc_type, exc_value, traceback):
        self.clear()

    def clear(self):
        if self.get_arg('cache-folder') != "":
            self.__details['temp-dir-obj'].clean()
