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
 * Linux functions.
 */

#define OX_LOG_TAG "ox_arch_linux"

#include "ox_internal.h"

#ifdef ARCH_LINUX

/**
 * Initialize the architecture related resource.
 */
void
ox_arch_init (void)
{
}

/**
 * Release the architecture related resource.
 */
void
ox_arch_deinit (void)
{
}

/**
 * Get the current executable program's path.
 * @param[out] buf The path output buffer.
 * @param len The buffer's length.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_exe_path (char *buf, size_t len)
{
    ssize_t r;

    r = readlink("/proc/self/exe", buf, len);
    if (r == -1)
        return OX_ERR;

    buf[r] = 0;
    return OX_OK;
}

/**
 * Get the current thread's ID.
 * @return The current thread's ID.
 */
int
ox_thread_id (void)
{
#if (__GLIBC__ > 2) || ( __GLIBC__ == 2 && __GLIBC_MINOR__ >= 30)
    return gettid();
#else
    return (int)pthread_self();
#endif
}

/**
 * Open a dynamic library.
 * @param ctxt The current running context.
 * @param name Name of the dynamic library.
 * @return The handle of the dynamic library.
 * @retval NULL On error.
 */
void*
ox_dl_open (OX_Context *ctxt, const char *name)
{
    void *handle;

    handle = dlopen(name, RTLD_NOW);
    if (!handle)
        ox_throw_system_error(ctxt,
                OX_TEXT("\"%s\" \"%s\" failed: %s"),
                "dlopen",
                name,
                dlerror());

    return handle;
}

/**
 * Normalize the path string.
 * @param ctxt The current running context.
 * @param path The input path.
 * @param[out] s Return the string value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_path_to_str (OX_Context *ctxt, const char *path, OX_Value *s)
{
    return ox_string_from_char_star(ctxt, s, path);
}

#endif /*ARCH_LINUX*/
