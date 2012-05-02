#include "co.h"

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
int flag_compile = 0;
char *eval = NULL;

void
cli_completion(const char *buf, linenoiseCompletions *lc)
{
    static const char *words[] = {
        "print",
        "if",
        "else",
        "elif",
        "try",
        "throw",
        "catch",
        "finally",
        "func",
        "while",
        "end",
        "None",
        "True",
        "False",
    };

    for (int i = 0; i < ARRAY_SIZE(words); i++) {
        if (!prefixcmp(words[i], buf)) {
            linenoiseAddCompletion(lc, (char *)words[i]);
        }
    }
}

COObject *
eval_wrapper(COObject *func)
{
    COObject *ret;

    ret = vm_eval(func);
    if (!ret) {
        if (COErr_Occurred()) {
            COErr_Print();
            return NULL;
        }
    }
    return ret;
}

int
run_file(FILE *fp, const char *filename)
{
    int exit_code = 0;
    struct arena *arena = arena_new();
    scanner_init(arena);
    COObject *f = COFile_FromFile(fp, (char *)filename, "r", fclose);
    COObject *source = COFile_Read(f, -1);
    scanner_setcode(COBytes_AsString(source));
    COObject *code = compile(arena);
    COObject *func = COFunction_New(code);
    exit_code = eval_wrapper(func) ? 0 : -1;
    CO_DECREF(source);
    CO_DECREF(f);
    CO_DECREF(code);
    /*COObject_dump(code);*/
    /*COObject_dump(func);*/
    CO_DECREF(func);
    COThreadState_DeleteCurrent();
    arena_free(arena);

    // check gc
    /*COObject_dump(CO_None);*/
    /*COObject_dump(CO_True);*/
    /*COObject_dump(CO_False);*/
    return exit_code;
}

int
run_string(const char *str)
{
    struct arena *arena = arena_new();
    scanner_init(arena);
    scanner_setcode((char*)str);
    COObject *code = compile(arena);
    COObject *func = COFunction_New(code);
    int exit_code = eval_wrapper(func) ? 0 : -1;
    CO_DECREF(code);
    /*COObject_dump(code);*/
    /*COObject_dump(func);*/
    CO_DECREF(func);
    COThreadState_DeleteCurrent();
    arena_free(arena);
    return exit_code;
}

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
        OPT_END(),
    };
    argparse_init(&argparse, options, usagestr, 0);
    argc = argparse_parse(&argparse, argc, argv);

    /* Init */
    COInt_Init();
    threadstate_current = COThreadState_New();

    /* test only */
    if (verbose) {
        return 0;
    }

    /* compilation */
    if (eval) {
        return run_string(eval);
    } else {
        FILE *f = stdin;
        const char *f_name = "<stdin>";
        if (argc > 0) {
            f = fopen(*argv, "rb");
            if (f == NULL) {
                error("open %s failed", *argv);
            }
            f_name = *argv;
        }

        if (isatty((int)fileno(f))) {
            char *eval;
            linenoiseSetCompletionCallback(cli_completion);
            char *home = getenv("HOME");
            char history[PATH_MAX];
            char *history_path = NULL;
            if (home) {
                history_path =
                    mksnpath(history, sizeof(history), "%s/.co_history", home);
                linenoiseHistoryLoad(history_path);
            }
            printf("COObject 0.1\n");
            /* Read-Eval-Print Loop */
            struct arena *arena;
            arena = arena_new();
            while ((eval = linenoise(">>> ")) != NULL) {
                scanner_setcode(eval);

                COObject *code = compile(arena);
                COObject *func = COFunction_New(code);
                eval_wrapper(func);
                CO_DECREF(code);
                CO_DECREF(func);

                linenoiseHistoryAdd(eval);
                if (history_path) {
                    linenoiseHistorySave(history_path);
                }
                free(code);
            }
            arena_free(arena);
            return 0;
        } else {
            return run_file(f, f_name);
        }
    }
}
