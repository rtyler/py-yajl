import os
from distutils.core import setup, Extension

base_modules = [
    Extension('yajl', [
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
            include_dirs=['includes/'],
            extra_compile_args=['-Wall', '-Werror']),
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


