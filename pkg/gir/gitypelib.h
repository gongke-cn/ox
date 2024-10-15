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
 * GITypelib
 */

/** Typelib data.*/
typedef struct {
    OX_HashEntry     he;   /**< Hash table entry data.*/
    OX_GIRepository *repo; /**< Repository.*/
    GITypelib       *lib;  /**< Type library.*/
    OX_Value         v;    /**< Value of the type library.*/
} OX_GITypeLib;

/*Scan referenced objects in the typelib.*/
static void
gitypelib_scan (OX_Context *ctxt, void *p)
{
    OX_GITypeLib *tl = p;

    ox_gc_scan_value(ctxt, &tl->repo->v);
}

/*Free the typelib.*/
static void
gitypelib_free (OX_Context *ctxt, void *p)
{
    OX_GITypeLib *tl = p;

    OX_DEL(ctxt, tl);
}

/*Operation functions of typelib.*/
static const OX_PrivateOps
gitypelib_ops = {
    gitypelib_scan,
    gitypelib_free
};

/*Lookup the typelib owned property.*/
static OX_Result
gitypelib_lookup (OX_Context *ctxt, OX_Value *o, OX_Value *k, OX_Value *v)
{
    OX_Result r;

    if (ox_value_is_string(ctxt, k)) {
        if ((r = ox_object_lookup(ctxt, o, k, v)) == OX_ERR)
            return r;

        if (ox_value_is_null(ctxt, v)) {
            OX_GITypeLib *tl;
            const char *ns;
            const char *name;
            GIBaseInfo *bi;

            tl = ox_object_get_priv(ctxt, o, &gitypelib_ops);
            assert(tl);

            ns = gi_typelib_get_namespace(tl->lib);
            name = ox_string_get_char_star(ctxt, k);

            bi = gi_repository_find_by_name(tl->repo->repo, ns, name);
            if (bi) {
                r = gibaseinfo_to_const_prop(ctxt, tl->repo, bi, o, v);
                gi_base_info_unref(bi);

                if (r != OX_FALSE)
                    return r;
            }
        }
    }

    return ox_object_lookup(ctxt, o, k, v);
}

/*Get the typelib's property.*/
static OX_Result
gitypelib_get (OX_Context *ctxt, OX_Value *o, OX_Value *k, OX_Value *v)
{
    OX_Result r;

    if (ox_value_is_string(ctxt, k)) {
        if ((r = ox_object_lookup(ctxt, o, k, v)) == OX_ERR)
            return r;

        if (ox_value_is_null(ctxt, v)) {
            OX_GITypeLib *tl;
            const char *ns;
            const char *name;
            GIBaseInfo *bi;

            tl = ox_object_get_priv(ctxt, o, &gitypelib_ops);
            assert(tl);

            ns = gi_typelib_get_namespace(tl->lib);
            name = ox_string_get_char_star(ctxt, k);

            bi = gi_repository_find_by_name(tl->repo->repo, ns, name);
            if (bi) {
                r = gibaseinfo_to_const_prop(ctxt, tl->repo, bi, o, v);
                gi_base_info_unref(bi);

                if (r != OX_FALSE)
                    return r;
            }
        }
    }

    return ox_object_get(ctxt, o, k, v);
}

/*Object operation fnuctions of typelib.*/
static OX_ObjectOps
gitypelib_obj_ops = {
    {
        OX_GCO_OBJECT,
        ox_object_scan,
        ox_object_free
    },
    ox_object_keys,
    gitypelib_lookup,
    gitypelib_get,
    ox_object_set,
    ox_object_del,
    ox_object_call
};

/*Load the typelib.*/
static OX_Result
gitypelib_load (OX_Context *ctxt, OX_GIRepository *repo, const char *ns, const char *ver, OX_Value *v)
{
    OX_VS_PUSH_2(ctxt, ev, depv)
    OX_GITypeLib *tl;
    GITypelib *lib;
    OX_HashEntry **pe;
    GError *error = NULL;
    size_t n_dep = 0, i;
    char **deps = NULL; 
    OX_Result r;

    tl = ox_hash_lookup_c(ctxt, &repo->lib_hash, (void*)ns, &pe, OX_GITypeLib, he);
    if (tl) {
        ox_value_copy(ctxt, v, &tl->v);
        r = OX_OK;
        goto end;
    }

    lib = gi_repository_require(repo->repo, ns, ver, GI_REPOSITORY_LOAD_FLAG_LAZY, &error);
    if (!lib) {
        gerror_to_value(ctxt, error, ev, &repo->script);
        ox_throw(ctxt, ev);
        error = NULL;
        r = OX_ERR;
        goto end;
    } else {
        OX_LOG_D(ctxt, "load typelib \"%s\" version \"%s\"", ns, ver);

        if ((r = ox_object_new(ctxt, v, NULL)) == OX_ERR)
            goto end;

        if (!OX_NEW(ctxt, tl)) {
            r = ox_throw_no_mem_error(ctxt);
            goto end;
        }

        tl->repo = repo;
        tl->lib = lib;

        ox_value_copy(ctxt, &tl->v, v);

        if ((r = ox_object_set_priv(ctxt, v, &gitypelib_ops, tl)) == OX_ERR) {
            gitypelib_free(ctxt, tl);
            goto end;
        }

        if ((r = ox_object_set_ops(ctxt, v, &gitypelib_obj_ops)) == OX_ERR)
            goto end;

        if ((r = ox_hash_insert(ctxt, &repo->lib_hash, (void*)gi_typelib_get_namespace(lib), &tl->he, pe)) == OX_ERR)
            goto end;

        /*Load dependencies.*/
        deps = gi_repository_get_immediate_dependencies(repo->repo, ns, &n_dep);
        if (deps) {
            for (i = 0; i < n_dep; i ++) {
                char *dep = deps[i];
                char *sep = strchr(dep, '-');
                char *dep_ns, *dep_ver;

                assert(sep);

                *sep = 0;
                dep_ns = dep;
                dep_ver = sep + 1;

                if ((r = gitypelib_load(ctxt, repo, dep_ns, dep_ver, depv)) == OX_ERR)
                    goto end;
            }
        }

        if ((r = ox_object_set_name_s(ctxt, v, ns)) == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    if (deps) {
        for (i = 0; i < n_dep; i ++)
            g_free(deps[i]);
        g_free(deps);
    }
    if (error)
        g_error_free(error);
    OX_VS_POP(ctxt, ev)
    return r;
}
