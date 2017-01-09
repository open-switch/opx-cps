#!/usr/bin/python

import os
import subprocess
import sys

def find_subdir(base, name):
    target = os.path.join(base,name)
    
    if os.path.exists(target):
        return target

    for i in os.listdir(base):
        if i==name:
            return os.path.join(base,i)
        rel = os.path.join(base,i)
        if os.path.isdir(rel):
            try:
                rc = find_subdir(rel,name)
                if rc!='':
                    return rc
            except:
                pass
    return ''


def run_program(program,ldp,pyp,metadata):
    print('LD_LIBRARY_PATH='+ldp)
    print('PYTHONPATH='+pyp)
    cur_env = os.environ.copy()
    p = subprocess.Popen(program,
            env=dict(cur_env,**{
                        'LD_LIBRARY_PATH':ldp,
                        'PYTHONPATH':pyp,
                        'CPS_API_METADATA_PATH':pyp}),
          stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    while True:
      line = p.stdout.readline()
      if not line: break
      print (line.rstrip())


def main():
    _cmd = sys.argv
    del _cmd[0]
    app = _cmd[0]


    p = subprocess.Popen(['ar_tool.py','sysroot'],stdout=subprocess.PIPE)
    root = p.communicate()[0].strip()
    relative = os.getcwd()    
    relative = relative[len(root)+1:]

    print ('Module - '+relative)

    workspace = os.path.join(root,'workspace')
    sysroot = find_subdir(workspace,'sysroot')
    sysroot_dev = find_subdir(workspace,'sysroot-dev')
    
    buildroot = find_subdir(workspace,'build')
    app_buildroot = os.path.join(buildroot,relative)
    app_sysroot = find_subdir(app_buildroot,'sysroot')

    app_sysroot_bin = find_subdir(app_sysroot,'bin')
    app_sysroot_lib = find_subdir(app_sysroot,'lib')
    app_sysroot_tests = find_subdir(app_sysroot,'tests')

    app_sos = find_subdir(buildroot,relative+'_sos')
    app_apps = find_subdir(buildroot,relative+'_apps')
    
    _lib_path=app_sos+":"+app_sysroot_lib+":"+sysroot_dev+"/usr/lib/x86_64-linux-gnu"+':'+sysroot+"/opt/dell/os10/lib"+':'+sysroot+'/opt/dell/os10/lib/cpsmetadata'

    print('Libs... '+_lib_path)
    
    if app == 'python':
        run_program(['/usr/bin/python'],_lib_path,_lib_path,_lib_path)
        sys.exit(0);
     
    bin=''
    for i in [ app_apps, app_sysroot_bin,app_sysroot_tests ] :
        _dir = find_subdir(i,app)
        if _dir!='':
            bin = _dir
            break

    if bin.find('.py')!=-1:
        _cmd.insert(0,'/usr/bin/python')
        _cmd[1] = bin
    else :
        _cmd[0] =  bin

    print('Running.. ',_cmd)
    run_program(_cmd,_lib_path,_lib_path,_lib_path);

if __name__ == "__main__":
    main()
