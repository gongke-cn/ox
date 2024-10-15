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
 * Pthread functions.
 */

#define OX_LOG_TAG "ox_pthread"

#include "ox_internal.h"

#ifdef OX_SUPPORT_PTHREAD

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
ox_thread_create (OX_Context *ctxt, OX_Thread *th, void *(entry)(void *arg), void *arg)
{
    int r;

    r = pthread_create(th, NULL, entry, arg);
    if (r != 0)
        return ox_throw_syntax_error(ctxt, OX_TEXT("\"%s\" failed: %s"),
                "pthread_create", strerror(r));

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
ox_thread_join (OX_Context *ctxt, OX_Thread *th)
{
    int r;

    r = pthread_join(*th, NULL);
    if (r != 0)
        return ox_throw_syntax_error(ctxt, OX_TEXT("\"%s\" failed: %s"),
                "pthread_join", strerror(r));

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
ox_thread_detach (OX_Context *ctxt, OX_Thread *th)
{
    int r;

    r = pthread_detach(*th);
    if (r != 0)
        return ox_throw_syntax_error(ctxt, OX_TEXT("\"%s\" failed: %s"),
                "pthread_detach", strerror(r));

    return OX_OK;
}

/**
 * Yield the thread.
 */
void
ox_thread_yield ()
{
    sched_yield();
}

/*Destructor of the context key.*/
static void
ctxt_key_destr (void *p)
{
    OX_Context *ctxt = p;

    ox_context_free(ctxt);
}

/**
 * Initialize the thread key.
 * @param key The key to be initialized.
 */
void
ox_thread_key_init (OX_ThreadKey *key)
{
    pthread_key_create(key, ctxt_key_destr);
}

/**
 * Release the thread key.
 * @param key The key to be released.
 */
void
ox_thread_key_deinit (OX_ThreadKey *key)
{
    pthread_key_delete(*key);
}

/**
 * Get the thread key's value.
 * @param key The thread key.
 * @return The value of the key.
 */
void*
ox_thread_key_get (OX_ThreadKey *key)
{
    return pthread_getspecific(*key);
}

/**
 * Set the thread key's value.
 * @param key The thread key.
 * @param v The new value of the key.
 */
void
ox_thread_key_set (OX_ThreadKey *key, void *v)
{
    pthread_setspecific(*key, v);
}

/**
 * Initialize the mutex.
 * @param mutex The mutex to be initialized.
 */
void
ox_mutex_init (OX_Mutex *mutex)
{
    pthread_mutex_init(mutex, NULL);
}

/**
 * Release the mutex.
 * @param mutex the mutex to be released.
 */
void
ox_mutex_deinit (OX_Mutex *mutex)
{
    pthread_mutex_destroy(mutex);
}

/**
 * Lock the mutex.
 * @param mutex The mutex.
 */
void
ox_mutex_lock (OX_Mutex *mutex)
{
    pthread_mutex_lock(mutex);
}

/**
 * Unlock the mutex.
 * @param mutex The mutex.
 */
void
ox_mutex_unlock (OX_Mutex *mutex)
{
    pthread_mutex_unlock(mutex);
}

/**
 * Initialize the condition variable.
 * @param cv The condition varaible to be initialized.
 */
void
ox_cond_var_init (OX_CondVar *cv)
{
    pthread_cond_init(cv, NULL);
}

/**
 * Release the condition variable.
 * @param cv The condition varaible to be released.
 */
void
ox_cond_var_deinit (OX_CondVar *cv)
{
    pthread_cond_destroy(cv);
}

/**
 * Wakeup a thread waiting for the condition variable.
 * @param cv The condition variable.
 */
void
ox_cond_var_signal (OX_CondVar *cv)
{
    pthread_cond_signal(cv);
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
ox_cond_var_wait (OX_CondVar *cv, OX_Mutex *mutex, int timeout_ms)
{
    OX_Result r;

    if (timeout_ms < 0) {
        r = pthread_cond_wait(cv, mutex);

        r = (r == 0) ? OX_OK : OX_ERR;
    } else {
        struct timespec ts;

        clock_gettime(CLOCK_REALTIME, &ts);

        ts.tv_sec  += timeout_ms / 1000;
        ts.tv_nsec += (timeout_ms % 1000) * 1000000;

        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec ++;
            ts.tv_nsec -= 1000000000;
        }

        r = pthread_cond_timedwait(cv, mutex, &ts);
        if (r == ETIMEDOUT)
            r = OX_FALSE;
        else if (r == 0)
            r = OX_OK;
        else
            r = OX_ERR;
    }

    return r;
}

#endif /*OX_SUPPORT_PTHREAD*/