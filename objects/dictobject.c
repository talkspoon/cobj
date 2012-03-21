#include "../co.h"

static COObject *dummy = NULL;

static COObject *
dict_repr(CODictObject *this)
{
#ifdef CO_DEBUG
    uint i;
    DictBucket *p;
    for (i = 0; i < this->nTableSize; i++) {
        p = this->arBuckets[i];
        while (p != NULL) {
            printf("%s <==> 0x%lX\n", p->pKey, p->h);
            p = p->pNext;
        }
    }

    p = this->pListTail;
    while (p != NULL) {
        while (p != NULL) {
            printf("%s <==> 0x%lX\n", p->pKey, p->h);
            p = p->pListLast;
        }
    }
#endif

    return COStr_FromFormat("<dict 'size: %d'>", this->nNumOfElements);
}

COTypeObject CODict_Type = {
    COObject_HEAD_INIT(&COType_Type),
    "dict",
    sizeof(CODictObject),
    0,
    (reprfunc)dict_repr,        /* tp_repr */
    0,                          /* tp_getattr */
    0,                          /* tp_setattr */
    0,                          /* tp_hash */
};


int
_dict_rehash(CODictObject *this)
{
    DictBucket *p;

    uint nIndex;

    memset(this->arBuckets, 0, this->nTableSize * sizeof(DictBucket *));
    p = this->pListHead;
    while (p != NULL) {
        nIndex = p->h & this->nTableMask;
        this->arBuckets[nIndex] = p;
        p = p->pListNext;
    }
    return 0;
}

static int
_dict_do_resize(CODictObject *this)
{
    DictBucket **t;

    if ((this->nTableSize << 1) > 0) {  // double the table size
        t = (DictBucket **)xrealloc(this->arBuckets,
                                    (this->nTableSize << 1) *
                                    sizeof(DictBucket *));
        this->arBuckets = t;
        this->nTableSize = this->nTableSize << 1;
        this->nTableMask = this->nTableSize - 1;
        _dict_rehash(this);
        return 0;
    }

    return -1;                // can not be larger
}

/* 
 * Dict lookup function.
 * It only support str/int object now.
 */
static DictBucket *
_dict_lookup(CODictObject *this, COObject *key)
{
    ulong h;
    uint nIndex;

    DictBucket *p;

    h = COObject_hash(key);
    nIndex = h & this->nTableMask;
    p = this->arBuckets[nIndex];

    while (p != NULL) {
        if (p->h == h) {
            if (p->pKey == dummy) {
                // TODO free slots
                continue;
            }
            if (COStr_Check(key)) {
                if (!memcmp(((COStrObject *)p->pKey)->co_sval, ((COStrObject *)key)->co_sval, CO_SIZE(key))) {
                    return p;
                }
            } else if (COInt_Check(key)) {
                if (COInt_AsLong(p->pKey) == COInt_AsLong(key)) {
                    return p;
                }
            } else {
                // TODO errors
                assert(0);
            }
        }

        /* key not equal */
        p = p->pNext;
    }

    return NULL;
}

static int
_dict_insert(CODictObject *this, COObject *key, COObject *item)
{
    DictBucket *p;
    ulong h = COObject_hash(key);
    p = (DictBucket *)xmalloc(sizeof(DictBucket));
    p->pKey = key;
    p->pData = item;
    p->h = h;

    uint nIndex = h & this->nTableMask;

    // connect to bucket dllist
    p->pNext = this->arBuckets[nIndex];
    p->pLast = NULL;
    if (p->pNext) {
        p->pNext->pLast = p;
    }

    // connect to global dllist
    p->pListLast = this->pListTail;
    this->pListTail = p;
    p->pListNext = NULL;
    if (p->pListLast != NULL) {
        p->pListLast->pListNext = p;
    }
    if (this->pListHead == NULL) {
        this->pListHead = p;
    }                                                   

    this->arBuckets[nIndex] = p;
    this->nNumOfElements++;

    if (this->nNumOfElements > this->nTableSize) {
        _dict_do_resize(this);
    }

    return 0;
}

void
_dict_destory(CODictObject *this)
{
    DictBucket *p, *q;

    p = this->pListHead;
    while (p != NULL) {
        q = p;
        free(q->pData);
        free(q);
        p = p->pListNext;
    }
    free(this->arBuckets);
}

COObject *
CODict_New(void)
{
    if (dummy == NULL) {
        dummy = COStr_FromString("<dummy key>");
    }
    DictBucket **tmp;
    CODictObject *dict = COObject_New(CODictObject, &CODict_Type);

    dict->nTableSize = CODict_MINSIZE;

    dict->nTableMask = dict->nTableSize - 1;
    dict->arBuckets = NULL;
    dict->nNumOfElements = 0;
    tmp = (DictBucket **)xcalloc(dict->nTableSize, sizeof(DictBucket *));
    dict->arBuckets = tmp;

    return (COObject *)dict;
}

/*
 * Return NULL if the key is not present.
 */
COObject *
CODict_GetItem(COObject *this, COObject *key)
{
    DictBucket *p = _dict_lookup((CODictObject *)this, key);
    if (!p) {
        return NULL;
    }
    return p->pData;
}

int
CODict_SetItem(COObject *this, COObject *key, COObject *item)
{
    DictBucket *p = _dict_lookup((CODictObject *)this, key);
    if (!p) {
        return _dict_insert((CODictObject *)this, key, item);
    }

    p->pData = item;

    return 0;
}

int
CODict_DelItem(COObject *this, COObject *key)
{
    DictBucket *p = _dict_lookup((CODictObject *)this, key);
    if (!p) {
        return -1;
    }
    
    p->pKey = dummy;

    ((CODictObject *)this)->nNumOfElements--;

    return 0;
}

void
CODict_Clear(COObject *this)
{
    DictBucket *p, *q;

    p = ((CODictObject *)this)->pListHead;
    while (p != NULL) {
        q = p;
        free(q->pData);
        free(q);
        p = p->pListNext;
    }
    memset(((CODictObject *)this)->arBuckets, 0, ((CODictObject *)this)->nTableSize * sizeof(DictBucket *));
    ((CODictObject *)this)->pListHead = NULL;
    ((CODictObject *)this)->pListTail = NULL;
    ((CODictObject *)this)->nNumOfElements = 0;
}

size_t
CODict_Size(COObject *this)
{
    return ((CODictObject *)this)->nNumOfElements;
}
