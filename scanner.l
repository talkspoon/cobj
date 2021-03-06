#include "cobj.h"

struct scanner_globals {
    unsigned int yy_leng;
    char *yy_text;
    char *yy_cursor;
    char *yy_marker;
    char *yy_limit;
    int yy_state;
    struct arena *arena;
};

struct scanner_globals scanner_globals;

/*!max:re2c*/

/*** S re2c interface code ***/
#define YYCTYPE     unsigned char
#define YYFILL(n)   { if ((YYCURSOR + n) >= (YYLIMIT + YYMAXFILL)) { return 0; }}
#define YYCURSOR    scanner_globals.yy_cursor
#define YYLIMIT     scanner_globals.yy_limit
#define YYMARKER    scanner_globals.yy_marker
#define YYGETCONDITION()  scanner_globals.yy_state
#define YYSETCONDITION(s) scanner_globals.yy_state = s
#undef YYDEBUG      /* conflicts with bison */
#ifdef YY_DEBUG
# define YYDEBUG(s, c)   printf("state: %d char: %c\n", s, c)
#else
# define YYDEBUG(s, c)
#endif
#define STATE(name) yyc##name
/*** E re2c interface code ***/

/*** S emulate flex constructs */
#define BEGIN(state)    YYSETCONDITION(STATE(state))
#define YYSTATE         YYGETCONDITION()
#define yytext      ((char*)scanner_globals.yy_text)
#define yyleng      scanner_globals.yy_leng
/*** E emulate flex constructs */

#define CO_IS_OCT(c)    ((c)>='0' && (c)<='7')
#define CO_IS_HEX(c)    (((c)>='0' && (c)<='9') || ((c)>='a' && (c)<='f') || ((c)>='A' && (c)<='F'))

static void
co_scan_singlequoted_string(COStrObject *co)
{
    char *s, *t;
    char *end;

    s = t = co->co_sval;
    end = s + CO_SIZE(co);
    while (s < end) {
        if (*s == '\\') {
            s++;
            if (s >= end) {
                *t++ = '\\';
                break;
            }
            switch (*s) {
            case '\'':
                *t++ = '\'';
                CO_SIZE(co)--;
                break;
            case '\\':
                *t++ = *s;
                CO_SIZE(co)--;
                break;
            default:
                *t++ = '\\';
                *t++ = *s;
                break;
            }
        } else {
            *t++ = *s;
        }
        s++;
    }
    *t = 0;
}

static void
co_scan_doublequoted_string(COStrObject *co)
{
    char *s, *t;
    char *end;

    s = t = co->co_sval;
    end = s + CO_SIZE(co);
    while (s < end) {
        if (*s == '\\') {
            s++;
            if (s >= end) {
                *t++ = '\\';
                break;
            }
            switch (*s) {
            case 'a':
                *t++ = '\a';
                CO_SIZE(co)--;
                break;
            case 'b':
                *t++ = '\b';
                CO_SIZE(co)--;
                break;
            case 'f':
                *t++ = '\f';
                CO_SIZE(co)--;
                break;
            case 'n':
                *t++ = '\n';
                CO_SIZE(co)--;
                break;
            case 'r':
                *t++ = '\r';
                CO_SIZE(co)--;
                break;
            case 't':
                *t++ = '\t';
                CO_SIZE(co)--;
                break;
            case 'v':
                *t++ = '\v';
                CO_SIZE(co)--;
                break;
            case '"':
                *t++ = '"';
                CO_SIZE(co)--;
                break;
            case '\\':
                *t++ = *s;
                CO_SIZE(co)--;
                break;
            case 'x':
                if (CO_IS_HEX(*(s+1)) && CO_IS_HEX(*(s+2))) {
                    char hex_buf[3] = {0, 0, 0};
                    CO_SIZE(co)--; /* for the 'x' */

                    hex_buf[0] = *(++s);
                    CO_SIZE(co)--;
                    hex_buf[1] = *(++s);
                    CO_SIZE(co)--;
                    *t++ = (char)strtol(hex_buf, NULL, 16);
                } else {
                    *t++ = '\\';
                    *t++ = *s;
                }
                break;
            default:
                /* check for octal digits */
                if (CO_IS_OCT(*s)) {
                    char octal_buf[4] = {0, 0, 0, 0};
                    octal_buf[0] = *s;
                    CO_SIZE(co)--;
                    if (CO_IS_OCT(*(s+1))) {
                        octal_buf[1] = *(++s);
                        CO_SIZE(co)--;
                        if (CO_IS_OCT(*(s+1))) {
                            octal_buf[2] = *(++s);
                            CO_SIZE(co)--;
                        }
                    }
                    *t++ = (char)strtol(octal_buf, NULL, 8);
                } else {
                    *t++ = '\\';
                    *t++ = *s;
                }
                break;
            }
        } else {
            *t++ = *s;
        }
        s++;
    }
    *t = 0;
}

void
scanner_init(struct arena *arena)
{
    scanner_globals.arena = arena;
}

int
scanner_setcode(char *code)
{
    scanner_globals.yy_cursor = code;
    scanner_globals.yy_limit = code + strlen(code);
    return 0;
}

int
scanner_lex(YYSTYPE *yylval)
{
    scanner_globals.yy_text = YYCURSOR;

/*!re2c
re2c:yyfill:check = 0;

NUM         = ([1-9][0-9]*|[0]|"0"[xX][0-9a-fA-F]+|"0"[oO][0-7]+|"0"[bB][01]+);
FNUM        = ([0-9]*"."[0-9]+);
NAME        = [a-zA-Z_][a-zA-Z0-9_]*;
TOKENS      = [-+()=>/<*^,;:{}%\[\]~.];
WHITESPACE  = [ \t]+;
NEWLINE     = [\n];
ANY_CHAR    = [^];

/* compute yyleng before each rule */
<!*> := yyleng = YYCURSOR - scanner_globals.yy_text;

<SC_INIT>NUM {
    char c = yytext[yyleng];
    yytext[yyleng] = '\0';
    yylval->node = node_new(scanner_globals.arena, NODE_CONST, NULL, NULL);
    yylval->node->u.o = COInt_FromString(yytext, NULL, 0);
    arena_addobject(scanner_globals.arena, yylval->node->u.o);
    yytext[yyleng] = c;
    return T_NUM;
}

<SC_INIT>FNUM {
    yylval->node = node_new(scanner_globals.arena, NODE_CONST, NULL, NULL);
    yylval->node->u.o = COFloat_FromString(yytext);
    arena_addobject(scanner_globals.arena, yylval->node->u.o);
    return T_FNUM;
}

<SC_INIT>"if" {
    return T_IF;
}

<SC_INIT>"else" {
    return T_ELSE;
}

<SC_INIT>"elif" {
    return T_ELIF;
}

<SC_INIT>"while" {
    return T_WHILE;
}

<SC_INIT>"func" {
    return T_FUNC;
}

<SC_INIT>"local" {
    return T_LOCAL;
}

<SC_INIT>"for" {
    return T_FOR;
}

<SC_INIT>"in" {
    return T_IN;
}


<SC_INIT>"end" {
    return T_END;
}

<SC_INIT>"return" {
    return T_RETURN;
}

<SC_INIT>"try" {
    return T_TRY;
}

<SC_INIT>"catch" {
    return T_CATCH;
}

<SC_INIT>"throw" {
    return T_THROW;
}

<SC_INIT>"finally" {
    return T_FINALLY;
}

<SC_INIT>"do" {
    return T_DO;
}

<SC_INIT>"None" {
    yylval->node = node_new(scanner_globals.arena, NODE_CONST, NULL, NULL);
    yylval->node->u.o = CO_None;
    return T_NONE;
}

<SC_INIT>"True" {
    yylval->node = node_new(scanner_globals.arena, NODE_CONST, NULL, NULL);
    yylval->node->u.o = CO_True;
    return T_BOOL;
}

<SC_INIT>"False" {
    yylval->node = node_new(scanner_globals.arena, NODE_CONST, NULL, NULL);
    yylval->node->u.o = CO_False;
    return T_BOOL;
}

<SC_INIT>"break" {
    return T_BREAK;
}

<SC_INIT>"continue" {
    return T_CONTINUE;
}

<SC_INIT>'"' {
    while (1) {
        if (YYCURSOR > YYLIMIT) {
            break;
        }

        if (*YYCURSOR == '\"') {
            YYCURSOR++;
            yyleng = YYCURSOR - scanner_globals.yy_text;
            break;
        } else if (*YYCURSOR++ == '\\') {
            YYCURSOR++;
        }
    }
    yylval->node = node_new(scanner_globals.arena, NODE_CONST, NULL, NULL);
    yylval->node->u.o = COStr_FromStringN(yytext + 1, yyleng - 2);
    co_scan_doublequoted_string((COStrObject *)yylval->node->u.o);
    arena_addobject(scanner_globals.arena, yylval->node->u.o);
    return T_STRING;
}

<SC_INIT>"'" {
    while (1) {
        if (YYCURSOR > YYLIMIT) {
            break;
        }
        if (*YYCURSOR == '\'') {
            YYCURSOR++;
            yyleng = YYCURSOR - scanner_globals.yy_text;
            break;
        } else if (*YYCURSOR++ == '\\') {
            YYCURSOR++;
        }
    } 
    yylval->node = node_new(scanner_globals.arena, NODE_CONST, NULL, NULL);
    yylval->node->u.o = COStr_FromStringN(yytext + 1, yyleng -2);
    co_scan_singlequoted_string((COStrObject *)yylval->node->u.o);
    arena_addobject(scanner_globals.arena, yylval->node->u.o);
    return T_STRING;
}

<SC_INIT>NAME {
    yylval->node = node_new(scanner_globals.arena, NODE_NAME, NULL, NULL);
    yylval->node->u.o = COStr_FromStringN(yytext, yyleng);
    arena_addobject(scanner_globals.arena, yylval->node->u.o);
    return T_NAME;
}

<SC_INIT>"<<" {
    return T_SL;
}

<SC_INIT>">>" {
    return T_SR;
}

<SC_INIT>"+=" {
    return T_ADD_ASSIGN;
}

<SC_INIT>"-=" {
    return T_SUB_ASSIGN;
}

<SC_INIT>"*=" {
    return T_MUL_ASSIGN;
}

<SC_INIT>"/=" {
    return T_DIV_ASSIGN;
}

<SC_INIT>"%=" {
    return T_MOD_ASSIGN;
}

<SC_INIT>"<<=" {
    return T_SL_ASSIGN;
}

<SC_INIT>">>=" {
    return T_SR_ASSIGN;
}

<SC_INIT>"==" {
    return T_EQUAL;
}

<SC_INIT>"!=" {
    return T_NOT_EQUAL;
}

<SC_INIT>"<=" {
    return T_SMALLER_OR_EQUAL;
}

<SC_INIT>">=" {
    return T_GREATER_OR_EQUAL;
}

<SC_INIT>"[" {
    return yytext[0];
}

<SC_INIT>"**" {
    return T_POW;
}

<SC_INIT>TOKENS {
    return yytext[0];
}

<SC_INIT>"#".*   {
    return T_COMMENT;
}

<SC_INIT>WHITESPACE   { 
    return T_WHITESPACE;
}

<SC_INIT>NEWLINE {
    return T_NEWLINE;
}

<SC_INIT>ANY_CHAR   {
    return coerror(NULL, "invalid token: %s", yytext);
}
*/
}
