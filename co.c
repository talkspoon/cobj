#include "co.h"

/* Change whenever the bytecode emmited by the compiler may no longer be
 * understood by old code evaluator.
 */
#define CODEDUMP_MAGIC  (314 << 16 | 'c' << 8 | 'o')

static const char *const usagestr[] = {
    "co [options] [file] [args]",
    NULL,
};

int
argparse_showversion(struct argparse *this,
                     const struct argparse_option *option)
{
    printf("CObject 0.01\n");
    exit(1);
}

int verbose = 0;
int compile = 0;
char *eval = NULL;
char *compile_outfile = NULL;

int
main(int argc, const char **argv)
{
    struct argparse argparse;
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_BOOLEAN('v', "version", NULL, "print the version number and exit",
                    argparse_showversion),
        OPT_BOOLEAN('V', "verbose", &verbose,
                    "show runtime info, can be supplied multiple times to increase verbosity",
                    NULL),
        OPT_STRING('e', "eval", &eval, "code passed as string", NULL),
        OPT_BOOLEAN('c', "compile", &compile, "compile only, write compiled code to file", NULL),
        OPT_STRING('o', "outfile", &compile_outfile, "compile output file, defaults to `co.out`", NULL),
        OPT_END(),
    };
    argparse_init(&argparse, options, usagestr);
    argc = argparse_parse(&argparse, argc, argv);

    if (verbose) {
        COObject *co = COList_New(0);
        COList_Append(co, COInt_FromLong(1));
        COList_Append(co, COInt_FromLong(2));
        COList_Append(co, COInt_FromLong(3));
        COObject *s = COObject_serialize(COList_AsTuple(co));
        COObject_dump(s);
        COObject_dump(COObject_unserialize(s));
        return 0;
    }

    /* compilation */
    co_scanner_startup();
    if (eval) {
        co_scanner_setcode(eval);
    } else {
        int fd = 0;
        if (argc > 0) {
            fd = open(*argv, O_RDONLY);
            if (fd < 0) {
                error("open %s failed", *argv);
            }
            // detect if it's a code cache
            COObject *first_object;
            FILE *outfile = fdopen(fd, "r");
            first_object = COObject_unserializeFromFile(outfile);
            if (first_object && COInt_Check(first_object) && COInt_AsLong(first_object) == CODEDUMP_MAGIC) {
                COObject *code = COObject_unserializeFromFile(outfile);
                if (!code) {
                    // TODO invalid code dump
                    return -1;
                }
                co_vm_execute(code);
                return 0;
            }
        }
        lseek(fd, 0, SEEK_SET);
        co_scanner_openfile(fd);
    }

    COCodeObject *co;
    co = co_compile();
    co_scanner_shutdown();

    if (compile) {
        FILE *f;
        if (compile_outfile) {
            f = fopen(compile_outfile, "w");
        } else {
            f = fopen("co.out", "w");
        }
        COObject *co_compiled_str = COObject_serialize((COObject *)co);
        COObject *code_magic = COObject_serialize(COInt_FromLong(CODEDUMP_MAGIC)); 
        fwrite(COBytes_AsString(code_magic), CO_SIZE(code_magic), sizeof(char), f);
        fwrite(COBytes_AsString(co_compiled_str), CO_SIZE(co_compiled_str), sizeof(char), f);
        return 0;
    }

    co_vm_execute(co);
    return 0;
}
