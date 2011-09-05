#!/bin/sh

python setup.py build &&
PYTHONPATH=.:build/lib.linux-x86_64-2.6 python tests/unit.py &&
zcat test_data/issue_11.gz| PYTHONPATH=build/lib.linux-x86_64-2.6 ./tests/issue_11.py && 

python3 setup.py build && 
PYTHONPATH=build/lib.linux-x86_64-3.1 python3 tests/unit.py &&
zcat test_data/issue_11.gz| PYTHONPATH=build/lib.linux-x86_64-3.1 python3 ./tests/issue_11.py 
