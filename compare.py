import time

#from pymongo.bson import BSON
#from mod import wbin
import cPickle
import pickle
import yajl
import cjson
import simplejson

default_data = {
    "name": "Foo",
    "type": "Bar",
    "count": 1,
    "info": {
        "x": 203,
        "y": 102,},}


def ttt(f, data=None, x=100*1000):
    start = time.time()
    while x:
        x -= 1
        foo = f(data)
    return time.time()-start


def profile(serial, deserial, data=None, x=100*1000):
    if not data:
        data = default_data
    squashed = serial(data)
    return (ttt(serial, data, x), ttt(deserial, squashed, x))


def test(serial, deserial, data=None):
    if not data:
        data = default_data
    assert deserial(serial(data)) == data


contenders = [
    # ('pickle', (pickle.dumps, pickle.loads)),
    #('bson', (BSON.from_dict, lambda x: BSON(x).to_dict())),
    #('wbin', (wbin.serialize, wbin.deserialize)),
    #('cPickle', (cPickle.dumps, cPickle.loads)),
    ('yajl', (lambda x: yajl.Encoder().encode(x), lambda x: yajl.Decoder().decode(x))),
    ('cjson', (cjson.encode, cjson.decode)),
    ('simplejson', (simplejson.dumps, simplejson.loads)),
]

for name, args in contenders:
    test(*args)
    x, y = profile(*args)
    print "%-10s serialize: %0.3f  deserialize: %0.3f  total: %0.3f" % (
        name, x, y, x+y)
