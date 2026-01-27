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
 * Script.
 */

#define OX_LOG_TAG "ox_script"

#include "ox_internal.h"

/*Scan referenced objects in the script function.*/
static void
script_func_scan (OX_Context *ctxt, OX_ScriptFunc *sfunc)
{
    OX_ScriptDecl *decl;

    ox_list_foreach_c(&sfunc->decl_list, decl, OX_ScriptDecl, ln) {
        ox_gc_mark(ctxt, decl->he.key);
    }
}

/*Release the script function.*/
static void
script_func_deinit (OX_Context *ctxt, OX_ScriptFunc *sfunc)
{
    OX_ScriptDecl *decl, *ndecl;

    ox_list_foreach_safe_c(&sfunc->decl_list, decl, ndecl, OX_ScriptDecl, ln) {
        OX_DEL(ctxt, decl);
    }

    ox_hash_deinit(ctxt, &sfunc->decl_hash);
}

/*Scan referenced objects in the reference item.*/
static void
ref_item_scan (OX_Context *ctxt, OX_ScriptRefItem *item)
{
    ox_gc_scan_value(ctxt, &item->orig);
    ox_gc_scan_value(ctxt, &item->name);
}

/*Scan referenced objects in the reference entry.*/
static void
ref_scan (OX_Context *ctxt, OX_ScriptRef *ref)
{
    ox_gc_scan_value(ctxt, &ref->filename);
    ox_gc_scan_value(ctxt, &ref->script);
}

/*Initialize the script.*/
static void
script_init (OX_Context *ctxt, OX_Script *s)
{
    s->state = OX_SCRIPT_STATE_UNINIT;
    s->frame = NULL;
    s->he.key = NULL;
    s->ref_num = 0;
    s->ref_item_num = 0;
    s->refs = NULL;
    s->ref_items = NULL;

    ox_list_init(&s->pub_list);
    ox_size_hash_init(&s->pub_hash);

    ox_value_set_null(ctxt, &s->func);
    ox_value_set_null(ctxt, &s->error);
    ox_value_set_null(ctxt, &s->text_domain);
}

/*Scan referenced objects in the script.*/
static void
script_scan (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_Script *s = (OX_Script*)gco;
    OX_ScriptPublic *sp;
    size_t i;

    for (i = 0; i < s->ref_item_num; i ++)
        ref_item_scan(ctxt, &s->ref_items[i]);

    for (i = 0; i < s->ref_num; i ++)
        ref_scan(ctxt, &s->refs[i]);

    ox_list_foreach_c(&s->pub_list, sp, OX_ScriptPublic, ln) {
        ox_gc_mark(ctxt, sp->he.key);
    }

    if (s->frame)
        ox_gc_mark(ctxt, s->frame);

    ox_gc_scan_value(ctxt, &s->func);
    ox_gc_scan_value(ctxt, &s->error);
    ox_gc_scan_value(ctxt, &s->text_domain);
}

/*Release the script.*/
static void
script_deinit (OX_Context *ctxt, OX_Script *s)
{
    OX_ScriptPublic *sp, *nsp;

    if (s->ref_items)
        OX_DEL_N(ctxt, s->ref_items, s->ref_item_num);

    if (s->refs)
        OX_DEL_N(ctxt, s->refs, s->ref_num);

    ox_list_foreach_safe_c(&s->pub_list, sp, nsp, OX_ScriptPublic, ln) {
        OX_DEL(ctxt, sp);
    }
    ox_hash_deinit(ctxt, &s->pub_hash);

    if (s->he.key)
        ox_strfree(ctxt, s->he.key);
}

/*Free the script.*/
static void
script_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_Script *s = (OX_Script*)gco;

    script_deinit(ctxt, s);
    OX_DEL(ctxt, s);
}

/*Scan referenced objects in the byte code script.*/
static void
bc_script_scan (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_BcScript *s = (OX_BcScript*)gco;
    size_t i;

    script_scan(ctxt, gco);

    ox_gc_mark(ctxt, s->base);
    ox_gc_mark(ctxt, s->input);

    if (s->cvs)
        ox_gc_scan_values(ctxt, s->cvs, s->cv_num);

    if (s->pps)
        ox_gc_scan_values(ctxt, s->pps, s->pp_num);

    if (s->ts)
        ox_gc_scan_values(ctxt, s->ts, s->t_num);

    if (s->lts)
        ox_gc_scan_values(ctxt, s->lts, s->t_num);

    if (s->tts)
        ox_gc_scan_values(ctxt, s->tts, s->tt_num);

    if (s->ltts)
        ox_gc_scan_values(ctxt, s->ltts, s->tt_num);

    for (i = 0; i < s->sfunc_num; i ++)
        script_func_scan(ctxt, &s->sfuncs[i]);
}

/*Free the byte code script.*/
static void
bc_script_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_BcScript *s = (OX_BcScript*)gco;

    if (s->cvs)
        OX_DEL_N(ctxt, s->cvs, s->cv_num);

    if (s->pps)
        OX_DEL_N(ctxt, s->pps, s->pp_num);

    if (s->ts)
        OX_DEL_N(ctxt, s->ts, s->t_num);

    if (s->lts)
        OX_DEL_N(ctxt, s->lts, s->t_num);

    if (s->tts)
        OX_DEL_N(ctxt, s->tts, s->tt_num);

    if (s->ltts)
        OX_DEL_N(ctxt, s->ltts, s->tt_num);

    if (s->sfuncs) {
        size_t i;

        for (i = 0; i < s->sfunc_num; i ++)
            script_func_deinit(ctxt, &s->sfuncs[i]);

        OX_DEL_N(ctxt, s->sfuncs, s->sfunc_num);
    }

    if (s->bc)
        OX_DEL_N(ctxt, s->bc, s->bc_len);

    if (s->loc_tab)
        OX_DEL_N(ctxt, s->loc_tab, s->loc_tab_len);

    script_deinit(ctxt, &s->script);

    OX_DEL(ctxt, s);
}

/*Scan referenced objects in the native script.*/
static void
native_script_scan (OX_Context *ctxt, OX_GcObject *gco)
{
    script_scan(ctxt, gco);
}

/*Free the native script.*/
static void
native_script_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_NativeScript *s = (OX_NativeScript*)gco;

    if (s->handle)
        dlclose(s->handle);

    script_deinit(ctxt, &s->script);

    OX_DEL(ctxt, s);
}

/*Get the keys of the script.*/
static OX_Result
script_keys (OX_Context *ctxt, OX_Value *o, OX_Value *keys)
{
    OX_Script *s = ox_value_get_gco(ctxt, o);
    OX_VS_PUSH(ctxt, k)
    size_t i, idx;
    OX_ScriptPublic *sp;
    OX_Result r;

    if ((r = ox_array_new(ctxt, keys, ox_hash_size(&s->pub_hash))) == OX_ERR)
        goto end;

    idx = 0;
    ox_hash_foreach_c(&s->pub_hash, i, sp, OX_ScriptPublic, he) {
        ox_value_set_gco(ctxt, k, sp->he.key);

        if ((r = ox_array_set_item(ctxt, keys, idx, k)) == OX_ERR)
            goto end;

        idx ++;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, k)
    return r;
}

/*Lookup the script owned property.*/
static OX_Result
script_lookup (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    OX_Script *s = ox_value_get_gco(ctxt, o);
    OX_Result r;

    if (s->frame && ox_value_is_string(ctxt, p)) {
        OX_String *k;
        OX_ScriptPublic *sp;

        if ((r = ox_string_singleton(ctxt, p)) == OX_ERR)
            return r;

        k = ox_value_get_gco(ctxt, p);

        sp = ox_hash_lookup_c(ctxt, &s->pub_hash, k, NULL, OX_ScriptPublic, he);
        if (sp) {
            ox_value_copy(ctxt, v, &s->frame->v[sp->id]);
            return OX_OK;
        }
    }

    ox_value_set_null(ctxt, v);
    return OX_OK;
}

/*Get property of the script.*/
static OX_Result
script_get (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    OX_Script *s = ox_value_get_gco(ctxt, o);
    OX_Result r;

    if (s->frame && ox_value_is_string(ctxt, p)) {
        OX_String *k;
        OX_ScriptPublic *sp;

        if ((r = ox_string_singleton(ctxt, p)) == OX_ERR)
            return r;

        k = ox_value_get_gco(ctxt, p);

        sp = ox_hash_lookup_c(ctxt, &s->pub_hash, k, NULL, OX_ScriptPublic, he);
        if (sp) {
            ox_value_copy(ctxt, v, &s->frame->v[sp->id]);
            return OX_OK;
        } else {
            return ox_object_get_t(ctxt, OX_OBJECT(ctxt, Script_inf), p, v, o);
        }
    }
    
    return OX_FALSE;
}

/*Set property of the script.*/
static OX_Result
script_set (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    return ox_throw_access_error(ctxt, OX_TEXT("cannot set property of a script"));
}

/*Delete the property of the script.*/
static OX_Result
script_del (OX_Context *ctxt, OX_Value *o, OX_Value *p)
{
    return ox_throw_access_error(ctxt, OX_TEXT("cannot delete property of a script"));
}

/*Call the script.*/
static OX_Result
script_call (OX_Context *ctxt, OX_Value *o, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Script *s = ox_value_get_gco(ctxt, o);
    OX_Frame *bf;
    OX_Result r;

    if (s->state == OX_SCRIPT_STATE_ERROR)
        return ox_throw(ctxt, &s->error);

    if (s->state == OX_SCRIPT_STATE_CALLED) {
        ox_value_set_null(ctxt, rv);
        return OX_OK;
    }

    if ((r = ox_script_init(ctxt, o)) == OX_ERR)
        return r;

    bf = ctxt->frames;

    if (s->state == OX_SCRIPT_STATE_INITED) {
        if (!ox_value_is_null(ctxt, &s->func)) {
            s->frame->bot = bf;
            ctxt->frames = s->frame;

            /*Call the entry function.*/
            if (s->gco.ops->type == OX_GCO_BC_SCRIPT) {
                r = ox_function_call(ctxt, &s->func, o, args, argc, rv, NULL);
            } else {
                r = ox_native_func_call(ctxt, &s->func, o, args, argc, rv);
            }
            if (r == OX_ERR)
                goto error;
        }

        s->state = OX_SCRIPT_STATE_CALLED;
    }

    ctxt->frames = bf;

    return OX_OK;
error:
    ctxt->frames = bf;
    ox_value_copy(ctxt, &s->error, &ctxt->error);
    s->state = OX_SCRIPT_STATE_ERROR;
    return OX_ERR;
}

/*Operation functions of the script.*/
static const OX_ObjectOps
script_ops = {
    {
        OX_GCO_SCRIPT,
        script_scan,
        script_free
    },
    script_keys,
    script_lookup,
    script_get,
    script_set,
    script_del,
    script_call
};

/*Operation functions of the byte code script.*/
static const OX_ObjectOps
bc_script_ops = {
    {
        OX_GCO_BC_SCRIPT,
        bc_script_scan,
        bc_script_free
    },
    script_keys,
    script_lookup,
    script_get,
    script_set,
    script_del,
    script_call
};

/*Operation functions of the native script.*/
static const OX_ObjectOps
native_script_ops = {
    {
        OX_GCO_NATIVE_SCRIPT,
        native_script_scan,
        native_script_free
    },
    script_keys,
    script_lookup,
    script_get,
    script_set,
    script_del,
    script_call
};

/*Add a declaration to the script function.*/
static int
script_func_add_decl (OX_Context *ctxt, OX_ScriptFunc *sfunc, OX_DeclType type, OX_String *k)
{
    OX_Result r;
    OX_ScriptDecl *sdecl;
    OX_HashEntry **pe;

    sdecl = ox_hash_lookup_c(ctxt, &sfunc->decl_hash, k, &pe, OX_ScriptDecl, he);
    if (!sdecl) {
        if (!OX_NEW(ctxt, sdecl)) {
            ox_throw_no_mem_error(ctxt);
            return -1;
        }

        sdecl->id = sfunc->decl_hash.e_num;
        sdecl->type = type;

        if ((r = ox_hash_insert(ctxt, &sfunc->decl_hash, k, &sdecl->he, pe)) == OX_ERR) {
            OX_DEL(ctxt, sdecl);
            return -1;
        }

        if (type & OX_DECL_AUTO_CLOSE)
            ox_list_prepend(&sfunc->decl_list, &sdecl->ln);
        else
            ox_list_append(&sfunc->decl_list, &sdecl->ln);
    }

    return sdecl->id;
}

/*Add a public declaration entry to the script.*/
static int
script_add_public (OX_Context *ctxt, OX_Script *script, OX_String *k, int id)
{
    OX_ScriptPublic *spub;
    OX_HashEntry **pe;
    OX_Result r;

    spub = ox_hash_lookup_c(ctxt, &script->pub_hash, k, &pe, OX_ScriptPublic, he);
    if (!spub) {
        if (id == -1)
            id = script->pub_hash.e_num;

        if (!OX_NEW(ctxt, spub)) {
            ox_throw_no_mem_error(ctxt);
            return -1;
        }

        spub->id = id;

        if ((r = ox_hash_insert(ctxt, &script->pub_hash, k, &spub->he, pe)) == OX_ERR) {
            OX_DEL(ctxt, spub);
            return -1;
        }

        ox_list_append(&script->pub_list, &spub->ln);
    }

    return spub->id;
}

/*Add the script to the hash table.*/
static OX_Result
script_hash_add (OX_Context *ctxt, OX_Script *script, const char *path)
{
    OX_VM *vm = ox_vm_get(ctxt);
    char buf[PATH_MAX];
    char *fpath, *key;
    OX_Result r;

    fpath = realpath(path, buf);
    if (!fpath)
        return ox_throw_system_error(ctxt, OX_TEXT("get real path of \"%s\" failed"), path);

    if (!(key = ox_strdup(ctxt, fpath)))
        return ox_throw_no_mem_error(ctxt);

    if ((r = ox_hash_insert(ctxt, &vm->script_hash, key, &script->he, NULL)) == OX_ERR) {
        ox_strfree(ctxt, key);
        return r;
    }

    return OX_OK;
}

/*Load the byte code script.*/
static OX_Result
bc_script_load (OX_Context *ctxt, char *path, OX_Value *sv)
{
    OX_VS_PUSH_2(ctxt, input, ast)
    OX_Result r;

    if ((r = ox_file_input_new(ctxt, input, path)) == OX_ERR)
        goto end;

    if ((r = ox_parse(ctxt, input, ast, 0)) == OX_ERR)
        goto end;

    if ((r = ox_compile(ctxt, input, ast, sv, OX_COMPILE_FL_REGISTER)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    OX_VS_POP(ctxt, input)
    return r;
}

/*Initialize the script's frame buffer.*/
static OX_Result
script_init_frame (OX_Context *ctxt, OX_Value *v)
{
    OX_Script *s = ox_value_get_gco(ctxt, v);
    size_t len = s->pub_hash.e_num;

    /*Create frame.*/
    if (!(s->frame = ox_frame_push(ctxt, ox_value_null(ctxt), len)))
        return OX_ERR;

    ox_frame_pop(ctxt);
    return OX_OK;
}

/*Initialize the byte code script's frame buffer.*/
static OX_Result
bc_script_init_frame (OX_Context *ctxt, OX_Value *v)
{
    OX_BcScript *bs = ox_value_get_gco(ctxt, v);
    OX_ScriptFunc *sf = &bs->sfuncs[0];
    OX_Result r;

    /*Create the entry function.*/
    if ((r = ox_function_new(ctxt, &bs->script.func, sf)) == OX_ERR)
        return r;

    /*Create the frame.*/
    if (!(bs->script.frame = ox_frame_push(ctxt, &bs->script.func, sf->decl_hash.e_num)))
        return OX_ERR;

    ox_frame_pop(ctxt);
    return OX_OK;
}

/*Initialize the native script.*/
static OX_Result
native_script_init (OX_Context *ctxt, OX_Value *v)
{
    OX_NativeScript *ns = ox_value_get_gco(ctxt, v);
    OX_Result r;
    OX_Result (*load_fn) (OX_Context *ctxt, OX_Value *s);

    load_fn = dlsym(ns->handle, "ox_load");
    if (!load_fn)
        return ox_throw_syntax_error(ctxt, OX_TEXT("cannot load \"ox_load\" symbol"));

    if ((r = load_fn(ctxt, v)) == OX_ERR)
        return r;

    return OX_OK;
}

/*Initialize the native script's frame buffer.*/
static OX_Result
native_script_init_frame (OX_Context *ctxt, OX_Value *v)
{
    OX_NativeScript *ns = ox_value_get_gco(ctxt, v);
    OX_Result r;
    OX_CFunc exec_fn;

    /*Create function.*/
    exec_fn = dlsym(ns->handle, "ox_exec");
    if (exec_fn) {
        if ((r = ox_native_func_new(ctxt, &ns->script.func, exec_fn)) == OX_ERR)
            return r;
    }

    /*Create frame.*/
    if (!(ns->script.frame = ox_frame_push(ctxt, &ns->script.func, ns->frame_size)))
        return OX_ERR;

    ox_frame_pop(ctxt);
    return OX_OK;
}

/*Load the native script.*/
static OX_Result
native_script_load (OX_Context *ctxt, char *path, OX_Value *sv)
{
    void *handle;
    OX_NativeScript *s;
    OX_Result r;

    if (!(handle = ox_dl_open(ctxt, path)))
        return OX_ERR;

    if (!OX_NEW(ctxt, s)) {
        dlclose(handle);
        return ox_throw_no_mem_error(ctxt);
    }

    script_init(ctxt, &s->script);

    s->script.gco.ops = (OX_GcObjectOps*)&native_script_ops;

    s->handle = handle;
    s->frame_size = 0;

    ox_value_set_gco(ctxt, sv, s);
    ox_gc_add(ctxt, s);

    /*Add the script to hash table.*/
    if ((r = script_hash_add(ctxt, &s->script, path)) == OX_ERR)
        return r;

    return OX_OK;
}

/*Load script from filename.*/
static OX_Result
load_script_from_file (OX_Context *ctxt, char *path, OX_Value *sv, OX_Value *td)
{
    OX_VM *vm = ox_vm_get(ctxt);
    char buf[PATH_MAX];
    char *fpath;
    OX_Script *s = NULL;
    OX_HashEntry **pe;
    size_t len;
    OX_Result r;

    /*Check if the script is already loaded.*/
    if (!(fpath = realpath(path, buf)))
        return OX_FALSE;

    if ((s = ox_hash_lookup_c(ctxt, &vm->script_hash, fpath, &pe, OX_Script, he))) {
        ox_value_set_gco(ctxt, sv, s);
        return OX_OK;
    }

    /*Load the script.*/
    len = strlen(fpath);
    if ((len >= 4) && !strcasecmp(fpath + len - 4, ".oxn")) {
        r = native_script_load(ctxt, fpath, sv);
    } else {
        r = bc_script_load(ctxt, fpath, sv);
    }

    /*Store the text domain.*/
    if ((r == OX_OK) && td) {
        r = ox_script_set_text_domain(ctxt, sv, td, NULL);
    }

    return r;
}

/*Load script from a pathname.*/
static OX_Result
load_script_from_path (OX_Context *ctxt, char *path, OX_Value *sv, OX_Value *td)
{
    size_t len = strlen(path);
    OX_Result r;

    if ((len >= 3) && !strcasecmp(path + len - 3, ".ox")) {
        r = load_script_from_file(ctxt, path, sv, td);
    } else if ((len >= 4) && !strcasecmp(path + len - 4, ".oxn")) {
        r = load_script_from_file(ctxt, path, sv, td);
    } else {
        if (len + 4 >= PATH_MAX) {
            return ox_throw_range_error(ctxt,
                    OX_TEXT("pathname \"%s\" is too long"),
                    path);
        }

        strcpy(path + len, ".ox");
        r = load_script_from_file(ctxt, path, sv, td);

        if (r == OX_FALSE) {
            strcpy(path + len, ".oxn");
            r = load_script_from_file(ctxt, path, sv, td);
        }
    }

    return r;
}

/*Load script from package manager.*/
static OX_Result
load_script_from_package (OX_Context *ctxt, const char *ncstr, OX_Value *sv)
{
    OX_VS_PUSH_4(ctxt, pname, lname, pkg, path)
    char buf[PATH_MAX];
    char *c;
    const char *pcstr;
    OX_Result r;

    c = strchr(ncstr, '/');
    if (c) {
        if ((r = ox_string_from_chars(ctxt, pname, ncstr, c - ncstr)) == OX_ERR)
            goto end;

        if ((r = ox_string_from_char_star(ctxt, lname, c + 1)) == OX_ERR)
            goto end;
    } else {
        if ((r = ox_string_from_char_star(ctxt, pname, ncstr)) == OX_ERR)
            goto end;
    }

    if ((r = ox_package_lookup(ctxt, pname, pkg)) == OX_ERR)
        goto end;

    if ((r = ox_package_get_lib(ctxt, pkg, lname, path)) == OX_ERR)
        goto end;

    if (ox_string_length(ctxt, path) >= PATH_MAX) {
        r = ox_throw_range_error(ctxt,
                OX_TEXT("pathname \"%s\" is too long"),
                path);
        goto end;
    }

    pcstr = ox_string_get_char_star(ctxt, path);
    strcpy(buf, pcstr);

    r = load_script_from_path(ctxt, buf, sv, pname);
end:
    OX_VS_POP(ctxt, pname)
    return r;
}

/*Execute the script.*/
static OX_Result
script_exec (OX_Context *ctxt, OX_Value *sv)
{
    OX_VS_PUSH(ctxt, rv)
    OX_Script *s = ox_value_get_gco(ctxt, sv);
    OX_Script *old_s = ctxt->curr_script;
    OX_Result r;

    if (s->state == OX_SCRIPT_STATE_LOADREF) {
        r = ox_throw_reference_error(ctxt, OX_TEXT("circular reference"));
        goto end;
    }

    ctxt->curr_script = s;
    r = ox_call(ctxt, sv, ox_value_null(ctxt), NULL, 0, rv);
    ctxt->curr_script = old_s;

end:
    OX_VS_POP(ctxt, rv)
    return r;
}

/*Load a script.*/
static OX_Result
load_script (OX_Context *ctxt, OX_Script *base, OX_Value *file, OX_Value *sv)
{
    const char *path = ox_string_get_char_star(ctxt, file);
    enum {
        REL_PATH,
        ABS_PATH,
        PACKAGE,
        PACKAGE_FILE
    } mode = PACKAGE;
    char buf[PATH_MAX], buf1[PATH_MAX];
    char *dir, *bpath;
    OX_Value *td = NULL;
    OX_Result r;

    if (path[0] == '.') {
        if (path[1] == '/')
            mode = REL_PATH;
        else if ((path[1] == '.') && (path[2] == '/'))
            mode = REL_PATH;
    } else if (path[0] == '/') {
        mode = ABS_PATH;
    } else if (ox_char_is_alpha(path[0]) && (path[1] == ':')) {
        mode = ABS_PATH;
    } else if (strchr(path, '/')) {
        mode = PACKAGE_FILE;
    }

    if (base) {
        td = &base->text_domain;

        if (mode == REL_PATH) {
            if (base->gco.ops->type == OX_GCO_BC_SCRIPT) {
                base = (OX_Script*)((OX_BcScript*)base)->base;
            }

            if (!base->he.key)
                mode = ABS_PATH;
        }
    } else if (mode == REL_PATH) {
        mode = ABS_PATH;
    }

    switch (mode) {
    case REL_PATH:
        /*Relative path.*/
        bpath = base->he.key;
        r = snprintf(buf1, sizeof(buf1), "%s", bpath);
        if (r >= sizeof(buf)) {
            return ox_throw_range_error(ctxt,
                    OX_TEXT("pathname \"%s\" is too long"),
                    bpath);
        }

        if (!(dir = dirname(buf1))) {
            return ox_throw_syntax_error(ctxt,
                    OX_TEXT("cannot get dirname of \"%s\""), bpath);
        }

        r = snprintf(buf, sizeof(buf), "%s/%s", dir, path);
        if (r >= sizeof(buf)) {
            return ox_throw_range_error(ctxt,
                    OX_TEXT("pathname \"%s\" is too long"),
                    bpath);
        }

        r = load_script_from_path(ctxt, buf, sv, td);
        break;
    case ABS_PATH:
        /*Absolute path.*/
        r = snprintf(buf, sizeof(buf), "%s", path);
        if (r >= sizeof(buf)) {
            return ox_throw_range_error(ctxt,
                    OX_TEXT("pathname \"%s\" is too long"),
                    path);
        }
        
        r = load_script_from_path(ctxt, buf, sv, td);
        break;
    case PACKAGE:
        /*Package.*/
        r = ox_package_script(ctxt, file, sv);
        break;
    case PACKAGE_FILE:
        /*Package file.*/
        r = load_script_from_package(ctxt, path, sv);
        break;
    }

    if (r == OX_ERR)
        return r;

    if (!r) {
        return ox_throw_reference_error(ctxt,
                OX_TEXT("cannot find script \"%s\""),
                path);
    }

    if ((r = script_exec(ctxt, sv)) == OX_ERR)
        return r;

    return OX_OK;
}

/*Add a reference.*/
static OX_Result
add_ref (OX_Context *ctxt, OX_Script *s, OX_Value *name, OX_Value *v, int item_id)
{
    OX_GcObjectType type = s->gco.ops->type;
    size_t id;

    switch (type) {
    case OX_GCO_SCRIPT:
        /*Script.*/
        if ((id = ox_script_add_public(ctxt, s, name, -1)) == -1)
            return OX_ERR;
        break;
    case OX_GCO_BC_SCRIPT: {
        /*Byte code script.*/
        OX_BcScript *bs = (OX_BcScript*)s;
        OX_ScriptFunc *sf = &bs->sfuncs[0];

        if ((id = ox_script_func_add_decl(ctxt, sf, OX_DECL_REF, name)) == -1)
            return OX_ERR;
        break;
    }
    case OX_GCO_NATIVE_SCRIPT:
        /*Native script.*/
        assert(item_id != -1);
        id = item_id + s->pub_hash.e_num;
        break;
    default:
        assert(0);
    }

    ox_value_copy(ctxt, &s->frame->v[id], v);
    return OX_OK;
}

/*Load all references.*/
static OX_Result
load_all_refs (OX_Context *ctxt, OX_Script *s, OX_Script *ref)
{
    OX_ScriptPublic *spub;
    OX_VS_PUSH(ctxt, name)

    ox_list_foreach_c(&ref->pub_list, spub, OX_ScriptPublic, ln) {
        ox_value_set_gco(ctxt, name, spub->he.key);

        add_ref(ctxt, s, name, &ref->frame->v[spub->id], -1);
    }

    OX_VS_POP(ctxt, name)
    return OX_OK;
}

/*Load a script as a declaration.*/
static OX_Result
load_script_as_decl (OX_Context *ctxt, OX_Script *s, OX_Script *ref, OX_Value *local, int item_id)
{
    OX_VS_PUSH(ctxt, v)
    OX_Result r;

    ox_value_set_gco(ctxt, v, ref);
    r = add_ref(ctxt, s, local, v, item_id);

    OX_VS_POP(ctxt, v)
    return r;
}

/*Load a reference.*/
static OX_Result
load_ref (OX_Context *ctxt, OX_Script *s, OX_Script *ref, OX_Value *orig, OX_Value *name, int item_id)
{
    OX_ScriptPublic *spub;
    OX_String *key;
    OX_Result r;

    if ((r = ox_string_singleton(ctxt, orig)) == OX_ERR)
        return r;

    key = ox_value_get_gco(ctxt, orig);

    spub = ox_hash_lookup_c(ctxt, &ref->pub_hash, key, NULL, OX_ScriptPublic, he);
    if (!spub) {
        return ox_throw_reference_error(ctxt, OX_TEXT("\"%s\" is not defined"),
                ox_string_get_char_star(ctxt, orig));
    }

    return add_ref(ctxt, s, name, &ref->frame->v[spub->id], item_id);
}

/*Load referenced scripts.*/
static OX_Result
load_scripts (OX_Context *ctxt, OX_Script *s)
{
    OX_Result r;
    size_t i;

    for (i = 0; i < s->ref_num; i ++) {
        OX_ScriptRef *ref = &s->refs[i];
        OX_Script *sref;
        size_t j;

        if ((r = load_script(ctxt, s, &ref->filename, &ref->script)) == OX_ERR)
            return r;

        sref = ox_value_get_gco(ctxt, &ref->script);

        for (j = 0; j < ref->item_num; j ++) {
            OX_ScriptRefItem *item = &s->ref_items[ref->item_start + j];

            if (ox_string_equal(ctxt, &item->orig, OX_STRING(ctxt, star))
                    && !ox_value_is_object(ctxt, &item->name)) {
                /*Add all references to local declarations.*/
                OX_ScriptPublic *spub;
                OX_ScriptFunc *sf = NULL;

                if (s->gco.ops->type == OX_GCO_BC_SCRIPT) {
                    OX_BcScript *bs = (OX_BcScript*)s;

                    sf = &bs->sfuncs[0];
                }

                ox_list_foreach_c(&sref->pub_list, spub, OX_ScriptPublic, ln) {
                    OX_String *key = spub->he.key;
                    int id = -1;

                    if (sf) {
                        /*For byte code script, add referenced item to the bottom stack frame.*/
                        if ((id = script_func_add_decl(ctxt, sf, OX_DECL_REF, key)) == -1)
                            return OX_ERR;
                    }

                    if (ox_value_get_bool(ctxt, &item->name)) {
                        /*Add public entry.*/
                        if (script_add_public(ctxt, s, key, id) == -1)
                            return OX_ERR;
                    }
                }
            }
        }
    }

    return OX_OK;
}

/*Load references.*/
static OX_Result
load_refs (OX_Context *ctxt, OX_Script *s)
{
    size_t i;

    for (i = 0; i < s->ref_num; i ++) {
        OX_ScriptRef *ref = &s->refs[i];
        OX_Script *sref;
        size_t j;

        sref = ox_value_get_gco(ctxt, &ref->script);

        for (j = 0; j < ref->item_num; j ++) {
            int item_id = ref->item_start + j;
            OX_ScriptRefItem *item = &s->ref_items[item_id];
            OX_Result r;

            if (ox_string_equal(ctxt, &item->orig, OX_STRING(ctxt, star))) {
                if (!ox_value_is_string(ctxt, &item->name)) {
                    r = load_all_refs(ctxt, s, sref);
                } else {
                    r = load_script_as_decl(ctxt, s, sref, &item->name, item_id);
                }
            } else {
                r = load_ref(ctxt, s, sref, &item->orig, &item->name, item_id);
            }
            if (r == OX_ERR)
                return r;
        }
    }

    return OX_OK;
}

/**
 * Create a new script.
 * @param ctxt The current running context.
 * @param[out] sv Return the new script value.
 * @param path The path of the script.
 * @return The new script.
 * @retval NULL On error.
 */
OX_Script*
ox_script_new (OX_Context *ctxt, OX_Value *sv, OX_Value *path)
{
    OX_Script *s;
    const char *cstr;

    assert(ctxt && sv && path);
    assert(ox_value_is_string(ctxt, path));

    cstr = ox_string_get_char_star(ctxt, path);

    if (!OX_NEW(ctxt, s)) {
        ox_throw_no_mem_error(ctxt);
        return NULL;
    }

    script_init(ctxt, s);

    s->gco.ops = (OX_GcObjectOps*)&script_ops;

    ox_value_set_gco(ctxt, sv, s);
    ox_gc_add(ctxt, s);

    /*Add the script to hash table.*/
    if (script_hash_add(ctxt, s, cstr) == OX_ERR)
        return NULL;

    return s;
}

/**
 * Create a new byte code script.
 * @param ctxt The current running context.
 * @param[out] sv Return the new script value.
 * @param input The input of the script.
 * @param reg Register the script the manager or not.
 * @return The new script.
 * @retval NULL On error.
 */
OX_BcScript*
ox_bc_script_new (OX_Context *ctxt, OX_Value *sv, OX_Input *input, OX_Bool reg)
{
    OX_BcScript *s;

    assert(ctxt && sv && input);

    if (!OX_NEW(ctxt, s)) {
        ox_throw_no_mem_error(ctxt);
        return NULL;
    }

    script_init(ctxt, &s->script);

    s->script.gco.ops = (OX_GcObjectOps*)&bc_script_ops;

    s->base = s;
    s->input = input;
    s->cvs = NULL;
    s->cv_num = 0;
    s->pps = NULL;
    s->pp_num = 0;
    s->ts = NULL;
    s->lts = NULL;
    s->t_num = 0;
    s->tts = NULL;
    s->ltts = NULL;
    s->tt_num = 0;
    s->sfuncs = NULL;
    s->sfunc_num = 0;
    s->bc = NULL;
    s->bc_len = 0;
    s->loc_tab = NULL;
    s->loc_tab_len = 0;

    ox_value_set_gco(ctxt, sv, s);
    ox_gc_add(ctxt, s);

    if (reg) {
        if (input->gco.ops->type == OX_GCO_FILE_INPUT) {
            /*Add the script to hash table.*/
            if (script_hash_add(ctxt, &s->script, input->name) == OX_ERR)
                return NULL;
        }
    }

    return s;
}

/**
 * Initialize the script.
 * @param ctxt The current running context.
 * @param script The script.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_script_init (OX_Context *ctxt, OX_Value *script)
{
    OX_Script *s;
    OX_Result r;

    assert(ctxt && script);
    assert(ox_value_is_script(ctxt, script));

    s = ox_value_get_gco(ctxt, script);

    if (s->state == OX_SCRIPT_STATE_ERROR)
        return ox_throw(ctxt, &s->error);

    if (s->state == OX_SCRIPT_STATE_UNINIT) {
        /*Initialize the native script.*/
        if (s->gco.ops->type == OX_GCO_NATIVE_SCRIPT) {
            if ((r = native_script_init(ctxt, script)) == OX_ERR)
                goto error;
        }

        s->state = OX_SCRIPT_STATE_LOADREF;

        /*Load references.*/
        if ((r = load_scripts(ctxt, s)) == OX_ERR)
            goto error;

        /*Create the main funciton.*/
        switch (s->gco.ops->type) {
        case OX_GCO_SCRIPT:
            r = script_init_frame(ctxt, script);
            break;
        case OX_GCO_BC_SCRIPT:
            r = bc_script_init_frame(ctxt, script);
            break;
        case OX_GCO_NATIVE_SCRIPT:
            r = native_script_init_frame(ctxt, script);
            break;
        default:
            assert(0);
        }

        if (r == OX_ERR)
            goto error;

        /*Load references.*/
        if ((r = load_refs(ctxt, s)) == OX_ERR)
            goto error;

        s->state = OX_SCRIPT_STATE_INITED;
    }

    return OX_OK;
error:
    ox_value_copy(ctxt, &s->error, &ctxt->error);
    s->state = OX_SCRIPT_STATE_ERROR;
    return OX_ERR;
}

/**
 * Set the byte code script's base script to the caller.
 * @param ctxt The current running context.
 * @param script The script to be set.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_bc_script_set_base (OX_Context *ctxt, OX_Value *script)
{
    OX_BcScript *sp;
    OX_Frame *frame;

    assert(ctxt && script);
    assert(ox_value_is_gco(ctxt, script, OX_GCO_BC_SCRIPT));

    sp = ox_value_get_gco(ctxt, script);

    frame = ctxt->frames;
    while (frame) {
        if (ox_value_is_gco(ctxt, &frame->func, OX_GCO_FUNCTION)) {
            OX_Function *func = ox_value_get_gco(ctxt, &frame->func);

            sp->base = func->sfunc->script->base;
            break;
        }

        frame = frame->bot;
    }

    return OX_OK;
}

/**
 * Allocate reference buffer for the script.
 * @param ctxt The current running context.
 * @param script The script.
 * @param ref_num Number of referenced files.
 * @param item_num Number of refereced items.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_script_alloc_refs (OX_Context *ctxt, OX_Script *script, size_t ref_num, size_t item_num)
{
    size_t i;

    assert(!script->refs && !script->ref_items);

    if (!ref_num)
        return OX_OK;

    if (!OX_NEW_N(ctxt, script->refs, ref_num))
        return ox_throw_no_mem_error(ctxt);

    if (item_num) {
        if (!OX_NEW_N(ctxt, script->ref_items, item_num)) {
            OX_DEL_N(ctxt, script->refs, ref_num);
            script->refs = NULL;
            return ox_throw_no_mem_error(ctxt);
        }
    }

    script->ref_num = ref_num;
    script->ref_item_num = item_num;

    for (i = 0; i < ref_num; i ++) {
        OX_ScriptRef *ref = &script->refs[i];

        ox_value_set_null(ctxt, &ref->script);
        ox_value_set_null(ctxt, &ref->filename);
    }

    for (i = 0; i < item_num; i ++) {
        OX_ScriptRefItem *item = &script->ref_items[i];

        ox_value_set_null(ctxt, &item->orig);
        ox_value_set_null(ctxt, &item->name);
    }

    return OX_OK;
}

/**
 * Add a declaration to the script function.
 * @param ctxt The current running context.
 * @param sfunc The script function.
 * @param type The declaration's type.
 * @param name The name of the declaration.
 * @return The declaration's index in the frame.
 * @retval -1 On error.
 */
int
ox_script_func_add_decl (OX_Context *ctxt, OX_ScriptFunc *sfunc, OX_DeclType type, OX_Value *name)
{
    OX_Result r;
    OX_String *k;

    if ((r = ox_string_singleton(ctxt, name)) == OX_ERR)
        return -1;

    k = ox_value_get_gco(ctxt, name);

    return script_func_add_decl(ctxt, sfunc, type, k);
}

/**
 * Lookup the source code's line number.
 * @param ctxt The current running context.
 * @param sf The script function.
 * @param ip The instruction pointer.
 * @return The line number.
 * @retval -1 Cannot find the line.
 */
int
ox_script_func_lookup_line (OX_Context *ctxt, OX_ScriptFunc *sf, int ip)
{
    size_t min, max;

    assert(ctxt && sf);

    min = sf->loc_start;
    max = min + sf->loc_len;

    while (1) {
        size_t mid = (min + max) >> 1;
        OX_ScriptLoc *sloc = &sf->script->loc_tab[mid];

        if (sloc->ip == ip)
            return sloc->line;

        if (sloc->ip < ip) {
            if (mid == min)
                break;

            min = mid;
        } else {
            if (mid == max)
                break;

            max = mid;
        }
    }

    return sf->script->loc_tab[min].line;
}

/**
 * Add a public declaration entry to the script.
 * @param ctxt The current running context.
 * @param script The script.
 * @param name The name of the declaration.
 * @param id The declaration's index in the frame.
 * If id == -1, allocate an index and return it.
 * @return The declaration's index in the frame.
 * @retval -1 On error.
 */
int
ox_script_add_public (OX_Context *ctxt, OX_Script *script, OX_Value *name, int id)
{
    OX_String *k;
    OX_Result r;

    if ((r = ox_string_singleton(ctxt, name)) == OX_ERR)
        return -1;

    k = ox_value_get_gco(ctxt, name);

    return script_add_public(ctxt, script, k, id);
}

/**
 * Initialize the script hash table.
 * @param ctxt The current running context.
 */
void
ox_script_hash_init (OX_Context *ctxt)
{
    OX_VM *vm = ox_vm_get(ctxt);

    ox_char_star_hash_init(&vm->script_hash);
}

/**
 * Release the script hash table.
 * @param ctxt The current running context.
 */
void
ox_script_hash_deinit (OX_Context *ctxt)
{
    OX_VM *vm = ox_vm_get(ctxt);

    ox_hash_deinit(ctxt, &vm->script_hash);
}

/**
 * Scan the scripts in the hash table.
 * @param ctxt The current running context.
 */
void
ox_gc_scan_script_hash (OX_Context *ctxt)
{
    OX_VM *vm = ox_vm_get(ctxt);
    OX_Script *s;
    size_t i;

    ox_hash_foreach_c(&vm->script_hash, i, s, OX_Script, he) {
        ox_gc_mark(ctxt, s);
    }
}

/*Set the script's reference table.*/
static OX_Result
script_set_refs (OX_Context *ctxt, OX_Script *sp, const OX_RefDesc *rtab)
{
    const OX_RefDesc *pr;
    size_t ref_num = 0;
    size_t item_num = 0;
    OX_VS_PUSH_7(ctxt, map, n, a, o, orig, local, iter)
    OX_ScriptRef *ref;
    OX_ScriptRefItem *item;
    OX_Result r;

    /*Get all reference items.*/
    if ((r = ox_object_new(ctxt, map, NULL)) == OX_ERR)
        goto end;

    for (pr = rtab; pr->script || pr->orig; pr ++) {
        if (pr->script) {
            if ((r = ox_string_from_const_char_star(ctxt, n, pr->script)) == OX_ERR)
                goto end;

            if ((r = ox_get(ctxt, map, n, a)) == OX_ERR)
                goto end;

            if (ox_value_is_null(ctxt, a)) {
                if ((r = ox_array_new(ctxt, a, 0)) == OX_ERR)
                    goto end;

                if ((r = ox_set(ctxt, map, n, a)) == OX_ERR)
                    goto end;

                ref_num ++;
            }
        }

        if (pr->orig) {
            if ((r = ox_object_new(ctxt, o, NULL)) == OX_ERR)
                goto end;
            if ((r = ox_array_append(ctxt, a, o)) == OX_ERR)
                goto end;
            if ((r = ox_string_from_const_char_star(ctxt, orig, pr->orig)) == OX_ERR)
                goto end;
            if ((r = ox_set(ctxt, o, OX_STRING(ctxt, orig), orig)) == OX_ERR)
                goto end;

            if (pr->local) {
                if (!strcmp(pr->local, "1")) {
                    ox_value_set_bool(ctxt, local, OX_TRUE);
                } else if (!strcmp(pr->local, "0")) {
                    ox_value_set_bool(ctxt, local, OX_FALSE);
                } else {
                    if ((r = ox_string_from_const_char_star(ctxt, local, pr->local)) == OX_ERR)
                        goto end;
                }
            } else {
                ox_value_set_bool(ctxt, local, OX_TRUE);
            }

            if ((r = ox_set(ctxt, o, OX_STRING(ctxt, name), local)) == OX_ERR)
                goto end;

            item_num ++;
        }
    }

    /*Build the map.*/
    if ((r = ox_script_alloc_refs(ctxt, sp, ref_num, item_num)) == OX_ERR)
        goto end;

    if ((r = ox_object_iter_new(ctxt, iter, map, OX_OBJECT_ITER_KEY_VALUE)) == OX_ERR)
        goto end;

    ref = sp->refs;
    item = sp->ref_items;

    while (!ox_iterator_end(ctxt, iter)) {
        ox_not_error(ox_iterator_value(ctxt, iter, o));
        ox_not_error(ox_array_get_item(ctxt, o, 0, n));
        ox_not_error(ox_array_get_item(ctxt, o, 1, a));

        ox_value_copy(ctxt, &ref->filename, n);
        ox_value_set_null(ctxt, &ref->script);
        ref->item_start = item - sp->ref_items;
        ref->item_num = 0;

        if (!ox_value_is_null(ctxt, a)) {
            size_t i, len;

            len = ox_array_length(ctxt, a);
            for (i = 0; i < len; i ++) {
                ox_not_error(ox_array_get_item(ctxt, a, i, o));
                ox_not_error(ox_get(ctxt, o, OX_STRING(ctxt, orig), orig));
                ox_not_error(ox_get(ctxt, o, OX_STRING(ctxt, name), local));

                ox_value_copy(ctxt, &item->orig, orig);
                ox_value_copy(ctxt, &item->name, local);
                ref->item_num ++;
                item ++;
            }
        }

        ox_not_error(ox_iterator_next(ctxt, iter));
        ref ++;
    }

    if ((r = ox_close(ctxt, iter)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    if (r == OX_ERR) {
        if (sp->refs) {
            OX_DEL_N(ctxt, sp->refs, sp->ref_num);
            sp->refs = NULL;
        }

        if (sp->ref_items) {
            OX_DEL_N(ctxt, sp->ref_items, sp->ref_item_num);
            sp->ref_items = NULL;
        }

        sp->ref_num = 0;
        sp->ref_item_num = 0;
    }
    OX_VS_POP(ctxt, map)
    return r;
}

/*Set the script's public declarations*/
static OX_Result
script_set_pubs (OX_Context *ctxt, OX_Script *sp, const char **ptab)
{
    const char **pp;
    OX_VS_PUSH(ctxt, n)
    int id;
    OX_Result r;

    for (pp = ptab, id = 0; *pp; pp ++, id ++) {
        if ((r = ox_string_from_const_char_star(ctxt, n, *pp)) == OX_ERR)
            goto end;

        if (ox_script_add_public(ctxt, sp, n, id) == -1) {
            r = OX_ERR;
            goto end;
        }
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, n)
    return r;
}

/**
 * Set the script's description.
 * @param ctxt The current running context.
 * @param s The script.
 * @param desc The script's description.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_script_set_desc (OX_Context *ctxt, OX_Value *s, const OX_ScriptDesc *desc)
{
    OX_NativeScript *ns;
    OX_Result r;

    assert(ctxt && s && desc);
    assert(ox_value_is_gco(ctxt, s, OX_GCO_NATIVE_SCRIPT));

    ns = ox_value_get_gco(ctxt, s);

    if (desc->refs) {
        if ((r = script_set_refs(ctxt, &ns->script, desc->refs)) == OX_ERR)
            return r;
    }

    if (desc->pubs) {
        if ((r = script_set_pubs(ctxt, &ns->script, desc->pubs)) == OX_ERR)
            return r;
    }

    ns->frame_size = desc->frame_size;

    return OX_OK;
}

/**
 * Set the script's internal value by its index.
 * @param ctxt The current running context.
 * @param s The script.
 * @param id The index of the value.
 * @param v The new value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_script_set_value (OX_Context *ctxt, OX_Value *s, int id, OX_Value *v)
{
    OX_Script *sp;

    assert(ctxt && s && v);
    assert(ox_value_is_script(ctxt, s));

    sp = ox_value_get_gco(ctxt, s);
    assert(sp->frame);
    assert(id < sp->frame->len);

    ox_value_copy(ctxt, &sp->frame->v[id], v);

    return OX_OK;
}

/**
 * Get the script's internal value's pointer.
 * @param ctxt The current running context.
 * @param s The script.
 * @param id The index of the value.
 * @return The internal value's pointer.
 */
OX_Value*
ox_script_get_value (OX_Context *ctxt, OX_Value *s, int id)
{
    OX_Script *sp;

    assert(ctxt && s);

    if (ox_value_is_script(ctxt, s)) {
        sp = ox_value_get_gco(ctxt, s);
    } else if (ox_value_is_gco(ctxt, s, OX_GCO_NATIVE_FUNC)) {
        OX_NativeFunc *nf = ox_value_get_gco(ctxt, s);

        sp = nf->script;
    } else {
        assert(0);
    }

    assert(sp->frame);
    assert(id < sp->frame->len);

    return &sp->frame->v[id];
}

/**
 * Get the current OX script.
 * @param ctxt The current running context.
 * @param[out] s Return the current script.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_script_current (OX_Context *ctxt, OX_Value *s)
{
    OX_Frame *frame;

    assert(ctxt && s);

    frame = ox_frame_get(ctxt);
    if (!frame) {
        ox_value_set_null(ctxt, s);
    } else {
        OX_Function *func = ox_value_get_gco(ctxt, &frame->func);

        ox_value_set_gco(ctxt, s, func->sfunc->script);
    }

    return OX_OK;
}

/**
 * Load the script by its name.
 * @param ctxt The current running context.
 * @param base The name script try to load the script.
 * @param name The name of the script.
 * If name is starting with ".", ".." or "/", it means the pathname of the script.
 * Else the name is "PACKAGE/FILE".
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_script_load (OX_Context *ctxt, OX_Value *s, OX_Value *base, OX_Value *name)
{
    OX_Script *bs;

    assert(ctxt && s && name);
    assert(ox_value_is_string(ctxt, name));

    if (base && !ox_value_is_null(ctxt, base)) {
        assert(ox_value_is_script(ctxt, base));
        bs = ox_value_get_gco(ctxt, base);
    } else {
        bs = NULL;
    }

    return load_script(ctxt, bs, name, s);
}

/**
 * Set the script's text domain name.
 * @param ctxt The current running context.
 * @param s The script.
 * @param td The text domain name.
 * @param dir The base directory for message catalogs belonging to domain.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_script_set_text_domain (OX_Context *ctxt, OX_Value *s, OX_Value *td, const char *dir)
{
    OX_Script *sp;

    assert(ctxt && s && td);
    assert(ox_value_is_script(ctxt, s));
    assert(ox_value_is_string(ctxt, td) || ox_value_is_null(ctxt, td));

    sp = ox_value_get_gco(ctxt, s);
    ox_value_copy(ctxt, &sp->text_domain, td);

    if (dir) {
        const char *tdcstr = ox_string_get_char_star(ctxt, td);

        bindtextdomain(tdcstr, dir);
    }

    return OX_OK;
}

/** Script iterator data.*/
typedef struct {
    OX_ObjectIterType type;   /**< Type of the iterator.*/
    OX_Value          script; /**< The script value.*/
    OX_ScriptPublic  *spub;   /**< Teh current public declaration entry.*/
} OX_ScriptIter;

/*Scan the referenced objects in the script iterator.*/
static void
script_iter_scan (OX_Context *ctxt, void *p)
{
    OX_ScriptIter *si = p;

    ox_gc_scan_value(ctxt, &si->script);
}

/*Free the script iterator.*/
static void
script_iter_free (OX_Context *ctxt, void *p)
{
    OX_ScriptIter *si = p;

    OX_DEL(ctxt, si);
}

/*Script iterator data's operation functions.*/
static const OX_PrivateOps
script_iter_ops = {
    script_iter_scan,
    script_iter_free
};

/*Check if the value is a script.*/
static OX_Result
check_script (OX_Context *ctxt, OX_Value *v)
{
    if (!ox_value_is_script(ctxt, v))
        return ox_throw_type_error(ctxt, OX_TEXT("the value is not a script"));

    return OX_OK;
}

/*Create a new script iterator.*/
static OX_Result
script_iter_new (OX_Context *ctxt, OX_Value *iter, OX_Value *script, OX_ObjectIterType type)
{
    OX_Script *sp;
    OX_ScriptIter *si;
    OX_Result r;

    if ((r = check_script(ctxt, script)) == OX_ERR)
        return r;

    if ((r = ox_object_new(ctxt, iter, OX_OBJECT(ctxt, ScriptIterator_inf))) == OX_ERR)
        return r;

    if (!OX_NEW(ctxt, si))
        return ox_throw_no_mem_error(ctxt);

    sp = ox_value_get_gco(ctxt, script);

    ox_value_copy(ctxt, &si->script, script);
    si->type = type;
    si->spub = ox_list_head_c(&sp->pub_list, OX_ScriptPublic, ln);

    if ((r = ox_object_set_priv(ctxt, iter, &script_iter_ops, si)) == OX_ERR) {
        script_iter_free(ctxt, si);
        return r;
    }

    return OX_OK;
}

/*Script.$inf.$iter*/
static OX_Result
Script_inf_iter (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    return script_iter_new(ctxt, rv, thiz, OX_OBJECT_ITER_KEY_VALUE);
}

/*Script.$inf.entries*/
static OX_Result
Script_inf_entries (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    return script_iter_new(ctxt, rv, thiz, OX_OBJECT_ITER_KEY_VALUE);
}

/*Script.$inf.keys*/
static OX_Result
Script_inf_keys (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    return script_iter_new(ctxt, rv, thiz, OX_OBJECT_ITER_KEY);
}

/*Script.$inf.values*/
static OX_Result
Script_inf_values (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    return script_iter_new(ctxt, rv, thiz, OX_OBJECT_ITER_VALUE);
}

/*Get script iterator data from the value.*/
static OX_ScriptIter*
get_script_iter (OX_Context *ctxt, OX_Value *v)
{
    OX_ScriptIter *si;

    si = ox_object_get_priv(ctxt, v, &script_iter_ops);
    if (!si)
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a script iterator"));

    return si;
}

/*ScriptIterator.$inf.next*/
static OX_Result
ScriptIterator_inf_next (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_ScriptIter *si;

    if (!(si = get_script_iter(ctxt, thiz)))
        return OX_ERR;

    if (si->spub) {
        OX_Script *s = ox_value_get_gco(ctxt, &si->script);

        si->spub = ox_list_next_c(&s->pub_list, si->spub, OX_ScriptPublic, ln);
    }

    return OX_OK;
}

/*ScriptIterator.$inf.end get*/
static OX_Result
ScriptIterator_inf_end_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_ScriptIter *si;

    if (!(si = get_script_iter(ctxt, thiz)))
        return OX_ERR;

     ox_value_set_bool(ctxt, rv, si->spub ? OX_FALSE : OX_TRUE);
    return OX_OK;
}

/*ScriptIterator.$inf.value get*/
static OX_Result
ScriptIterator_inf_value_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_ScriptIter *si;
    OX_Script *s;
    OX_Result r;

    if (!(si = get_script_iter(ctxt, thiz)))
        return OX_ERR;

    s = ox_value_get_gco(ctxt, &si->script);

    if (si->spub) {
        switch (si->type) {
        case OX_OBJECT_ITER_KEY:
            ox_value_set_gco(ctxt, rv, si->spub->he.key);
            break;
        case OX_OBJECT_ITER_VALUE:
            if (s->frame)
                ox_value_copy(ctxt, rv, &s->frame->v[si->spub->id]);
            else
                ox_value_set_null(ctxt, rv);
            break;
        case OX_OBJECT_ITER_KEY_VALUE: {
            OX_Value *v;

            if ((r = ox_array_new(ctxt, rv, 2)) == OX_ERR)
                return r;

            v = ox_value_stack_push(ctxt);

            ox_value_set_gco(ctxt, v, si->spub->he.key);
            r = ox_array_set_item(ctxt, rv, 0, v);
            if (r == OX_OK) {
                if (s->frame)
                    r = ox_array_set_item(ctxt, rv, 1, &s->frame->v[si->spub->id]);
                else
                    r = ox_array_set_item(ctxt, rv, 1, ox_value_null(ctxt));
            }
            ox_value_stack_pop(ctxt, v);

            if (r == OX_ERR)
                return r;
            break;
        }
        default:
            assert(0);
        }
    } else {
        ox_value_set_null(ctxt, rv);
    }

    return OX_OK;
}

/*?
 *? @lib {Script} Script.
 *?
 *? @class{ Script The script class.
 *? When an OX file is compiled or a source string is compiled by "eval()",
 *? it will generate a script object.
 *?
 *? @func $iter Create an iterator to traverse the public symbols in the script.
 *? The value of the iterator is an array, item 0 is the name of the symbol,
 *? and item 1 is the value of the symbol.
 *? @return {Iterator[[String,Any]]} The iterator used to traverse the public symbols.
 *?
 *? @func entries Create an iterator to traverse the public symbols in the script.
 *? The value of the iterator is an array, item 0 is the name of the symbol,
 *? and item 1 is the value of the symbol.
 *? @return {Iterator[String,Any]} The iterator used to traverse the public symbols.
 *?
 *? @func keys Create and iterator to traverse the public symbols' names of the script.
 *? The value of the iterator is a string, it is the name of the symbol.
 *? @return {Iterator[String]} The iterator used to traverse the public symbols' names.
 *?
 *? @func values Create and iterator to traverse the public symbols' values of the script.
 *? The value of the iterator is the symbol's value.
 *? @return {Iterator[Any]} The iterator used to traverse the public symbols' values.
 *?
 *? @class}
 */

/**
 * Initialize the script object.
 * @param ctxt The current running context.
 */
void
ox_script_object_init (OX_Context *ctxt)
{
    OX_VS_PUSH(ctxt, iter)

    /*Script_inf.*/
    ox_not_error(ox_interface_new(ctxt, OX_OBJECT(ctxt, Script_inf)));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Script_inf),
            "$iter", Script_inf_iter));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Script_inf),
            "entries", Script_inf_entries));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Script_inf),
            "keys", Script_inf_keys));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, Script_inf),
            "values", Script_inf_values));

    /*Script iterator.*/
    ox_not_error(ox_class_new(ctxt, iter, OX_OBJECT(ctxt, ScriptIterator_inf)));
    ox_not_error(ox_class_inherit(ctxt, iter, OX_OBJECT(ctxt, Iterator)));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, ScriptIterator_inf),
            "next", ScriptIterator_inf_next));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, ScriptIterator_inf),
            "end", ScriptIterator_inf_end_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, ScriptIterator_inf),
            "value", ScriptIterator_inf_value_get, NULL));

    OX_VS_POP(ctxt, iter)
}
