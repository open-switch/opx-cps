import cps
import time

def get_cb(methods, params):
    print "Get..."
    for entries in params['keys']:
        print entries
        d = {}
        d['node'] = 'Cliff'
        params['result'][entries] = d

    return True

def trans_cb(methods, params):
    print "Trans..."
    print params['operation']

    if params['operation'] == 'set':
        for i in params['change'].keys():
            params['change'][i]['node'] = "Clifford"
            params['change'][i]['time'] = time.asctime()
    print params
    return True

if __name__=='__main__':
    handle = cps.obj_init()
    d = {}
    d['get'] = get_cb
    d['transaction'] = trans_cb

    cps.obj_register(handle,'1.2.3.4',d)
    cps.config('10','2.3','pyobj','',True,"node")
    cps.config('11','2.3.1','pyobj/node','',False,"node")
    cps.config('12','2.3.2','pyobj/time','',False,"node")
    while True:
        time.sleep(1)


