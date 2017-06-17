import subprocess

def run_cmd(args):
    p = subprocess.Popen(args, stdout=subprocess.PIPE)
    stdout = p.communicate()[0]
    wv = p.wait()
    if wv != 0:            
        print('%s failed = rc %d' % (' '.join(args),wv))
        print('Will not terminate parsing - attempting to continue.')