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
 * Windows functions.
 */

#define OX_LOG_TAG "ox_arch_win"

#include "ox_internal.h"

#ifdef ARCH_WIN

/*Console mode.*/
static DWORD old_stdout_mode, old_stderr_mode;

/**
 * Initialize the architecture related resource.
 */
void ox_arch_init(void)
{
    GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &old_stdout_mode);
    GetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), &old_stderr_mode);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE),
                   old_stdout_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleMode(GetStdHandle(STD_ERROR_HANDLE),
                   old_stderr_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

/**
 * Release the architecture related resource.
 */
void ox_arch_deinit(void)
{
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), old_stdout_mode);
    SetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), old_stderr_mode);
}

/**
 * Get the current executable program's path.
 * @param[out] buf The path output buffer.
 * @param len The buffer's length.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_exe_path(char *buf, size_t len)
{
    char tmp[MAX_PATH];
    char *s, *d;
    DWORD r;

    r = GetModuleFileNameA(NULL, tmp, MAX_PATH);
    if (r == 0)
        return OX_ERR;

    s = tmp;
    d = buf;

    while (*s)
    {
        if (*s == '\\')
        {
            *d = '/';
        }
        else
        {
            *d = *s;
        }

        d++;
        s++;
    }

    *d = 0;
    return OX_OK;
}

/**
 * Get the current thread's ID.
 * @return The current thread's ID.
 */
int ox_thread_id(void)
{
    return GetCurrentThreadId();
}

#ifndef OX_SUPPORT_PTHREAD

/**
 * Create a new thread.
 * @param ctxt The current running context.
 * @param[out] th Return the new thread.
 * @param entry The entry function.
 * @param arg The argument of the entry function.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_thread_create(OX_Context *ctxt, OX_Thread *th, void *(entry)(void *arg), void *arg)
{
    HANDLE handle;

    handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)entry, arg, 0, NULL);
    if (handle == NULL)
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
            (LPTSTR)&lpMsgBuf,
            0, NULL);

        ox_throw_system_error(ctxt, OX_TEXT("\"%s\" failed: %s"),
                              "CreateThread", lpMsgBuf);

        LocalFree(lpMsgBuf);
        return OX_ERR;
    }

    *th = handle;
    return OX_OK;
}

/**
 * Wait until the thread is end and release the thread's resource.
 * @param ctxt The current running context.
 * @param th The thread.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_thread_join(OX_Context *ctxt, OX_Thread *th)
{
    HANDLE handle = *th;

    if (handle)
    {
        WaitForSingleObject(handle, INFINITE);
        CloseHandle(handle);
        *th = NULL;
    }

    return OX_OK;
}

/**
 * Set the thread as detached.
 * Detached means the thread's resource will be released by itself.
 * @param ctxt The current running context.
 * @param th The thread.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_thread_detach(OX_Context *ctxt, OX_Thread *th)
{
    HANDLE handle = *th;

    if (handle)
    {
        CloseHandle(handle);
        *th = NULL;
    }

    return OX_OK;
}

/**
 * Yield the thread.
 */
void ox_thread_yield()
{
    SwitchToThread();
}

/*Fls's callback function.*/
static void
fls_callback(void *data)
{
    OX_Context *ctxt = data;

    ox_context_free(ctxt);
}

/**
 * Initialize the thread key.
 * @param key The key to be initialized.
 */
void ox_thread_key_init(OX_ThreadKey *key)
{
    DWORD fls;

    fls = FlsAlloc(fls_callback);
    assert(fls != FLS_OUT_OF_INDEXES);

    *key = fls;
}

/**
 * Release the thread key.
 * @param key The key to be released.
 */
void ox_thread_key_deinit(OX_ThreadKey *key)
{
    FlsFree(*key);
}

/**
 * Get the thread key's value.
 * @param key The thread key.
 * @return The value of the key.
 */
void *
ox_thread_key_get(OX_ThreadKey *key)
{
    return FlsGetValue(*key);
}

/**
 * Set the thread key's value.
 * @param key The thread key.
 * @param v The new value of the key.
 */
void ox_thread_key_set(OX_ThreadKey *key, void *v)
{
    FlsSetValue(*key, v);
}

/**
 * Initialize the mutex.
 * @param mutex The mutex to be initialized.
 */
void ox_mutex_init(OX_Mutex *mutex)
{
    InitializeCriticalSection(mutex);
}

/**
 * Release the mutex.
 * @param mutex the mutex to be released.
 */
void ox_mutex_deinit(OX_Mutex *mutex)
{
    DeleteCriticalSection(mutex);
}

/**
 * Lock the mutex.
 * @param mutex The mutex.
 */
void ox_mutex_lock(OX_Mutex *mutex)
{
    EnterCriticalSection(mutex);
}

/**
 * Unlock the mutex.
 * @param mutex The mutex.
 */
void ox_mutex_unlock(OX_Mutex *mutex)
{
    LeaveCriticalSection(mutex);
}

/**
 * Initialize the condition variable.
 * @param cv The condition varaible to be initialized.
 */
void ox_cond_var_init(OX_CondVar *cv)
{
    InitializeConditionVariable(cv);
}

/**
 * Release the condition variable.
 * @param cv The condition varaible to be released.
 */
void ox_cond_var_deinit(OX_CondVar *cv)
{
}

/**
 * Wakeup a thread waiting for the condition variable.
 * @param cv The condition variable.
 */
void ox_cond_var_signal(OX_CondVar *cv)
{
    WakeConditionVariable(cv);
}

/**
 * Suspend the current thread and wait for the conditon variable.
 * @param cv The condition variable.
 * @param mutex The mutex.
 * @param timeout_ms Waiting timeout in microseconds.
 * @retval OX_OK On success.
 * @retval OX_FALSE Timeout.
 */
OX_Result
ox_cond_var_wait(OX_CondVar *cv, OX_Mutex *mutex, int timeout_ms)
{
    DWORD ms;
    BOOL b;
    OX_Result r;

    if (timeout_ms < 0)
        ms = INFINITE;
    else
        ms = timeout_ms;

    b = SleepConditionVariableCS(cv, mutex, ms);
    if (b)
        r = OX_OK;
    else if (GetLastError() == ERROR_TIMEOUT)
        r = OX_FALSE;
    else
        r = OX_ERR;

    return r;
}

#endif /* OX_SUPPORT_PTHREAD */

/**
 * Open a dynamic library.
 * @param ctxt The current running context.
 * @param name Name of the dynamic library.
 * @return The handle of the dynamic library.
 * @retval NULL On error.
 */
void *
ox_dl_open(OX_Context *ctxt, const char *name)
{
    HMODULE handle;

    handle = LoadLibraryA(name);
    if (handle == NULL)
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
            (LPTSTR)&lpMsgBuf,
            0, NULL);

        ox_throw_system_error(ctxt,
                              OX_TEXT("\"%s\" \"%s\" failed: %s"),
                              "LoadLibraryA",
                              name,
                              lpMsgBuf);

        LocalFree(lpMsgBuf);
    }

    return (void *)handle;
}

/*Close dynamic library file.*/
int dlclose(void *handle)
{
    HMODULE h = (HMODULE)handle;
    BOOL b;

    b = FreeLibrary(h);
    if (!b)
        return -1;

    return 0;
}

/*Get symbol from the dynamic library file.*/
void *
dlsym(void *handle, const char *symbol)
{
    HMODULE h = (HMODULE)handle;
    void *p;

    p = GetProcAddress(h, symbol);

    return p;
}

/*Get the real pathname.*/
char *
realpath(const char *path, char *resolved_path)
{
    DWORD size;

    assert(path && resolved_path);

    size = GetFullPathNameA(path, PATH_MAX, resolved_path, NULL);
    if (size < PATH_MAX)
    {
        struct stat st;
        int r;

        r = stat(resolved_path, &st);
        if (r == 0)
            return resolved_path;
    }

    return NULL;
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
ox_path_to_str(OX_Context *ctxt, const char *path, OX_Value *s)
{
    OX_CharBuffer cb;
    OX_Result r;
    const char *c;

    ox_char_buffer_init(&cb);

    c = path;

    while (*c)
    {
        if (*c == '\\')
        {
            r = ox_char_buffer_append_char(ctxt, &cb, '/');
        }
        else
        {
            r = ox_char_buffer_append_char(ctxt, &cb, *c);
        }
        if (r == OX_ERR)
            goto end;

        c++;
    }

    r = ox_char_buffer_get_string(ctxt, &cb, s);
end:
    ox_char_buffer_deinit(ctxt, &cb);
    return r;
}

#endif /*ARCH_WIN*/