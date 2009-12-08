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

/*
 * A header only, highly efficient custom pointer stack implementation,
 * used in py-yajl to maintain parse state.
 */

#ifndef __PY_YAJL_BYTESTACK_H__
#define __PY_YAJL_BYTESTACK_H__

#include <Python.h>
#include "assert.h"

#define PY_YAJL_PS_INC 128

typedef struct py_yajl_bytestack_t
{
    PyObject ** stack;
    unsigned int size;
    unsigned int used;
} py_yajl_bytestack;

/* initialize a bytestack */
#define py_yajl_ps_init(ops) {                  \
        (ops).stack = NULL;                     \
        (ops).size = 0;                         \
        (ops).used = 0;                         \
    }                                           \


/* initialize a bytestack */
#define py_yajl_ps_free(ops)                 \
    if ((ops).stack) free((ops).stack);   

#define py_yajl_ps_current(ops)               \
    (assert((ops).used > 0), (ops).stack[(ops).used - 1])

#define py_yajl_ps_length(ops) ((ops).used)

#define py_yajl_ps_push(ops, pointer) {                       \
    if (((ops).size - (ops).used) == 0) {               \
        (ops).size += PY_YAJL_PS_INC;                      \
        (ops).stack = realloc((void *) (ops).stack, sizeof(PyObject *) * (ops).size); \
    }                                                   \
    (ops).stack[((ops).used)++] = (pointer);               \
}
    
/* removes the top item of the stack, returns nothing */
#define py_yajl_ps_pop(ops) { ((ops).used)--; }

#define py_yajl_ps_set(ops, pointer)                          \
    (ops).stack[((ops).used) - 1] = (pointer);             
    

#endif
