#include "co.h"

struct compiler {
    COObject *c_oplines;
    uint c_numoftmpvars;
};

struct compiler c;

COCodeObject *
co_compile(void)
{
    c.c_oplines = COList_New(0);

    // do parse
    coparse(&c);

#ifdef CO_DEBUG
    co_print_opcode(c.c_oplines);
#endif

    COCodeObject *co =
        (COCodeObject *)COCode_New(COList_AsTuple(c.c_oplines),
                                   c.c_numoftmpvars);

    return co;
}

uint
co_get_next_opline_num(void)
{
    return COList_Size(c.c_oplines);
}

static struct co_opline *
next_op()
{
    struct co_opline *next_op = xmalloc(sizeof(struct co_opline));
    memset(next_op, 0, sizeof(struct co_opline));
    next_op->op1.type = IS_UNUSED;
    next_op->op2.type = IS_UNUSED;
    next_op->result.type = IS_UNUSED;

    COList_Append(c.c_oplines, (COObject *)next_op);

    return next_op;
}

static uint
get_temporary_variable()
{
    return c.c_numoftmpvars++ * sizeof(COObject *);
}

static void
check_laod_op(const struct cnode *node)
{
    struct co_opline *op;
    if (node->type == IS_CONST) {
        op = next_op();
        op->opcode = OP_LOAD_CONST;
        /*op->op1 = */
    }
}

void
co_binary_op(uchar opcode, struct cnode *result, const struct cnode *op1,
             const struct cnode *op2)
{
    struct co_opline *op = next_op();

    op->opcode = opcode;
    op->op1 = *op1;
    op->op2 = *op2;
    op->result.type = IS_TMP_VAR;
    op->result.u.var = get_temporary_variable();
    *result = op->result;
}

void
co_assign(struct cnode *result, struct cnode *variable,
          const struct cnode *value)
{
    struct co_opline *op;
    op = next_op();
    op->opcode = OP_LOAD_NAME;
    op->op1 = *variable;

    op = next_op();
    op->opcode = OP_ASSIGN;
    op->op1 = *variable;
    op->op2 = *value;
    op->result.type = IS_TMP_VAR;
    op->result.u.var = get_temporary_variable();
    *result = op->result;
}

void
co_print(const struct cnode *arg)
{
    struct co_opline *op = next_op();

    op->opcode = OP_PRINT;
    op->op1 = *arg;
}

void
co_return(const struct cnode *expr)
{
    struct co_opline *op = next_op();

    op->opcode = OP_RETURN;
    op->op1 = *expr;
}

void
co_if_cond(const struct cnode *cond, struct cnode *if_token)
{
    int if_cond_opline_num = CO_SIZE(c.c_oplines);
    struct co_opline *opline = next_op();
    opline->opcode = OP_JMPZ;
    opline->op1 = *cond;
    if_token->u.opline_num = if_cond_opline_num;
}

void
co_if_after_stmt(struct cnode *if_token)
{
    int if_after_stmt_op_num = CO_SIZE(c.c_oplines);
    struct co_opline *opline = next_op();
    struct co_opline *ifopline =
        COList_GetItem(c.c_oplines, if_token->u.opline_num);
    ifopline->op2.u.opline_num =
        if_after_stmt_op_num + 1 - if_token->u.opline_num;
    if_token->u.opline_num = if_after_stmt_op_num;
    opline->opcode = OP_JMP;
}

void
co_if_end(const struct cnode *if_token)
{
    int if_end_op_num = CO_SIZE(c.c_oplines);
    struct co_opline *ifopline =
        COList_GetItem(c.c_oplines, if_token->u.opline_num);
    ifopline->op1.u.opline_num = if_end_op_num - if_token->u.opline_num;
}

void
co_while_cond(const struct cnode *cond, struct cnode *while_token)
{
    int while_cond_opline_num = CO_SIZE(c.c_oplines);
    struct co_opline *opline = next_op();
    opline->opcode = OP_JMPZ;
    opline->op1 = *cond;
    opline->op2.u.opline_num = while_token->u.opline_num;       // while start
    while_token->u.opline_num = while_cond_opline_num;
}

void
co_while_end(const struct cnode *while_token)
{
    // add unconditional jumpback
    int while_end_opline_num = CO_SIZE(c.c_oplines);
    struct co_opline *op = next_op();
    op->opcode = OP_JMP;
    struct co_opline *whileopline =
        COList_GetItem(c.c_oplines, while_token->u.opline_num);
    op->op1.u.opline_num = whileopline->op2.u.opline_num - while_end_opline_num;        // while start offset

    int while_end_stmt_op_num = CO_SIZE(c.c_oplines);
    whileopline->op2.u.opline_num =
        while_end_stmt_op_num - while_token->u.opline_num;
}

void
co_begin_func_declaration(struct cnode *func_token, struct cnode *func_name)
{
    struct co_opline *op;
    if (func_name) {
        op = next_op();
        op->opcode = OP_LOAD_NAME;
        op->op1 = *func_name;
    }

    int func_opline_num = CO_SIZE(c.c_oplines);
    op = next_op();
    op->opcode = OP_DECLARE_FUNCTION;
    if (func_name) {
        op->op1 = *func_name;
    }
    func_token->u.opline_num = func_opline_num;
}

void
co_end_func_declaration(const struct cnode *func_token, struct cnode *result)
{
    struct co_opline *op = next_op();
    op->opcode = OP_RETURN;

    int func_end_opline_num = CO_SIZE(c.c_oplines);
    struct co_opline *funcopline =
        COList_GetItem(c.c_oplines, func_token->u.opline_num);
    funcopline->op2.u.opline_num =
        func_end_opline_num - func_token->u.opline_num - 1;

    if (result) {
        funcopline->result.type = IS_TMP_VAR;
        funcopline->result.u.var = get_temporary_variable();
        *result = funcopline->result;
    }
}

void
co_end_func_call(struct cnode *func_name, struct cnode *result)
{
    struct co_opline *op = next_op();
    op->opcode = OP_DO_FCALL;
    op->op1 = *func_name;
    op->result.type = IS_TMP_VAR;
    op->result.u.var = get_temporary_variable();
    *result = op->result;
}

void
co_recv_param(struct cnode *param)
{
    struct co_opline *op;
    op = next_op();
    op->opcode = OP_LOAD_NAME;
    op->op1 = *param;

    op = next_op();
    op->opcode = OP_RECV_PARAM;
    op->op1 = *param;
}

void
co_pass_param(struct cnode *param)
{
    struct co_opline *op = next_op();
    op->opcode = OP_PASS_PARAM, op->op1 = *param;
}

void
co_list_build(struct cnode *result, struct cnode *tag)
{
    struct co_opline *op = next_op();
    op->opcode = OP_LIST_BUILD;
    op->result.type = IS_TMP_VAR;
    op->result.u.var = get_temporary_variable();
    *result = op->result;

    *tag = op->result;
}

void
co_tuple_build(struct cnode *result, struct cnode *tag)
{
    struct co_opline *op = next_op();
    op->opcode = OP_TUPLE_BUILD;
    op->result.type = IS_TMP_VAR;
    op->result.u.var = get_temporary_variable();
    *result = op->result;

    *tag = op->result;
}

void
co_append_element(struct cnode *node, struct cnode *element)
{
    struct co_opline *op = next_op();
    op->opcode = OP_APPEND_ELEMENT;
    op->op1 = *node;
    op->op2 = *element;
}

void
co_end_compilation()
{
    struct co_opline *op = next_op();

    op->opcode = OP_EXIT;
}

int
colex(struct cnode *colval)
{
    int retval;

again:
    retval = co_scanner_lex(colval);
    switch (retval) {
    case T_WHITESPACE:
    case T_COMMENT:
    case T_IGNORED:
        goto again;
    default:
        break;
    }

    return retval;
}

void
coerror(const char *err, ...)
{
    va_list params;

    va_start(params, err);
    error(err, params);
    va_end(params);
    exit(128);
}
