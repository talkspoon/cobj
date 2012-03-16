#ifndef VM_EXECUTE_H
#define VM_EXECUTE_H

#include "compile.h"
#include "parser.h"
#include "error.h"
#include "object.h"
#include "objects/functionobject.h"

/* execution frame */
struct co_exec_data {
    struct co_opline *op;
    struct co_opline_array *opline_array;
    struct co_exec_data *prev_exec_data;
    COObject *function_called;
    COObject *symbol_table; /* dict object for names */
    COObject **ts;  /* temp objects */
};

extern void co_vm_init();
extern void co_vm_execute(COFunctionObject *main);
extern void co_vm_shutdown();

/* COObject handlers */
extern COObject *COObject_get(COObject *co);
extern int COObject_put(COObject *name, COObject *co);
extern int COObject_del(COObject *name);
extern void COObject_print(COObject *co);

#endif
