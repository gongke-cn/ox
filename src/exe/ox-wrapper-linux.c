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

#include <sys/wait.h>
#include <sys/stat.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

/*Main function.*/
int
main (int argc, char **argv)
{
    char self_path[PATH_MAX];
    char delay_path[PATH_MAX];
    char *dir;
    pid_t pid;
    ssize_t r;
    int code;
    struct stat st;

    /*Get the current program's pathname.*/
    r = readlink("/proc/self/exe", self_path, sizeof(self_path));
    if (r == -1) {
        fprintf(stderr, "cannot read \"/proc/self/exe\": %s\n", strerror(errno));
        return 1;
    }

    self_path[r] = 0;
    dir = dirname(self_path);
    if (!dir) {
        fprintf(stderr, "\"dirname\" failed: %s\n", strerror(errno));
        return 1;
    }

    pid = fork();
    if (pid == -1) {
        fprintf(stderr, "\"fork\" failed: %s\n", strerror(errno));
        return 1;
    }

    if (pid == 0) {
        /*Run "ox-cli".*/
        char exe_path[PATH_MAX];
        char *exe_argv[argc + 1];

        snprintf(exe_path, sizeof(exe_path), "%s/ox-cli", dir);

        exe_argv[0] = exe_path;

        if (argc > 1) {
            memcpy(exe_argv + 1, argv + 1, (argc - 1) * sizeof(char*));
        }

        exe_argv[argc] = NULL;

        execv(exe_path, exe_argv);
    }

    /*Waiting "ox-cli".*/
    while (1) {
        int status;

        if (waitpid(pid, &status, 0) == -1) {
            fprintf(stderr, "\"waitpid\" failed: %s\n", strerror(errno));
            return 1;
        }

        if (WIFEXITED(status)) {
            code = WEXITSTATUS(status);
            break;
        }
    }

    /*Run delay task.*/
    dir = dirname(dir);
    if (!dir) {
        fprintf(stderr, "\"dirname\" failed: %s\n", strerror(errno));
        return code;
    }

    snprintf(delay_path, sizeof(delay_path), "%s/share/ox/delay_task.bat", dir);
    if (stat(delay_path, &st) != -1) {
        if (system(delay_path) == -1)
            fprintf(stderr, "\"system\" failed: %s\n", strerror(errno));
        unlink(delay_path);
    }

    return code;
}
