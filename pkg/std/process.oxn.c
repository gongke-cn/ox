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
 * Process functions.
 */

#define OX_LOG_TAG "process"

#ifdef ARCH_LINUX
#include <sys/wait.h>
#endif /*ARCH_LINUX*/

#include "std.h"

#define OX_PROCESS_FL_STDIN   (1 << 0)
#define OX_PROCESS_FL_STDOUT  (1 << 1)
#define OX_PROCESS_FL_STDERR  (1 << 2)
#define OX_PROCESS_FL_ERR2OUT (1 << 3)
#define OX_PROCESS_FL_NULLIN  (1 << 4)
#define OX_PROCESS_FL_NULLOUT (1 << 5)
#define OX_PROCESS_FL_NULLERR (1 << 6)
#define OX_PROCESS_FL_NULLIOE\
    (OX_PROCESS_FL_NULLIN|OX_PROCESS_FL_NULLOUT|OX_PROCESS_FL_NULLERR)

/*Declaration index.*/
enum {
    ID_Process,
    ID_File,
    ID_FILE,
    ID_STR_from_c,
    ID_MAX
};

/*Reference table.*/
static const OX_RefDesc
ref_tab[] = {
    {"std/io", "File", "File"},
    {"std/io", "FILE", "FILE"},
    {NULL, NULL, NULL}
};

/*Public table.*/
static const char*
pub_tab[] = {
    "Process",
    NULL
};

/*Script descrition.*/
static const OX_ScriptDesc
script_desc = {
    ref_tab,
    pub_tab,
    ID_MAX
};

/** Process.*/
typedef struct {
#ifdef ARCH_LINUX
    pid_t    pid;    /**< Process's ID.*/
#endif /*ARCH_LINUX*/
#ifdef ARCH_WIN
    PROCESS_INFORMATION pi; /**< Process information.*/
#endif /*ARCH_WIN*/
    OX_Value v_stdin;  /**< Standard input.*/
    OX_Value v_stdout; /**< Standard output.*/
    OX_Value v_stderr; /**< Standard error output.*/
} OX_Process;

/*Scan reference objects in the process.*/
static void
process_scan (OX_Context *ctxt, void *ptr)
{
    OX_Process *p = ptr;

    ox_gc_scan_value(ctxt, &p->v_stdin);
    ox_gc_scan_value(ctxt, &p->v_stdout);
    ox_gc_scan_value(ctxt, &p->v_stderr);
}

/*Free the process.*/
static void
process_free (OX_Context *ctxt, void *ptr)
{
    OX_Process *p = ptr;

    OX_DEL(ctxt, p);
}

/*Operation functions of process.*/
static const OX_PrivateOps
process_ops = {
    process_scan,
    process_free
};

/*Get the process data from the value.*/
static OX_Process*
process_get (OX_Context *ctxt, OX_Value *v)
{
    OX_Process *p = ox_object_get_priv(ctxt, v, &process_ops);

    if (!p)
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a process"));

    return p;
}

/*Create a file object.*/
static OX_Result
create_file (OX_Context *ctxt, OX_Value *f, int fd, const char *mode, OX_Value *v)
{
    OX_VS_PUSH_2(ctxt, cv, pty)
    OX_Value *ty, *clazz, *str;
    OX_CValueInfo cvi;
    FILE *fp;
    OX_Result r;

    if (!(fp = fdopen(fd, mode))) {
        r = std_system_error(ctxt, "fdopen");
        goto end;
    }

    cvi.v.p = fp;
    cvi.base = NULL;
    cvi.own = OX_CPTR_NON_OWNER;

    ty = ox_script_get_value(ctxt, f, ID_FILE);
    if ((r = ox_ctype_pointer(ctxt, pty, ty, -1)) == OX_ERR)
        goto end;

    if ((r = ox_cvalue_new(ctxt, cv, pty, &cvi)) == OX_ERR)
        goto end;

    clazz = ox_script_get_value(ctxt, f, ID_File);
    str = ox_script_get_value(ctxt, f, ID_STR_from_c);

    r = ox_call_method(ctxt, clazz, str, cv, 1, v);
end:
    OX_VS_POP(ctxt, cv)
    return r;
}

#ifdef ARCH_LINUX
typedef OX_VECTOR_TYPE_DECL(char*) OX_ProcArguments;

static void
close_pair (int pair[2])
{
    if (pair[0] != -1)
        close(pair[0]);
    if (pair[1] != -1)
        close(pair[1]);
}

/*Parse an argument.*/
static OX_Result
parse_arg (OX_Context *ctxt, const char **pc, OX_ProcArguments *args, OX_CharBuffer *cb)
{
    const char *c = *pc;
    OX_Bool has_arg = OX_FALSE;
    char *arg;
    OX_Result r;

    cb->len = 0;

    while (ox_char_is_space(*c))
        c ++;

    if (*c == '\'') {
        c ++;
        
        while (1) {
            if (*c == '\'') {
                c ++;
                break;
            } else if (*c == 0) {
                break;
            }

            if ((r = ox_char_buffer_append_char(ctxt, cb, *c)) == OX_ERR)
                return r;

            c ++;
        }

        has_arg = OX_TRUE;
    } else if (*c == '\"') {
        c ++;

        while (1) {
            int code;

            if (*c == '\"') {
                c ++;
                break;
            } else if (*c == 0) {
                break;
            } else if (*c == '\\') {
                if (c[1] != 0) {
                    code = c[1];
                    c += 2;
                } else {
                    c ++;
                    break;
                }
            } else {
                code = *c;
                c ++;
            }

            if ((r = ox_char_buffer_append_char(ctxt, cb, code)) == OX_ERR)
                return r;
        }

        has_arg = OX_TRUE;
    } else {
        while (1) {
            if ((*c == 0) || ox_char_is_space(*c))
                break;

            if ((r = ox_char_buffer_append_char(ctxt, cb, *c)) == OX_ERR)
                return r;
            c ++;
        }

        if (cb->len)
            has_arg = OX_TRUE;
    }

    if (has_arg) {
        if (!(arg = ox_strndup(ctxt, cb->items, cb->len)))
            return ox_throw_no_mem_error(ctxt);

        if ((r = ox_vector_append(ctxt, args, arg)) == OX_ERR)
            return ox_throw_no_mem_error(ctxt);
    }

    *pc = c;
    return OX_OK;
}

/*Parse the arguments.*/
static OX_Result
parse_args (OX_Context *ctxt, const char *cmd, OX_ProcArguments *args)
{
    const char *c = cmd;
    OX_CharBuffer cb;
    OX_Result r = OX_ERR;

    ox_char_buffer_init(&cb);

    while (*c) {
        if ((r = parse_arg(ctxt, &c, args, &cb)) == OX_ERR)
            goto end;
    }

    if ((r = ox_vector_append(ctxt, args, NULL)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    ox_char_buffer_deinit(ctxt, &cb);
    return r;
}

/*Initialize the process.*/
static OX_Result
process_init (OX_Context *ctxt, OX_Process *proc, OX_Value *f, const char *cmd, int flags)
{
    OX_ProcArguments args;
    size_t i;
    int in_pair[2] = {-1, -1};
    int out_pair[2] = {-1, -1};
    int err_pair[2] = {-1, -1};
    OX_Result r;

    /*Parse arguments.*/
    ox_vector_init(&args);

    if ((r = parse_args(ctxt, cmd, &args)) == OX_ERR)
        goto end;

    if (args.len == 0) {
        r = ox_throw_type_error(ctxt, OX_TEXT("illegal command line"));
        goto end;
    }

    if ((flags & (OX_PROCESS_FL_STDOUT|OX_PROCESS_FL_STDERR))
            != (OX_PROCESS_FL_STDOUT|OX_PROCESS_FL_STDERR)) {
        flags &= ~OX_PROCESS_FL_ERR2OUT;
    }

    if (flags & OX_PROCESS_FL_NULLIN)
        flags &= ~OX_PROCESS_FL_STDIN;
    if (flags & OX_PROCESS_FL_NULLOUT)
        flags &= ~OX_PROCESS_FL_STDOUT;
    if (flags & OX_PROCESS_FL_NULLERR)
        flags &= ~OX_PROCESS_FL_STDERR;

    /*Create pipes.*/
    if (flags & OX_PROCESS_FL_STDIN) {
        if ((r = pipe(in_pair)) == -1) {
            r = std_system_error(ctxt, "pipe");
            goto end;
        }
    }

    if (flags & OX_PROCESS_FL_STDOUT) {
        if ((r = pipe(out_pair)) == -1) {
            r = std_system_error(ctxt, "pipe");
            goto end;
        }
    }

    if (flags & OX_PROCESS_FL_STDERR) {
        if ((r = pipe(err_pair)) == -1) {
            r = std_system_error(ctxt, "pipe");
            goto end;
        }
    }

    /*Fork and execure the program.*/
    proc->pid = fork();
    if (proc->pid == 0) {
        if (flags & OX_PROCESS_FL_NULLIN) {
            close(0);
            open("/dev/null", O_RDONLY);
        } else if (flags & OX_PROCESS_FL_STDIN) {
            close(0);
            r = dup(in_pair[0]);
            close_pair(in_pair);
        }

        if (flags & OX_PROCESS_FL_NULLOUT) {
            close(1);
            open("/dev/null", O_WRONLY);
        } else if (flags & OX_PROCESS_FL_STDOUT) {
            close(1);
            r = dup(out_pair[1]);
            close_pair(out_pair);
        }

        if (flags & OX_PROCESS_FL_NULLERR) {
            close(2);
            open("/dev/null", O_WRONLY);
        } else if (flags & OX_PROCESS_FL_STDERR) {
            close(2);

            if (flags & OX_PROCESS_FL_ERR2OUT) {
                r = dup(1);
            } else {
                r = dup(err_pair[1]);
                close_pair(err_pair);
            }
        }

        r = execvp(args.items[0], args.items);
        if (r == -1) {
            OX_VS_PUSH_2(ctxt, err, s)

            std_system_error(ctxt, "execv");
            ox_catch(ctxt, err);
            if (ox_to_string(ctxt, err, s) == OX_OK) {
                char *col = isatty(2) ? "\033[31;1m" : NULL;

                fprintf(stderr,
                        OX_TEXT("%suncaught error%s: %s\n"),
                        col ? col : "",
                        col ? "\033[0m" : "",
                        ox_string_get_char_star(ctxt, s));
            }
            
            ox_stack_dump(ctxt, stderr);
            exit(1);
        }
    } else {
        if (flags & OX_PROCESS_FL_STDIN) {
            close(in_pair[0]);
            in_pair[0] = -1;

            if ((r = create_file(ctxt, f, in_pair[1], "wb", &proc->v_stdin)) == OX_ERR)
                goto end;
            in_pair[1] = -1;
        }

        if (flags & OX_PROCESS_FL_STDOUT) {
            close(out_pair[1]);
            out_pair[1] = -1;

            if ((r = create_file(ctxt, f, out_pair[0], "rb", &proc->v_stdout)) == OX_ERR)
                goto end;
            out_pair[0] = -1;
        }

        if (flags & OX_PROCESS_FL_STDERR) {
            if (flags & OX_PROCESS_FL_ERR2OUT) {
                ox_value_copy(ctxt, &proc->v_stderr, &proc->v_stdout);
            } else {
                close(err_pair[1]);
                err_pair[1] = -1;

                if ((r = create_file(ctxt, f, err_pair[0], "rb", &proc->v_stderr)) == OX_ERR)
                    goto end;
                err_pair[0] = -1;
            }
        }
    }

    r = OX_OK;
end:
    for (i = 0; i < args.len; i ++) {
        ox_strfree(ctxt, args.items[i]);
    }

    if (r == OX_ERR) {
        close_pair(in_pair);
        close_pair(out_pair);
        close_pair(err_pair);

        if (!ox_value_is_null(ctxt, &proc->v_stdin))
            ox_close(ctxt, &proc->v_stdin);
        if (!ox_value_is_null(ctxt, &proc->v_stdout))
            ox_close(ctxt, &proc->v_stdout);
        if (!ox_value_is_null(ctxt, &proc->v_stderr) && !(flags & OX_PROCESS_FL_ERR2OUT))
            ox_close(ctxt, &proc->v_stderr);
    }

    ox_vector_deinit(ctxt, &args);
    return r;
}

/*Wait the process.*/
static OX_Result
process_wait (OX_Context *ctxt, OX_Process *proc, int *code)
{
    if (proc->pid == -1) {
        return ox_throw_reference_error(ctxt, OX_TEXT("process is already stopped"));
    }

    while (1) {
        int status;

        if ((waitpid(proc->pid, &status, 0)) == -1)
            return std_system_error(ctxt, "waitpid");

        if (WIFEXITED(status)) {
            *code = WEXITSTATUS(status);
            break;
        }
    }

    if (!ox_value_is_null(ctxt, &proc->v_stdin))
        ox_close(ctxt, &proc->v_stdin);
    if (!ox_value_is_null(ctxt, &proc->v_stdout))
        ox_close(ctxt, &proc->v_stdout);
    if (!ox_value_is_null(ctxt, &proc->v_stderr))
        ox_close(ctxt, &proc->v_stderr);

    proc->pid = -1;
    return OX_OK;
}
#endif /*ARCH_LINUX*/

#ifdef ARCH_WIN
/*Throw an error.*/
static OX_Result
error (OX_Context *ctxt, const char *fn)
{
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            dw,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0, NULL);

    ox_throw_system_error(ctxt, OX_TEXT("\"%s\" failed: %d: %s"),
            fn, dw, lpMsgBuf);

    LocalFree(lpMsgBuf);
    return OX_ERR;
}

/*Create a file object from handle.*/
static OX_Result
create_file_from_handle (OX_Context *ctxt, OX_Value *f, HANDLE *phandle, int flags, const char *mode, OX_Value *v)
{
    HANDLE handle = *phandle;
    int fd;
    OX_Result r;

    fd = _open_osfhandle((intptr_t)handle, flags);
    if (fd == -1)
        return error(ctxt, "_open_osfhandle");

    *phandle = NULL;

    r = create_file(ctxt, f, fd, mode, v);
    if (r == OX_ERR) {
        _close(fd);
        return r;
    }

    return OX_OK;
}

/*Initialize the process.*/
static OX_Result
process_init (OX_Context *ctxt, OX_Process *proc, OX_Value *f, const char *cmd, int flags)
{
    STARTUPINFO si;
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    HANDLE in_rd = INVALID_HANDLE_VALUE, in_wr = INVALID_HANDLE_VALUE;
    HANDLE out_rd = INVALID_HANDLE_VALUE, out_wr = INVALID_HANDLE_VALUE;
    HANDLE err_rd = INVALID_HANDLE_VALUE, err_wr = INVALID_HANDLE_VALUE;
    BOOL b;
    OX_Result r = OX_ERR;
    HANDLE h_nul_rd = INVALID_HANDLE_VALUE;
    HANDLE h_nul_wr = INVALID_HANDLE_VALUE;
    DWORD c_flags = 0;

    if ((flags & (OX_PROCESS_FL_STDOUT|OX_PROCESS_FL_STDERR))
            != (OX_PROCESS_FL_STDOUT|OX_PROCESS_FL_STDERR)) {
        flags &= ~OX_PROCESS_FL_ERR2OUT;
    }

    if (flags & OX_PROCESS_FL_NULLIN)
        flags &= ~OX_PROCESS_FL_STDIN;
    if (flags & OX_PROCESS_FL_NULLOUT)
        flags &= ~OX_PROCESS_FL_STDOUT;
    if (flags & OX_PROCESS_FL_NULLERR)
        flags &= ~OX_PROCESS_FL_STDERR;

    memset(&si, 0, sizeof(si));

    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;

    if (!(flags & OX_PROCESS_FL_NULLIN))
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    else
        si.hStdInput = INVALID_HANDLE_VALUE;
    if (!(flags & OX_PROCESS_FL_NULLOUT))
        si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    else
        si.hStdOutput = INVALID_HANDLE_VALUE;
    if (!(flags & OX_PROCESS_FL_NULLERR))
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    else
        si.hStdError = INVALID_HANDLE_VALUE;

    /*Create pipes.*/
    if (flags & OX_PROCESS_FL_STDIN) {
        if (!CreatePipe(&in_rd, &in_wr, &sa, 0)) {
            error(ctxt, "CreatePipe");
            goto end;
        }
        si.hStdInput = in_rd;
    }

    if (flags & OX_PROCESS_FL_STDOUT) {
        if (!CreatePipe(&out_rd, &out_wr, &sa, 0)) {
            error(ctxt, "CreatePipe");
            goto end;
        }
        si.hStdOutput = out_wr;
    }

    if (flags & OX_PROCESS_FL_ERR2OUT) {
        si.hStdError = si.hStdOutput;
    } else if (flags & OX_PROCESS_FL_STDERR) {
        if (!CreatePipe(&err_rd, &err_wr, &sa, 0)) {
            error(ctxt, "CreatePipe");
            goto end;
        }
        si.hStdError = err_wr;
    }

    /*Open NUL device.*/
    if (si.hStdInput == INVALID_HANDLE_VALUE) {
        h_nul_rd = CreateFile("NUL",
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            &sa,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        if (h_nul_rd == INVALID_HANDLE_VALUE) {
            error(ctxt, "CreateFile");
            goto end;
        }

        si.hStdInput = h_nul_rd;
    }

    if ((si.hStdOutput == INVALID_HANDLE_VALUE) || (si.hStdError == INVALID_HANDLE_VALUE)) {
        h_nul_wr = CreateFile("NUL",
            GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            &sa,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        if (h_nul_wr == INVALID_HANDLE_VALUE) {
            error(ctxt, "CreateFile");
            goto end;
        }

        if (si.hStdOutput == INVALID_HANDLE_VALUE)
            si.hStdOutput = h_nul_wr;

        if (si.hStdError == INVALID_HANDLE_VALUE)
            si.hStdError = h_nul_wr;
    }

    if ((si.hStdInput != GetStdHandle(STD_INPUT_HANDLE))
            && (si.hStdOutput != GetStdHandle(STD_OUTPUT_HANDLE))
            && (si.hStdError != GetStdHandle(STD_ERROR_HANDLE)))
        c_flags = CREATE_NO_WINDOW;

    b = CreateProcessA(NULL, (LPSTR)cmd, NULL, NULL, OX_TRUE, c_flags, NULL, NULL, &si, &proc->pi);
    if (!b) {
        error(ctxt, "CreateProcessA");
        goto end;
    }

    if (flags & OX_PROCESS_FL_STDIN) {
        CloseHandle(in_rd);
        in_rd = NULL;
        if ((r = create_file_from_handle(ctxt, f, &in_wr, _O_WRONLY, "w", &proc->v_stdin)) == OX_ERR)
            goto end;
    }

    if (flags & OX_PROCESS_FL_STDOUT) {
        CloseHandle(out_wr);
        out_wr = NULL;
        if ((r = create_file_from_handle(ctxt, f, &out_rd, _O_RDONLY, "r", &proc->v_stdout)) == OX_ERR)
            goto end;
    }

    if (flags & OX_PROCESS_FL_STDERR) {
        if (flags & OX_PROCESS_FL_ERR2OUT) {
            ox_value_copy(ctxt, &proc->v_stderr, &proc->v_stdout);
        } else {
            CloseHandle(err_wr);
            err_wr = NULL;
            if ((r = create_file_from_handle(ctxt, f, &err_rd, _O_RDONLY, "r", &proc->v_stderr)) == OX_ERR)
                goto end;
        }
    }

    r = OX_OK;
end:
    if (h_nul_rd != INVALID_HANDLE_VALUE)
        CloseHandle(h_nul_rd);
    if (h_nul_wr != INVALID_HANDLE_VALUE)
        CloseHandle(h_nul_wr);
    if (r == OX_ERR) {
        if (in_rd != INVALID_HANDLE_VALUE)
            CloseHandle(in_rd);
        if (in_wr != INVALID_HANDLE_VALUE)
            CloseHandle(in_wr);
        if (out_rd != INVALID_HANDLE_VALUE)
            CloseHandle(out_rd);
        if (out_wr != INVALID_HANDLE_VALUE)
            CloseHandle(out_wr);
        if (err_rd != INVALID_HANDLE_VALUE)
            CloseHandle(err_rd);
        if (err_wr != INVALID_HANDLE_VALUE)
            CloseHandle(err_wr);

        if (!ox_value_is_null(ctxt, &proc->v_stdin))
            ox_close(ctxt, &proc->v_stdin);
        if (!ox_value_is_null(ctxt, &proc->v_stdout))
            ox_close(ctxt, &proc->v_stdout);
        if (!ox_value_is_null(ctxt, &proc->v_stderr))
            ox_close(ctxt, &proc->v_stderr);
    }
    return r;
}

/*Wait the process.*/
static OX_Result
process_wait (OX_Context *ctxt, OX_Process *proc, int *code)
{
    DWORD dw, c;
    BOOL b;

    if (proc->pi.hProcess == NULL) {
        return ox_throw_reference_error(ctxt, OX_TEXT("process is already stopped"));
    }

    dw = WaitForSingleObject(proc->pi.hProcess, INFINITE);
    if (dw == WAIT_FAILED)
        return error(ctxt, "WaitForSingleObject");
    b = GetExitCodeProcess(proc->pi.hProcess, &c);
    if (!b)
        return error(ctxt, "GetExitCodeProcess");

    CloseHandle(proc->pi.hProcess);
    CloseHandle(proc->pi.hThread);
    proc->pi.hProcess = NULL;

    *code = c;
    return OX_OK;
}
#endif /*ARCH_WIN*/

/*Process.$inf.$init*/
static OX_Result
Process_inf_init (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *cmd_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *flags_arg = ox_argument(ctxt, args, argc, 1);
    OX_VS_PUSH(ctxt, cmd_str)
    const char *cmd;
    int32_t flags = 0;
    OX_Process *proc = NULL;
    OX_Result r;

    if ((r = ox_to_string(ctxt, cmd_arg, cmd_str)) == OX_ERR)
        goto end;

    cmd = ox_string_get_char_star(ctxt, cmd_str);

    if (argc > 1) {
        if ((r = ox_to_int32(ctxt, flags_arg, &flags)) == OX_ERR)
            goto end;
    }

    if (!OX_NEW(ctxt, proc)) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    ox_value_set_null(ctxt, &proc->v_stdin);
    ox_value_set_null(ctxt, &proc->v_stdout);
    ox_value_set_null(ctxt, &proc->v_stderr);

    if ((r = ox_object_set_priv(ctxt, thiz, &process_ops, proc)) == OX_ERR)
        goto end;

    r = process_init(ctxt, proc, f, cmd, flags);
end:
    OX_VS_POP(ctxt, cmd_str)
    return r;
}

/*Process.$inf.wait*/
static OX_Result
Process_inf_wait (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Process *proc;
    OX_Result r;
    int status = 0;

    if (!(proc = process_get(ctxt, thiz)))
        return OX_ERR;

    if ((r = process_wait(ctxt, proc, &status)) == OX_ERR)
        return r;

    ox_value_set_number(ctxt, rv, status);
    return OX_OK;
}

/*Process.$inf.stdin get*/
static OX_Result
Process_inf_stdin_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Process *proc;

    if (!(proc = process_get(ctxt, thiz)))
        return OX_ERR;

    ox_value_copy(ctxt, rv, &proc->v_stdin);
    return OX_OK;
}

/*Process.$inf.stdout get*/
static OX_Result
Process_inf_stdout_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Process *proc;

    if (!(proc = process_get(ctxt, thiz)))
        return OX_ERR;

    ox_value_copy(ctxt, rv, &proc->v_stdout);
    return OX_OK;
}

/*Process.$inf.stderr get*/
static OX_Result
Process_inf_stderr_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Process *proc;

    if (!(proc = process_get(ctxt, thiz)))
        return OX_ERR;

    ox_value_copy(ctxt, rv, &proc->v_stderr);
    return OX_OK;
}

/*Load this module.*/
OX_Result
ox_load (OX_Context *ctxt, OX_Value *s)
{
    ox_not_error(ox_script_set_desc(ctxt, s, &script_desc));
    return OX_OK;
}

/*?
 *? @lib Process.
 *?
 *? @class{ Process Process.
 *?
 *? @const STDIN {Number} Process flags: Create standard input pipe.
 *? @const STDOUT {Number} Process flags: Create standard output pipe.
 *? @const STDERR {Number} Process flags: Create standard error output pipe.
 *? @const ERR2OUT {Number} Process flags: Standard error output to standard output.
 *? @const NULLIN {Number} Process flags: Use null device as standard input.
 *? @const NULLOUT {Number} Process flags: Use null device as standard output.
 *? @const NULLERR {Number} Process flags: Use null device as standard error output.
 *? @const NULLIOE {Number} Process flags: Use null device as standard input, output and error output.
 *?
 *? @func $init Initialize the process object.
 *? @param cmd {String} Command line for running the process.
 *? @param flags {Number} Process flags.
 *? @throw {SystemError} Create the process failed.
 *?
 *? @func wait Wait for the process to exit.
 *? @return {Number} The exit code of the process.
 *? @throw {SystemError} Wait the process failed.
 *?
 *? @roacc stdin {File} The write port of the standard input of the process.
 *? If the process has flag Process.STDIN, you can use this file to write data
 *? to the process's standard input.
 *?
 *? @roacc stdout {File} The read port of the standard output of the process.
 *? If the process has flag Process.STDOUT, you can use this file to read the
 *? process's standard output.
 *?
 *? @roacc stderr {File} The read port of the standard error output of the process.
 *? If the process has flag Process.STDERR, you can use this file to read the
 *? process's standard error output.
 *?
 *? @class}
 */

/*Execute.*/
OX_Result
ox_exec (OX_Context *ctxt, OX_Value *f, OX_Value *s, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH_3(ctxt, c, inf, v)

    /*Process.*/
    ox_not_error(ox_named_class_new_s(ctxt, c, inf, NULL, "Process"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_Process, c));

    /*Flags.*/
    ox_value_set_number(ctxt, v, OX_PROCESS_FL_STDIN);
    ox_not_error(ox_object_add_const_s(ctxt, c, "STDIN", v));
    ox_value_set_number(ctxt, v, OX_PROCESS_FL_STDOUT);
    ox_not_error(ox_object_add_const_s(ctxt, c, "STDOUT", v));
    ox_value_set_number(ctxt, v, OX_PROCESS_FL_STDERR);
    ox_not_error(ox_object_add_const_s(ctxt, c, "STDERR", v));
    ox_value_set_number(ctxt, v, OX_PROCESS_FL_ERR2OUT);
    ox_not_error(ox_object_add_const_s(ctxt, c, "ERR2OUT", v));
    ox_value_set_number(ctxt, v, OX_PROCESS_FL_NULLIN);
    ox_not_error(ox_object_add_const_s(ctxt, c, "NULLIN", v));
    ox_value_set_number(ctxt, v, OX_PROCESS_FL_NULLOUT);
    ox_not_error(ox_object_add_const_s(ctxt, c, "NULLOUT", v));
    ox_value_set_number(ctxt, v, OX_PROCESS_FL_NULLERR);
    ox_not_error(ox_object_add_const_s(ctxt, c, "NULLERR", v));
    ox_value_set_number(ctxt, v, OX_PROCESS_FL_NULLIOE);
    ox_not_error(ox_object_add_const_s(ctxt, c, "NULLIOE", v));

    /*Process.$inf.$init*/
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$init", Process_inf_init));

    /*Process.$inf.wait*/
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "wait", Process_inf_wait));

    /*Process.$inf.stdin*/
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "stdin", Process_inf_stdin_get, NULL));

    /*Process.$inf.stdout*/
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "stdout", Process_inf_stdout_get, NULL));

    /*Process.$inf.stderr*/
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "stderr", Process_inf_stderr_get, NULL));

    /*"from_c"*/
    ox_not_error(ox_string_from_const_char_star(ctxt, v, "from_c"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_STR_from_c, v));

    OX_VS_POP(ctxt, c)
    return OX_OK;
}
