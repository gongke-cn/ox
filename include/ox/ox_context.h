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
 * Running context.
 */

#ifndef _OX_CONTEXT_H_
#define _OX_CONTEXT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get the virtual machine from the context.
 * @param ctxt The current running context.
 * @return The virtual machine contains the context.
 */
static inline OX_VM*
ox_vm_get (OX_Context *ctxt)
{
    assert(ctxt);

    return ((OX_BaseContext*)ctxt)->vm;
}

/**
 * Get the current running context.
 * @param vm The virtual machine contains this context.
 * @return The new running context.
 */
extern OX_Context*
ox_context_get (OX_VM *vm);

/**
 * Set the file's character encoding.
 * @param ctxt The current running context.
 * @param enc Character encoding.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_set_file_enc (OX_Context *ctxt, const char *enc);

/**
 * Set the OX to dump stack when throw an error.
 * @param ctxt The current running context.
 * @param b Dump stack or not.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_set_dump_throw (OX_Context *ctxt, OX_Bool b);

/**
 * Set the OX installation directory.
 * @param ctxt The current running context.
 * @param dir The installation directory.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_set_install_dir (OX_Context *ctxt, const char *dir);

/**
 * Get the OX installation directory.
 * @param ctxt The current running context.
 * @return The installation directory.
 */
extern const char*
ox_get_install_dir (OX_Context *ctxt);

/**
 * Get the OX library's sub directory name from the installation direction.
 * @return The library's directory.
 */
extern const char*
ox_get_lib_dir (OX_Context *ctxt);

/**
 * Dump the stack information.
 * @param ctxt The current running context.
 * @param fp Output file.
 */
extern void
ox_stack_dump (OX_Context *ctxt, FILE *fp);

/**
 * Set the private data of the context.
 * @param ctxt The current running context.
 */
static inline void*
ox_get_priv_data (OX_Context *ctxt)
{
    return ((OX_BaseContext*)ctxt)->priv;
}

/**
 * Set the private data of the context.
 * @param ctxt The current running context.
 * @param data The private data.
 */
static inline void
ox_set_priv_data (OX_Context *ctxt, void *data)
{
    ((OX_BaseContext*)ctxt)->priv = data;
}

/**
 * Get the global object.
 * @param ctxt The current running context.
 * @return The global object.
 */
extern OX_Value*
ox_global_object (OX_Context *ctxt);

/**
 * Get the iterator object.
 * @param ctxt The current running context.
 * @return The iterator object.
 */
extern OX_Value*
ox_iterator_object (OX_Context *ctxt);

/**
 * Lock the virtual machine's lock.
 * @param ctxt The current running context.
 */
extern void
ox_lock (OX_Context *ctxt);

/**
 * Unlock the virtual machine's lock.
 * @param ctxt The current running context.
 */
extern void
ox_unlock (OX_Context *ctxt);

/**
 * If the schedule flag is set, suspend the current context.
 * @param ctxt The current running context.
 */
static inline void
ox_suspend (OX_Context *ctxt)
{
    if (((OX_BaseContext*)ctxt)->sched_cnt)
        ox_unlock(ctxt);
}

/**
 * Resume the suspended context.
 * @param ctxt The current running context.
 */
static inline void
ox_resume (OX_Context *ctxt)
{
    if (((OX_BaseContext*)ctxt)->sched_cnt)
        ox_lock(ctxt);
}

/** Run the command. The VM lock will be released when the schedule flag is set.*/
#define OX_SCHED(ctxt, cmd)\
    OX_STMT_BEGIN\
        ox_suspend(ctxt);\
        cmd;\
        ox_resume(ctxt);\
    OX_STMT_END

/**
 * Add a global reference to the object.
 * @param ctxt The current running context.
 * @param v The object value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_global_ref (OX_Context *ctxt, OX_Value *v);

/**
 * Remove a global reference to the object.
 * @param ctxt The current running context.
 * @param v The object value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_global_unref (OX_Context *ctxt, OX_Value *v);

#ifdef __cplusplus
}
#endif

#endif
