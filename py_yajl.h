/*
 * Copyright 2009, R. Tyler Ballance <tyler@monkeypox.org>
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 * 
 *  3. Neither the name of R. Tyler Ballance nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */ 

#ifndef _PY_YAJL_H_
#define _PY_YAJL_H_

#include <Python.h>
#include <yajl/yajl_gen.h>
#include "ptrstack.h"

#if PY_MAJOR_VERSION >= 3
#define IS_PYTHON3
#endif

typedef struct {
    PyObject_HEAD

    py_yajl_bytestack elements;
    py_yajl_bytestack keys;
    PyObject *root;

} _YajlDecoder;

typedef struct {
    PyObject_HEAD
    /* type specifics */
    void *_generator;
} _YajlEncoder;

#define PYARGS PyObject *self, PyObject *args, PyObject *kwargs
enum { failure, success };

#define PY_YAJL_CHUNK_SZ 64

/* Defining the Py_SIZE macro for 2.4/2.5 compat */
#ifndef Py_SIZE
#define Py_SIZE(ob)     (((PyVarObject*)(ob))->ob_size)
#endif

/* 
 * PyUnicode_FromStringAndSize isn't defined in Python 2.4/2.5; IIRC 
 * JSON strings /should/ be UTF-8 encoded, so PyUnicode_DecodeUTF8() 
 * seems like the most logical extension
 */
#ifndef PyUnicode_FromStringAndSize
#define PyUnicode_FromStringAndSize(a, b)  PyUnicode_DecodeUTF8(a, b, NULL)
#endif
#ifndef PyUnicode_FromString
#define PyUnicode_FromString(s) PyUnicode_DecodeUTF8(s, strlen(s), NULL)
#endif

/* On Python 2.4 Py_ssize_t doesn't exist */
#ifndef Py_ssize_t
#define Py_ssize_t ssize_t
#endif

/*
 * Methods defined for the YajlDecoder type in decoder.c
 */
extern PyObject *py_yajldecoder_decode(PYARGS);
extern int yajldecoder_init(PYARGS);
extern void yajldecoder_dealloc(_YajlDecoder *self);
extern PyObject *_internal_decode(_YajlDecoder *self, char *buffer, unsigned int buflen);


/*
 * Methods defined for the YajlEncoder type in encoder.c
 */
extern PyObject *py_yajlencoder_encode(PYARGS);
extern int yajlencoder_init(PYARGS);
extern void yajlencoder_dealloc(_YajlEncoder *self);
extern PyObject *_internal_encode(_YajlEncoder *self, PyObject *obj, yajl_gen_config config);

#endif

