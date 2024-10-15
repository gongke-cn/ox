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
 * OX wrapper program of windows.
 */

#include <windows.h>
#include <shlwapi.h>
#include <stdio.h>

typedef struct {
    char  *b;
    size_t len;
    size_t size;
} CmdLine;

/*Initialize the command line.*/
static void
cl_init (CmdLine *cl)
{
    cl->b = NULL;
    cl->len = 0;
    cl->size = 0;
}

/*Add a character to the command line.*/
static int
cl_add_char (CmdLine *cl, char c)
{
    if (cl->len >= cl->size) {
        size_t nsize = cl->size * 12;
        char *nb;

        if (nsize < 256)
            nsize = 256;
        
        if (!(nb = realloc(cl->b, nsize))) {
            fprintf(stderr, "allocate memory failed\n");
            return -1;
        }

        cl->b = nb;
        cl->size = nsize;
    }

    cl->b[cl->len ++] = c;
    return 0;
}

/*Add an argument to the command line.*/
static int
cl_add_arg (CmdLine *cl, char *arg)
{
    char *c = arg;
    int r;

    if ((r = cl_add_char(cl, '\"')) == -1)
        return r;

    while (*c) {
        if (*c == '\"') {
            if ((r = cl_add_char(cl, '\\')) == -1)
                return r;
            if ((r = cl_add_char(cl, '\"')) == -1)
                return r;
        } else if (*c == '\\') {
            if ((r = cl_add_char(cl, '\\')) == -1)
                return r;
            if ((r = cl_add_char(cl, '\\')) == -1)
                return r;
        } else {
            if ((r = cl_add_char(cl, *c)) == -1)
                return r;
        }

        c ++;
    }

    if ((r = cl_add_char(cl, '\"')) == -1)
        return r;

    return 0;
}

/*Build command line.*/
static char*
build_cmd_line (char *pn, int argc, char **argv)
{
    CmdLine cl;
    int r, i;

    cl_init(&cl);

    if ((r = cl_add_arg(&cl, pn)) == -1)
        goto end;

    for (i = 0; i < argc; i ++) {
        if ((r = cl_add_char(&cl, ' ')) == -1)
            goto end;
        if ((r = cl_add_arg(&cl, argv[i])) == -1)
            goto end;
    }

    if ((r = cl_add_char(&cl, 0)) == -1)
        goto end;

    r = 0;
end:
    if (r == -1) {
        if (cl.b) {
            free(cl.b);
            cl.b = NULL;
        }
    }

    return cl.b;
}

/*Main function.*/
int
main (int argc, char **argv)
{
    char tmp[MAX_PATH];
    char pn_buf[MAX_PATH];
    char *cl = NULL;
    char *pn;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    DWORD code;
    char *bat_argv[2];
    int r = 1;

    /*Get the program's directory.*/
    if (GetModuleFileNameA(NULL, tmp, sizeof(tmp)) == 0) {
        fprintf(stderr, "\"GetModuleFileNameA\" failed: %ld\n", GetLastError());
        goto end;
    }

    if (!PathRemoveFileSpec(tmp)) {
        fprintf(stderr, "\"PathRemoveFileSpec\" failed\n");
        goto end;
    }

    if (!(pn = PathCombineA(pn_buf, tmp, "ox-cli.exe"))) {
        fprintf(stderr, "\"PathCombineA\" failed\n");
        goto end;
    }

    /*Build command line.*/
    if (!(cl = build_cmd_line(pn, argc - 1, argv + 1)))
        goto end;

    /*Create the process.*/
    memset(&si, 0, sizeof(si));

    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    if (!CreateProcessA(pn, cl, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        fprintf(stderr, "\"CreateProcessA\" failed: %ld\n", GetLastError());
        goto end;
    }

    /*Wait the process.*/
    if (WaitForSingleObject(pi.hProcess, INFINITE) == WAIT_FAILED) {
        fprintf(stderr, "\"WaitForSingleObject\" failed: %ld\n", GetLastError());
        goto end;
    }

    if (!GetExitCodeProcess(pi.hProcess, &code)) {
        fprintf(stderr, "\"GetExitCodeProcess\" failed: %ld\n", GetLastError());
        goto end;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    free(cl);
    cl = NULL;

    /*Run delay task.*/
    if (!PathRemoveFileSpec(tmp)) {
        fprintf(stderr, "\"PathRemoveFileSpec\" failed\n");
        goto end;
    }

    if (!(pn = PathCombineA(pn_buf, tmp, "share\\ox\\delay_task.bat"))) {
        fprintf(stderr, "\"PathCombineA\" failed\n");
        goto end;
    }

    //Does the delay task exist?
    if (PathFileExists(pn)) {
        /*Run delay task.*/
        bat_argv[0] = "/c";
        bat_argv[1] = pn;

        if (!(cl = build_cmd_line("cmd.exe", 2, bat_argv)))
            goto end;

        

        if (!CreateProcessA(NULL, cl, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
            fprintf(stderr, "\"CreateProcessA\" failed: %ld\n", GetLastError());
            goto end;
        }

        /*Wait the process.*/
        if (WaitForSingleObject(pi.hProcess, INFINITE) == WAIT_FAILED) {
            fprintf(stderr, "\"WaitForSingleObject\" failed: %ld\n", GetLastError());
            goto end;
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        DeleteFile(pn);
    }

    r = code;
end:
    if (cl)
        free(cl);
    return r;
}
