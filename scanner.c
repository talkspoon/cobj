/* Generated by re2c 0.13.5 on Wed Feb 15 18:48:19 2012 */
#line 1 "scanner.l"
#include "co.h"
#include "compile.h"
#include "scanner.h"
#include "parser.h"
#include "dstring.h"

struct co_scanner_globals {
    int yy_in;

    unsigned int yy_leng;
    char *yy_start;
    char *yy_text;
    char *yy_cursor;
    char *yy_marker;
    char *yy_limit;
    int yy_state;
    co_stack state_stack;
};

struct co_scanner_globals scanner_globals;

#define YYMAXFILL 6

/*** S re2c interface code ***/
#define YYCTYPE     unsigned char
#define YYFILL(n)   { if ((YYCURSOR + n) >= (YYLIMIT + YYMAXFILL)) { return 0; }}
#define YYCURSOR    scanner_globals.yy_cursor
#define YYLIMIT     scanner_globals.yy_limit
#define YYMARKER    scanner_globals.yy_marker
#define YYGETCONDITION()  scanner_globals.yy_state
#define YYSETCONDITION(s) scanner_globals.yy_state = s
#ifdef CO_DEBUG
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
co_scan_singlequoted_string(cval *cval)
{
    register char *s, *t;
    char *end;

    s = t = cval->u.str.val;
    end = s + cval->u.str.len;
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
                cval->u.str.len--;
                break;
            case '\\':
                *t++ = *s;
                cval->u.str.len--;
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
co_scan_doublequoted_string(cval *cval)
{
    register char *s, *t;
    char *end;

    s = t = cval->u.str.val;
    end = s + cval->u.str.len;
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
                cval->u.str.len--;
                break;
            case 'b':
                *t++ = '\b';
                cval->u.str.len--;
                break;
            case 'f':
                *t++ = '\f';
                cval->u.str.len--;
                break;
            case 'n':
                *t++ = '\n';
                cval->u.str.len--;
                break;
            case 'r':
                *t++ = '\r';
                cval->u.str.len--;
                break;
            case 't':
                *t++ = '\t';
                cval->u.str.len--;
                break;
            case 'v':
                *t++ = '\v';
                cval->u.str.len--;
                break;
            case '"':
                *t++ = '"';
                cval->u.str.len--;
                break;
            case '\\':
                *t++ = *s;
                cval->u.str.len--;
                break;
            case 'x':
                if (CO_IS_HEX(*(s+1)) && CO_IS_HEX(*(s+2))) {
                    char hex_buf[3] = {0, 0, 0};
                    cval->u.str.len--; /* for the 'x' */

                    hex_buf[0] = *(++s);
                    cval->u.str.len--;
                    hex_buf[1] = *(++s);
                    cval->u.str.len--;
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
                    cval->u.str.len--;
                    if (CO_IS_OCT(*(s+1))) {
                        octal_buf[1] = *(++s);
                        cval->u.str.len--;
                        if (CO_IS_OCT(*(s+1))) {
                            octal_buf[2] = *(++s);
                            cval->u.str.len--;
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
co_scanner_startup()
{
    co_stack_init(&scanner_globals.state_stack);
}

void
co_scanner_shutdown()
{
    co_stack_destory(&scanner_globals.state_stack);
}

int
co_scanner_openfile(int fd)
{
    scanner_globals.yy_in = fd;
    scanner_globals.yy_start = NULL;
    dstring code = dstring_readfd(fd);
    scanner_globals.yy_cursor = code;
    scanner_globals.yy_limit = code + strlen(code);

    return 0;
}

int
co_scanner_lex(cnode *yylval, void *compiler_globals)
{
#ifdef CO_DEBUG
printf("DEBUG: %s\n", YYCURSOR);
#endif
restart:
    scanner_globals.yy_text = YYCURSOR;


#line 220 "scanner.c"
{
	YYCTYPE yych;
	if (YYGETCONDITION() < 1) {
		goto yyc_SC_INIT;
	} else {
		goto yyc_SC_DOUBLE_QUOTES;
	}
/* *********************************** */
yyc_SC_DOUBLE_QUOTES:

	YYDEBUG(0, *YYCURSOR);
	YYFILL(1);
	yych = *YYCURSOR;
	if (yych != '"') goto yy4;
	YYDEBUG(2, *YYCURSOR);
	++YYCURSOR;
	YYDEBUG(3, *YYCURSOR);
	yyleng = YYCURSOR - scanner_globals.yy_text;
#line 295 "scanner.l"
	{
    BEGIN(SC_INIT);
    return T_IGNORED;
}
#line 244 "scanner.c"
yy4:
	YYDEBUG(4, *YYCURSOR);
	++YYCURSOR;
	YYDEBUG(5, *YYCURSOR);
	yyleng = YYCURSOR - scanner_globals.yy_text;
#line 300 "scanner.l"
	{
    if (YYCURSOR > YYLIMIT) {
        return 0;
    }
    if (yytext[0] == '\\' && YYCURSOR < YYLIMIT) {
        YYCURSOR++;
    }
    while (YYCURSOR < YYLIMIT) {
        switch (*YYCURSOR++) {
            case '"':
                break;
            case '\\':
                if (YYCURSOR < YYLIMIT) {
                    YYCURSOR++;
                }
                /* fall through */
            default:
                continue;
        }

        YYCURSOR--;
        break;
    }
    yyleng = YYCURSOR - scanner_globals.yy_text;
    yylval->u.val.u.str.val = xstrndup(yytext, yyleng);
    yylval->u.val.u.str.len = yyleng;
    co_scan_doublequoted_string(&yylval->u.val);
    yylval->u.val.type = CVAL_IS_STRING;
    yylval->op_type = IS_CONST;
    return T_STRING;
}
#line 282 "scanner.c"
/* *********************************** */
yyc_SC_INIT:
	{
		static const unsigned char yybm[] = {
			 32,  32,  32,  32,  32,  32,  32,  32, 
			 32,  48,  16,  32,  32,  32,  32,  32, 
			 32,  32,  32,  32,  32,  32,  32,  32, 
			 32,  32,  32,  32,  32,  32,  32,  32, 
			 48,  32,  32,  32,  32,  32,  32,  32, 
			 32,  32,  32,  32,  32,  32,  32,  32, 
			224, 224, 224, 224, 224, 224, 224, 224, 
			224, 224,  32,  32,  32,  32,  32,  32, 
			 32,  96,  96,  96,  96,  96,  96,  96, 
			 96,  96,  96,  96,  96,  96,  96,  96, 
			 96,  96,  96,  96,  96,  96,  96,  96, 
			 96,  96,  96,  32,  32,  32,  32,  96, 
			 32,  96,  96,  96,  96,  96,  96,  96, 
			 96,  96,  96,  96,  96,  96,  96,  96, 
			 96,  96,  96,  96,  96,  96,  96,  96, 
			 96,  96,  96,  32,  32,  32,  32,  32, 
			 32,  32,  32,  32,  32,  32,  32,  32, 
			 32,  32,  32,  32,  32,  32,  32,  32, 
			 32,  32,  32,  32,  32,  32,  32,  32, 
			 32,  32,  32,  32,  32,  32,  32,  32, 
			 32,  32,  32,  32,  32,  32,  32,  32, 
			 32,  32,  32,  32,  32,  32,  32,  32, 
			 32,  32,  32,  32,  32,  32,  32,  32, 
			 32,  32,  32,  32,  32,  32,  32,  32, 
			 32,  32,  32,  32,  32,  32,  32,  32, 
			 32,  32,  32,  32,  32,  32,  32,  32, 
			 32,  32,  32,  32,  32,  32,  32,  32, 
			 32,  32,  32,  32,  32,  32,  32,  32, 
			 32,  32,  32,  32,  32,  32,  32,  32, 
			 32,  32,  32,  32,  32,  32,  32,  32, 
			 32,  32,  32,  32,  32,  32,  32,  32, 
			 32,  32,  32,  32,  32,  32,  32,  32, 
		};
		YYDEBUG(6, *YYCURSOR);
		YYFILL(6);
		yych = *YYCURSOR;
		YYDEBUG(-1, yych);
		switch (yych) {
		case '\t':
		case '\n':
		case ' ':	goto yy30;
		case '"':	goto yy21;
		case '#':	goto yy28;
		case '%':
		case '(':
		case ')':
		case '*':
		case '+':
		case ',':
		case '-':
		case '/':
		case ':':
		case ';':
		case '<':
		case '=':
		case '>':
		case '^':
		case '{':
		case '}':	goto yy26;
		case '\'':	goto yy23;
		case '.':	goto yy10;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':	goto yy8;
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		case '_':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'g':
		case 'h':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'x':
		case 'y':
		case 'z':	goto yy25;
		case 'F':	goto yy20;
		case 'N':	goto yy18;
		case 'T':	goto yy19;
		case 'e':	goto yy14;
		case 'f':	goto yy16;
		case 'i':	goto yy12;
		case 'p':	goto yy17;
		case 'w':	goto yy15;
		default:	goto yy32;
		}
yy8:
		YYDEBUG(8, *YYCURSOR);
		yych = *(YYMARKER = ++YYCURSOR);
		goto yy78;
yy9:
		YYDEBUG(9, *YYCURSOR);
		yyleng = YYCURSOR - scanner_globals.yy_text;
#line 230 "scanner.l"
		{
    if (yyleng < MAX_LENGTH_OF_LONG - 1) { /* Won't overflow */
        yylval->u.val.u.ival = strtol(yytext, NULL, 0); 
    } else { /* Maybe overflow */
        errno = 0;
        yylval->u.val.u.ival = strtol(yytext, NULL, 0); 
    }   
    yylval->u.val.type = CVAL_IS_INT;
    yylval->op_type = IS_CONST;
    return T_NUM;
}
#line 432 "scanner.c"
yy10:
		YYDEBUG(10, *YYCURSOR);
		++YYCURSOR;
		if (yybm[0+(yych = *YYCURSOR)] & 128) {
			goto yy72;
		}
yy11:
		YYDEBUG(11, *YYCURSOR);
		yyleng = YYCURSOR - scanner_globals.yy_text;
#line 373 "scanner.l"
		{
    die("invalid token: %s", yytext);
}
#line 446 "scanner.c"
yy12:
		YYDEBUG(12, *YYCURSOR);
		++YYCURSOR;
		if ((yych = *YYCURSOR) == 'f') goto yy70;
		goto yy38;
yy13:
		YYDEBUG(13, *YYCURSOR);
		yyleng = YYCURSOR - scanner_globals.yy_text;
#line 353 "scanner.l"
		{
    yylval->u.val.u.str.val = xstrndup(yytext, yyleng);
    yylval->u.val.u.str.len = yyleng;
    yylval->u.val.type = CVAL_IS_STRING;
    yylval->op_type = IS_VAR;
    return T_NAME;
}
#line 463 "scanner.c"
yy14:
		YYDEBUG(14, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yych == 'l') goto yy66;
		goto yy38;
yy15:
		YYDEBUG(15, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yych == 'h') goto yy61;
		goto yy38;
yy16:
		YYDEBUG(16, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yych == 'u') goto yy57;
		goto yy38;
yy17:
		YYDEBUG(17, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yych == 'r') goto yy52;
		goto yy38;
yy18:
		YYDEBUG(18, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yych == 'o') goto yy48;
		goto yy38;
yy19:
		YYDEBUG(19, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yych == 'r') goto yy44;
		goto yy38;
yy20:
		YYDEBUG(20, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yych == 'a') goto yy39;
		goto yy38;
yy21:
		YYDEBUG(21, *YYCURSOR);
		++YYCURSOR;
		YYDEBUG(22, *YYCURSOR);
		yyleng = YYCURSOR - scanner_globals.yy_text;
#line 290 "scanner.l"
		{
    BEGIN(SC_DOUBLE_QUOTES);
    return T_IGNORED;
}
#line 509 "scanner.c"
yy23:
		YYDEBUG(23, *YYCURSOR);
		++YYCURSOR;
		YYDEBUG(24, *YYCURSOR);
		yyleng = YYCURSOR - scanner_globals.yy_text;
#line 332 "scanner.l"
		{
    while (1) {
        if (YYCURSOR > YYLIMIT) {
            return 0;
        }
        if (*YYCURSOR == '\'') {
            YYCURSOR++;
            yyleng = YYCURSOR - scanner_globals.yy_text;
            break;
        } else if (*YYCURSOR++ == '\\') {
            YYCURSOR++;
        }
    } 
    yylval->u.val.u.str.val = xstrndup(yytext + 1, yyleng - 2);
    yylval->u.val.u.str.len = yyleng;
    co_scan_singlequoted_string(&yylval->u.val);
    yylval->u.val.type = CVAL_IS_STRING;
    yylval->op_type = IS_CONST;
    return T_STRING;
}
#line 536 "scanner.c"
yy25:
		YYDEBUG(25, *YYCURSOR);
		yych = *++YYCURSOR;
		goto yy38;
yy26:
		YYDEBUG(26, *YYCURSOR);
		++YYCURSOR;
		YYDEBUG(27, *YYCURSOR);
		yyleng = YYCURSOR - scanner_globals.yy_text;
#line 361 "scanner.l"
		{
    return yytext[0];
}
#line 550 "scanner.c"
yy28:
		YYDEBUG(28, *YYCURSOR);
		++YYCURSOR;
		yych = *YYCURSOR;
		goto yy36;
yy29:
		YYDEBUG(29, *YYCURSOR);
		yyleng = YYCURSOR - scanner_globals.yy_text;
#line 365 "scanner.l"
		{
    return T_COMMENT;
}
#line 563 "scanner.c"
yy30:
		YYDEBUG(30, *YYCURSOR);
		++YYCURSOR;
		yych = *YYCURSOR;
		goto yy34;
yy31:
		YYDEBUG(31, *YYCURSOR);
		yyleng = YYCURSOR - scanner_globals.yy_text;
#line 369 "scanner.l"
		{ 
    return T_WHITESPACE;
}
#line 576 "scanner.c"
yy32:
		YYDEBUG(32, *YYCURSOR);
		yych = *++YYCURSOR;
		goto yy11;
yy33:
		YYDEBUG(33, *YYCURSOR);
		++YYCURSOR;
		YYFILL(1);
		yych = *YYCURSOR;
yy34:
		YYDEBUG(34, *YYCURSOR);
		if (yybm[0+yych] & 16) {
			goto yy33;
		}
		goto yy31;
yy35:
		YYDEBUG(35, *YYCURSOR);
		++YYCURSOR;
		YYFILL(1);
		yych = *YYCURSOR;
yy36:
		YYDEBUG(36, *YYCURSOR);
		if (yybm[0+yych] & 32) {
			goto yy35;
		}
		goto yy29;
yy37:
		YYDEBUG(37, *YYCURSOR);
		++YYCURSOR;
		YYFILL(1);
		yych = *YYCURSOR;
yy38:
		YYDEBUG(38, *YYCURSOR);
		if (yybm[0+yych] & 64) {
			goto yy37;
		}
		goto yy13;
yy39:
		YYDEBUG(39, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yych != 'l') goto yy38;
		YYDEBUG(40, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yych != 's') goto yy38;
		YYDEBUG(41, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yych != 'e') goto yy38;
		YYDEBUG(42, *YYCURSOR);
		++YYCURSOR;
		if (yybm[0+(yych = *YYCURSOR)] & 64) {
			goto yy37;
		}
		YYDEBUG(43, *YYCURSOR);
		yyleng = YYCURSOR - scanner_globals.yy_text;
#line 283 "scanner.l"
		{
    yylval->u.val.u.ival = 0;
    yylval->u.val.type = CVAL_IS_BOOL;
    yylval->op_type = IS_CONST;
    return T_BOOL;
}
#line 638 "scanner.c"
yy44:
		YYDEBUG(44, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yych != 'u') goto yy38;
		YYDEBUG(45, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yych != 'e') goto yy38;
		YYDEBUG(46, *YYCURSOR);
		++YYCURSOR;
		if (yybm[0+(yych = *YYCURSOR)] & 64) {
			goto yy37;
		}
		YYDEBUG(47, *YYCURSOR);
		yyleng = YYCURSOR - scanner_globals.yy_text;
#line 276 "scanner.l"
		{
    yylval->u.val.u.ival = 1;
    yylval->u.val.type = CVAL_IS_BOOL;
    yylval->op_type = IS_CONST;
    return T_BOOL;
}
#line 660 "scanner.c"
yy48:
		YYDEBUG(48, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yych != 'n') goto yy38;
		YYDEBUG(49, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yych != 'e') goto yy38;
		YYDEBUG(50, *YYCURSOR);
		++YYCURSOR;
		if (yybm[0+(yych = *YYCURSOR)] & 64) {
			goto yy37;
		}
		YYDEBUG(51, *YYCURSOR);
		yyleng = YYCURSOR - scanner_globals.yy_text;
#line 269 "scanner.l"
		{
    yylval->u.val.u.ival = 1;
    yylval->u.val.type = CVAL_IS_NONE;
    yylval->op_type = IS_CONST;
    return T_NONE;
}
#line 682 "scanner.c"
yy52:
		YYDEBUG(52, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yych != 'i') goto yy38;
		YYDEBUG(53, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yych != 'n') goto yy38;
		YYDEBUG(54, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yych != 't') goto yy38;
		YYDEBUG(55, *YYCURSOR);
		++YYCURSOR;
		if (yybm[0+(yych = *YYCURSOR)] & 64) {
			goto yy37;
		}
		YYDEBUG(56, *YYCURSOR);
		yyleng = YYCURSOR - scanner_globals.yy_text;
#line 265 "scanner.l"
		{
    return T_PRINT;
}
#line 704 "scanner.c"
yy57:
		YYDEBUG(57, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yych != 'n') goto yy38;
		YYDEBUG(58, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yych != 'c') goto yy38;
		YYDEBUG(59, *YYCURSOR);
		++YYCURSOR;
		if (yybm[0+(yych = *YYCURSOR)] & 64) {
			goto yy37;
		}
		YYDEBUG(60, *YYCURSOR);
		yyleng = YYCURSOR - scanner_globals.yy_text;
#line 261 "scanner.l"
		{
    return T_FUNC;
}
#line 723 "scanner.c"
yy61:
		YYDEBUG(61, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yych != 'i') goto yy38;
		YYDEBUG(62, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yych != 'l') goto yy38;
		YYDEBUG(63, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yych != 'e') goto yy38;
		YYDEBUG(64, *YYCURSOR);
		++YYCURSOR;
		if (yybm[0+(yych = *YYCURSOR)] & 64) {
			goto yy37;
		}
		YYDEBUG(65, *YYCURSOR);
		yyleng = YYCURSOR - scanner_globals.yy_text;
#line 257 "scanner.l"
		{
    return T_WHILE;
}
#line 745 "scanner.c"
yy66:
		YYDEBUG(66, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yych != 's') goto yy38;
		YYDEBUG(67, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yych != 'e') goto yy38;
		YYDEBUG(68, *YYCURSOR);
		++YYCURSOR;
		if (yybm[0+(yych = *YYCURSOR)] & 64) {
			goto yy37;
		}
		YYDEBUG(69, *YYCURSOR);
		yyleng = YYCURSOR - scanner_globals.yy_text;
#line 253 "scanner.l"
		{
    return T_ELSE;
}
#line 764 "scanner.c"
yy70:
		YYDEBUG(70, *YYCURSOR);
		++YYCURSOR;
		if (yybm[0+(yych = *YYCURSOR)] & 64) {
			goto yy37;
		}
		YYDEBUG(71, *YYCURSOR);
		yyleng = YYCURSOR - scanner_globals.yy_text;
#line 249 "scanner.l"
		{
    return T_IF;
}
#line 777 "scanner.c"
yy72:
		YYDEBUG(72, *YYCURSOR);
		++YYCURSOR;
		YYFILL(1);
		yych = *YYCURSOR;
		YYDEBUG(73, *YYCURSOR);
		if (yybm[0+yych] & 128) {
			goto yy72;
		}
		YYDEBUG(74, *YYCURSOR);
		yyleng = YYCURSOR - scanner_globals.yy_text;
#line 242 "scanner.l"
		{
    yylval->u.val.u.fval = strtof(yytext, NULL);
    yylval->u.val.type = CVAL_IS_FLOAT;
    yylval->op_type = IS_CONST;
    return T_FNUM;
}
#line 796 "scanner.c"
yy75:
		YYDEBUG(75, *YYCURSOR);
		yych = *++YYCURSOR;
		if (yybm[0+yych] & 128) {
			goto yy72;
		}
		YYDEBUG(76, *YYCURSOR);
		YYCURSOR = YYMARKER;
		goto yy9;
yy77:
		YYDEBUG(77, *YYCURSOR);
		YYMARKER = ++YYCURSOR;
		YYFILL(2);
		yych = *YYCURSOR;
yy78:
		YYDEBUG(78, *YYCURSOR);
		if (yych == '.') goto yy75;
		if (yych <= '/') goto yy9;
		if (yych <= '9') goto yy77;
		goto yy9;
	}
}
#line 376 "scanner.l"

}