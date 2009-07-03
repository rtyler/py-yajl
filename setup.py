import os
from distutils.core import setup, Extension

base_modules = [
    Extension('yajl', [
                'yajl.c',
            ],
            libraries=['yajl'],
            extra_compile_args=['-Wall', '-Werror', '-I../src/api']),
]

packages = ['yajl']


setup(
    name = 'yajl',
    description = '''A CPython module for Yet-Another-Json-Library''',
    version = '1.0',
    author = 'R. Tyler Ballance',
    author_email = 'tyler@slide.com',
    ext_modules=base_modules,
    py_modules=[])


