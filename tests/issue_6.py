#!/usr/bin/env python

import sys
import yajl as json


def foo():

    for l in sys.stdin:
        l = l.rstrip('\n')
        t = json.loads(l)
        yield t

if __name__ == '__main__':
    i = 0
    for r in foo():
        i += 1
        if i % 1000 == 0:
            print '.',
