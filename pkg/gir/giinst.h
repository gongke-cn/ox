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
 * GI object instance.
 */

/** GI object instance.*/
typedef struct {
    OX_HashEntry  he;   /**< Hash table entry data.*/
    OX_GIType    *type; /**< Type of the instance.*/
    OX_Value      v;    /**< Value of the instance.*/
} OX_GIInst;

/*Scan referenced objects in the instance.*/
static void
giinst_scan (OX_Context *ctxt, void *p)
{
    OX_GIInst *inst = p;

    ox_gc_scan_value(ctxt, &inst->type->v);
}

/*Free the instance.*/
static void
giinst_free (OX_Context *ctxt, void *p)
{
    OX_GIInst *inst = p;

    if (inst->he.key) {
        OX_GIRepository *repo = inst->type->repo;

        ox_hash_remove(ctxt, &repo->inst_hash, inst->he.key, NULL);
    }

    OX_DEL(ctxt, inst);
}

/*GI instance's operation functions.*/
static const OX_PrivateOps
giinst_ops = {
    giinst_scan,
    giinst_free
};

/*Get GI instance data from the value.*/
static OX_GIInst*
giinst_get (OX_Context *ctxt, OX_Value *v)
{
    OX_GIInst *inst = ox_object_get_priv(ctxt, v, &giinst_ops);

    if (!inst)
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a GObject instance"));

    return inst;
}

/*Set the GI instance's pointer as NULL.*/
static void
giinst_set_null (OX_Context *ctxt, OX_Value *v)
{
    OX_GIInst *inst = giinst_get(ctxt, v);

    if (inst && inst->he.key) {
        ox_hash_remove(ctxt, &inst->type->repo->inst_hash, inst->he.key, NULL);

        if (GI_IS_STRUCT_INFO(inst->type->rti) || GI_IS_UNION_INFO(inst->type->rti)) {
            ox_cvalue_set_null(ctxt, v);
        } else if (GI_IS_OBJECT_INFO(inst->type->rti) || GI_IS_INTERFACE_INFO(inst->type->rti) ) {
            inst->he.key = NULL;
        }
    }
}

/*Convert the pointer to GI instance.*/
static OX_Result
giinst_to_value (OX_Context *ctxt, OX_GIRepository *repo, void *p, OX_GIOwn *pown, GIBaseInfo *bi, OX_Value *v)
{
    OX_VS_PUSH_2(ctxt, tyv, inf)
    OX_GIType *ty;
    OX_GIInst *inst;
    OX_HashEntry **pe;
    OX_Result r;

    inst = ox_hash_lookup_c(ctxt, &repo->inst_hash, p, &pe, OX_GIInst, he);
    if (inst) {
        ox_value_copy(ctxt, v, &inst->v);
        r = OX_OK;
        goto end;
    }

    if ((r = gibaseinfo_to_value(ctxt, repo, bi, tyv)) == OX_ERR)
        goto end;

    ty = gitype_get(ctxt, tyv);

    if (GI_IS_STRUCT_INFO(bi) || GI_IS_UNION_INFO(bi)) {
        OX_CValueInfo cvi;

        cvi.v.p = p;
        cvi.base = NULL;

        if (*pown) {
            *pown = OX_GI_OWN_NOTHING;
            cvi.own = OX_CPTR_EXTERNAL;
        } else {
            cvi.own = OX_CPTR_NON_OWNER;
        }

        if ((r = ox_cvalue_new(ctxt, v, tyv, &cvi)) == OX_ERR)
            goto end;
    } else {
        if ((r = ox_get(ctxt, tyv, OX_STRING(ctxt, _inf), inf)) == OX_ERR)
            goto end;

        if ((r = ox_object_new(ctxt, v, inf)) == OX_ERR)
            goto end;

        if (*pown) {
            *pown = OX_GI_OWN_NOTHING;
        } else {
            GIObjectInfoRefFunction fn;

            fn = gi_object_info_get_ref_function_pointer(GI_OBJECT_INFO(bi));
            if (fn)
                fn(p);
        }
    }

    if (!OX_NEW(ctxt, inst)) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    inst->type = ty;
    ox_value_copy(ctxt, &inst->v, v);

    if ((r = ox_object_set_priv(ctxt, v, &giinst_ops, inst)) == OX_ERR) {
        giinst_free(ctxt, inst);
        goto end;
    }

    r = ox_hash_insert(ctxt, &repo->inst_hash, p, &inst->he, pe);
end:
    OX_VS_POP(ctxt, tyv)
    return r;
}