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
        if ((child) && (child != Py_None)) {
            Py_XDECREF(child);
        }
        return success;
    } else if (PyDict_Check(parent)) {
        PyObject* key = py_yajl_ps_current(self->keys);
        PyDict_SetItem(parent, key, child);
        py_yajl_ps_pop(self->keys);
        // child is now owned by parent!
        Py_XDECREF(key);
        if ((child) && (child != Py_None)) {
            Py_XDECREF(child);
        }
        return success;
    }
    return failure;
}

int PlaceObject(_YajlDecoder *self, PyObject *object)
{
    unsigned int length = py_yajl_ps_length(self->elements);

    if (length == 0) {
        /*
         * When the length is zero, and we're entering this code path
         * we should only be handling "primitive types" i.e. strings and
         * numbers, not dict/list.
         */
        PyList_Append(self->decoded_objects,object);
        return success;
    }
    return _PlaceObject(self, py_yajl_ps_current(self->elements), object);
}


static int handle_null(void *ctx)
{
    Py_INCREF(Py_None);
    return PlaceObject(ctx, Py_None);
}

static int handle_bool(void *ctx, int value)
{
    return PlaceObject(ctx, PyBool_FromLong((long)(value)));
}

static int handle_number(void *ctx, const char *value, size_t length)
{
    _YajlDecoder *self = (_YajlDecoder *)(ctx);
    PyObject *object;
#ifdef IS_PYTHON3
    PyBytesObject *string;
#else
    PyObject *string;
#endif

    int floaty_char;

    // take a moment here to scan the input string to see if there's
    // any chars which suggest this is a floating point number
    for (floaty_char = 0; floaty_char < length; floaty_char++) {
        switch (value[floaty_char]) {
            case '.': case 'e': case 'E': goto floatin;
        }
    }

  floatin:
#ifdef IS_PYTHON3
    string = (PyBytesObject *)PyBytes_FromStringAndSize(value, length);
    if (floaty_char >= length) {
        object = PyLong_FromString(string->ob_sval, NULL, 10);
    } else {
        object = PyFloat_FromString((PyObject *)string);
    }
#else
    string = PyString_FromStringAndSize(value, length);
    if (floaty_char >= length) {
        object = PyInt_FromString(PyString_AS_STRING(string), NULL, 10);
    } else {
        object = PyFloat_FromString(string, NULL);
    }
#endif
    Py_XDECREF(string);
    return PlaceObject(self, object);
}

static int handle_string(void *ctx, const unsigned char *value, size_t length)
{
    return PlaceObject(ctx, PyUnicode_FromStringAndSize((char *)value, length));
}

static int handle_start_dict(void *ctx)
{
    PyObject *object = PyDict_New();
    if (!object)
        return failure;

    py_yajl_ps_push(((_YajlDecoder *)(ctx))->elements, object);
    return success;
}

static int handle_dict_key(void *ctx, const unsigned char *value, size_t length)
{
    PyObject *object = PyUnicode_FromStringAndSize((const char *) value, length);

    if (object == NULL)
        return failure;

    py_yajl_ps_push(((_YajlDecoder *)(ctx))->keys, object);
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
        PyList_Append(self->decoded_objects,py_yajl_ps_current(self->elements));
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
    PyObject *object = PyList_New(0);

    if (!object)
        return failure;

    py_yajl_ps_push(((_YajlDecoder *)(ctx))->elements, object);
    return success;
}

static int handle_end_list(void *ctx)
{
    _YajlDecoder *self = (_YajlDecoder *)(ctx);
    PyObject *last, *popped;
    unsigned int length;

    length = py_yajl_ps_length(self->elements);
    if (length == 1) {
        PyList_Append(self->decoded_objects,py_yajl_ps_current(self->elements));
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
/*
Parse the contents of *buffer using the yajl json parser
*/
PyObject *_internal_decode(_YajlDecoder *self, char *buffer, unsigned int buflen)
{
    yajl_status yrc;
    yrc = yajl_parse(self->parser, (const unsigned char *)(buffer), buflen);

    if (yrc != yajl_status_ok) {
        PyErr_SetObject(PyExc_ValueError,
                PyUnicode_FromString(yajl_status_to_string(yrc)));
        return NULL;
    }
    Py_RETURN_NONE;
}

/*
Return an object from the decoded_objects list
*/
PyObject *_fetchObject(_YajlDecoder *self) 
{
    PyObject *result = NULL;
    int len;

    len = PySequence_Size(self->decoded_objects); 
    if (len == 0) {
        PyErr_SetObject(PyExc_ValueError,
                PyUnicode_FromString("No Objects Decoded"));
        return NULL;
    }
 
    result = PySequence_GetItem(self->decoded_objects,0);
    PySequence_DelItem(self->decoded_objects,0);
    Py_DECREF(result);
    return result;
}

/*
 * Decode a chunk of input at a time
 */
PyObject *py_yajldecoder_iterdecode(_YajlDecoder *self, PyObject *args)
{
    char *buffer = NULL;
    PyObject *pybuffer = NULL;
    PyObject *result = NULL;
    Py_ssize_t buflen = 0;

    if (!PyArg_ParseTuple(args, "O", &pybuffer))
        return NULL;

    Py_INCREF(pybuffer);

    if (PyUnicode_Check(pybuffer)) {
        if (!(result = PyUnicode_AsUTF8String(pybuffer))) {
            Py_DECREF(pybuffer);
            return NULL;
        }
        Py_DECREF(pybuffer);
        pybuffer = result;
        result = NULL;
    }

    if (PyString_Check(pybuffer)) {
        if (PyString_AsStringAndSize(pybuffer, &buffer, &buflen)) {
            Py_DECREF(pybuffer);
            return NULL;
        }
    } else {
        /* really seems like this should be a TypeError, but
           tests/unit.py:ErrorCasesTests.test_None disagrees */
        Py_DECREF(pybuffer);
        PyErr_SetString(PyExc_ValueError, "string or unicode expected");
        return NULL;
    }

    if (!buflen) {
        Py_DECREF(pybuffer);
        Py_RETURN_NONE;
    }

    result = _internal_decode(self, buffer, (unsigned int)buflen);
    if (!result) {
        Py_DECREF(pybuffer);
        return NULL;
    }

    Py_DECREF(pybuffer);
    Py_INCREF(self->decoded_objects);
    result = self->decoded_objects;
    if (PySequence_Size(self->decoded_objects) > 0) {
        Py_DECREF(self->decoded_objects);
        self->decoded_objects = PyList_New(0);
    }
    return result;
}

/*
External interface "decode" 
*/
PyObject *py_yajldecoder_decode(_YajlDecoder *self, PyObject *args)
{
    char *buffer = NULL;
    PyObject *pybuffer = NULL;
    PyObject *result = NULL;
    Py_ssize_t buflen = 0;
    yajl_status yrc;

    if (!PyArg_ParseTuple(args, "O", &pybuffer))
        return NULL;

    Py_INCREF(pybuffer);

    if (PyUnicode_Check(pybuffer)) {
        if (!(result = PyUnicode_AsUTF8String(pybuffer))) {
            Py_DECREF(pybuffer);
            return NULL;
        }
        Py_DECREF(pybuffer);
        pybuffer = result;
        result = NULL;
    }

    if (PyString_Check(pybuffer)) {
        if (PyString_AsStringAndSize(pybuffer, &buffer, &buflen)) {
            Py_DECREF(pybuffer);
            return NULL;
        }
    } else {
        /* really seems like this should be a TypeError, but
           tests/unit.py:ErrorCasesTests.test_None disagrees */
        Py_DECREF(pybuffer);
        PyErr_SetString(PyExc_ValueError, "string or unicode expected");
        return NULL;
    }

    if (!buflen) {
        Py_DECREF(pybuffer);
        PyErr_SetObject(PyExc_ValueError,
                PyUnicode_FromString("Cannot parse an empty buffer"));
        return NULL;
    }

    result = _internal_decode(self, buffer, (unsigned int)buflen);
    if (!result) {
        Py_DECREF(pybuffer);
        return NULL;
    }

    yrc = yajl_complete_parse(self->parser);
    if (yrc != yajl_status_ok) {
        PyErr_SetObject(PyExc_ValueError,
            PyUnicode_FromString(yajl_status_to_string(yrc)));
        return NULL;
    }
    result = _fetchObject(self);
    return result;
}

PyObject *py_yajldecoder_iter(PyObject *self)
{
    Py_INCREF(self);
    return self;
}

PyObject *py_yajldecoder_iternext(PyObject *self) 
{
    _YajlDecoder *d = (_YajlDecoder *)self;

    PyObject *buffer = NULL;
    PyObject *result = NULL;
#ifdef IS_PYTHON3
    PyObject *bufferstring = NULL;
#endif

    // return an object from the list if there is one available.
    //len = PySequence_Size(d->decoded_objects); 
    if (PySequence_Size(d->decoded_objects) != 0) {
        return _fetchObject(d);
    } 

    // return NULL if stream is not set
    if (!d->stream) {
        PyErr_SetObject(PyExc_TypeError, PyUnicode_FromString("no stream to iterate over"));
        return NULL;
    }

    // while there are no complete objects keep trying to read from the stream
    while (PySequence_Size(d->decoded_objects) < 1) {
        buffer = PyObject_CallMethod(d->stream,d->read_fn,"O",d->bufsize);
        if (!buffer) {
            Py_XDECREF(buffer);
            return NULL;
        }

#ifdef IS_PYTHON3
        bufferstring = PyUnicode_AsUTF8String(buffer);
        if (!bufferstring)
            return NULL;
#endif

#ifdef IS_PYTHON3
        if (PyBytes_Size(bufferstring) == 0) {
            Py_XDECREF(buffer);
            return NULL;
        }
#else
        if (PyString_Size(buffer) == 0) {
            Py_XDECREF(buffer);
            return NULL;
        }
#endif

#ifdef IS_PYTHON3
        result = _internal_decode((_YajlDecoder *)d, PyBytes_AsString(bufferstring),
                  PyBytes_Size(bufferstring));
        Py_XDECREF(bufferstring);
#else
        result = _internal_decode((_YajlDecoder *)d, PyString_AsString(buffer),
                  PyString_Size(buffer));
#endif
        Py_XDECREF(buffer);
        if (result == NULL) {
            // error set by _internal_decode
            return NULL;
        }
    }
    return _fetchObject(d); 
}

/*
Reset the parser to its starting state
This will also setup the parser for the first time
*/
PyObject *py_yajldecoder_reset(_YajlDecoder *self,PyObject *args)
{
    // reset pointer structures
    if (self->elements.used > 0) {
        py_yajl_ps_free(self->elements);
        py_yajl_ps_init(self->elements);
    }

    if (self->keys.used > 0) {
        py_yajl_ps_free(self->keys);
        py_yajl_ps_init(self->keys);
    }

    // reset decoded object list
    Py_XDECREF(self->decoded_objects);
    self->decoded_objects = PyList_New(0);

    // reset the parser
    if (self->parser) {
        yajl_free(self->parser);
    } 

    self->parser = yajl_alloc(&decode_callbacks,NULL, (void *)(self));
    yajl_config(self->parser,yajl_allow_comments,1);
    yajl_config(self->parser,yajl_allow_partial_values,1);
    yajl_config(self->parser,yajl_dont_validate_strings,0);
    if (PyLong_AsUnsignedLongMask(self->allow_multiple_values) == 1) {
        yajl_config(self->parser,yajl_allow_multiple_values,1);
    }

    Py_RETURN_NONE;
}

/*
Return the number of decoded objects in the internal buffer
Useful if allow_multiple_values is enabled
*/
Py_ssize_t decoder_len(_YajlDecoder *self)
{
    return PySequence_Size(self->decoded_objects);
}

/*
Init the decoder python object
*/
int yajldecoder_init(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *allow_multiple_values = NULL;
    PyObject *result = NULL;
    PyObject *stream = NULL;
    PyObject *bufsize = NULL;

    static char *kwlist[] = {"allow_multiple_values","stream","bufsize",NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|OOO", kwlist, \
                                &allow_multiple_values, &stream,\
                                &bufsize)) {
        return -1;
    }
    
    if (allow_multiple_values) {
        if (!PyBool_Check(allow_multiple_values)) {
            PyErr_SetObject(PyExc_TypeError, PyUnicode_FromString("allow_multiple_values must be a bool"));
            return -1;
        }
    } else {
        // default to false
        Py_INCREF(Py_False);
        allow_multiple_values = Py_False;
    }


    // basic setup
    _YajlDecoder *me = (_YajlDecoder *)(self);
    me->allow_multiple_values = allow_multiple_values;
    py_yajl_ps_init(me->elements);
    py_yajl_ps_init(me->keys); 

    // stream setup
    if (stream) {
        me->read_fn = "read";
        if (!PyObject_HasAttrString(stream,"read")) {
            if (!PyObject_HasAttrString(stream,"recv")) {
                PyErr_SetObject(PyExc_TypeError, PyUnicode_FromString("stream object must have a read or recv attribute"));
                return -1;
            }
            me->read_fn = "recv";
        }
        Py_INCREF(stream);
    }

    me->stream = stream;
    if (bufsize) {
#ifdef IS_PYTHON3
        if (!PyLong_Check(bufsize)) {
#else
        if (!PyInt_Check(bufsize)) {
#endif
            PyErr_SetObject(PyExc_TypeError, PyUnicode_FromString("bufsize must be a int"));
            return -1;
        }
#ifdef IS_PYTHON3
        if (!(PyLong_AsLong(bufsize) >= 1)) {
#else
        if (!(PyInt_AsLong(bufsize) >=  1)) {
#endif
            PyErr_SetObject(PyExc_TypeError, PyUnicode_FromString("bufsize must be >= 1"));
            return -1;
        }
        Py_INCREF(bufsize);
        me->bufsize = bufsize;
    } else {
#ifdef IS_PYTHON3
        me->bufsize = PyLong_FromLong(512);
#else
        me->bufsize = PyInt_FromLong(512);
#endif
    }

    // reset method also initialises data structures
    result = PyObject_CallMethod(self,"reset",NULL); 
    if (!result) {
        PyErr_SetObject(PyExc_TypeError, PyUnicode_FromString("failed to call self.reset()"));
        return -1;
    } 
    Py_XDECREF(result);
    return 0;
}

void yajldecoder_dealloc(_YajlDecoder *self)
{
    // free pointer stack
    py_yajl_ps_free(self->elements);
    py_yajl_ps_free(self->keys);

    // if we have decoded objects free those
    if (self->decoded_objects) {
        Py_XDECREF(self->decoded_objects);
    }

    // free the parser
    if (self->parser) {
        yajl_free(self->parser);
    }

    // free bufsize
    if (self->bufsize) {
        Py_XDECREF(self->bufsize);
    }

    // free stream
    if (self->stream) {
        Py_XDECREF(self->stream);
    }
#ifdef IS_PYTHON3
    Py_TYPE(self)->tp_free((PyObject*)self);
#else
    self->ob_type->tp_free((PyObject*)self);
#endif
}
