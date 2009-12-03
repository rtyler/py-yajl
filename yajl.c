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

#include "py_yajl.h"

static PyMethodDef yajldecoder_methods[] = {
    {"decode", (PyCFunction)(py_yajldecoder_decode), METH_VARARGS, NULL},
    {NULL}
};

static PyMethodDef yajlencoder_methods[] = {
    {"encode", (PyCFunction)(py_yajlencoder_encode), METH_VARARGS, NULL},
    {NULL}
};

static PyTypeObject YajlDecoderType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "yajl.YajlDecoder",        /*tp_name*/
    sizeof(_YajlDecoder),      /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)yajldecoder_dealloc,       /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "Yajl-based decoder",      /* tp_doc */
    0,                     /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    yajldecoder_methods,  /* tp_methods */
    NULL,                 /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)(yajldecoder_init),/* tp_init */
    0,                         /* tp_alloc */
}; 

static PyTypeObject YajlEncoderType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "yajl.YajlEncoder",        /*tp_name*/
    sizeof(_YajlEncoder),      /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)yajlencoder_dealloc,       /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "Yajl-based encoder",      /* tp_doc */
    0,                     /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    yajlencoder_methods,  /* tp_methods */
    NULL,                 /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)(yajlencoder_init),/* tp_init */
    0,                         /* tp_alloc */
}; 

static PyObject *py_loads(PYARGS)
{
    PyObject *decoder = NULL;
    PyObject *result = NULL;
    char *buffer = NULL;
    unsigned int buflen = 0;

    if (!PyArg_ParseTuple(args, "z#", &buffer, &buflen)) {
        return NULL;
    }
    
    decoder = PyObject_Call((PyObject *)(&YajlDecoderType), NULL, NULL);
    if (decoder == NULL) {
        return NULL;
    }

    result = _internal_decode((_YajlDecoder *)decoder, buffer, buflen);
    Py_XDECREF(decoder);
    return result;
}

static PyObject *py_dumps(PYARGS)
{
    PyObject *encoder = NULL;
    PyObject *obj = NULL;
    PyObject *result = NULL;

    if (!PyArg_ParseTuple(args, "O", &obj)) {
        return NULL;
    }

    encoder = PyObject_Call((PyObject *)(&YajlEncoderType), NULL, NULL);
    if (encoder == NULL) {
        return NULL;
    }

    result = _internal_encode((_YajlEncoder *)encoder, obj);
    Py_XDECREF(encoder);
    return result;
}

static PyObject *__read = NULL;
static PyObject *_internal_stream_load(PyObject *args, unsigned int blocking)
{
    PyObject *decoder = NULL;
    PyObject *stream = NULL;
    PyObject *buffer = NULL;
    PyObject *result = NULL;

    if (!PyArg_ParseTuple(args, "O", &stream)) {
        goto bad_type;
    }

    if (__read == NULL) {
        __read = PyString_FromString("read");
    }

    if (!PyObject_HasAttr(stream, __read)) {
        goto bad_type;
    }

    buffer = PyObject_CallMethodObjArgs(stream, __read, NULL);

    if (!buffer)
        return NULL;

    decoder = PyObject_Call((PyObject *)(&YajlDecoderType), NULL, NULL);
    if (decoder == NULL) {
        return NULL;
    }

    result = _internal_decode((_YajlDecoder *)decoder, PyString_AsString(buffer),
                  PyString_Size(buffer));
    Py_XDECREF(decoder);
    Py_XDECREF(buffer);
    return result;

bad_type:
    PyErr_SetObject(PyExc_TypeError, PyString_FromString("Must pass a single stream object"));
    return NULL;
}

static PyObject *py_load(PYARGS)
{
    return _internal_stream_load(args, 1);
}
static PyObject *py_iterload(PYARGS)
{
    return _internal_stream_load(args, 0);
}

static PyObject *__write = NULL;
static PyObject *_internal_stream_dump(PyObject *args, unsigned int blocking)
{
    PyObject *encoder = NULL;
    PyObject *stream = NULL;
    PyObject *buffer = NULL;
    PyObject *object = NULL;

    if (!PyArg_ParseTuple(args, "OO", &object, &stream)) {
        goto bad_type;
    }

    if (__write == NULL) {
        __write = PyString_FromString("write");
    }

    if (!PyObject_HasAttr(stream, __write)) {
        goto bad_type;
    }

    encoder = PyObject_Call((PyObject *)(&YajlEncoderType), NULL, NULL);
    if (encoder == NULL) {
        return NULL;
    }

    buffer = _internal_encode((_YajlEncoder *)encoder, object);
    PyObject_CallMethodObjArgs(stream, __write, buffer, NULL);
    Py_XDECREF(encoder);
    return Py_True;

bad_type:
    PyErr_SetObject(PyExc_TypeError, PyString_FromString("Must pass a stream object"));
    return NULL;
}
static PyObject *py_dump(PYARGS)
{
    return _internal_stream_dump(args, 0);
}

static struct PyMethodDef yajl_methods[] = {
    {"dumps", (PyCFunction)(py_dumps), METH_VARARGS, 
"yajl.dumps(obj)\n\n\
Returns an encoded JSON string of the specified `obj`"},
    {"loads", (PyCFunction)(py_loads), METH_VARARGS, 
"yajl.loads(string)\n\n\
Returns a decoded object based on the given JSON `string`"},
    {"load", (PyCFunction)(py_load), METH_VARARGS, 
"yajl.load(fp)\n\n\
Returns a decoded object based on the JSON read from the `fp` stream-like\n\
object; *Note:* It is expected that `fp` supports the `read()` method"},
    {"dump", (PyCFunction)(py_dump), METH_VARARGS, 
"yajl.dump(obj, fp)\n\n\
Encodes the given `obj` and writes it to the `fp` stream-like object. \n\
*Note*: It is expected that `fp` supports the `write()` method"},
    /*
     {"iterload", (PyCFunction)(py_iterload), METH_VARARGS, NULL},
     */
    {NULL}
};



PyMODINIT_FUNC inityajl(void)
{

    PyObject *module = Py_InitModule3("yajl", yajl_methods, 
"Providing a pythonic interface to the yajl (Yet Another JSON Library) parser\n\n\
The interface is similar to that of simplejson or jsonlib providing a consistent syntax for JSON\n\
encoding and decoding. Unlike simplejson or jsonlib, yajl is **fast** :)\n\n\
The following benchmark was done on a dual core MacBook Pro with a fairly large (100K) JSON document:\n\
json.loads():\t\t21351.313ms\n\
simplejson.loads():\t1378.6492ms\n\
yajl.loads():\t\t502.4572ms\n\
\n\
json.dumps():\t\t7760.6348ms\n\
simplejson.dumps():\t930.9748ms\n\
yajl.dumps():\t\t681.0221ms");

    YajlDecoderType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&YajlDecoderType) < 0)
        return;

    Py_INCREF(&YajlDecoderType);
    PyModule_AddObject(module, "Decoder", (PyObject *)(&YajlDecoderType));

    YajlEncoderType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&YajlEncoderType) < 0)
        return;

    Py_INCREF(&YajlEncoderType);
    PyModule_AddObject(module, "Encoder", (PyObject *)(&YajlEncoderType));
}

