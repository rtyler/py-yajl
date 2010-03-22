
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

#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>
#include <yajl_alloc.h>
#include <yajl_buf.h>

#include "py_yajl.h"

static yajl_gen_status ProcessObject(_YajlEncoder *self, PyObject *object)
{
    yajl_gen handle = (yajl_gen)(self->_generator);
    yajl_gen_status status = yajl_gen_in_error_state;
    PyObject *iterator, *item;
    unsigned short int decref = 0;

    if (object == Py_None) {
        return yajl_gen_null(handle);
    }
    if (object == Py_True) {
        return yajl_gen_bool(handle, 1);
    }
    if (object == Py_False) {
        return yajl_gen_bool(handle, 0);
    }
    if (PyUnicode_Check(object)) {
        object = PyUnicode_AsUTF8String(object);
        decref = 1;
    }
#ifdef IS_PYTHON3
    if (PyBytes_Check(object)) {
#else
    if (PyString_Check(object)) {
#endif
        const unsigned char *buffer = NULL;
        Py_ssize_t length;
#ifdef IS_PYTHON3
        PyBytes_AsStringAndSize(object, (char **)&buffer, &length);
#else
        PyString_AsStringAndSize(object, (char **)&buffer, &length);
#endif
        status = yajl_gen_string(handle, buffer, (unsigned int)(length));
        if (decref) {
            Py_XDECREF(object);
        }
        return status;
    }
#ifndef IS_PYTHON3
    if (PyInt_Check(object)) {
        return yajl_gen_integer(handle, PyInt_AsLong(object));
    }
#endif
    if (PyLong_Check(object)) {
        return yajl_gen_integer(handle, PyLong_AsLong(object));
    }
    if (PyFloat_Check(object)) {
        return yajl_gen_double(handle, PyFloat_AsDouble(object));
    }
    if (PyList_Check(object)) {
        /*
         * Recurse and handle the list
         */
        iterator = PyObject_GetIter(object);
        if (iterator == NULL)
            goto exit;
        status = yajl_gen_array_open(handle);
        while ((item = PyIter_Next(iterator))) {
            status = ProcessObject(self, item);
            Py_XDECREF(item);
        }
        Py_XDECREF(iterator);
        yajl_gen_status close_status = yajl_gen_array_close(handle);
        if (status == yajl_gen_in_error_state)
            return status;
        return close_status;
    }
    if (PyTuple_Check(object)) {
        /*
         * If we have a tuple, convert to a list
         */
        Py_ssize_t size = PyTuple_Size(object);
        PyObject *converted = PyList_New(size);
        unsigned int i = 0;

        for (; i < size; ++i) {
            PyList_SET_ITEM(converted, (Py_ssize_t)(i), PyTuple_GetItem(object, i));
        }
        return ProcessObject(self, converted);
    }
    if (PyDict_Check(object)) {
        PyObject *key, *value;
        Py_ssize_t position = 0;

        status = yajl_gen_map_open(handle);
        while (PyDict_Next(object, &position, &key, &value)) {
            status = ProcessObject(self, key);
            status = ProcessObject(self, value);
        }
        return yajl_gen_map_close(handle);
    }

    exit:
        return yajl_gen_in_error_state;
}

yajl_alloc_funcs *y_allocs = NULL;

/* a structure used to pass context to our printer function */
struct StringAndUsedCount
{
    PyObject * str;
    size_t used;
};


static void py_yajl_printer(void * ctx,
                            const char * str,
                            unsigned int len)
{
    struct StringAndUsedCount * sauc = (struct StringAndUsedCount *) ctx;
    size_t newsize;

    if (!sauc || !sauc->str) return;

    /* resize our string if necc */
    newsize = Py_SIZE(sauc->str);
    while (sauc->used + len > newsize) newsize *= 2;
    if (newsize != Py_SIZE(sauc->str)) {
#ifdef IS_PYTHON3
        _PyBytes_Resize(&(sauc->str), newsize);
#else
        _PyString_Resize(&(sauc->str), newsize);
#endif
        if (!sauc->str)
            return;
    }

    /* and append data if available */
    if (len && str) {
#ifdef IS_PYTHON3
        memcpy((void *)(((PyBytesObject *)sauc->str)->ob_sval + sauc->used), str, len);
#else
        memcpy((void *) (((PyStringObject *) sauc->str)->ob_sval + sauc->used), str, len);
#endif
        sauc->used += len;
    }
}

/* Efficiently allocate a python string of a fixed size containing uninitialized memory */
static PyObject * lowLevelStringAlloc(Py_ssize_t size)
{
#ifdef IS_PYTHON3
    PyBytesObject * op = (PyBytesObject *)PyObject_MALLOC(sizeof(PyBytesObject) + size);
    if (op) {
        PyObject_INIT_VAR(op, &PyBytes_Type, size);
    }
#else
    PyStringObject * op = (PyStringObject *)PyObject_MALLOC(sizeof(PyStringObject) + size);
    if (op) {
        PyObject_INIT_VAR(op, &PyString_Type, size);
        op->ob_shash = -1;
        op->ob_sstate = SSTATE_NOT_INTERNED;
    }
#endif
    return (PyObject *) op;
}

PyObject *_internal_encode(_YajlEncoder *self, PyObject *obj, yajl_gen_config genconfig)
{
    yajl_gen generator = NULL;
    yajl_gen_status status;
    struct StringAndUsedCount sauc;
#ifdef IS_PYTHON3
    PyObject *result = NULL;
#endif

    /* initialize context for our printer function which
     * performs low level string appending, using the python
     * string implementation as a chunked growth buffer */
    sauc.used = 0;
    sauc.str = lowLevelStringAlloc(PY_YAJL_CHUNK_SZ);

    generator = yajl_gen_alloc2(py_yajl_printer, &genconfig, NULL, (void *) &sauc);

    self->_generator = generator;

    status = ProcessObject(self, obj);

    yajl_gen_free(generator);
    self->_generator = NULL;

    /* if resize failed inside our printer function we'll have a null sauc.str */
    if (!sauc.str) {
        PyErr_SetObject(PyExc_ValueError, PyUnicode_FromString("Allocation failure"));
        return NULL;
    }

    if ( (status == yajl_gen_in_error_state) ||
          (status != yajl_gen_status_ok) ) {
        PyErr_SetObject(PyExc_TypeError, PyUnicode_FromString("Object is not JSON serializable"));
        Py_XDECREF(sauc.str);
        return NULL;
    }

#ifdef IS_PYTHON3
    result = PyUnicode_DecodeUTF8(((PyBytesObject *)sauc.str)->ob_sval, sauc.used, "strict");
    Py_XDECREF(sauc.str);
    return result;
#else
    /* truncate to used size, and resize will handle the null plugging */
    _PyString_Resize(&sauc.str, sauc.used);
    return sauc.str;
#endif
}

PyObject *py_yajlencoder_encode(PYARGS)
{
    _YajlEncoder *encoder = (_YajlEncoder *)(self);
    yajl_gen_config config = {0, NULL};
    PyObject *value;

    if (!PyArg_ParseTuple(args, "O", &value))
        return NULL;
    return _internal_encode(encoder, value, config);
}

int yajlencoder_init(PYARGS)
{
    _YajlEncoder *me = (_YajlEncoder *)(self);

    if (!me)
        return 1;
    return 0;
}

void yajlencoder_dealloc(_YajlEncoder *self)
{
#ifdef IS_PYTHON3
    Py_TYPE(self)->tp_free((PyObject*)self);
#else
    self->ob_type->tp_free((PyObject*)self);
#endif
}
