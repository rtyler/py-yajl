#!/usr/bin/env python

import os
import subprocess
import sys

USE_SETUPTOOLS = False
try:
    from setuptools import setup, Extension
    USE_SETUPTOOLS = True
except ImportError:
    from distutils.core import setup, Extension

base_modules = [
    Extension('yajl',  [
                'yajl.c',
                'encoder.c',
                'decoder.c',
                'yajl/src/yajl_alloc.c',
                'yajl/src/yajl_buf.c',
                'yajl/src/yajl.c',
                'yajl/src/yajl_encode.c',
                'yajl/src/yajl_gen.c',
                'yajl/src/yajl_lex.c',
                'yajl/src/yajl_parser.c',
            ],
            include_dirs=('.', 'includes/', 'yajl/src'),
            extra_compile_args=['-Wall',],
            language='c'),
        ]


packages = ('yajl',)


setup_kwargs = dict(
    name = 'yajl',
    description = '''A CPython module for Yet-Another-Json-Library''',
    version = '0.3.3',
    author = 'R. Tyler Ballance',
    author_email = 'tyler@monkeypox.org',
    url = 'http://rtyler.github.com/py-yajl',
    long_description='''The `yajl` module provides a Python binding to the Yajl library originally written by `Lloyd Hilaiel <http://github.com/lloyd>`_.

Mailing List
==============
You can discuss the C library **Yajl** or py-yajl on the Yajl mailing list,
simply send your email to yajl@librelist.com
    ''',
    ext_modules=base_modules,
)

if USE_SETUPTOOLS:
    setup_kwargs.update({'test_suite' : 'tests.unit'})

if not os.listdir('yajl'):
    # Submodule hasn't been created, let's inform the user
    print('>>> It looks like the `yajl` submodule hasn\'t been initialized')
    print('>>> I\'ll try to do that, but if I fail, you can run:')
    print('>>>      `git submodule update --init`')
    subprocess.call(['git', 'submodule', 'update', '--init'])

if not os.path.exists('includes'):
    # Our symlink into the yajl directory isn't there, let's fixulate that
    os.mkdir('includes')

if not os.path.exists(os.path.join('includes', 'yajl')):
    print('>>> Creating a symlink for compilationg: includes/yajl -> yajl/src/api')
    # Now that we have a directory, we need a symlink
    os.symlink(os.path.join('..', 'yajl', 'src', 'api'), os.path.join('includes', 'yajl'))

setup(**setup_kwargs)
