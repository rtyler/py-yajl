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


#include <Python.h>

#include <string.h>

#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>

#include "py_yajl.h"

int _PlaceObject(_YajlDecoder *self, PyObject *parent, PyObject *child)
{
    if ( (!self) || (!child) || (!parent) )
        return failure;

    if (PyList_Check(parent)) {
        PyList_Append(parent, child);
        // child is now owned by parent!
        Py_XDECREF(child);
        return success;
    } else if (PyDict_Check(parent)) {
        PyObject* key = py_yajl_ps_current(self->keys);
        PyDict_SetItem(parent, key, child);
        py_yajl_ps_pop(self->keys);
        // child is now owned by parent!
        Py_XDECREF(key);
        Py_XDECREF(child);
        return success;
    }
    return failure;
}

int PlaceObject(_YajlDecoder *self, PyObject *object)
{
    unsigned int length;

    /* if (self->elements == NULL) */
    /*     return failure; */

    length = py_yajl_ps_length(self->elements);

    if (length == 0) {
        /*
         * When the length is zero, and we're entering this code path
         * we should only be handling "primitive types" i.e. strings and
         * numbers, not dict/list.
         */
        self->root = object;
        return success;
    }
    return _PlaceObject(self, py_yajl_ps_current(self->elements), object);
}


static int handle_null(void *ctx)
{
    _YajlDecoder *self = (_YajlDecoder *)(ctx);
    int rc = -1;

    rc = PlaceObject(self, Py_None);

    if (rc == success)
        return success;
    return failure;
}

static int handle_bool(void *ctx, int value)
{
    _YajlDecoder *self = (_YajlDecoder *)(ctx);
    PyObject *object = PyBool_FromLong((long)(value));

    return PlaceObject(self, object);
}

static int handle_number(void *ctx, const char *value, unsigned int length)
{
    _YajlDecoder *self = (_YajlDecoder *)(ctx);
    PyObject *string, *object;

    string = PyString_FromStringAndSize(value, length);
    object = PyFloat_FromString(string, NULL);

    Py_XDECREF(string);
    
    return PlaceObject(self, object);
}

static int handle_string(void *ctx, const unsigned char *value, unsigned int length)
{
    _YajlDecoder *self = (_YajlDecoder *)(ctx);
    PyObject *object = PyString_FromStringAndSize((char *)(value), length);

    return PlaceObject(self, object);
}

static int handle_start_dict(void *ctx)
{
    _YajlDecoder *self = (_YajlDecoder *)(ctx);
    PyObject *object = PyDict_New();

    if (!object)
        return failure;

    py_yajl_ps_push(self->elements, object);
    return success;;
}

static int handle_dict_key(void *ctx, const unsigned char *value, unsigned int length)
{
    _YajlDecoder *self = (_YajlDecoder *)(ctx);
    PyObject *object = NULL;

    object = PyString_FromStringAndSize((const char *) value, length);

    if (NULL == object)
        return failure;

    py_yajl_ps_push(self->keys, object);
    return success;
}

static int handle_end_dict(void *ctx)
{
    _YajlDecoder *self = (_YajlDecoder *)(ctx);
    PyObject *last, *popped;
    unsigned int length;

    length = py_yajl_ps_length(self->elements);
    if (length == 1) {
        /* 
         * If this is the last element in the stack
         * then it's "root" and we should finish up
         */
        self->root = py_yajl_ps_current(self->elements);    
        py_yajl_ps_pop(self->elements);    
        return success;
    } else if (length < 2) {
        return failure;
    }
    
    /*
     * If not, then we should properly add this dict
     * to it's appropriate parent
     */
    popped = py_yajl_ps_current(self->elements);    
    py_yajl_ps_pop(self->elements);    
    last = py_yajl_ps_current(self->elements);

    return _PlaceObject(self, last, popped);
}

static int handle_start_list(void *ctx)
{
    _YajlDecoder *self = (_YajlDecoder *)(ctx);
    PyObject *object = PyList_New(0);

    if (!object)
        return failure;

    py_yajl_ps_push(self->elements, object);
    return success;
}

static int handle_end_list(void *ctx)
{
    _YajlDecoder *self = (_YajlDecoder *)(ctx);
    PyObject *last, *popped;
    unsigned int length;

    length = py_yajl_ps_length(self->elements);
    if (length == 1) {
        self->root = py_yajl_ps_current(self->elements);    
        py_yajl_ps_pop(self->elements);    
        return success;
    } else if (length < 2) {
        return failure;
    }
    
    popped = py_yajl_ps_current(self->elements);    
    py_yajl_ps_pop(self->elements);    
    last = py_yajl_ps_current(self->elements);

    return _PlaceObject(self, last, popped);
}

static yajl_callbacks decode_callbacks = {
    handle_null,
    handle_bool, 
    NULL, 
    NULL,
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
    char *buffer = NULL;
    unsigned int buflen = 0;
    yajl_handle parser = NULL;
    yajl_status yrc;
    yajl_parser_config config = { 1, 1 };

    if (!PyArg_ParseTuple(args, "z#", &buffer, &buflen))
        return NULL;

    if (!buflen) {
        PyErr_SetObject(PyExc_ValueError, 
                PyString_FromString("Cannot parse an empty buffer"));
        return NULL;
    }

    if (decoder->elements.stack) {
        py_yajl_ps_free(decoder->elements);
        py_yajl_ps_init(decoder->elements);
    }
    if (decoder->keys.stack) {
        py_yajl_ps_free(decoder->keys);
        py_yajl_ps_init(decoder->keys);
    }

    /* callbacks, config, allocfuncs */
    parser = yajl_alloc(&decode_callbacks, &config, NULL, (void *)(self));
    yrc = yajl_parse(parser, (const unsigned char *)(buffer), buflen);
    yajl_parse_complete(parser);
    yajl_free(parser);

    if (yrc != yajl_status_ok) {
        PyErr_SetObject(PyExc_ValueError, 
                PyString_FromString(yajl_status_to_string(yrc)));
        return NULL;
    }

    if (decoder->root == NULL) {
        PyErr_SetObject(PyExc_ValueError, 
                PyString_FromString("The root object is NULL"));
        return NULL;
    }
    
    // important! callee owned!
    Py_XINCREF(decoder->root);

    return decoder->root;
}

PyObject *py_yajldecoder_raw_decode(PYARGS)
{
    return NULL;
}

int yajldecoder_init(PYARGS)
{
    _YajlDecoder *me = (_YajlDecoder *)(self);
    py_yajl_ps_init(me->elements);
    py_yajl_ps_init(me->keys);
    me->root = NULL;

    return 0;
}

void yajldecoder_dealloc(_YajlDecoder *self)
{
    py_yajl_ps_free(self->elements);
    py_yajl_ps_init(self->elements);
    py_yajl_ps_free(self->keys);
    py_yajl_ps_init(self->keys);
    Py_XDECREF(self->root);
    self->ob_type->tp_free((PyObject*)self);
}
