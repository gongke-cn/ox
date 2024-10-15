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

#define OX_LOG_TAG "ox_context"

#include "ox_internal.h"

/**
 * Set the file's character encoding.
 * @param ctxt The current running context.
 * @param enc Character encoding.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_set_file_enc (OX_Context *ctxt, const char *enc)
{
    OX_VM *vm;

    assert(ctxt && enc);

    vm = ox_vm_get(ctxt);

    if (vm->file_enc)
        ox_strfree(ctxt, vm->file_enc);

    vm->file_enc = ox_strdup(ctxt, enc);

    return vm->file_enc ? OX_OK : OX_ERR;
}

/**
 * Set the OX installation directory.
 * @param ctxt The current running context.
 * @param dir The installation directory.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_set_install_dir (OX_Context *ctxt, const char *dir)
{
    OX_VM *vm;

    assert(ctxt && dir);

    vm = ox_vm_get(ctxt);

    if (vm->install_dir)
        ox_strfree(ctxt, vm->install_dir);

    vm->install_dir = ox_strdup(ctxt, dir);

    return vm->install_dir ? OX_OK : OX_ERR;
}

/**
 * Get the OX installation directory.
 * @param ctxt The current running context.
 * @return The installation directory.
 */
const char*
ox_get_install_dir (OX_Context *ctxt)
{
    OX_VM *vm;

    assert(ctxt);

    vm = ox_vm_get(ctxt);

    return vm->install_dir;
}

/**
 * Get the OX library's sub directory name from the installation direction.
 * @return The library's directory.
 */
extern const char*
ox_get_lib_dir (OX_Context *ctxt)
{
#ifdef ARCH_LINUX
    const char *inst = ox_get_install_dir(ctxt);
    char path[PATH_MAX];
    char *lib;
    struct stat st;
    int r;

#ifdef __x86_64__
    lib = "lib/x86_64-linux-gnu";
    snprintf(path, sizeof(path), "%s/%s/libc.so", inst, lib);
    if ((r = stat(path, &st)) == 0)
        return lib;
#endif /*__x86_64__*/

#ifdef __i686__
    lib = "lib/i386-linux-gnu";
    snprintf(path, sizeof(path), "%s/%s/libc.so", inst, lib);
    if ((r = stat(path, &st)) == 0)
        return lib;
#endif /*__i686__*/

#if __SIZEOF_POINTER__  == 8
    lib = "lib64";
    snprintf(path, sizeof(path), "%s/%s/libc.so", inst, lib);
    if ((r = stat(path, &st)) == 0)
        return lib;
#elif __SIZEOF_POINTER__  == 4
    lib = "lib32";
    snprintf(path, sizeof(path), "%s/%s/libc.so", inst, lib);
    if ((r = stat(path, &st)) == 0)
        return lib;
#endif /*__SIZEOF_POINTER__*/
#endif /*ARCH_LINUX*/

    return "lib";
}

/**
 * Set the OX to dump stack when throw an error.
 * @param ctxt The current running context.
 * @param b Dump stack or not.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_set_dump_throw (OX_Context *ctxt, OX_Bool b)
{
    OX_VM *vm;

    assert(ctxt);

    vm = ox_vm_get(ctxt);

    vm->dump_throw = b;

    return OX_OK;
}

/**
 * Dump the stack information.
 * @param ctxt The current running context.
 * @param fp Output file.
 */
void
ox_stack_dump (OX_Context *ctxt, FILE *fp)
{
    assert(ctxt && fp);

    if (ctxt->error_frames) {
        OX_VS_PUSH(ctxt, name)
        OX_Frame *frame;
        OX_Result r;
        int i = 0;

        fprintf(fp, OX_TEXT("stack:\n"));

        frame = ctxt->error_frames;
        while (frame) {
            OX_Bool need_lf = OX_TRUE;

            fprintf(fp, "#%d: ", i);

            if (ox_value_get_gco_type(ctxt, &frame->func) == OX_GCO_FUNCTION) {
                /*Byte code function.*/
                OX_Function *f = ox_value_get_gco(ctxt, &frame->func);
                OX_BcScript *s = f->sfunc->script;

                if ((r = ox_get_full_name(ctxt, &frame->func, name)) == OX_OK) {
                    if (ox_string_length(ctxt, name))
                        fprintf(fp, "%s", ox_string_get_char_star(ctxt, name));
                    else
                        fprintf(fp, OX_TEXT("[noname function]"));
                }

                if (s->input)
                    fprintf(fp, " \"%s\"", s->input->name);

                if (frame->ip != -1) {
                    int line;

                    line = ox_function_lookup_line(ctxt, &frame->func, frame->ip);
                    if (line > 0) {
                        OX_Location loc;

                        fprintf(fp, OX_TEXT(" line: %d\n"), line);
                        need_lf = OX_FALSE;

                        loc.first_line = loc.last_line = line;
                        loc.first_column = loc.last_column = 0;

                        ox_input_show_text(ctxt, s->input, &loc, NULL);
                    }
                }
            } else {
                /*Native function.*/
                OX_NativeFunc *f = ox_value_get_gco(ctxt, &frame->func);
                OX_Bool has_name = OX_FALSE;

                if ((r = ox_get_full_name(ctxt, &frame->func, name)) == OX_OK) {
                    if (ox_string_length(ctxt, name)) {
                        fprintf(fp, "%s", ox_string_get_char_star(ctxt, name));
                        has_name = OX_TRUE;
                    }
                }

                if (!has_name)
                    fprintf(fp, OX_TEXT("[native function]"));

                if (f->script && f->script->he.key) {
                    char *path = f->script->he.key;
                    char buf[PATH_MAX];
                    char *p;

                    snprintf(buf, sizeof(buf), "%s", path);

                    p = basename(buf);

                    if (p)
                        fprintf(fp, " \"%s\"", p);
                }
            }

            if (need_lf)
                fprintf(fp, "\n");

            i ++;
            frame = frame->bot;
        }

        OX_VS_POP(ctxt, name)
    }
}

/**
 * Get the global object.
 * @param ctxt The current running context.
 * @return The global object.
 */
OX_Value*
ox_global_object (OX_Context *ctxt)
{
    return OX_OBJECT(ctxt, Global);
}

/**
 * Get the iterator object.
 * @param ctxt The current running context.
 * @return The iterator object.
 */
OX_Value*
ox_iterator_object (OX_Context *ctxt)
{
    return OX_OBJECT(ctxt, Iterator);
}

/**
 * Lock the virtual machine's lock.
 * @param ctxt The current running context.
 */
void
ox_lock (OX_Context *ctxt)
{
    assert(ctxt);

    if (ctxt->lock_cnt == 0) {
        OX_VM *vm = ox_vm_get(ctxt);

        ox_mutex_lock(&vm->lock);
    }

    ctxt->lock_cnt ++;
}

/**
 * Unlock the virtual machine's lock.
 * @param ctxt The current running context.
 */
extern void
ox_unlock (OX_Context *ctxt)
{
    assert(ctxt);
    assert(ctxt->lock_cnt);

    ctxt->lock_cnt --;

    if (ctxt->lock_cnt == 0) {
        OX_VM *vm = ox_vm_get(ctxt);

        ox_mutex_unlock(&vm->lock);
    }
}

/**
 * Add a global reference to the object.
 * @param ctxt The current running context.
 * @param v The object value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_global_ref (OX_Context *ctxt, OX_Value *v)
{
    OX_GcObject *o;
    OX_GlobalRef *ref;
    OX_HashEntry **pe;
    OX_Result r;
    OX_VM *vm = ox_vm_get(ctxt);

    assert(ctxt && v);
    assert(ox_value_is_gco(ctxt, v, -1));

    o = ox_value_get_gco(ctxt, v);

    ref = ox_hash_lookup_c(ctxt, &vm->global_ref_hash, o, &pe, OX_GlobalRef, he);
    if (ref) {
        ref->ref ++;
    } else {
        if (!OX_NEW(ctxt, ref))
            return ox_throw_no_mem_error(ctxt);

        ref->ref = 1;
        if ((r = ox_hash_insert(ctxt, &vm->global_ref_hash, o, &ref->he, pe)) == OX_ERR) {
            OX_DEL(ctxt, ref);
            return r;
        }
    }

    return OX_OK;
}

/**
 * Remove a global reference to the object.
 * @param ctxt The current running context.
 * @param v The object value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_global_unref (OX_Context *ctxt, OX_Value *v)
{
    OX_GcObject *o;
    OX_GlobalRef *ref;
    OX_HashEntry **pe;
    OX_VM *vm = ox_vm_get(ctxt);

    assert(ctxt && v);
    assert(ox_value_is_gco(ctxt, v, -1));

    o = ox_value_get_gco(ctxt, v);

    ref = ox_hash_lookup_c(ctxt, &vm->global_ref_hash, o, &pe, OX_GlobalRef, he);
    if (ref) {
        ref->ref --;
        if (ref->ref == 0) {
            ox_hash_remove(ctxt, &vm->global_ref_hash, o, pe);
            OX_DEL(ctxt, ref);
        }
    }

    return OX_OK;
}
