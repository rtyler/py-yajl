import time

import pickle
import string
import yajl
try:
    import cjson
except ImportError:
    cjson = None
try:
    import simplejson
except ImportError:
    simplejson = None
try:
    import json
except ImportError:
    json = None

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


def format(flt, prec=3):
    s = str(round(flt, prec))
    return padright(s, s.index(".") + 4, "0")


def padright(s, upto, padchar=" "):
    return s + (padchar * (upto - len(s)))


contenders = [
    ('yajl', (yajl.Encoder().encode, yajl.Decoder().decode)),
]
if cjson:
    contenders.append(('cjson', (cjson.encode, cjson.decode)))
if simplejson:
    contenders.append(('simplejson', (simplejson.dumps, simplejson.loads)))
if json:
    contenders.append(('stdlib json', (json.dumps, json.loads)))

tmpl = string.Template("$name serialize: $ser  deserialize: $des   total: $tot")
for name, args in contenders:
    test(*args)
    x, y = profile(*args)
    print(tmpl.substitute(
        name=padright(name, 11),
        ser=format(x),
        des=format(y),
        tot=format(x + y)
    ))
