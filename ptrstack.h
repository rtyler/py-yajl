/*
 * originally by lloyd, put in the public domain.  (relicense this as
 * your own tyler, to keep the licensing of the py ext clean)
 */ 

/*
 * A header only, highly efficient custom pointer stack implementation, used in
 * py-yajl to maintain parse state.
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
