#!/usr/bin/env python

import yajl
import sys

for i, l in enumerate(sys.stdin):
    l = l.rstrip('\n').split('\t')
    d = yajl.dumps(tuple(l))
