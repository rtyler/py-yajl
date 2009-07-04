/*
 * Copyright 2009, R. Tyler Ballance <tyler@slide.com>
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
 *  3. Neither the name of R. Tyler Ballancenor the names of its
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

#include <Python.h>

#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>

#include "py_yajl.h"

enum { failure, success };

PyObject *LastElement(_YajlDecoder *self)
{
    unsigned int length = 0;

    if (self->elements == NULL)
        return NULL;

    length = (int)(PyList_Size(self->elements));
    if (!length)
        return NULL;

    return PyList_GetItem(self->elements, length - 1);
}

int PlaceObject(_YajlDecoder *self, PyObject *object)
{
    PyObject *root = LastElement(self);
    
    if (root == NULL) {
        PyList_Append(self->elements, object);
        return success;
    }

    if (PyList_Check(root)) {
        PyList_Append(root, object);
        return success;
    }

    if (PyDict_Check(root)) {
        if (!self->key)
            return failure;

        PyDict_SetItem(root, self->key, object);
        Py_XDECREF(self->key);
        self->key = NULL;
        return success;
    }
    return failure; 
}


int handle_null(void *ctx)
{
    _YajlDecoder *self = (_YajlDecoder *)(ctx);
    PyObject *object = Py_None;
    int rc = -1;

    Py_INCREF(Py_None);

    rc = PlaceObject(self, object);

    if (rc == success)
        return success;
    Py_DECREF(Py_None);
    return failure;
}

int handle_int(void *ctx, long value)
{
    _YajlDecoder *self = (_YajlDecoder *)(ctx);
    PyObject *object = PyInt_FromLong(value);

    return PlaceObject(self, object);
}

int handle_bool(void *ctx, int value)
{
    _YajlDecoder *self = (_YajlDecoder *)(ctx);
    PyObject *object = PyBool_FromLong((long)(value));

    return PlaceObject(self, object);
}

int handle_double(void *ctx, double value)
{
    _YajlDecoder *self = (_YajlDecoder *)(ctx);
    PyObject *object = PyFloat_FromDouble(value);

    return PlaceObject(self, object);
}

int handle_number(void *ctx, const char *value, unsigned int length)
{
    _YajlDecoder *self = (_YajlDecoder *)(ctx);
    PyObject *object = PyInt_FromString((char *)(value), NULL, 10);

    return PlaceObject(self, object);
}

int handle_string(void *ctx, const unsigned char *value, unsigned int length)
{
    _YajlDecoder *self = (_YajlDecoder *)(ctx);
    PyObject *object = PyString_FromStringAndSize((char *)(value), length);

    return PlaceObject(self, object);
}

int handle_start_dict(void *ctx)
{
    _YajlDecoder *self = (_YajlDecoder *)(ctx);
    PyObject *object = PyDict_New();

    if (!object)
        return failure;

    PyList_Append(self->elements, object);
    return success;
}

int handle_dict_key(void *ctx, const unsigned char *value, unsigned int length)
{
    _YajlDecoder *self = (_YajlDecoder *)(ctx);
    PyObject *object = PyString_FromStringAndSize((char *)(value), length);

    if (!object)
        return failure;

    self->key = object;
    return success;
}

int handle_end_dict(void *ctx)
{
    _YajlDecoder *self = (_YajlDecoder *)(ctx);
    if (!self->elements)
        return failure;
    return success;
}

int handle_start_list(void *ctx)
{
    _YajlDecoder *self = (_YajlDecoder *)(ctx);
    PyObject *object = PyList_New(0);

    if (!object)
        return failure;

    PyList_Append(self->elements, object);
    return success;
}

int handle_end_list(void *ctx)
{
    _YajlDecoder *self = (_YajlDecoder *)(ctx);
    if (!self->elements)
        return failure;
    return success;
}

static yajl_callbacks decode_callbacks = {
    handle_null,
    handle_bool, 
    handle_int, 
    handle_double,
    handle_number,
    handle_string,
    handle_start_dict,
    handle_dict_key,
    handle_end_dict,
    handle_start_list, 
    handle_end_list
};

PyObject *py_yajldecoder_decode(PYARGS)
{
    _YajlDecoder *decoder = (_YajlDecoder *)(self);
    PyObject *root = NULL;
    char *buffer = NULL;
    unsigned int buflen = 0;
    yajl_handle parser = NULL;
    yajl_status yrc;
    yajl_parser_config config = { 1, 1 };

    if (!PyArg_ParseTuple(args, "z#", &buffer, &buflen))
        return NULL;

    if (!buflen) {
        /* Raise some sort of exception? */
        return NULL;
    }

    if (decoder->elements) {
        Py_XDECREF(decoder->elements);
        decoder->elements = PyList_New(0);
    }

    /* callbacks, config, allocfuncs */
    parser = yajl_alloc(&decode_callbacks, &config, NULL, (void *)(self));

    yrc = yajl_parse(parser, (const unsigned char *)(buffer), buflen);
    yajl_parse_complete(parser);

    if (yrc != yajl_status_ok) {
        /* Raise some sort of exception */
        return NULL;
    }
    
    root = PyList_GetItem(decoder->elements, 0);
    if (!root) 
        return NULL;
    return root; 
}

PyObject *py_yajldecoder_raw_decode(PYARGS)
{
    return NULL;
}

int yajldecoder_init(PYARGS)
{
    _YajlDecoder *me = (_YajlDecoder *)(self);

    /* Set to NULL so the parser callbacks will function properly */
    me->elements = PyList_New(0);
    me->key = NULL;

    return 0;
}
