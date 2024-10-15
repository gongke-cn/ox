/******************************************************************************
 *                                 OX Language                                *
 *                                                                            *
 * Copyright 2024 Gong Ke                                                     *
 *                                                                            *
 * Permission is hereby granted, free of charge, to any person obtaining a    *
 * copy of this software and associated documentation files (the              *
 * "Software"), to deal in the Software without restriction, including        *
 * without limitation the rights to use, copy, modify, merge, publish,        *
 * distribute, sublicense, and/or sell copies of the Software, and to permit  *
 * persons to whom the Software is furnished to do so, subject to the         *
 * following conditions:                                                      *
 *                                                                            *
 * The above copyright notice and this permission notice shall be included    *
 * in all copies or substantial portions of the Software.                     *
 *                                                                            *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS    *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF                 *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN  *
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR      *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE  *
 * USE OR OTHER DEALINGS IN THE SOFTWARE.                                     *
 ******************************************************************************/

/**
 * @file
 * Entry of OX program.
 */

#define OX_LOG_TAG "ox"

#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <libintl.h>
#include <locale.h>
#include <ox_internal.h>

/*Option values.*/
enum {
    OPTION_HELP = 256,
    OPTION_VERSION,
    OPTION_LOG,
    OPTION_LOG_FIELD,
    OPTION_LOG_FILE,
    OPTION_AST,
    OPTION_BC,
    OPTION_PR,
    OPTION_ENC,
    OPTION_DUMP_THROW
};

/*Log file.*/
static FILE *log_file = NULL;
/*Source file.*/
static char *source_filename = NULL;
/*Script source.*/
static char *script_source = NULL;
/*Has package lookup directory in options.*/
static OX_Bool has_dir_opt = OX_FALSE;
/*Parse only.*/
static OX_Bool parse_only = OX_FALSE;
/*Compile only.*/
static OX_Bool compile_only = OX_FALSE;
/*Output AST.*/
static OX_Bool output_ast = OX_FALSE;
/*Output bytecode.*/
static OX_Bool output_bc = OX_FALSE;
/*Print the result.*/
static OX_Bool print_result = OX_FALSE;
/*Run the program managed by package manager.*/
static OX_Bool run_pm_exe = OX_FALSE;

/*Show usage message.*/
static void
show_usage (char *cmd)
{
    printf(OX_TEXT(
"Usage: %s [OPTION]... FILE [OX_OPTION]...\n"
"Option:\n"
"  --ast             Output the abstract syntax tree\n"
"  --bc              Output the bytecode of the source file\n"
"  -c                Only compiles the source file, not running it\n"
"  -d DIR            Add package lookup directory\n"
"  --dump-throw      Dump stack when throw an error\n"
"  --enc ENCODING    Set the files' character encoding\n"
"  --help            Show this help message\n"
"  --log LEVEL       Set the log output level\n"
"                    LEVEL should be any of the following values:\n"
"                      a  output all log message\n"
"                      d  output log message which level >= debug\n"
"                      i  output log message which level >= information\n"
"                      w  output log message which level >= warning\n"
"                      e  output log message which level >= error\n"
"                      f  output log message which level >= fatal error\n"
"                      n  do not output any log message\n"
"  --log-field FIELD\n"
"                    Set the log output fields\n"
"                    FIELD should contain the following characters:\n"
"                      l  log level\n"
"                      d  date in YYYY-MM-DD\n"
"                      t  time in HH:MM:SS\n"
"                      m  milliseconds\n"
"                      T  log message tag\n"
"                      f  filename\n"
"                      F  function name\n"
"                      L  line number\n"
"                      i  thread id\n"
"  --log-file FILE   Set the log filename\n"
"  -p                Only parses the source file, not compiling it\n"
"  --pr              Print the result value\n"
"  -r                Run the executable program managed by the package manager\n"
"                    instead of running an OX file.\n"
"                    FILE is used as \"PACKAGE/PROGRAM\"\n"
"  -s SOURCE         Use SOURCE string as the script source instead of the\n"
"                    source file\n"
"  --version         Show version number\n"
        ),
        cmd
    );
}

/*Add package lookup directories.*/
static void
add_package_dirs (OX_Context *ctxt)
{
    char *v = getenv("OX_PACKAGE_DIRS");

    if (v) {
        char *c = v;
        char *n;

        while (1) {
            n = strchr(c, ',');
            if (n) {
                size_t len = n - c;
                char buf[len + 1];

                strncpy(buf, c, len);
                buf[len] = 0;

                ox_package_add_dir(ctxt, buf);
                c = n + 1;
            } else {
                ox_package_add_dir(ctxt, c);
                break;
            }
        }
    } else {
        const char *ox_dir = ox_get_install_dir(ctxt);
        char path[PATH_MAX];

        snprintf(path, sizeof(path), "%s/share/ox/pkg/%s", ox_dir, TARGET);
        ox_package_add_dir(ctxt, path);

        snprintf(path, sizeof(path), "%s/share/ox/pkg/all", ox_dir);
        ox_package_add_dir(ctxt, path);
   }
}

/*Show version number.*/
static void
show_version (void)
{
    printf("%s\n", ox_get_version());
}

/*Set the log output level.*/
static OX_Result
set_log_level (OX_Context *ctxt, char *optarg)
{
    OX_LogLevel level;

    if (!strcasecmp(optarg, "a"))
        level = OX_LOG_LEVEL_ALL;
    else if (!strcasecmp(optarg, "d"))
        level = OX_LOG_LEVEL_DEBUG;
    else if (!strcasecmp(optarg, "i"))
        level = OX_LOG_LEVEL_INFO;
    else if (!strcasecmp(optarg, "w"))
        level = OX_LOG_LEVEL_WARNING;
    else if (!strcasecmp(optarg, "e"))
        level = OX_LOG_LEVEL_ERROR;
    else if (!strcasecmp(optarg, "f"))
        level = OX_LOG_LEVEL_FATAL;
    else if (!strcasecmp(optarg, "n"))
        level = OX_LOG_LEVEL_NONE;
    else {
        fprintf(stderr, OX_TEXT("illegal output level \"%s\"\n"), optarg);
        return OX_ERR;
    }

    return ox_log_set_level(ctxt, level);
}

/*Set the log output fields.*/
static OX_Result
set_log_fields (OX_Context *ctxt, char *optarg)
{
    OX_LogField fields = 0;
    char *c = optarg;

    while (*c) {
        switch (*c) {
        case 'l':
            fields |= OX_LOG_FIELD_LEVEL;
            break;
        case 'd':
            fields |= OX_LOG_FIELD_DATE;
            break;
        case 't':
            fields |= OX_LOG_FIELD_TIME;
            break;
        case 'm':
            fields |= OX_LOG_FIELD_MSEC;
            break;
        case 'T':
            fields |= OX_LOG_FIELD_TAG;
            break;
        case 'f':
            fields |= OX_LOG_FIELD_FILE;
            break;
        case 'F':
            fields |= OX_LOG_FIELD_FUNC;
            break;
        case 'L':
            fields |= OX_LOG_FIELD_LINE;
            break;
        case 'i':
            fields |= OX_LOG_FIELD_THREAD;
            break;
        default:
            break;
        }
        c ++;
    }

    return ox_log_set_fields(ctxt, fields);
}

/*Set log file.*/
static OX_Result
set_log_file (OX_Context *ctxt, char *optarg)
{
    FILE *fp;

    fp = fopen(optarg, "ab");
    if (!fp) {
        fprintf(stderr, OX_TEXT("cannot open log file \"%s\"\n"), optarg);
        return OX_ERR;
    }

    if (log_file)
        fclose(log_file);

    log_file = fp;

    return ox_log_set_file(ctxt, log_file);
}

/*Parse options.*/
static OX_Result
parse_options (OX_Context *ctxt, int argc, char **argv)
{
    const char *short_opts = "+s:pcd:r";
    static const struct option long_opts[] = {
        {"ast",       no_argument,       0, OPTION_AST},
        {"bc",        no_argument,       0, OPTION_BC},
        {"dump-throw",no_argument,       0, OPTION_DUMP_THROW},
        {"pr",        no_argument,       0, OPTION_PR},
        {"help",      no_argument,       0, OPTION_HELP},
        {"version",   no_argument,       0, OPTION_VERSION},
        {"log",       required_argument, 0, OPTION_LOG},
        {"log-field", required_argument, 0, OPTION_LOG_FIELD},
        {"log-file",  required_argument, 0, OPTION_LOG_FILE},
        {0,           0,                 0, 0}
    };
    OX_Bool need_file = OX_TRUE;
    OX_Result r;

    while (1) {
        int c, option_index = 0;

        c = getopt_long(argc, argv, short_opts, long_opts, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 'd':
            if ((r = ox_package_add_dir(ctxt, optarg)) == OX_ERR)
                return r;
            has_dir_opt = OX_TRUE;
            break;
        case 's':
            script_source = optarg;
            need_file = OX_FALSE;
            break;
        case 'p':
            parse_only = OX_TRUE;
            break;
        case 'c':
            compile_only = OX_TRUE;
            break;
        case 'r':
            run_pm_exe = OX_TRUE;
            break;
        case OPTION_AST:
            output_ast = OX_TRUE;
            break;
        case OPTION_BC:
            output_bc = OX_TRUE;
            break;
        case OPTION_DUMP_THROW:
            ox_set_dump_throw(ctxt, OX_TRUE);
            break;
        case OPTION_ENC:
            if ((r = ox_set_file_enc(ctxt, optarg)) == OX_ERR)
                return r;
            break;
        case OPTION_PR:
            print_result = OX_TRUE;
            break;
        case OPTION_HELP:
            show_usage(argv[0]);
            need_file = OX_FALSE;
            break;
        case OPTION_VERSION:
            show_version();
            need_file = OX_FALSE;
            break;
        case OPTION_LOG:
            if ((r = set_log_level(ctxt, optarg)) == OX_ERR)
                return r;
            break;
        case OPTION_LOG_FIELD:
            if ((r = set_log_fields(ctxt, optarg)) == OX_ERR)
                return r;
            break;
        case OPTION_LOG_FILE:
            if ((r = set_log_file(ctxt, optarg)) == OX_ERR)
                return r;
            break;
        default:
            return OX_ERR;
        }
    }

    if (need_file) {
        if (optind >= argc) {
            fprintf(stderr, OX_TEXT("no input file specified\n"));
            return OX_ERR;
        }

        source_filename = argv[optind];
    }

    return OX_OK;
}

/*The the "OX" directory.*/
static OX_Result
get_ox_dir (OX_Context *ctxt)
{
    char exe_path[PATH_MAX];
    char tmp[PATH_MAX];
    char *exe_dir, *bin_dir, *env_dir;
    OX_Result r;

    if ((r = ox_exe_path(exe_path, sizeof(exe_path))) == OX_ERR)
        return ox_throw_system_error(ctxt, OX_TEXT("cannot get the executable program's path"));

    exe_dir = dirname(exe_path);
    strcpy(tmp, exe_dir);
    bin_dir = basename(tmp);
    if (strcmp(bin_dir, "bin"))
        return ox_throw_system_error(ctxt, OX_TEXT("cannot get OX environment"));

    strcpy(tmp, exe_dir);
    env_dir = dirname(tmp);

    return ox_set_install_dir(ctxt, env_dir);
}

/*Main function of "ox"*/
int
ox_main (int argc, char **argv)
{
    OX_VM *vm;
    OX_Context *ctxt;
    OX_Value *input, *ast, *script, *rv, *args, *text_domain;
    size_t narg;
    int parse_flags = 0;
    int compile_flags = OX_COMPILE_FL_REGISTER;
    OX_Result r;
    OX_Number n;
    int rc = 1;
    char path[PATH_MAX];
    const char *ox_dir;

    ox_arch_init();

    vm = ox_vm_new();
    ctxt = ox_context_get(vm);

    ox_lock(ctxt);

    /*Get the OX directory.*/
    if ((r = get_ox_dir(ctxt)) == OX_ERR)
        goto end;

    ox_dir = ox_get_install_dir(ctxt);
    setlocale(LC_ALL, "");
    snprintf(path, sizeof(path), "%s/share/locale", ox_dir);
    bindtextdomain("ox", path);

    /*Parse options.*/
    if ((r = parse_options(ctxt, argc, argv)) == OX_ERR)
        goto end;

    if (!script_source && !source_filename) {
        rc = 0;
        goto end;
    }

    if (!has_dir_opt) {
        /*Add package lookup directories.*/
        add_package_dirs(ctxt);
    }

    ast = ox_value_stack_push(ctxt);
    input = ox_value_stack_push(ctxt);
    text_domain = ox_value_stack_push(ctxt);

    if (script_source) {
        /*Parse script string.*/
        OX_Value *src = ox_value_stack_push(ctxt);

        ox_not_error(ox_string_from_const_char_star(ctxt, src, script_source));
        ox_not_error(ox_string_input_new(ctxt, input, src));

        compile_flags = OX_COMPILE_FL_EXPR;
    } else if (run_pm_exe) {
        /*Run package managed program.*/
        OX_VS_PUSH_4(ctxt, pn, exe, pkg, path)
        const char *sep, *pcstr;
        char pbuf[PATH_MAX];
        struct stat sb;

        sep = strchr(source_filename, '/');
        if (sep) {
            ox_not_error(ox_string_from_chars(ctxt, pn, source_filename, sep - source_filename));
            ox_not_error(ox_string_from_const_char_star(ctxt, exe, sep + 1));
        } else {
            ox_not_error(ox_string_from_const_char_star(ctxt, pn, source_filename));
            ox_value_copy(ctxt, exe, pn);
        }

        if ((r = ox_package_lookup(ctxt, pn, pkg)) == OX_ERR)
            goto end;

        if ((r = ox_package_get_exe(ctxt, pkg, exe, path)) == OX_ERR)
            goto end;

        pcstr = ox_string_get_char_star(ctxt, path);
        if (((r = stat(pcstr, &sb)) == -1) || !S_ISREG(sb.st_mode)) {
            snprintf(pbuf, sizeof(pbuf), "%s.ox", pcstr);

            if ((r = stat(pbuf, &sb)) == 0) {
                pcstr = pbuf;
            }
        }

        if ((r = ox_file_input_new(ctxt, input, pcstr)) == OX_ERR)
            goto end;

        ox_value_copy(ctxt, text_domain, pn);

        parse_flags = OX_PARSE_FL_RETURN;
    } else {
        /*Parse source file.*/
        if ((r = ox_file_input_new(ctxt, input, source_filename)) == OX_ERR)
            goto end;

        parse_flags = OX_PARSE_FL_RETURN;
    }

    if ((r = ox_parse(ctxt, input, ast, parse_flags)) == OX_ERR)
        goto end;

    ox_input_close(ctxt, input);

    /*Output AST.*/
    if (output_ast) {
        OX_VS_PUSH(ctxt, s)
        const char *cstr;

        if ((r = ox_ast_to_string(ctxt, ast, s)) == OX_ERR)
            goto end;

        cstr = ox_string_get_char_star(ctxt, s);
        printf("%s\n", cstr);
    }

    if (parse_only) {
        rc = 0;
        goto end;
    }

    /*Compile.*/
    script = ox_value_stack_push(ctxt);
    if ((r = ox_compile(ctxt, input, ast, script, compile_flags)) == OX_ERR)
        goto end;

    if (!ox_value_is_null(ctxt, text_domain))
        ox_script_set_text_domain(ctxt, script, text_domain, NULL);

    /*Output bytecode.*/
    if (output_bc) {
        ox_decompile(ctxt, script, stdout);
    }

    if (compile_only) {
        rc = 0;
        goto end;
    }

    /*Run the script.*/
    rv = ox_value_stack_push(ctxt);
    narg = argc - optind;
    if (narg) {
        /*Convert the arguments.*/
        size_t i;
        OX_Value *arg;

        args = ox_value_stack_push_n(ctxt, narg);

        for (i = 0; i < narg; i ++) {
            char *pa = argv[optind + i];

            arg = ox_values_item(ctxt, args, i);
            ox_string_from_const_char_star(ctxt, arg, pa);
        }
    } else {
        args = NULL;
    }

    if ((r = ox_call(ctxt, script, ox_value_null(ctxt), args, narg, rv)) == OX_ERR)
        goto end;

    if (print_result) {
        OX_VS_PUSH(ctxt, str)

        if ((r = ox_to_string(ctxt, rv, str)) == OX_ERR)
            goto end;

        printf("%s", ox_string_get_char_star(ctxt, str));

        rc = 0;
    } else {
        if ((r = ox_to_number(ctxt, rv, &n)) == OX_ERR)
            goto end;

        if (isfinite(n))
            rc = n;
    }
end:
    if (rc) {
        OX_VS_PUSH_2(ctxt, err, s)

        ox_catch(ctxt, err);

        if (!ox_value_is_null(ctxt, err)) {
            if (ox_to_string(ctxt, err, s) == OX_OK) {
                char *col = isatty(2) ? "\033[31;1m" : NULL;

                fprintf(stderr,
                        OX_TEXT("%suncaught error%s: %s\n"),
                        col ? col : "",
                        col ? "\033[0m" : "",
                        ox_string_get_char_star(ctxt, s));
                ox_stack_dump(ctxt, stderr);
            }
        }
    }

    ox_unlock(ctxt);
    ox_vm_free(vm);

    if (log_file)
        fclose(log_file);

    ox_arch_deinit();
    return rc;
}
