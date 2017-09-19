#
# Copyright (c) 2015 Dell Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
#
# THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
# CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
# LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
# FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
#
# See the Apache Version 2.0 License for specific language governing
# permissions and limitations under the License.
#

# parser for global variable reservation
import ConfigParser

import sys

import os


class IndexTracker:

    """
    Holds a max enum value and a list of names to values.
    """

    def __init__(self, name_map, start):
        """Take a start index as the newest possible value.
        @start this is the start range for this set of values
        """
        self._last_index = 0
        self._name_map = name_map

        if start is not None:
            self.last_index = start

    def get_free_enum(self):
        """
        Return the next valid index
        """
        _last_index = self.last_index
        self.last_index += 1
        return _last_index

    def get_element_name(self, value):
        """
        Return the name that owns the value
        @value find the name for the given value
        """
        # more speedy check to see if value added
        if not value in self._name_map.itervalues():
            return ""

        for _name, _value in self._name_map.iteritems():
            if _value == value:
                return _name
        return ""

    def get_element_value(self, name):
        """
        Find the value for the given name.  Throw the exception on error
        @name the item being searched for
        """
        if name in self._name_map:
            return self._name_map[name]
        return None

    def add_element(self, name, value, value_must_be_unique=False):
        """
        Add the name/value combination to the map
        @name is the name of the element
        @value is the value for the element
        @value_must_be_unique if this is true, check to see if the value has already been used
        """

        if name in self._name_map and value is None:
            value = self._name_map[name]

        if value_must_be_unique and value is not None:
            __name = self.get_element_name(value)

            if len(__name) > 0 and __name != name:
                raise Exception(
                    "Adding element - found duplicate names for value %d - %s and %s " %
                    (value, name, __name))

        if name in self._name_map and self._name_map[name] != value:
            raise Exception(
                "Adding element - found duplicate names %s - values are %d and %d" %
                (name, value, self._name_map[name]))

        if value is None:
            value = self.get_free_enum()

        self._name_map[name] = value

        if self.last_index < value + 1:
            self.last_index = value + 1
        return value

    def replace_element(self, name, value, value_must_be_unique=False):
        """
        Replace the existing name (if exists) with a new value.  Report if the value is already used
        @name the name of the element
        @value the value for the element
        """
        if name in self._name_map:
            del self._name_map[name]

        return self.add_element(name, value, value_must_be_unique)

    def walk_elements(self, handler, context):
        """
        For each element call the handler passing in the name, value and the user supplied context
        @handler a callback that takes three parameters - context, name, value
        @context the context passed to the handler
        """
        for _value in self._enum_map:
            _name = self._name_map[_value]
            handler(context, _name, _value)


class Model_Element_IndexTracker(IndexTracker):

    def __init__(self, name_map, name, start):
        self.__name = name
        IndexTracker.__init__(self, name_map, start)

    def add_element(self, name, value):
        return IndexTracker.add_element(self, name, value, True)

    def replace_element(self, name, value):
        return IndexTracker.replace_element(self, name, value, True)


class Model_Enum_IndexTracker(IndexTracker):

    def __init__(self, name_map, name, start):
        self.name = name;
        IndexTracker.__init__(self, name_map, start)

    def add_element(self, name, value):
        return IndexTracker.add_element(self, name, value, False)

    def replace_element(self, name, value):
        return IndexTracker.replace_element(self, name, False)


class YangHistory_ConfigFile_Base:
    GLOBAL_SECTION = "global"

    @staticmethod
    def get_cfg_parser():
        _config = ConfigParser.ConfigParser()
        _config.optionxform = str
        return _config

    @staticmethod
    def empty_config_file():
        _d = {}
        _d['global'] = {}
        _d['global']['range-start'] = 0  # start range of folder
        _d['global']['range-end'] = 0  # end range of folder
        _d['global']['category'] = ""
        _d['items'] = {}
        return _d


class YangHistory_ConfigFile_v1:

    """
    Read an index file and
    """
    GLOBAL_SECTION = "global"

    def __init__(self, filename):
        self._filename = filename

    def get(self):
        _category = None
        _d = YangHistory_ConfigFile_Base.empty_config_file()
        with open(self._filename, 'r') as file:
            for l in file:
                if l.find("#") == 0:
                    continue
                if l.find("++obj ") == -1:
                    continue
                line = l.split()
                if len(line) < 2:
                    print("Invalid line in file " + l)
                    sys.exit(1)

                _name = line[1]

#                d['name'].append(_name)
#                d[_name] = {}

                if _name != 'global' and _name not in _d['items']:
                    _d['items'][_name] = {}

                for el in file:
                    if el.find("--obj ") != -1:
                        break
                    if el.find("=") == -1:
                        continue
                    line = el.split("=")
                    if len(line) < 2:
                        print("Badly formatted line " + line)
                        sys.exit(1)

                    if _name == 'global':
                        _d['global']['category'] = _category = line[0]
                        _d['global']['range-start'] = long(line[1]) << 16
                        _d['global']['range-end'] = (long(line[1]) + 1) << 16
                        _d['global']['id'] = long(line[1])
                    else:
                        _d['items'][_name][line[0]] = long(line[1])

        if _category is not None and _category in _d['items']:
            for _entry in _d['items'][_category].keys():
                _d['items'][_category][_entry] += _d['global']['range-start']
        return _d


class YangHistory_ConfigFile_v2:
    GLOBAL_SECTION = "global"

    def __init__(self, filename):
        self._filename = filename

    def store(self, data, filter_function):
        _config = YangHistory_ConfigFile_Base.get_cfg_parser()
        _config.add_section('global')

        _config.set('global', 'category', data['global']['category'])
        _config.set('global', 'range-start', data['global']['range-start'])
        _config.set('global', 'range-end', data['global']['range-end'])
        _config.set('global', 'id', data['global']['id'])

        for _section in data['items'].keys():
            _config.add_section(_section)
            for _key, _value in sorted(data['items'][_section].iteritems(), key=lambda k_v: (k_v[1], k_v[0])):
                if filter_function(_key):
                    continue

                if _section == data['global']['category']:
                    _value = str(
                        _value) + ' ' + str(
                        _value - data[
                            'global'][
                        'range-start'])
                _config.set(_section, _key, str(_value))

        with open(self._filename, 'w') as file:
            _config.write(file)

    def get(self):
        _config = YangHistory_ConfigFile_Base.get_cfg_parser()
        _config.read(self._filename)

        d = YangHistory_ConfigFile_Base.empty_config_file()

        try:
            d['global']['range-start'] = long(
                _config.get('global', 'range-start'))
            d['global']['range-end'] = long(_config.get('global', 'range-end'))
            d['global']['category'] = _config.get('global', 'category')
            d['global']['id'] = long(_config.get('global', 'id'))

            if len(d['global']['category'])==0:
                for i in _config.sections():
                    if i.find('cps_api_obj_CAT_')==0:
                        d['global']['category'] = i

            for _section in _config.sections():
                if _section == 'global':
                    continue
                d['items'][_section] = {}
                for _elem, _value in _config.items(_section):
                    d['items'][_section][_elem] = long(_value.split(' ')[0])
        except:
            pass

        return d


class YangHistory_HistoryFile:

    """
    Responsible fro opening loading, adding to and closing yang history files.

    The history files maintain the previously allocated IDs.
    """

    LATEST_FILE_VERSION = 2

    def __get_hist_version(self):
        """
        This function will find a way to determine the version of the yhist file.

        note: implemention of the actual version check could be placed as a static function
        of the class for the appropriate version
        """
        _file_version = None
        try:
            with open(self._filename, "r") as _file:
                _open = 0
                _close = 0
                for l in _file:
                    if '++obj' in l:
                        _open += 1
                    if '--obj' in l:
                        _close += 1
                    if '[global]' in l:
                        _file_version = 2
                if _file_version is None and (_open == _close and _open > 0):
                    _file_version = 1
            if _file_version == 2:
                # version 2 and beyond are ini file formats
                _config = YangHistory_ConfigFile_Base.get_cfg_parser()
                _config.read(self._filename)
                _file_version = _config.getint('global', 'file-version')
        except:
            pass

        if _file_version is None:
            return YangHistory_HistoryFile.LATEST_FILE_VERSION

        return _file_version

    def __init__(self, context, filename, category=None):
        """
            Create a history object loaded from a file provided.
            @context contains program details/context
            @filename the name of the file to load or write
            @category the category of this file
        """
        self._context = context
        self._filename = filename
        self._category = category
        self._data = YangHistory_ConfigFile_Base.empty_config_file()

        self._filename_version = self.__get_hist_version()

        if self._filename_version == 1:
            _cfg = YangHistory_ConfigFile_v1(self._filename)
        else:
            _cfg = YangHistory_ConfigFile_v2(self._filename)

        self._data = _cfg.get()

        if 'category' in self._data['global'] and category is None:
            self._category = self._data['global']['category']
            category = self._category

        if self._category is None:
            raise Exception('No category provided - for %s' % filename)
        
        self._data['global']['category'] = self._category
        
        if self._data['global']['range-start'] == 0:
            self._category_value = self._context[
                'history']['category'].get_category(category)
            self._data['global']['id'] = self._category_value
            self._data['global']['range-start'] = self._category_value << 16
            self._data['global']['range-end'] = (
                self._category_value + 1) << 16
            
        else:
            self._category_value = self._data['global']['id']
            self._context[
                'history'][
                    'category'].set_category(
                        category,
                        self._category_value)
        
        self.__modified = False

        for _enum_type in self._data['items'].keys():
            for _enum_name in self._data['items'][_enum_type].keys():
                self.add_enum(_enum_type, _enum_name, None)

    def __get_indexer(self, enum_type):
        if enum_type == self._category:
            return Model_Element_IndexTracker(self._data['items'][enum_type], self._category, self._data['global']['range-start'])

        return Model_Enum_IndexTracker(self._data['items'][enum_type], self._category, 0)

    def __valid_value(self, enum_type, enum_name):
        value = self._data['items'][enum_type][enum_name]

        if enum_type == self._category:
            if value < self._data['global']['range-start'] or value >= self._data['global']['range-end']:
                raise Exception(
                    "Invalid value %d for enum %s " %
                    (value, enum_type))

    def add_enum(self, enum_type, enum_name, enum_value):
        if enum_type not in self._data['items']:
            self._data['items'][enum_type] = {}

        if '..indexer..' not in self._data['items'][enum_type]:
            self._data['items'][enum_type][
                '..indexer..'] = self.__get_indexer(enum_type)

        _indexer = self._data['items'][enum_type]['..indexer..']

        __current_value = _indexer.get_element_value(enum_name)

        _value = _indexer.add_element(enum_name, enum_value)

        self.__valid_value(enum_type, enum_name)
        if _value != __current_value:
            self.__modified = True
        return _value

    def write(self):
        if self.__modified:
            _cfg = YangHistory_ConfigFile_v2(self._filename);
            _cfg.store(self._data, lambda key: key == '..indexer..')
            self.__modified = False
        else:
            if self._context['debug']:
                print ("Indexes haven't changed and therefore no output")


class YangHistory_CategoryParser:

    @staticmethod
    def __get_cfg_parser():
        _config = ConfigParser.ConfigParser()
        _config.optionxform = str
        return _config

    def _load_file_values(self):
        _config = YangHistory_CategoryParser.__get_cfg_parser()

        _config.read(self._filename)

        self.__range_start = long(_config.get('range', 'start'))
        self.__range_end = long(_config.get('range', 'end'))

        for _elem, _value in _config.items('reserved'):
            self.__index.add_element(_elem, long(_value))
        try:
            self._auto_gen_enums = bool(_config.get('range', 'auto-generate'))
        except:
            self._auto_gen_enums = False

        self.__modified = False

    def _init_file_values(self):
        self.__range_start = 10
        self.__range_end = 0
        self._auto_gen_enums = True
        self.__modified = False
        self._write_file = False

        for i in os.listdir(self._context['history']['sources']):
            if '.yhist' not in i:
                continue
            _file = os.path.join(self._context['history']['sources'], i)

            _hist = YangHistory_HistoryFile(self._context, _file)


    def __init__(self, context, filename, fail_if_missing=True):
        """
        This will initialize the module history file.
        @context contains the context of the appliation.  Essentially a map of objects that contain application details
        @filename the name of the history file to parse
        @catagory the catagory if known
        """
        self.__categories = {}
        self.__index = Model_Element_IndexTracker(
            self.__categories, 'global', 0)
        self._context = context
        self._filename = filename
        self._write_file = True
        self._context['history']['category'] = self

        if not os.path.exists(filename):
            if fail_if_missing:
                raise Exception(
                    'Could not open configuration file %s for reading' %
                    filename)
            self._init_file_values()
        else:
            self._load_file_values()

    @staticmethod
    def init_file(filename, range_start, range_end, values, auto_gen_enums):
        """
        Initialize a file from a range start, range end and list/iteratiable tuples(name,value)
        Generally speaking - one category is reserved per yang file
        @range_start the start of the range of numbers
        @range_end the last number in the available range
        @values an interatable set of tuples (name,value)
        """
        _config = YangHistory_CategoryParser.__get_cfg_parser()

        _config.add_section('range')
        _config.add_section('reserved')

        _config.set('range', 'start', range_start)
        _config.set('range', 'end', range_end)
        _config.set('range', 'auto-generate', auto_gen_enums)
        for _elem, _value in values:
            _config.set('reserved', _elem, str(_value))

        with open(filename, 'w') as f:
            _config.write(f)

    def __get_next_free(self):
        if self._auto_gen_enums:
            _max = 0
            for i in self.__categories.itervalues():
                i = int(i)
                if i > _max:
                    _max = i
            return _max + 1

        for i in range(self.__range_start, self.__range_end):
            if i in self.__categories.itervalues():
                continue
            return i
        raise Exception(
            'No space left in category range for model.  Please specify manually or update %s' %
            self._filename)

    def write(self):
        """Write out the category tracker file"""
        if self._write_file is False:
            return

        if self.__modified is True:
            YangHistory_CategoryParser.init_file(self._filename,
                        self.__range_start, self.__range_end, self.__categories.iteritems(), self._auto_gen_enums)
        else:
            if self._context['debug']:
                print ("Indexes haven't changed and therefore no output")

    def set_category(self, name, value):
        if value is None and name not in self.__categories:
            value = self.__get_next_free()

        if name not in self.__categories and value is not None:
            self.__modified = True
            self.__categories[name] = value

        return self.__categories[name]

    def get_category(self, name):
        """
        Get the existing value for a category or create a new entry and use the category value
        """
        if name in self.__categories:
            return self.__categories[name]

        return self.set_category(name, None)


def init(context, file, category):
    h = YangHistory_HistoryFile(context, file, category)
    return h


def close(h):
    h.write()

if __name__ == '__main__':
    # hf = history(sys.argv[1])
    __context = {}
    __context['history'] = {}
    __context['history']['category'] = YangHistory_CategoryParser(
        __context, '/tmp/category.txt')
    __init_values = [('ONE', 1), ('TWO', 2)]

    _category_test = __context['history']['category']

    for _name, _value in __init_values:
        _tmp_val = _category_test.set_category(_name, _value)

    __added_values = []
    for i in range(0, 30):
        __added_values.append(
            ('entry_' + str(i), _category_test.get_category('entry_' + str(i))))

    _category_test.write()

    __context['history']['category'] = YangHistory_CategoryParser(
        __context, '/tmp/category.txt')
    _category_test = __context['history']['category']

    for t in __added_values:
        if t[1] != _category_test.get_category(t[0]):
            raise Exception('Invalid value... - test failed')

    _hist = YangHistory_HistoryFile(
        __context,
        '/tmp/dell-base-cps.yhist',
        'cps_api_obj_CAT_CPS');

    _hist.add_enum('cps_api_obj_CAT_CPS', "Cliff", None)
    _hist.write()
    _category_test.write()
