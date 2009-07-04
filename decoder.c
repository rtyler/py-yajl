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

int handle_null(void *ctx)
{
    return -1;
}

int handle_int(void *ctx, long value)
{
    return -1;
}

int handle_bool(void *ctx, int value)
{
    return -1;
}

int handle_double(void *ctx, double value)
{
    return -1;
}

int handle_number(void *ctx, const char *value, unsigned int length)
{
    return -1;
}

int handle_string(void *ctx, const unsigned char *value, unsigned int length)
{
    return -1;
}

int handle_start_dict(void *ctx)
{
    return -1;
}

int handle_dict_key(void *ctx, const unsigned char *value, unsigned int length)
{
    return -1;
}

int handle_end_dict(void *ctx)
{
    return -1;
}

int handle_start_list(void *ctx)
{
    return -1;
}

int handle_end_list(void *ctx)
{
    return -1;
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

    /* callbacks, config, allocfuncs */
    parser = yajl_alloc(&decode_callbacks, &config, NULL, (void *)(self));

    yrc = yajl_parse(parser, (const unsigned char *)(buffer), buflen);

    if (yrc != yajl_status_ok) {
        /* Raise some sort of exception */
        return NULL;
    }

    return NULL;
}

PyObject *py_yajldecoder_raw_decode(PYARGS)
{
    return NULL;
}

int yajldecoder_init(PYARGS)
{
    return 0;
}
