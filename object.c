#include "cobj.h"

COObject *
COObject_Repr(COObject *o)
{
    if (!o)
        return COStr_FromString("<NULL>");
    return CO_TYPE(o)->tp_repr(o);
}

void
COObject_Dump(COObject *o)
{
    if (o == NULL) {
        fprintf(stderr, "NULL\n");
    } else {
        fprintf(stderr, "object(%p):\n", o);
        fprintf(stderr, "    type: %s\n",
                CO_TYPE(o) == NULL ? "NULL" : CO_TYPE(o)->tp_name);
        fprintf(stderr, "    refcnt: %d\n", o->co_refcnt);
        COStrObject *s = (COStrObject *)COObject_Repr(o);
        fprintf(stderr, "    repr: %s\n", s->co_sval);
        CO_DECREF(s);
    }
}

long
_CO_HashPointer(void *p)
{
    long x;
    size_t y = (size_t) p;
    /* bottom 3 or 4 bits are likely to be 0; rotate y by 4 to avoid
       excessive hash collisions for dicts and sets */
    y = (y >> 4) | (y << (8 * sizeof(void *) - 4));
    x = (long)y;
    if (x == -1)
        x = -2;
    return x;
}

long
COObject_Hash(COObject *o)
{
    COTypeObject *tp = o->co_type;

    if (tp->tp_hash != NULL) {
        return tp->tp_hash(o);
    }

    if (tp->tp_compare == NULL) {
        /* imply it's unique in world, so use this address as hash value */
        return _CO_HashPointer(o);
    }

    COErr_Format(COException_TypeError, "unhashable type: '%.200s'",
                 tp->tp_name);
    return -1;
}

COObject *
COObject_New(COTypeObject *tp)
{
    COObject *o;
    o = (COObject *)COObject_Mem_MALLOC(tp->tp_basicsize);
    if (o == NULL) {
        return COErr_NoMemory();
    }

    return COObject_INIT(o, tp);
}

COObject *
COVarObject_New(COTypeObject *tp, ssize_t n)
{
    COVarObject *o;
    const size_t size = COObject_VAR_SIZE(tp, n);
    o = (COVarObject *)COObject_Mem_MALLOC(size);
    if (o == NULL) {
        return COErr_NoMemory();
    }
    return (COObject *)COVarObject_INIT(o, tp, n);
}

int
COObject_Print(COObject *o, FILE *fp)
{
    if (!o) {
        fprintf(fp, "<null>");
        return 0;
    }
    if (CO_REFCNT(o) <= 0) {
        fprintf(fp, "<refcnt %ld at %p>", (long)CO_REFCNT(o), o);
        return 0;
    } else if (CO_TYPE(o)->tp_print == NULL) {
        COObject *s = CO_TYPE(o)->tp_repr(o);
        if (!s)
            return -1;
        int ret = 0;
        ret = COObject_Print(s, fp);
        CO_DECREF(s);
        return ret;
    } else {
        return CO_TYPE(o)->tp_print(o, fp);
    }
}

/*
 * COObject Compare
 * 
 * Every object's type have a tp_compare function slot, it's not NULL, it
 * gets called with two objects and Cmp_Op, and should return an object as follows:
 *  - NULL if an exception occurred
 *  - False object is comparison is false
 *  - True object is comparison is true
 */
static COObject *
do_compare(COObject *a, COObject *b, int op)
{
    static char *opstrings[] = { "<", "<=", "==", "!=", ">", ">=" };

    comparefunc f;
    COObject *x;

    if (a->co_type == b->co_type && (f = a->co_type->tp_compare) != NULL) {
        x = (*f) (a, b, op);
    } else {
        COErr_Format(COException_UndefinedError,
                     "undefined comparison: %.100s() %s %.100s()",
                     a->co_type->tp_name, opstrings[op], b->co_type->tp_name);
        return NULL;
    }
    return x;
}

COObject *
COObject_Compare(COObject *a, COObject *b, int op)
{
    assert(Cmp_LT <= op && op <= Cmp_GE);
    COObject *x;

    if (!a || !b) {
        COErr_BadInternalCall();
        return NULL;
    }

    x = do_compare(a, b, op);
    return x;
}

/*
 * -1 for error, 0 for falce, 1 for true
 */
int
COObject_CompareBool(COObject *a, COObject *b, int op)
{
    COObject *x;
    int ok;
    /* Quick result when objects are the same.
       Guarantees that identity implies equality. */
    if (a == b) {
        if (op == Cmp_EQ)
            return 1;
        else if (op == Cmp_NE)
            return 0;
    }
    x = COObject_Compare(a, b, op);
    if (!x)
        return -1;
    if (x == CO_True)
        ok = 1;
    else
        ok = 0;
    CO_DECREF(x);
    return ok;
}

/*
 * Return -1 if an error occured. 0 for false, 1 for true.
 */
int
COObject_IsTrue(COObject *o)
{
    ssize_t size;
    if (o == CO_True)
        return 1;
    if (o == CO_False)
        return 0;
    if (o == CO_None)
        return 0;
    if (COInt_Check(o)) {
        size = CO_SIZE(o);
    } else {
        // default
        return 1;
    }
    return (size == 0) ? 0 : 1;
}

void
_CO_NegativeRefCnt(const char *fname, int lineno, COObject *co)
{
    char buf[256];
    snprintf(buf, sizeof(buf), "%s:%i object at %p has negative ref count: %d",
             fname, lineno, co, co->co_refcnt);
    fprintf(stderr, "%s\n", buf);
    exit(-1);
}

int
COObject_ParseArgs(COObject *args, ...)
{
    va_list va;

    if (args == NULL || !COTuple_Check(args)) {
        COErr_BadInternalCall();
        return 0;
    }

    va_start(va, args);

    int i;
    COObject **o;

    i = 0;
    while (true) {
        o = va_arg(va, COObject **);
        if (!o)
            break;
        *o = COTuple_GET_ITEM(args, i);
        i++;
    }

    va_end(va);

    return 1;
}

COObject *
COObject_Str(COObject *o)
{
    if (!o)
        return COStr_FromString("<NULL>");

    if (COStr_Check(o)) {
        CO_INCREF(o);
        return o;
    }

    return COObject_Repr(o);
}

COObject *
COObject_Call(COObject *this, COObject *args)
{
    binaryfunc call;
    if ((call = this->co_type->tp_call) != NULL) {
        COObject *result;
        result = call(this, args);
        if (!result && !COErr_Occurred())
            COErr_SetString(COException_SystemError,
                            "NULL result without error in COObject_Call");
        return result;
    }
    COErr_Format(COException_TypeError, "'%.200s' object is not callable",
                 this->co_type->tp_name);
    return NULL;
}

COObject *
COObject_GetIter(COObject *o)
{
    COTypeObject *tp = o->co_type;
    getiterfunc f = NULL;
    f = tp->tp_iter;
    if (f == NULL) {
        COErr_BadInternalCall();
        return NULL;
    } else {
        COObject *res = (*f) (o);
        return res;
    }
}

COObject *
COObject_GetSelf(COObject *o)
{
    CO_INCREF(o);
    return o;
}
