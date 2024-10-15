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
 * GIRepository.
 */

/*Scan the referenced objects in the repository.*/
static void
girepository_scan (OX_Context *ctxt, void *p)
{
    OX_GIRepository *repo = p;
    size_t i;
    OX_GIType *ty;
    OX_GITypeLib *lib;

    ox_gc_scan_value(ctxt, &repo->script);

    ox_hash_foreach_c(&repo->type_hash, i, ty, OX_GIType, he) {
        ox_gc_scan_value(ctxt, &ty->v);
    }

    ox_hash_foreach_c(&repo->lib_hash, i, lib, OX_GITypeLib, he) {
        ox_gc_scan_value(ctxt, &lib->v);
    }
}

/*Free the repository.*/
static void
girepository_free (OX_Context *ctxt, void *p)
{
    OX_GIRepository *repo = p;
    size_t i;
    OX_GIInst *inst;
    OX_GICallback *cb;

    ox_hash_foreach_c(&repo->inst_hash, i, inst, OX_GIInst, he) {
        inst->he.key = NULL;
    }

    ox_hash_foreach_c(&repo->cb_hash, i, cb, OX_GICallback, he) {
        inst->he.key = NULL;
    }

    ox_hash_deinit(ctxt, &repo->type_hash);
    ox_hash_deinit(ctxt, &repo->lib_hash);
    ox_hash_deinit(ctxt, &repo->inst_hash);
    ox_hash_deinit(ctxt, &repo->cb_hash);

    g_object_unref(repo->repo);

    OX_DEL(ctxt, repo);
}

/*Operation functions of GIRepository.*/
static const OX_PrivateOps
girepository_ops = {
    girepository_scan,
    girepository_free
};

/*Get GIRepository from a value.*/
static OX_GIRepository*
girepository_get (OX_Context *ctxt, OX_Value *v)
{
    OX_GIRepository *repo = ox_object_get_priv(ctxt, v, &girepository_ops);

    if (!repo)
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a GIRepository"));

    return repo;
}

/*GIRepository.$inf.$init*/
static OX_Result
GIRepository_inf_init (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_GIRepository *repo;
    OX_Result r;

    if (!OX_NEW(ctxt, repo)) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    if (!(repo->repo = gi_repository_new())) {
        r = ox_throw_no_mem_error(ctxt);
        OX_DEL(ctxt, repo);
        goto end;
    }

    ox_value_copy(ctxt, &repo->v, thiz);
    ox_size_hash_init(&repo->type_hash);
    ox_char_star_hash_init(&repo->lib_hash);
    ox_size_hash_init(&repo->inst_hash);
    ox_size_hash_init(&repo->cb_hash);
    ox_native_func_get_script(ctxt, f, &repo->script);

    if ((r = ox_object_set_priv(ctxt, thiz, &girepository_ops, repo)) == OX_ERR) {
        girepository_free(ctxt, repo);
        goto end;
    }

    r = OX_OK;
end:
    return r;
}

/*GIRepository.$inf.prepend_search_path*/
static OX_Result
GIRepository_inf_prepend_search_path (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *dirv = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, dirs)
    const char *dir;
    OX_GIRepository *repo;
    OX_Result r;

    if (!(repo = girepository_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_to_string(ctxt, dirv, dirs)) == OX_ERR)
        goto end;

    dir = ox_string_get_char_star(ctxt, dirs);

    gi_repository_prepend_search_path(repo->repo, dir);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, dirs)
    return r;
}

/*GIRepository.$inf.prepend_library_path*/
static OX_Result
GIRepository_inf_prepend_library_path (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *dirv = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, dirs)
    const char *dir;
    OX_GIRepository *repo;
    OX_Result r;

    if (!(repo = girepository_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_to_string(ctxt, dirv, dirs)) == OX_ERR)
        goto end;

    dir = ox_string_get_char_star(ctxt, dirs);

    gi_repository_prepend_library_path(repo->repo, dir);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, dirs)
    return r;
}

/*GIRepository.$inf.get_search_path*/
static OX_Result
GIRepository_inf_get_search_path (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, dirv)
    OX_GIRepository *repo;
    const char * const *dirs;
    size_t n_dir;
    OX_Result r;

    if (!(repo = girepository_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    dirs = gi_repository_get_search_path(repo->repo, &n_dir);
    if (!dirs) {
        ox_value_set_null(ctxt, rv);
    } else {
        const char *dir;
        size_t i;

        if ((r = ox_array_new(ctxt, rv, n_dir)) == OX_ERR)
            goto end;

        for (i = 0; i < n_dir; i ++) {
            dir = dirs[i];

            if ((r = ox_string_from_char_star(ctxt, dirv, dir)) == OX_ERR)
                goto end;
            if ((r = ox_array_set_item(ctxt, rv, i, dirv)) == OX_ERR)
                goto end;
        }
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, dirv)
    return r;
}

/*GIRepository.$inf.get_library_path*/
static OX_Result
GIRepository_inf_get_library_path (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, dirv)
    OX_GIRepository *repo;
    const char * const *dirs;
    size_t n_dir;
    OX_Result r;

    if (!(repo = girepository_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    dirs = gi_repository_get_library_path(repo->repo, &n_dir);
    if (!dirs) {
        ox_value_set_null(ctxt, rv);
    } else {
        const char *dir;
        size_t i;

        if ((r = ox_array_new(ctxt, rv, n_dir)) == OX_ERR)
            goto end;

        for (i = 0; i < n_dir; i ++) {
            dir = dirs[i];

            if ((r = ox_string_from_char_star(ctxt, dirv, dir)) == OX_ERR)
                goto end;
            if ((r = ox_array_set_item(ctxt, rv, i, dirv)) == OX_ERR)
                goto end;
        }
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, dirv)
    return r;
}

/*GIRepository.$inf.enumerate_versions*/
static OX_Result
GIRepository_inf_enumerate_versions (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *ns_arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH_2(ctxt, nsv, vv)
    OX_GIRepository *repo;
    const char *ns;
    char **versions = NULL;
    size_t n_version = 0, i;
    OX_Result r;

    if (!(repo = girepository_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_to_string(ctxt, ns_arg, nsv)) == OX_ERR)
        goto end;
    ns = ox_string_get_char_star(ctxt, nsv);

    versions = gi_repository_enumerate_versions(repo->repo, ns, &n_version);
    if (!versions) {
        ox_value_set_null(ctxt, rv);
    } else {
        char *version;

        if ((r = ox_array_new(ctxt, rv, n_version)) == OX_ERR)
            goto end;

        for (i = 0; i < n_version; i ++) {
            version = versions[i];

            if ((r = ox_string_from_char_star(ctxt, vv, version)) == OX_ERR)
                goto end;
            if ((r = ox_array_set_item(ctxt, rv, i, vv)) == OX_ERR)
                goto end;
        }
    }

    r = OX_OK;
end:
    if (versions) {
        for (i = 0; i < n_version; i ++) {
            g_free(versions[i]);
        }
        g_free(versions);
    }
    OX_VS_POP(ctxt, nsv)
    return r;
}

/*GIRepository.$inf.require*/
static OX_Result
GIRepository_inf_require (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *ns_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *ver_arg = ox_argument(ctxt, args, argc, 1);
    OX_VS_PUSH_2(ctxt, nsv, verv)
    OX_GIRepository *repo;
    const char *ns, *ver;
    OX_Result r;

    if (!(repo = girepository_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_to_string(ctxt, ns_arg, nsv)) == OX_ERR)
        goto end;
    ns = ox_string_get_char_star(ctxt, nsv);

    if ((r = ox_to_string(ctxt, ver_arg, verv)) == OX_ERR)
        goto end;
    ver = ox_string_get_char_star(ctxt, verv);

    /*Load the typelib.*/
    r = gitypelib_load(ctxt, repo, ns, ver, rv);
end:
    OX_VS_POP(ctxt, nsv)
    return r;
}

/*Initialize the GIRepository class.*/
static void
girepository_class_init (OX_Context *ctxt, OX_Value *s)
{
    OX_VS_PUSH_2(ctxt, c, inf)

    /*GIRepository*/
    ox_not_error(ox_named_class_new_s(ctxt, c, inf, NULL, "GIRepository"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_GIRepository, c));

    /*GIRepository.$inf*/
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$init", GIRepository_inf_init));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "prepend_search_path", GIRepository_inf_prepend_search_path));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "prepend_library_path", GIRepository_inf_prepend_library_path));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "get_search_path", GIRepository_inf_get_search_path));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "get_library_path", GIRepository_inf_get_library_path));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "enumerate_versions", GIRepository_inf_enumerate_versions));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "require", GIRepository_inf_require));

    OX_VS_POP(ctxt, c)
}
