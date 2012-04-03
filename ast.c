#include "co.h"

/*
 * Create a node.
 */
Node *
node_new(Node_Type type, Node *nleft, Node *nright)
{
    Node *n;
    n = COMem_MALLOC(sizeof(*n));
    n->type = type;
    n->left = nleft;
    n->right = nright;
    return n;
}

/*
 * Append a node into list.
 */
NodeList *
node_append(NodeList *l, Node *n)
{
    return node_concat(l, node_list(n, NULL));
}
 
/*
 * Concatenate two list.
 */
NodeList *
node_concat(NodeList *a, NodeList *b)
{
    a->end->next = b;
    a->end = b->end;
    b->end = NULL;
    return a;
}

/* 
 * Construct a node list from list of nodes, last arg should be NULL to end
 * argument list.
 */
NodeList *
node_list(Node *n, ...)
{
    va_list params;

    va_start(params, n);

    NodeList *l;

    if (n) {
        l = COMem_MALLOC(sizeof(*l));
        l->n = n;
        l->next = NULL;
        l->end = l;
    }

    while (true) {
        n = va_arg(params, Node *);
        if (!n)
            break;
        l = node_concat(l, node_list(n, NULL));
    }

    va_end(params);
    return l;
}

const char *
node_type(Node_Type type)
{
#define GIVE_NAME(type) \
    case (type):        \
        return #type

    switch (type) {
        GIVE_NAME(NODE_IF);
        GIVE_NAME(NODE_WHILE);
        GIVE_NAME(NODE_BIN);
        GIVE_NAME(NODE_RETURN);
        GIVE_NAME(NODE_CONST);
        GIVE_NAME(NODE_NAME);
        GIVE_NAME(NODE_ASSIGN);
        GIVE_NAME(NODE_PRINT);
        GIVE_NAME(NODE_LIST_BUILD);
        GIVE_NAME(NODE_LIST_ADD);
        GIVE_NAME(NODE_DICT_BUILD);
        GIVE_NAME(NODE_DICT_ADD);
    }
    error("unknow type: %d\n", type);
    return NULL;
}

static void
indent_printf(int level, const char *fmt, ...)
{
    va_list params;
    va_start(params, fmt);

    for (int i = 0; i < level; i++)
        printf("----");
    vprintf(fmt, params);

    va_end(params);
}

void
node_print(Node *n)
{
    static int level = 1;
    indent_printf(level, "Node(%p, %s)\n", n, node_type(n->type));
    level++;
    if (n->left) {
        indent_printf(level, "->left\n", n, node_type(n->type));
        node_print(n->left);
    }
    if (n->right) {
        indent_printf(level, "->right\n", n, node_type(n->type));
        node_print(n->right);
    }
    if (n->list) {
        indent_printf(level, "->list\n", n, node_type(n->type));
        node_listtree(n->list);
    }
    level--;
}

/*
 * List node tree.
 */
void
node_listtree(NodeList *l)
{
    indent_printf(0, "NodeList(%)\n", l);
    Node *n;
    for (; l; l = l->next) {
        n = l->n;
        node_print(n);
    }
}