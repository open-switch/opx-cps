


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


