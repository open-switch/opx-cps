import os
import tempfile
import shutil


class Directory:

    def __init__(self):
        self.__tmpdir = tempfile.mkdtemp()

    def clean(self):
        shutil.rmtree(self.__tmpdir)

    def get_path(self):
        return self.__tmpdir

    def make_path(self, filename):
        return os.path.join(self.__tmpdir, filename)
