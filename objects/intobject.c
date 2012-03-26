#include "../co.h"

/*
 * Small integers are preallocated in this array so that they can be shared.
 *
 * [SMALL_NEG_INT, SMALL_POS_INT)
 */
#define SMALL_NEG_INT   10
#define SMALL_POS_INT   257
static COIntObject small_ints[SMALL_NEG_INT + SMALL_POS_INT];

#define CHECK_SMALL_INT(ival)   \
    do if (-SMALL_NEG_INT <= ival && ival < SMALL_POS_INT) {    \
        return (COObject *)(small_ints + ival + SMALL_NEG_INT); \
    } while (0);

static COObject *
int_repr(COIntObject *this)
{
    char buf[sizeof(long) * 8 / 3 + 6], *p, *bufend;
    long n = this->co_ival;
    unsigned long absn;
    p = bufend = buf + sizeof(buf);
    absn = n < 0 ? 0UL - n : n;
    do {
        *--p = '0' + (char)(absn % 10);
        absn /= 10;
    } while (absn);
    if (n < 0)
        *--p = '-';
    return COStr_FromStringN(p, bufend - p);
}

static long
int_hash(COIntObject *o)
{
    if (o->co_ival == -1) {
        return -2;
    } else {
        return o->co_ival;
    }
}

COTypeObject COInt_Type = {
    COObject_HEAD_INIT(&COType_Type),
    "int",
    sizeof(COIntObject),
    0,
    (reprfunc)int_repr,         /* tp_repr */
    0,                          /* tp_getattr */
    0,                          /* tp_setattr */
    (hashfunc)int_hash,         /* tp_hash */
};

int
COInt_Init(void)
{
    int ival;
    COIntObject *o = small_ints;
    for (ival = -SMALL_NEG_INT; ival < SMALL_POS_INT; ival++, o++) {
        (void)COObject_Init(o, &COInt_Type);
        o->co_ival = ival;
    }
    return 0;
}

long
COInt_AsLong(COObject *co)
{
    return ((COIntObject *)co)->co_ival;
}

COObject *
COInt_FromString(char *s, int base)
{
    COIntObject *num;

    if (base != 0 && (base < 2 || base > 36)) {
        // TODO errors
        return NULL;
    }
    long ival = strtol(s, NULL, base);

    CHECK_SMALL_INT(ival);

    num = COObject_New(COIntObject, &COInt_Type);
    num->co_ival = ival;
    return (COObject *)num;
}

COObject *
COInt_FromLong(long ival)
{
    COIntObject *num;
    CHECK_SMALL_INT(ival);

    num = COObject_New(COIntObject, &COInt_Type);
    num->co_ival = ival;
    return (COObject *)num;
}
