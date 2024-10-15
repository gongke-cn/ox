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
 * C type.
 */

#define OX_LOG_TAG "ox_ctype"

#include "ox_internal.h"

/*Check if 2 C types are equal.*/
static OX_Bool
ctype_equal (OX_Context *ctxt, OX_Value *cty1, OX_Value *cty2, OX_Bool strict);
/*Get the C function pointer from the value.*/
static OX_CFuncPtr*
cfunc_ptr_get (OX_Context *ctxt, OX_Value *v);

/*Scan referenced objects in the C type.*/
static void
ctype_scan (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_CType *cty = (OX_CType*)gco;

    ox_object_scan(ctxt, gco);

    ox_gc_scan_value(ctxt, &cty->pty);
    ox_gc_scan_value(ctxt, &cty->aty);

    switch (cty->kind) {
    case OX_CTYPE_PTR: {
        OX_CPtrType *pty = (OX_CPtrType*)cty;

        ox_gc_scan_value(ctxt, &pty->vty);
        break;
    }
    case OX_CTYPE_ARRAY: {
        OX_CArrayType *aty = (OX_CArrayType*)cty;

        ox_gc_scan_value(ctxt, &aty->ity);
        break;
    }
    case OX_CTYPE_FUNC: {
        OX_CFuncType *fty = (OX_CFuncType*)cty;

        ox_gc_scan_value(ctxt, &fty->rty);
        ox_gc_scan_values(ctxt, fty->atys, fty->argc);
        break;
    }
    case OX_CTYPE_STRUCT: {
        OX_CStructType *sty = (OX_CStructType*)cty;
        OX_CField *f;

        ox_list_foreach_c(&sty->f_list, f, OX_CField, ln) {
            ox_gc_mark(ctxt, f->he.key);
            ox_gc_scan_value(ctxt, &f->cty);
        }
        break;
    }
    default:
        break;
    }
}

/*Free the C type.*/
static void
ctype_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_CType *cty = (OX_CType*)gco;

    ox_object_deinit(ctxt, &cty->o);

    switch (cty->kind) {
    case OX_CTYPE_PTR: {
        OX_CPtrType *pty = (OX_CPtrType*)cty;

        OX_DEL(ctxt, pty);
        break;
    }
    case OX_CTYPE_ARRAY: {
        OX_CArrayType *aty = (OX_CArrayType*)cty;

        OX_DEL(ctxt, aty);
        break;
    }
    case OX_CTYPE_FUNC: {
        OX_CFuncType *fty = (OX_CFuncType*)cty;

        if (fty->atys)
            OX_DEL_N(ctxt, fty->atys, fty->argc);
        if (fty->atypes)
            OX_DEL_N(ctxt, fty->atypes, fty->argc);

        OX_DEL(ctxt, fty);
        break;
    }
    case OX_CTYPE_STRUCT: {
        OX_CStructType *sty = (OX_CStructType*)cty;
        OX_CField *f, *nf;

        ox_list_foreach_safe_c(&sty->f_list, f, nf, OX_CField, ln) {
            OX_DEL(ctxt, f);
        }
        ox_hash_deinit(ctxt, &sty->f_hash);

        OX_DEL(ctxt, sty);
        break;
    }
    default:
        OX_DEL(ctxt, cty);
        break;
    }
}

/*Call the C type.*/
static OX_Result
ctype_call (OX_Context *ctxt, OX_Value *o, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_CType *t = ox_value_get_gco(ctxt, o);
    OX_VS_PUSH(ctxt, ty)
    OX_CValueInfo cvi;
    size_t len = 1;
    size_t alloc = 0;
    OX_Bool is_array = OX_FALSE;
    OX_Result r;

    if (t->size == -1) {
        r = ox_throw_type_error(ctxt, OX_TEXT("size of C type is unknown"));
        goto end;
    }

    if (!ox_value_is_null(ctxt, arg)) {
        if ((r = ox_to_index(ctxt, arg, &len)) == OX_ERR)
            goto end;

        is_array = OX_TRUE;
    }

    if (is_array || t->kind == OX_CTYPE_STRUCT) {
        if ((r = ox_ctype_pointer(ctxt, ty, o, len)) == OX_ERR)
            goto end;

        alloc = t->size * len;

        cvi.v.p = ox_alloc_0(ctxt, alloc);
        if (!cvi.v.p) {
            r = ox_throw_no_mem_error(ctxt);
            goto end;
        }

        cvi.own = OX_CPTR_INTERNAL;
        cvi.size = alloc;
    } else {
        ox_value_copy(ctxt, ty, o);

        cvi.v.u64 = 0;
        cvi.own = OX_CPTR_NON_OWNER;
    }

    cvi.base = NULL;
    cvi.own |= OX_CPTR_ALLOC;

    if ((r = ox_cvalue_new(ctxt, rv, ty, &cvi)) == OX_ERR) {
        if (cvi.own)
            ox_free(ctxt, cvi.v.p, alloc);
        goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, ty)
    return r;
}

/*Operation functions of The C type.*/
static const OX_ObjectOps
ctype_ops = {
    {
        OX_GCO_CTYPE,
        ctype_scan,
        ctype_free
    },
    ox_object_keys,
    ox_object_lookup,
    ox_object_get,
    ox_object_set,
    ox_object_del,
    ctype_call
};

/*Initialize the C type.*/
static void
ctype_init (OX_Context *ctxt, OX_CType *cty, OX_CTypeKind kind, ssize_t size)
{
    ox_object_init(ctxt, &cty->o, OX_OBJECT(ctxt, CType_inf));

    cty->o.gco.ops = (OX_GcObjectOps*)&ctype_ops;
    cty->kind = kind;
    cty->size = size;

    ox_value_set_null(ctxt, &cty->aty);
    ox_value_set_null(ctxt, &cty->pty);
}

/*Create a new C type.*/
static OX_Result
ctype_new (OX_Context *ctxt, OX_Value *v, OX_CTypeKind kind, ssize_t size)
{
    OX_CType *t;

    if (!OX_NEW(ctxt, t))
        return ox_throw_no_mem_error(ctxt);

    ctype_init(ctxt, t, kind, size);

    ox_value_set_gco(ctxt, v, t);
    ox_gc_add(ctxt, t);

    return OX_OK;
}

/*Scan referenced objects in the C value.*/
static void
cvalue_scan (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_CValue *cv = (OX_CValue*)gco;

    ox_object_scan(ctxt, gco);

    ox_gc_scan_value(ctxt, &cv->cty);
    ox_gc_scan_value(ctxt, &cv->base);
}

/*Free the C value with an owned pointer in it.*/
static void
ptr_cvalue_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_CValue *cv = (OX_CValue*)gco;

    ox_object_deinit(ctxt, &cv->o);

    if (cv->v.p) {
        size_t len = ox_value_get_number(ctxt, &cv->base);

        ox_free(ctxt, cv->v.p, len);
    }

    OX_DEL(ctxt, cv);
}

/*Free the external C value with an owned pointer in it.*/
static void
ext_ptr_cvalue_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_CValue *cv = (OX_CValue*)gco;

    ox_object_deinit(ctxt, &cv->o);

    if (cv->v.p)
        free(cv->v.p);

    OX_DEL(ctxt, cv);
}

/*Free the C value.*/
static void
cvalue_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_CValue *cv = (OX_CValue*)gco;

    ox_object_deinit(ctxt, &cv->o);

    OX_DEL(ctxt, cv);
}

/*Check if the functions' return type are equal.*/
static OX_Bool
rtype_equal (OX_Context *ctxt, OX_Value *rt1, OX_Value *rt2)
{
    if (ox_value_is_null(ctxt, rt1) && ox_value_is_null(ctxt, rt2))
        return OX_TRUE;

    return ctype_equal(ctxt, rt1, rt2, OX_TRUE);
}

/*Check if 2 C types are equal.*/
static OX_Bool
ctype_equal (OX_Context *ctxt, OX_Value *cty1, OX_Value *cty2, OX_Bool strict)
{
    OX_CType *t1, *t2;
    OX_Bool r;

    assert(ox_value_is_ctype(ctxt, cty1));
    assert(ox_value_is_ctype(ctxt, cty2));

    t1 = ox_value_get_gco(ctxt, cty1);
    t2 = ox_value_get_gco(ctxt, cty2);

    if (t1 == t2)
        return OX_TRUE;

    if ((t1->kind == OX_CTYPE_VOID) || (t2->kind == OX_CTYPE_VOID))
        return OX_TRUE;

    if (t1->kind != t2->kind)
        return OX_FALSE;

    switch (t1->kind) {
    case OX_CTYPE_STRUCT: {
        r = OX_FALSE;
        break;
    }
    case OX_CTYPE_PTR: {
        OX_CPtrType *pt1 = (OX_CPtrType*)t1;
        OX_CPtrType *pt2 = (OX_CPtrType*)t2;

        if (!ctype_equal(ctxt, &pt1->vty, &pt2->vty, OX_TRUE))
            r = OX_FALSE;
        else if ((pt1->len == -1) || (pt2->len == -1))
            r = OX_TRUE;
        else if (strict)
            r = (pt1->len == pt2->len);
        else
            r = (pt1->len <= pt2->len);
        break;
    }
    case OX_CTYPE_ARRAY: {
        OX_CArrayType *at1 = (OX_CArrayType*)t1;
        OX_CArrayType *at2 = (OX_CArrayType*)t2;

        if (!ctype_equal(ctxt, &at1->ity, &at2->ity, OX_TRUE))
            r = OX_FALSE;
        else if ((at1->len == -1) || (at2->len == -1))
            r = OX_TRUE;
        else if (strict)
            r = (at1->len == at2->len);
        else
            r = (at1->len <= at2->len);
        break;
    }
    case OX_CTYPE_FUNC: {
        OX_CFuncType *ft1 = (OX_CFuncType*)t1;
        OX_CFuncType *ft2 = (OX_CFuncType*)t2;

        if (!rtype_equal(ctxt, &ft1->rty, &ft2->rty)) {
            r = OX_FALSE;
        } else if (ft1->argc != ft2->argc) {
            r = OX_FALSE;
        } else {
            size_t i;

            r = OX_TRUE;
            for (i = 0; i < ft1->argc; i ++) {
                if (!ctype_equal(ctxt, &ft1->atys[i], &ft2->atys[i], OX_TRUE)) {
                    r = OX_FALSE;
                    break;
                }
            }
        }
        break;
    }
    default:
        r = OX_TRUE;
        break;
    }

    return r;
}

/**
 * Get the C value from a pointer.
 * @param ctxt The current running context.
 * @param cty The type of the pointed value.
 * @param cvi The C pointer's information.
 * @param[out] v Return the pointed C value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_cptr_get_value (OX_Context *ctxt, OX_Value *cty, OX_CValueInfo *cvi, OX_Value *v)
{
    OX_VS_PUSH(ctxt, pty)
    OX_CType *t = ox_value_get_gco(ctxt, cty);
    OX_Result r = OX_OK;

    switch (t->kind) {
    case OX_CTYPE_U8: {
        uint8_t i = *(uint8_t*)cvi->v.p;
        ox_value_set_number(ctxt, v, i);
        break;
    }
    case OX_CTYPE_U16: {
        uint16_t i = *(uint16_t*)cvi->v.p;
        ox_value_set_number(ctxt, v, i);
        break;
    }
    case OX_CTYPE_U32: {
        uint32_t i = *(uint32_t*)cvi->v.p;
        ox_value_set_number(ctxt, v, i);
        break;
    }
    case OX_CTYPE_U64: {
        OX_CValueInfo vi;

        vi.v.u64 = *(uint64_t*)cvi->v.p;

        r = ox_cvalue_new(ctxt, v, cty, &vi);
        break;
    }
    case OX_CTYPE_I8: {
        int8_t i = *(int8_t*)cvi->v.p;
        ox_value_set_number(ctxt, v, i);
        break;
    }
    case OX_CTYPE_I16: {
        int16_t i = *(int16_t*)cvi->v.p;
        ox_value_set_number(ctxt, v, i);
        break;
    }
    case OX_CTYPE_I32: {
        int32_t i = *(int32_t*)cvi->v.p;
        ox_value_set_number(ctxt, v, i);
        break;
    }
    case OX_CTYPE_I64: {
        OX_CValueInfo vi;

        vi.v.i64 = *(int64_t*)cvi->v.p;

        r = ox_cvalue_new(ctxt, v, cty, &vi);
        break;
    }
    case OX_CTYPE_F32: {
        float f = *(float*)cvi->v.p;
        ox_value_set_number(ctxt, v, f);
        break;
    }
    case OX_CTYPE_F64: {
        double d = *(double*)cvi->v.p;
        ox_value_set_number(ctxt, v, d);
        break;
    }
    case OX_CTYPE_STR: {
        const char *c = *(const char**)cvi->v.p;

        if (c)
            r = ox_string_from_char_star(ctxt, v, c);
        else
            ox_value_set_null(ctxt, v);
        break;
    }
    case OX_CTYPE_ARRAY: {
        OX_CArrayType *at = ox_value_get_gco(ctxt, cty);

        assert(cvi->v.p);

        r = ox_ctype_pointer(ctxt, pty, &at->ity, at->len);
        if (r == OX_OK)
            r = ox_cvalue_new(ctxt, v, pty, cvi);
        break;
    }
    case OX_CTYPE_STRUCT: {
        assert(cvi->v.p);

        if ((r = ox_ctype_pointer(ctxt, pty, cty, 1)) == OX_OK)
            r = ox_cvalue_new(ctxt, v, pty, cvi);
        break;
    }
    case OX_CTYPE_PTR: {
        OX_CValueInfo vi;

        vi.v.p = *(void**)cvi->v.p;

        if (vi.v.p) {
            vi.base = cvi->base;
            vi.own = OX_CPTR_NON_OWNER;

            r = ox_cvalue_new(ctxt, v, cty, &vi);
        } else {
            ox_value_set_null(ctxt, v);
        }
        break;
    }
    case OX_CTYPE_FUNC: {
        OX_CValueInfo vi;

        vi.v.p = *(void**)cvi->v.p;
        if (vi.v.p) {
            vi.base = NULL;
            vi.own = OX_CPTR_NON_OWNER;

            r = ox_cvalue_new(ctxt, v, cty, &vi);
        } else {
            ox_value_set_null(ctxt, v);
        }
        break;
    }
    case OX_CTYPE_VOID: {
        r = ox_throw_type_error(ctxt, OX_TEXT("cannot get value from void pointer"));
    }
    }

    OX_VS_POP(ctxt, pty)
    return r;
}

/**
 * Set the value to the C pointer.
 * @param ctxt The current running context.
 * @param cty The type of the pointed value.
 * @param cvi The C pointer's information.
 * @param v The new value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_cptr_set_value (OX_Context *ctxt, OX_Value *cty, OX_CValueInfo *cvi, OX_Value *v)
{
    OX_CType *t = ox_value_get_gco(ctxt, cty);
    OX_Result r = OX_OK;
    OX_Number n;

    switch (t->kind) {
    case OX_CTYPE_U8:
        if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR)
            return r;

        *(uint8_t*)cvi->v.p = n;
        break;
    case OX_CTYPE_U16:
        if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR)
            return r;

        *(uint16_t*)cvi->v.p = n;
        break;
    case OX_CTYPE_U32:
        if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR)
            return r;

        *(uint32_t*)cvi->v.p = n;
        break;
    case OX_CTYPE_U64: {
        uint64_t i;

        if (ox_value_is_int64(ctxt, v)) {
            i = ox_value_get_uint64(ctxt, v);
        } else {
            if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR)
                return r;
            i = n;
        }

        *(uint64_t*)cvi->v.p = i;
        break;
    }
    case OX_CTYPE_I8:
        if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR)
            return r;

        *(int8_t*)cvi->v.p = n;
        break;
    case OX_CTYPE_I16:
        if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR)
            return r;

        *(int16_t*)cvi->v.p = n;
        break;
    case OX_CTYPE_I32:
        if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR)
            return r;

        *(int32_t*)cvi->v.p = n;
        break;
    case OX_CTYPE_I64: {
        int64_t i;

        if (ox_value_is_int64(ctxt, v)) {
            i = ox_value_get_int64(ctxt, v);
        } else {
            if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR)
                return r;
            i = n;
        }

        *(int64_t*)cvi->v.p = i;
        break;
    }
    case OX_CTYPE_F32:
        if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR)
            return r;

        *(float*)cvi->v.p = n;
        break;
    case OX_CTYPE_F64:
        if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR)
            return r;

        *(double*)cvi->v.p = n;
        break;
    case OX_CTYPE_STR:
        if (ox_value_is_null(ctxt, v))
            *(char**)cvi->v.p = NULL;
        else if (ox_value_is_string(ctxt, v))
            *(const char**)cvi->v.p = ox_string_get_char_star(ctxt, v);
        else
            return ox_throw_type_error(ctxt, OX_TEXT("the value is not a string"));
        break;
    case OX_CTYPE_ARRAY: {
        OX_Value *ty;
        OX_CValue *cv;
        OX_CPtrType *pt;
        OX_CArrayType *at = ox_value_get_gco(ctxt, cty);

        if (!ox_value_is_cvalue(ctxt, v))
            return ox_throw_type_error(ctxt, OX_TEXT("the value is not a C value"));

        ty = ox_cvalue_get_ctype(ctxt, v);
        if (ox_ctype_get_kind(ctxt, ty) != OX_CTYPE_PTR)
            return ox_throw_type_error(ctxt, OX_TEXT("the C value's type mismatch"));

        pt = ox_value_get_gco(ctxt, ty);
        if (!ctype_equal(ctxt, &at->ity, &pt->vty, OX_FALSE))
            return ox_throw_type_error(ctxt, OX_TEXT("the C value's type mismatch"));

        if (at->cty.size == -1)
            return ox_throw_type_error(ctxt, OX_TEXT("the array's size is unknown"));

        cv = ox_value_get_gco(ctxt, v);
        memcpy(cvi->v.p, cv->v.p, at->cty.size);
        break;
    }
    case OX_CTYPE_STRUCT: {
        OX_Value *ty;
        OX_CValue *cv;
        OX_CType *t = ox_value_get_gco(ctxt, cty);
        OX_CPtrType *pt;

        if (!ox_value_is_cvalue(ctxt, v))
            return ox_throw_type_error(ctxt, OX_TEXT("the value is not a C value"));

        ty = ox_cvalue_get_ctype(ctxt, v);
        if (ox_ctype_get_kind(ctxt, ty) != OX_CTYPE_PTR)
            return ox_throw_type_error(ctxt, OX_TEXT("the C value's type mismatch"));

        pt = ox_value_get_gco(ctxt, ty);
        if (!ctype_equal(ctxt, cty, &pt->vty, OX_FALSE))
            return ox_throw_type_error(ctxt, OX_TEXT("the C value's type mismatch"));

        if (t->size == -1)
            return ox_throw_type_error(ctxt, OX_TEXT("the structure's size is unknown"));

        cv = ox_value_get_gco(ctxt, v);
        memcpy(cvi->v.p, cv->v.p, t->size);
        break;
    }
    case OX_CTYPE_PTR: {
        if (ox_value_is_null(ctxt, v)) {
            *(void**)cvi->v.p = NULL;
        } else if (ox_value_is_string(ctxt, v)) {
            OX_CTypeKind kind;
            OX_CPtrType *pt = (OX_CPtrType*)t;

            kind = ox_ctype_get_kind(ctxt, &pt->vty);

            if ((kind != OX_CTYPE_I8) && (kind != OX_CTYPE_U8))
                return ox_throw_type_error(ctxt, OX_TEXT("the string cannot be used as the C pointer"));

            *(void**)cvi->v.p = (char*)ox_string_get_char_star(ctxt, v);
        } else {
            OX_Value *ty;
            OX_CValue *cv;

            if (!ox_value_is_cvalue(ctxt, v))
                return ox_throw_type_error(ctxt, OX_TEXT("the value is not a C value"));

            ty = ox_cvalue_get_ctype(ctxt, v);
            if (!ctype_equal(ctxt, cty, ty, OX_FALSE))
                return ox_throw_type_error(ctxt, OX_TEXT("the C value's type mismatch"));

            cv = ox_value_get_gco(ctxt, v);
            *(void**)cvi->v.p = cv->v.p;
        }
        break;
    }
    case OX_CTYPE_FUNC: {
        if (ox_value_is_null(ctxt, v)) {
            *(void**)cvi->v.p = NULL;
        } else if (ox_value_get_gco_type(ctxt, v) == OX_GCO_FUNCTION) {
            void *p;

            if ((r = ox_function_get_cptr(ctxt, v, cty, &p)) == OX_ERR)
                return r;
            
            *(void**)cvi->v.p = p;
        } else {
            OX_CFuncPtr *f;

            if (!(f = cfunc_ptr_get(ctxt, v)))
                return OX_ERR;

            if (!ctype_equal(ctxt, cty, &f->cty, OX_FALSE))
                return ox_throw_type_error(ctxt, OX_TEXT("the C value's type mismatch"));

            *(void**)cvi->v.p = f->p;
        }
        break;
    }
    case OX_CTYPE_VOID: {
        r = ox_throw_type_error(ctxt, OX_TEXT("cannot set value of void pointer"));
    }
    }

    return r;
}

/*Get the property of the C value.*/
static OX_Result
cvalue_get_ex (OX_Context *ctxt, OX_Value *o, OX_Value *k, OX_Value *v, OX_Bool lookup)
{
    OX_Result r;

    if (ox_value_is_number(ctxt, k)) {
        OX_CValue *cv;
        OX_Value *cty;
        OX_CTypeKind kind;
        size_t id;
        OX_Value *ity = NULL;
        size_t isize;
        OX_CValueInfo cvi;

        cty = ox_cvalue_get_ctype(ctxt, o);
        kind = ox_ctype_get_kind(ctxt, cty);
        if ((kind != OX_CTYPE_ARRAY) && (kind != OX_CTYPE_PTR)) {
            return ox_throw_type_error(ctxt, OX_TEXT("the value is not a C pointer"));
        }

        if ((r = ox_to_index(ctxt, k, &id)) == OX_ERR)
            return r;

        if (kind == OX_CTYPE_ARRAY) {
            OX_CArrayType *at = ox_value_get_gco(ctxt, cty);

            if ((at->len != -1) && (id >= at->len)) {
                ox_value_set_null(ctxt, v);
                return OX_OK;
            }

            ity = &at->ity;
        } else if (kind == OX_CTYPE_PTR) {
            OX_CPtrType *pt = ox_value_get_gco(ctxt, cty);

            if ((pt->len != -1) && (id >= pt->len)) {
                ox_value_set_null(ctxt, v);
                return OX_OK;
            }

            ity = &pt->vty;
        } else {
            assert(0);
        }

        isize = ox_ctype_get_size(ctxt, ity);
        if ((id != 0) && (isize == -1))
            return ox_throw_type_error(ctxt, OX_TEXT("item's size is unknown"));

        cv = ox_value_get_gco(ctxt, o);
        if (!cv->v.p)
            return ox_throw_null_error(ctxt, OX_TEXT("the C pointer is null"));

        if (isize == -1)
            cvi.v.p = cv->v.p;
        else
            cvi.v.p = ((uint8_t*)cv->v.p) + isize * id;
        cvi.base = v;
        cvi.own = OX_CPTR_NON_OWNER;

        r = ox_cptr_get_value(ctxt, ity, &cvi, v);
    } else if (ox_value_is_string(ctxt, k)) {
        if (ox_value_is_struct_cptr(ctxt, o)) {
            OX_Value *ty = ox_cvalue_get_ctype(ctxt, o);
            OX_CPtrType *pt = ox_value_get_gco(ctxt, ty);
            OX_CStructType *st = ox_value_get_gco(ctxt, &pt->vty);
            OX_CValue *cv;
            OX_String *s;
            OX_CField *f;

            if ((r = ox_string_singleton(ctxt, k)) == OX_ERR)
                return r;

            s = ox_value_get_gco(ctxt, k);
            f = ox_hash_lookup_c(ctxt, &st->f_hash, s, NULL, OX_CField, he);

            if (f) {
                uint8_t *p;
                OX_CValueInfo cvi;

                cv = ox_value_get_gco(ctxt, o);
                if (!cv->v.p)
                    return ox_throw_null_error(ctxt, OX_TEXT("the C pointer is null"));

                p = cv->v.p;
                p += f->offset;

                cvi.v.p = p;
                cvi.base = o;
                cvi.own = OX_CPTR_NON_OWNER;

                return ox_cptr_get_value(ctxt, &f->cty, &cvi, v);
            }
        }

        if (lookup)
            r = ox_object_lookup(ctxt, o, k, v);
        else
            r = ox_object_get(ctxt, o, k, v);
    } else {
        r = OX_FALSE;
    }

    return r;
}

/*Lookup the C value ownded property.*/
static OX_Result
cvalue_lookup (OX_Context *ctxt, OX_Value *o, OX_Value *k, OX_Value *v)
{
    return cvalue_get_ex(ctxt, o, k, v, OX_TRUE);
}

/*Get the property of the C value.*/
static OX_Result
cvalue_get (OX_Context *ctxt, OX_Value *o, OX_Value *k, OX_Value *v)
{
    return cvalue_get_ex(ctxt, o, k, v, OX_FALSE);
}

/*Set the property of the C value.*/
static OX_Result
cvalue_set (OX_Context *ctxt, OX_Value *o, OX_Value *k, OX_Value *v)
{
    OX_Result r;

    if (ox_value_is_number(ctxt, k)) {
        OX_CValue *cv;
        OX_Value *cty;
        OX_CTypeKind kind;
        size_t id;
        OX_Value *ity = NULL;
        size_t isize;
        OX_CValueInfo cvi;

        cty = ox_cvalue_get_ctype(ctxt, o);
        kind = ox_ctype_get_kind(ctxt, cty);
        if ((kind != OX_CTYPE_ARRAY) && (kind != OX_CTYPE_PTR))
            return ox_throw_type_error(ctxt, OX_TEXT("the value is not a C pointer"));

        if ((r = ox_to_index(ctxt, k, &id)) == OX_ERR)
            return r;

        if (kind == OX_CTYPE_ARRAY) {
            OX_CArrayType *at = ox_value_get_gco(ctxt, cty);

            if ((at->len != -1) && (id >= at->len))
                return ox_throw_range_error(ctxt, OX_TEXT("item's index overflow"));

            ity = &at->ity;
        } else if (kind == OX_CTYPE_PTR) {
            OX_CPtrType *pt = ox_value_get_gco(ctxt, cty);

            if ((pt->len != -1) && (id >= pt->len))
                return ox_throw_range_error(ctxt, OX_TEXT("item's index overflow"));

            ity = &pt->vty;
        } else {
            assert(0);
        }

        isize = ox_ctype_get_size(ctxt, ity);
        if ((id != 0) && (isize == -1))
            return ox_throw_type_error(ctxt, OX_TEXT("item's size is unknown"));

        cv = ox_value_get_gco(ctxt, o);
        if (!cv->v.p)
            return ox_throw_null_error(ctxt, OX_TEXT("the C pointer is null"));

        if (isize == -1)
            cvi.v.p = cv->v.p;
        else
            cvi.v.p = ((uint8_t*)cv->v.p) + isize * id;

        cvi.own = OX_CPTR_NON_OWNER;

        r = ox_cptr_set_value(ctxt, ity, &cvi, v);
    } else {
        OX_Object *op;

        if (ox_value_is_string(ctxt, k)) {
            if (ox_value_is_struct_cptr(ctxt, o)) {
                OX_Value *ty = ox_cvalue_get_ctype(ctxt, o);
                OX_CPtrType *pt = ox_value_get_gco(ctxt, ty);
                OX_CStructType *st = ox_value_get_gco(ctxt, &pt->vty);
                OX_CValue *cv;
                OX_String *s;
                OX_CField *f;

                if ((r = ox_string_singleton(ctxt, k)) == OX_ERR)
                    return r;

                s = ox_value_get_gco(ctxt, k);
                f = ox_hash_lookup_c(ctxt, &st->f_hash, s, NULL, OX_CField, he);
                if (f) {
                    uint8_t *p;
                    OX_CValueInfo cvi;

                    cv = ox_value_get_gco(ctxt, o);
                    if (!cv->v.p)
                        return ox_throw_null_error(ctxt, OX_TEXT("the C pointer is null"));

                    p = cv->v.p;
                    p += f->offset;

                    cvi.v.p = p;
                    cvi.base = o;
                    cvi.own = OX_CPTR_NON_OWNER;

                    return ox_cptr_set_value(ctxt, &f->cty, &cvi, v);
                }
            }
        }
        
        op = ox_value_get_gco(ctxt, o);
        r = ox_object_set_t(ctxt, &op->inf, k, v, o);
    }

    return r;
}

/*Operation functions of C value.*/
static const OX_ObjectOps
cvalue_ops = {
    {
        OX_GCO_CVALUE,
        cvalue_scan,
        cvalue_free
    },
    ox_object_keys,
    cvalue_lookup,
    cvalue_get,
    cvalue_set,
    ox_object_del,
    ox_object_call
};

/*Operation functions of C value with owned pointer in it.*/
static const OX_ObjectOps
ptr_cvalue_ops = {
    {
        OX_GCO_CVALUE,
        cvalue_scan,
        ptr_cvalue_free
    },
    ox_object_keys,
    cvalue_lookup,
    cvalue_get,
    cvalue_set,
    ox_object_del,
    ox_object_call
};

/*Operation functions of C value with owned external pointer in it.*/
static const OX_ObjectOps
ext_ptr_cvalue_ops = {
    {
        OX_GCO_CVALUE,
        cvalue_scan,
        ext_ptr_cvalue_free
    },
    ox_object_keys,
    cvalue_lookup,
    cvalue_get,
    cvalue_set,
    ox_object_del,
    ox_object_call
};

/*Scan referenced objects in the C function pointer.*/
static void
cfunc_ptr_scan (OX_Context *ctxt, void *p)
{
    OX_CFuncPtr *f = p;

    ox_gc_scan_value(ctxt, &f->v);
    ox_gc_scan_value(ctxt, &f->cty);
}

/*Free the C function pointer.*/
static void
cfunc_ptr_free (OX_Context *ctxt, void *p)
{
    OX_CFuncPtr *f = p;

    if (f->closure)
        ffi_closure_free(f->closure);

    OX_DEL(ctxt, f);
}

/*Operation functions of the C function pointer.*/
static const OX_PrivateOps
cfunc_ptr_ops = {
    cfunc_ptr_scan,
    cfunc_ptr_free
};

/*Get the C function pointer from the value.*/
static OX_CFuncPtr*
cfunc_ptr_get (OX_Context *ctxt, OX_Value *v)
{
    OX_CFuncPtr *f = ox_object_get_priv(ctxt, v, &cfunc_ptr_ops);

    if (!f)
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a C function"));

    return f;
}

/**
 * Get the basic C type value.
 * @param ctxt The current running context.
 * @param kind The kind of the C type.
 * @return The C type value.
 */
OX_Value*
ox_ctype_get (OX_Context *ctxt, OX_CTypeKind kind)
{
    switch (kind) {
    case OX_CTYPE_U8:
        return OX_OBJECT(ctxt, UInt8);
    case OX_CTYPE_U16:
        return OX_OBJECT(ctxt, UInt16);
    case OX_CTYPE_U32:
        return OX_OBJECT(ctxt, UInt32);
    case OX_CTYPE_U64:
        return OX_OBJECT(ctxt, UInt64);
    case OX_CTYPE_I8:
        return OX_OBJECT(ctxt, Int8);
    case OX_CTYPE_I16:
        return OX_OBJECT(ctxt, Int16);
    case OX_CTYPE_I32:
        return OX_OBJECT(ctxt, Int32);
    case OX_CTYPE_I64:
        return OX_OBJECT(ctxt, Int64);
    case OX_CTYPE_F32:
        return OX_OBJECT(ctxt, Float32);
    case OX_CTYPE_F64:
        return OX_OBJECT(ctxt, Float64);
    case OX_CTYPE_VOID:
        return OX_OBJECT(ctxt, Void);
    default:
        assert(0);
    }
}

/**
 * Create a new C pointer type.
 * @param ctxt The current running context.
 * @param[out] pty Return the new pointer type.
 * @param vty The value's C type.
 * @param len Items' number in the buffer.
 * -1 means not know the length of the buffer.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_ctype_pointer (OX_Context *ctxt, OX_Value *pty, OX_Value *vty, ssize_t len)
{
    OX_CPtrType *p;
    OX_CType *vt;

    assert(ctxt && pty && vty);
    assert(ox_value_is_ctype(ctxt, vty));

    vt = ox_value_get_gco(ctxt, vty);

    if (len == -1) {
        if (!ox_value_is_null(ctxt, &vt->pty)) {
            ox_value_copy(ctxt, pty, &vt->pty);
            return OX_OK;
        }
    }

    if (!OX_NEW(ctxt, p))
        return ox_throw_no_mem_error(ctxt);

    ctype_init(ctxt, &p->cty, OX_CTYPE_PTR, sizeof(void*));

    ox_value_copy(ctxt, &p->vty, vty);
    p->len = len;

    ox_value_set_gco(ctxt, pty, p);
    ox_gc_add(ctxt, p);

    if (len == -1)
        ox_value_copy(ctxt, &vt->pty, pty);

    return OX_OK;
}

/**
 * Create a new C array type.
 * @param ctxt The current running context.
 * @param[out] aty Return the new array type.
 * @param ity The item's C type.
 * @param len Items' number in the array.
 * -1 means not know the length of the array.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_ctype_array (OX_Context *ctxt, OX_Value *aty, OX_Value *ity, ssize_t len)
{
    OX_CType *it;
    OX_CArrayType *at;
    ssize_t size;

    assert(ctxt && aty && ity);
    assert(ox_value_is_ctype(ctxt, ity));

    it = ox_value_get_gco(ctxt, ity);
    if (it->size == -1)
        return ox_throw_type_error(ctxt, OX_TEXT("item's size is unknown"));

    if (len == -1) {
        if (!ox_value_is_null(ctxt, &it->aty)) {
            ox_value_copy(ctxt, aty, &it->aty);
            return OX_OK;
        }
    }

    if (!OX_NEW(ctxt, at))
        return ox_throw_no_mem_error(ctxt);

    size = (len == -1) ? -1 : len * it->size;

    ctype_init(ctxt, &at->cty, OX_CTYPE_ARRAY, size);

    ox_value_copy(ctxt, &at->ity, ity);
    at->len = len;

    ox_value_set_gco(ctxt, aty, at);
    ox_gc_add(ctxt, at);

    if (len == -1)
        ox_value_copy(ctxt, &it->aty, aty);

    return OX_OK;
}

/*Convert the C type to FFI type.*/
static ffi_type*
ctype_to_ffi (OX_Context *ctxt, OX_Value *cty)
{
    OX_CType *t = ox_value_get_gco(ctxt, cty);

    switch (t->kind) {
    case OX_CTYPE_I8:
        return &ffi_type_sint8;
    case OX_CTYPE_I16:
        return &ffi_type_sint16;
    case OX_CTYPE_I32:
        return &ffi_type_sint32;
    case OX_CTYPE_I64:
        return &ffi_type_sint64;
    case OX_CTYPE_U8:
        return &ffi_type_uint8;
    case OX_CTYPE_U16:
        return &ffi_type_uint16;
    case OX_CTYPE_U32:
        return &ffi_type_uint32;
    case OX_CTYPE_U64:
        return &ffi_type_uint64;
    case OX_CTYPE_F32:
        return &ffi_type_float;
    case OX_CTYPE_F64:
        return &ffi_type_double;
    default:
        return &ffi_type_pointer;
    }
}

/**
 * Create a new C function type.
 * @param ctxt The current running context.
 * @param[out] fty Return the new function type.
 * @param rty The return value's C type.
 * NULL means the return type is void.
 * @param atys Arguments' C types.
 * @param argc Arguments' count.
 * @param vaarg The function has variable arguments.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_ctype_func (OX_Context *ctxt, OX_Value *fty, OX_Value *rty, OX_Value *atys, size_t argc, OX_Bool vaarg)
{
    OX_CFuncType *ft;
    ffi_type *rtype;
    ffi_status status;

    assert(ctxt && fty);

    /*Allocate the function type.*/
    if (!OX_NEW(ctxt, ft))
        return ox_throw_no_mem_error(ctxt);

    ctype_init(ctxt, &ft->cty, OX_CTYPE_FUNC, sizeof(void*));

    ox_value_set_null(ctxt, &ft->rty);
    ft->atys = NULL;
    ft->atypes = NULL;
    ft->argc = 0;
    ft->vaarg = vaarg;

    if (rty && !ox_value_is_null(ctxt, rty)) {
        ox_value_copy(ctxt, &ft->rty, rty);
    } else {
        ox_value_set_null(ctxt, &ft->rty);
    }

    if (argc) {
        if (!OX_NEW_N(ctxt, ft->atys, argc)) {
            ctype_free(ctxt, &ft->cty.o.gco);
            return ox_throw_no_mem_error(ctxt);
        }

        ft->argc = argc;
        ox_values_copy(ctxt, ft->atys, atys, argc);
    }

    /*Prepare FFI CIF.*/
    if (!ft->vaarg) {
        if (rty && !ox_value_is_null(ctxt, rty)) {
            assert(ox_value_is_ctype(ctxt, rty));

            rtype = ctype_to_ffi(ctxt, rty);
        } else {
            rtype = &ffi_type_void;
        }

        if (argc) {
            size_t i;

            if (!OX_NEW_N(ctxt, ft->atypes, argc)) {
                ctype_free(ctxt, &ft->cty.o.gco);
                return ox_throw_no_mem_error(ctxt);
            }

            for (i = 0; i < argc; i ++) {
                OX_Value *aty = ox_values_item(ctxt, atys, i);

                ft->atypes[i] = ctype_to_ffi(ctxt, aty);
            }
        }

        status = ffi_prep_cif(&ft->cif, FFI_DEFAULT_ABI, argc, rtype, ft->atypes);
        if (status != FFI_OK) {
            ctype_free(ctxt, &ft->cty.o.gco);
            return ox_throw_system_error(ctxt, OX_TEXT("\"%s\" failed: %d"),
                    "ffi_prep_cif", status);
        }
    }

    ox_value_set_gco(ctxt, fty, ft);
    ox_gc_add(ctxt, ft);

    return OX_OK;
}

/**
 * Create a new C structure type.
 * @param ctxt The current running context.
 * @param[out] sty Return the new structure type.
 * @param[out] inf Return the interface of the structure.
 * @param size Size of the structure in bytes.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_ctype_struct (OX_Context *ctxt, OX_Value *sty, OX_Value *inf, ssize_t size)
{
    OX_CStructType *st;
    OX_Result r;

    assert(ctxt && sty);

    if (!OX_NEW(ctxt, st))
        return ox_throw_no_mem_error(ctxt);

    ctype_init(ctxt, &st->cty, OX_CTYPE_STRUCT, size);

    ox_list_init(&st->f_list);
    ox_size_hash_init(&st->f_hash);

    ox_value_set_gco(ctxt, sty, st);
    ox_gc_add(ctxt, st);

    if (inf) {
        if ((r = ox_interface_new(ctxt, inf)) == OX_ERR)
            return r;

        if ((r = ox_set(ctxt, sty, OX_STRING(ctxt, _inf), inf)) == OX_ERR)
            return r;
    }

    return OX_OK;
}

/**
 * Create a new C structure type with its scope and name.
 * @param ctxt The current running context.
 * @param[out] sty Return the new structure type.
 * @param[out] inf Return the interface of the structure.
 * @param size Size of the structure in bytes.
 * @param scope The scope of the structure.
 * @param name The name of the structure.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_named_ctype_struct (OX_Context *ctxt, OX_Value *sty, OX_Value *inf, size_t size,
        OX_Value *scope, OX_Value *name)
{
    OX_Result r;

    if ((r = ox_ctype_struct(ctxt, sty, inf, size)) == OX_ERR)
        return r;

    if (scope) {
        if ((r = ox_object_set_scope(ctxt, sty, scope)) == OX_ERR)
            return r;
    }

    if (name) {
        if ((r = ox_object_set_name(ctxt, sty, name)) == OX_ERR)
            return r;
    }

    return OX_OK;
}

/**
 * Create a new C structure type with its scope and name.
 * name is a 0 terminated characters string.
 * @param ctxt The current running context.
 * @param[out] sty Return the new structure type.
 * @param size Size of the structure in bytes.
 * @param scope The scope of the structure.
 * @param name The name of the structure.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_named_ctype_struct_s (OX_Context *ctxt, OX_Value *sty, OX_Value *inf, size_t size,
        OX_Value *scope, const char *name)
{
    OX_VS_PUSH(ctxt, nv)
    OX_Result r;

    assert(name);

    r = ox_string_from_const_char_star(ctxt, nv, name);
    if (r == OX_OK)
        r = ox_named_ctype_struct(ctxt, sty, inf, size, scope, nv);

    OX_VS_POP(ctxt, nv)
    return r;
}

/**
 * Add a field to the C strucutre.
 * @param ctxt The current running context.
 * @param sty The structure type.
 * @param cty The type of the field.
 * @param name The name of the field.
 * @param off The offset of the field.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_cstruct_add_field (OX_Context *ctxt, OX_Value *sty, OX_Value *cty, OX_Value *name, size_t off)
{
    OX_CStructType *st;
    OX_HashEntry **pe;
    OX_CField *f;
    OX_String *s;
    OX_Result r;

    assert(ctxt && sty && cty && name);
    assert(ox_value_is_ctype(ctxt, sty) && ox_value_is_ctype(ctxt, cty));
    assert(ox_ctype_get_kind(ctxt, sty) == OX_CTYPE_STRUCT);

    st = ox_value_get_gco(ctxt, sty);

    if ((r = ox_string_singleton(ctxt, name)) == OX_ERR)
        return r;

    s = ox_value_get_gco(ctxt, name);
    f = ox_hash_lookup_c(ctxt, &st->f_hash, s, &pe, OX_CField, he);
    if (f)
        return ox_throw_reference_error(ctxt, OX_TEXT("field \"%s\" is already defined"),
            ox_string_get_char_star(ctxt, name));

    if (!OX_NEW(ctxt, f))
        return ox_throw_no_mem_error(ctxt);

    f->offset = off;
    ox_value_copy(ctxt, &f->cty, cty);

    if ((r = ox_hash_insert(ctxt, &st->f_hash, s, &f->he, pe)) == OX_ERR) {
        OX_DEL(ctxt, f);
        return r;
    }

    ox_list_append(&st->f_list, &f->ln);
    return OX_OK;
}

/**
 * Add a field to the C strucutre.
 * name is a 0 terminated characters string.
 * @param ctxt The current running context.
 * @param sty The structure type.
 * @param cty The type of the field.
 * @param name The name of the field.
 * @param off The offset of the field.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_cstruct_add_field_s (OX_Context *ctxt, OX_Value *sty, OX_Value *cty, const char *name, size_t off)
{
    OX_VS_PUSH(ctxt, nv)
    OX_Result r;

    r = ox_string_from_const_char_star(ctxt, nv, name);
    if (r == OX_OK)
        r = ox_cstruct_add_field(ctxt, sty, cty, nv, off);

    OX_VS_POP(ctxt, nv)
    return r;
}

/*Call the C function pointer.*/
static OX_Result
cfunc_ptr_call (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_CFuncPtr *cf = ox_object_get_priv(ctxt, f, &cfunc_ptr_ops);
    OX_CFuncType *ft;
    OX_Result r;

    if (!cf)
        return ox_throw_type_error(ctxt, OX_TEXT("the value is not a C function"));

    ft = ox_value_get_gco(ctxt, &cf->cty);

    /*{
        OX_Value *s = ox_value_stack_push(ctxt);

        ox_get_full_name(ctxt, f, s);

        OX_LOG_D(ctxt, "call %s", ox_string_get_char_star(ctxt, s));
    }*/

    {
        size_t total_argc = OX_MAX(argc, ft->argc);
        OX_CVBuf rbuf;
        OX_CVBuf abuf[total_argc];
        void *rvalue;
        void *avalue[total_argc];
        ffi_type *atypes[total_argc];
        size_t i;
        OX_CValueInfo cvi;

        if (ox_value_is_null(ctxt, &ft->rty))
            rvalue = NULL;
        else
            rvalue = &rbuf;

        cvi.base = NULL;
        cvi.own = OX_CPTR_NON_OWNER;

        for (i = 0; i < ft->argc; i ++) {
            OX_Value *arg = ox_argument(ctxt, args, argc, i);

            avalue[i] = &abuf[i];
            cvi.v.p = avalue[i];

            if ((r = ox_cptr_set_value(ctxt, &ft->atys[i], &cvi, arg)) == OX_ERR)
                return r;
        }

        if (ft->vaarg) {
            ffi_type *rtype;
            ffi_status status;

            if (!ox_value_is_null(ctxt, &ft->rty)) {
                rtype = ctype_to_ffi(ctxt, &ft->rty);
            } else {
                rtype = &ffi_type_void;
            }
    
            for (i = 0; i < ft->argc; i ++) {
                atypes[i] = ctype_to_ffi(ctxt, &ft->atys[i]);
            }

            while (i < total_argc) {
                OX_Value *arg = ox_argument(ctxt, args, argc, i);

                avalue[i] = &abuf[i];
                cvi.v.p = avalue[i];

                if (ox_value_is_cvalue(ctxt, arg)) {
                    OX_Value *ty = ox_cvalue_get_ctype(ctxt, arg);

                    atypes[i] = ctype_to_ffi(ctxt, ty);

                    if ((r = ox_cptr_set_value(ctxt, ty, &cvi, arg)) == OX_ERR)
                        return r;
                } else if (ox_value_is_number(ctxt, arg)) {
                    atypes[i] = &ffi_type_sint32;
                    abuf[i].i32 = ox_value_get_number(ctxt, arg);
                } else if (ox_value_is_string(ctxt, arg)) {
                    atypes[i] = &ffi_type_pointer;
                    abuf[i].p = (char*)ox_string_get_char_star(ctxt, arg);
                } else if (ox_value_is_bool(ctxt, arg)) {
                    atypes[i] = &ffi_type_sint32;
                    abuf[i].i32 = ox_value_get_bool(ctxt, arg);
                } else if (ox_value_is_null(ctxt, arg)) {
                    atypes[i] = &ffi_type_pointer;
                    abuf[i].p = NULL;
                } else if (ox_value_is_function(ctxt, arg)) {
                    OX_CFuncPtr *fp = ox_object_get_priv(ctxt, arg, &cfunc_ptr_ops);

                    if (fp) {
                        atypes[i] = &ffi_type_pointer;
                        abuf[i].p = fp->p;
                    } else {
                        return ox_throw_type_error(ctxt, OX_TEXT("the function cannot be converted to C function"));
                    }
                } else {
                    return ox_throw_type_error(ctxt, OX_TEXT("the value cannot be converted to C value"));
                }

                i ++;
            }

            status = ffi_prep_cif_var(&ft->cif, FFI_DEFAULT_ABI, ft->argc, total_argc, rtype, atypes);
            if (status != FFI_OK)
                return ox_throw_system_error(ctxt, OX_TEXT("\"%s\" failed: %d"),
                        "ffi_prep_cif_var", status);
        }

        OX_SCHED(ctxt, ffi_call(&ft->cif, cf->p, rvalue, avalue));

        if (rvalue) {
            cvi.v.p = rvalue;
            cvi.own = OX_CPTR_NON_OWNER;
            cvi.base = NULL;

            if ((r = ox_cptr_get_value(ctxt, &ft->rty, &cvi, rv)) == OX_ERR)
                return r;
        }
    }

    return OX_OK;
}

/*Create a new C function pointer.*/
static OX_Result
cfunc_ptr_new (OX_Context *ctxt, OX_Value *cv, OX_Value *cty, void *p)
{
    OX_Result r;
    OX_CFuncPtr *f;

    if ((r = ox_native_func_new(ctxt, cv, cfunc_ptr_call)) == OX_ERR)
        return r;

    if (!OX_NEW(ctxt, f))
        return ox_throw_no_mem_error(ctxt);

    ox_value_copy(ctxt, &f->cty, cty);
    ox_value_copy(ctxt, &f->v, cv);

    f->vm = NULL;
    f->closure = NULL;
    f->p = p;

    if ((r = ox_object_set_priv(ctxt, cv, &cfunc_ptr_ops, f)) == OX_ERR) {
        cfunc_ptr_free(ctxt, f);
        return r;
    }

    return OX_OK;
}

/*Create a new C value.*/
static OX_Result
cvalue_new (OX_Context *ctxt, OX_Value *cv, OX_Value *cty, OX_CValueInfo *cvi)
{
    OX_VS_PUSH(ctxt, inf)
    OX_CValue *v;
    OX_CType *t;
    const OX_ObjectOps *ops;
    ssize_t size = -1;
    OX_Result r;

    if (!OX_NEW(ctxt, v)) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    t = ox_value_get_gco(ctxt, cty);

    ops = &cvalue_ops;

    switch (t->kind) {
    case OX_CTYPE_PTR: {
        OX_CPtrType *pt = ox_value_get_gco(ctxt, cty);

        if (ox_ctype_get_kind(ctxt, &pt->vty) == OX_CTYPE_STRUCT) {
            if ((r = ox_get(ctxt, &pt->vty, OX_STRING(ctxt, _inf), inf)) == OX_ERR)
                goto end;
        } else {
            ox_value_copy(ctxt, inf, OX_OBJECT(ctxt, CPointer_inf));
        }

        if (!cvi->base) {
            if (cvi->own & OX_CPTR_INTERNAL) {
                ops = &ptr_cvalue_ops;
                size = cvi->size;
            } else if (cvi->own & OX_CPTR_EXTERNAL) {
                ops = &ext_ptr_cvalue_ops;
            }
        }
        break;
    }
    default:
        if (t->kind & OX_CTYPE_FL_NUM)
            ox_value_copy(ctxt, inf, OX_OBJECT(ctxt, CNumber_inf));
        break;
    }

    ox_object_init(ctxt, &v->o, inf);

    v->o.gco.ops = (OX_GcObjectOps*)ops;
    ox_value_copy(ctxt, &v->cty, cty);

    v->v = cvi->v;

    if ((t->kind & OX_CTYPE_FL_PTR) && cvi->base)
        ox_value_copy(ctxt, &v->base, cvi->base);
    else if (size != -1)
        ox_value_set_number(ctxt, &v->base, size);
    else
        ox_value_set_null(ctxt, &v->base);

    ox_value_set_gco(ctxt, cv, v);
    ox_gc_add(ctxt, v);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, inf)
    return r;
}

/**
 * Create a new C value.
 * @param ctxt The current running context.
 * @param cv Return the new C value.
 * @param cty The C type of the value.
 * @param cvi The value's information.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_cvalue_new (OX_Context *ctxt, OX_Value *cv, OX_Value *cty, OX_CValueInfo *cvi)
{
    OX_CTypeKind kind;
    OX_CType *t;
    OX_Result r;

    assert(ctxt && cv && cty && cvi);
    assert(ox_value_is_ctype(ctxt, cty));

    kind = ox_ctype_get_kind(ctxt, cty);

    assert((kind != OX_CTYPE_STRUCT) && (kind != OX_CTYPE_ARRAY));

    t = ox_value_get_gco(ctxt, cty);

    if (kind & OX_CTYPE_FL_PTR) {
        if (!cvi->v.p && !(cvi->own & OX_CPTR_ALLOC)) {
            ox_value_set_null(ctxt, cv);
            return OX_OK;
        }
    }

    if (t->kind == OX_CTYPE_FUNC) {
        r = cfunc_ptr_new(ctxt, cv, cty, cvi->v.p);
    } else {
        r = cvalue_new(ctxt, cv, cty, cvi);
    }

    return r;
}

/**
 * Get the C type of the C value.
 * @param ctxt The current running context.
 * @param cv The C value.
 * @return The C type's pointer.
 */
OX_Value*
ox_cvalue_get_ctype (OX_Context *ctxt, OX_Value *cv)
{
    OX_Value *ct;

    assert(ctxt && cv);

    if (ox_value_is_cvalue(ctxt, cv)) {
        OX_CValue *v;

        v = ox_value_get_gco(ctxt, cv);

        ct = &v->cty;
    } else if (ox_value_is_function(ctxt, cv)) {
        OX_CFuncPtr *f = ox_object_get_priv(ctxt, cv, &cfunc_ptr_ops);

        assert(f);

        ct = &f->cty;
    } else {
        assert(0);
    }

    return ct;
}

/**
 * Get the C value's pointer.
 * @param ctxt The current running context.
 * @param cv The C value.
 * @return The pointer of the C value.
 */
void*
ox_cvalue_get_pointer (OX_Context *ctxt, OX_Value *cv)
{
    void *p;

    if (ox_value_is_cvalue(ctxt, cv)) {
        OX_CValue *v;
        OX_Value *cty;
        OX_CTypeKind kind;

        v = ox_value_get_gco(ctxt, cv);
        cty = ox_cvalue_get_ctype(ctxt, cv);
        kind = ox_ctype_get_kind(ctxt, cty);

        if (kind & OX_CTYPE_FL_PTR)
            p = v->v.p;
        else
            p = &v->v;
    } else if (ox_value_is_function(ctxt, cv)) {
        OX_CFuncPtr *f = ox_object_get_priv(ctxt, cv, &cfunc_ptr_ops);

        assert(f);

        p = f->p;
    } else {
        assert(0);
    }

    return p;
}

/**
 * Set the C pointer value to NULL.
 * @param ctxt The current running context.
 * @param cv The C value.
 */
void
ox_cvalue_set_null (OX_Context *ctxt, OX_Value *cv)
{
    OX_CValue *p;

    assert(ctxt && cv);
    assert(ox_value_is_cptr(ctxt, cv, NULL));

    p = ox_value_get_gco(ctxt, cv);

    p->o.gco.ops = (OX_GcObjectOps*)&cvalue_ops;
    p->v.p = NULL;
}

/**
 * Convert the C array to json.
 * @param ctxt The current running context.
 * @param cv The C array value.
 * @param[out] a Return the OX array object.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_carray_to_array (OX_Context *ctxt, OX_Value *cv, OX_Value *a)
{
    OX_VS_PUSH_2(ctxt, ik, iv)
    ssize_t len;
    size_t i;
    OX_Result r;

    assert(ctxt && cv && a);

    len = ox_carray_length(ctxt, cv);
    if (len == -1)
        len = 0;

    if ((r = ox_array_new(ctxt, a, len)) == OX_ERR)
        goto end;

    for (i = 0; i < len; i ++) {
        ox_value_set_number(ctxt, ik, i);

        if ((r = ox_get_throw(ctxt, cv, ik, iv)) == OX_ERR)
            goto end;

        if ((r = ox_array_set_item(ctxt, a, i, iv)) == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, ik)
    return r;
}

/*Call the OX function's C function wrapper.*/
static void
cfunc_call (ffi_cif *cif, void *rvalue, void **avalue, void *userdata)
{
    OX_CFuncPtr *fp = userdata;
    OX_Context *ctxt = ox_context_get(fp->vm);
    OX_CFuncType *ft;
    OX_Value *argv = NULL;
    OX_Value *rv;
    OX_Result r;
    OX_CValueInfo cvi;

    ox_lock(ctxt);

    cvi.base = NULL;
    cvi.own = OX_CPTR_NON_OWNER;

    ft = ox_value_get_gco(ctxt, &fp->cty);
    rv = ox_value_stack_push_n(ctxt, ft->argc + 1);
    if (ft->argc) {
        size_t i;

        argv = ox_values_item(ctxt, rv, 1);

        for (i = 0; i < ft->argc; i ++) {
            OX_Value *arg = ox_values_item(ctxt, argv, i);

            cvi.v.p = avalue[i];

            if ((r = ox_cptr_get_value(ctxt, &ft->atys[i], &cvi, arg)) == OX_ERR)
                goto end;
        }
    }

    r = ox_call(ctxt, &fp->v, ox_value_null(ctxt), argv, ft->argc, rv);
    if ((r == OX_OK) && !ox_value_is_null(ctxt, &ft->rty)) {
        cvi.v.p = rvalue;

        if ((r = ox_cptr_set_value(ctxt, &ft->rty, &cvi, rv)) == OX_ERR)
            goto end;
    }
end:
    ox_value_stack_pop(ctxt, rv);

    ox_unlock(ctxt);
}

/**
 * Get the C function pointer from the function value.
 * @param ctxt The current running context.
 * @param f The function value.
 * @param cty The C function's type.
 * @param[out] p Return the C function pointer.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_function_get_cptr (OX_Context *ctxt, OX_Value *f, OX_Value *cty, void **p)
{
    OX_CFuncPtr *fp;
    OX_CFuncType *ft;
    OX_Result r;
    ffi_status status;

    assert(ctxt && f && cty && p);
    assert(ox_value_is_function(ctxt, f));
    assert(ox_value_is_ctype(ctxt, cty));

    fp = ox_object_get_priv(ctxt, f, &cfunc_ptr_ops);
    if (fp) {
        *p = fp->p;
        return OX_OK;
    }

    ft = ox_value_get_gco(ctxt, cty);
    assert(ft->cty.kind == OX_CTYPE_FUNC);

    if (!OX_NEW(ctxt, fp))
        return ox_throw_no_mem_error(ctxt);

    ox_value_copy(ctxt, &fp->cty, cty);
    ox_value_copy(ctxt, &fp->v, f);
    fp->vm = ox_vm_get(ctxt);
    fp->p = NULL;
    fp->closure = ffi_closure_alloc(sizeof(ffi_closure), &fp->p);
    if (!fp->closure) {
        cfunc_ptr_free(ctxt, fp);
        return ox_throw_no_mem_error(ctxt);
    }

    status = ffi_prep_closure_loc(fp->closure, &ft->cif, cfunc_call, fp, NULL);
    if (status != FFI_OK) {
        cfunc_ptr_free(ctxt, fp);
        return ox_throw_syntax_error(ctxt, OX_TEXT("\"%s\" failed: %d"),
                "ffi_prep_closure", status);
    }

    if ((r = ox_object_set_priv(ctxt, f, &cfunc_ptr_ops, fp)) == OX_ERR) {
        cfunc_ptr_free(ctxt, fp);
        return r;
    }

    *p = fp->p;
    return OX_OK;
}

/**
 * Create a new C function object.
 * @param ctxt The current running context.
 * @param[out] v Return the new C function object.
 * @param fty The function's type.
 * @param p The function's pointer.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_cfunc_new (OX_Context *ctxt, OX_Value *v, OX_Value *fty, void *p)
{
    OX_CValueInfo cvi;

    assert(ctxt && v && fty && p);

    cvi.v.p = p;
    cvi.base = NULL;
    cvi.own = OX_CPTR_NON_OWNER;

    return ox_cvalue_new(ctxt, v, fty, &cvi);
}

/**
 * Create a new C function object with its scope and name.
 * @param ctxt The current running context.
 * @param[out] v Return the new C function object.
 * @param fty The function's type.
 * @param p The function's pointer.
 * @param scope The scope of the function.
 * @param name The name of the function.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_named_cfunc_new (OX_Context *ctxt, OX_Value *v, OX_Value *fty, void *p,
        OX_Value *scope, OX_Value *name)
{
    OX_Result r;

    if ((r = ox_cfunc_new(ctxt, v, fty, p)) == OX_ERR)
        return r;

    if (scope) {
        if ((r = ox_object_set_scope(ctxt, v, scope)) == OX_ERR)
            return r;
    }

    if (name) {
        if ((r = ox_object_set_name(ctxt, v, name)) == OX_ERR)
            return r;
    }

    return OX_OK;
}

/**
 * Create a new C function object with its scope and name.
 * name is a 0 terminated string.
 * @param ctxt The current running context.
 * @param[out] v Return the new C function object.
 * @param fty The function's type.
 * @param p The function's pointer.
 * @param scope The scope of the function.
 * @param name The name of the function.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_named_cfunc_new_s (OX_Context *ctxt, OX_Value *v, OX_Value *fty, void *p,
        OX_Value *scope, const char *name)
{
    OX_VS_PUSH(ctxt, nv)
    OX_Result r;

    r = ox_string_from_const_char_star(ctxt, nv, name);
    if (r == OX_OK)
        r = ox_named_cfunc_new(ctxt, v, fty, p, scope, nv);

    OX_VS_POP(ctxt, nv)
    return r;
}

/**
 * Check if 2 C types are equal.
 * @param ctxt The current running context.
 * @param t1 C type 1.
 * @param t2 C type 2.
 * @return 2 types are equal or not.
 */
OX_Bool
ox_ctype_equal (OX_Context *ctxt, OX_Value *t1, OX_Value *t2)
{
    return ctype_equal(ctxt, t1, t2, OX_TRUE);
}

/*Initialize a ctype class.*/
static void
ctype_class_init (OX_Context *ctxt, OX_Value *v, OX_CTypeKind kind, const char *name, ssize_t size)
{
    OX_VS_PUSH_2(ctxt, sv, nv)

    ox_not_error(ctype_new(ctxt, v, kind, size));
    ox_not_error(ox_string_from_const_char_star(ctxt, nv, name));
    ox_not_error(ox_object_add_const_s(ctxt, v, "$name", nv));
    ox_not_error(ox_object_add_const_s(ctxt, OX_OBJECT(ctxt, Global), name, v));

    OX_VS_POP(ctxt, sv)
}

/*Check if the value is a C type.*/
static OX_Result
value_is_ctype (OX_Context *ctxt, OX_Value *v)
{
    if (!ox_value_is_ctype(ctxt, v))
        return ox_throw_type_error(ctxt, OX_TEXT("the value is not a C type"));

    return OX_OK;
}

/*CType.$inf.pointer get.*/
static OX_Result
CType_inf_pointer_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    size_t idx;
    ssize_t len = -1;
    OX_Result r;

    if ((r = value_is_ctype(ctxt, thiz)) == OX_ERR)
        return r;

    if (!ox_value_is_null(ctxt, arg)) {
        if ((r = ox_to_index(ctxt, arg, &idx)) == OX_ERR)
            return r;

        len = idx;
    }

    return ox_ctype_pointer(ctxt, rv, thiz, len);
}

/*CType.$inf.size get.*/
static OX_Result
CType_inf_size_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_CType *cty;
    OX_Result r;

    if ((r = value_is_ctype(ctxt, thiz)) == OX_ERR)
        return r;

    cty = ox_value_get_gco(ctxt, thiz);
    ox_value_set_number(ctxt, rv, cty->size);

    return OX_OK;
}

/*CType.$inf.length get.*/
static OX_Result
CType_inf_length_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_CType *cty;
    ssize_t len;
    OX_Result r;

    if ((r = value_is_ctype(ctxt, thiz)) == OX_ERR)
        return r;

    cty = ox_value_get_gco(ctxt, thiz);
    if (cty->kind == OX_CTYPE_ARRAY) {
        OX_CArrayType *at = (OX_CArrayType*)cty;

        len = at->len;
    } else {
        len = 1;
    }

    ox_value_set_number(ctxt, rv, len);
    return OX_OK;
}

/*Get the C value from the value.*/
static OX_CValue*
value_get_cvalue (OX_Context *ctxt, OX_Value *v)
{
    if (!ox_value_is_cvalue(ctxt, v)) {
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a C value"));
        return NULL;
    }

    return ox_value_get_gco(ctxt, v);
}

/*Get C number from the value.*/
static OX_CValue*
value_get_cnumber (OX_Context *ctxt, OX_Value *v)
{
    OX_CValue *cv;
    OX_CType *t;

    if (!(cv = value_get_cvalue(ctxt, v)))
        return NULL;

    t = ox_value_get_gco(ctxt, &cv->cty);
    if (!(t->kind & OX_CTYPE_FL_NUM)) {
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a C number"));
        return NULL;
    }

    return cv;
}

/*CNumber.$inf.$to_num.*/
static OX_Result
CNumber_inf_to_num (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_CValue *cv;
    OX_CTypeKind kind;
    OX_Result r;

    if (!(cv = value_get_cnumber(ctxt, thiz)))
        return OX_ERR;

    if ((kind = ox_value_is_int64(ctxt, thiz))) {
        OX_Number n;

        if (kind == OX_CTYPE_I64) {
            n = ox_value_get_int64(ctxt, thiz);
        } else {
            n = ox_value_get_uint64(ctxt, thiz);
        }

        ox_value_set_number(ctxt, rv, n);
        r = OX_OK;
    } else {
        OX_CValueInfo cvi;

        cvi.v.p = &cv->v;
        cvi.base = NULL;
        cvi.own = OX_CPTR_NON_OWNER;

        r = ox_cptr_get_value(ctxt, &cv->cty, &cvi, rv);
    }

    return r;
}

/*CNumber.$inf.$to_str.*/
static OX_Result
CNumber_inf_to_str (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_CValue *cv;
    OX_CTypeKind kind;
    OX_Result r;

    if (!(cv = value_get_cnumber(ctxt, thiz)))
        return OX_ERR;

    if ((kind = ox_value_is_int64(ctxt, thiz))) {
        char buf[64];

        if (kind == OX_CTYPE_I64) {
            int64_t i;

            i = ox_value_get_int64(ctxt, thiz);

            snprintf(buf, sizeof(buf), "%lld", (long long)i);
        } else {
            uint64_t i;

            i = ox_value_get_uint64(ctxt, thiz);

            snprintf(buf, sizeof(buf), "%llu", (unsigned long long)i);
        }

        r = ox_string_from_char_star(ctxt, rv, buf);
    } else {
        OX_Number n;
        OX_Value *tv;

        if ((r = ox_to_number(ctxt, thiz, &n)) == OX_ERR)
            return r;

        tv = ox_value_stack_push(ctxt);
        ox_value_set_number(ctxt, tv, n);

        r = ox_to_string(ctxt, tv, rv);

        ox_value_stack_pop(ctxt, tv);
    }

    return r;
}

/*CNumber.$inf.value get.*/
static OX_Result
CNumber_inf_value_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_CValue *cv;
    OX_CValueInfo cvi;

    if (!(cv = value_get_cnumber(ctxt, thiz)))
        return OX_ERR;

    cvi.v.p = &cv->v;
    cvi.base = NULL;
    cvi.own = OX_CPTR_NON_OWNER;

    return ox_cptr_get_value(ctxt, &cv->cty, &cvi, rv);
}

/*CNumber.$inf.value set.*/
static OX_Result
CNumber_inf_value_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_CValue *cv;
    OX_CValueInfo cvi;

    if (!(cv = value_get_cnumber(ctxt, thiz)))
        return OX_ERR;

    cvi.v.p = &cv->v;
    cvi.base = NULL;
    cvi.own = OX_CPTR_NON_OWNER;

    return ox_cptr_set_value(ctxt, &cv->cty, &cvi, arg);
}

/*C.cast*/
static OX_Result
C_cast (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *v = ox_argument(ctxt, args, argc, 0);
    OX_Value *dty = ox_argument(ctxt, args, argc, 1);
    OX_CTypeKind kind, dkind;
    OX_Value *cty;
    OX_CValue *cv;
    OX_CValueInfo cvi;
    OX_Result r;

    if (!ox_value_is_cvalue(ctxt, v)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a C value"));
        goto end;
    }

    if (!ox_value_is_ctype(ctxt, dty)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a C type"));
        goto end;
    }

    cty = ox_cvalue_get_ctype(ctxt, v);
    kind = ox_ctype_get_kind(ctxt, cty);
    dkind = ox_ctype_get_kind(ctxt, dty);
    cv = ox_value_get_gco(ctxt, v);

    if (kind & OX_CTYPE_FL_PTR) {
        cvi.v.p = cv->v.p;
    } else if (kind & OX_CTYPE_FL_NUM) {
        double n;

        if (kind == OX_CTYPE_F32)
            n = cv->v.f32;
        else
            n = cv->v.f64;

        if (dkind & OX_CTYPE_FL_NUM) {
            if (dkind == OX_CTYPE_F32) {
                cvi.v.f32 = n;
            } else {
                cvi.v.f64 = n;
            }
        } else if (dkind & OX_CTYPE_FL_INT) {
            switch (kind) {
            case OX_CTYPE_I8:
                cvi.v.i8 = n;
                break;
            case OX_CTYPE_I16:
                cvi.v.i16 = n;
                break;
            case OX_CTYPE_I32:
                cvi.v.i32 = n;
                break;
            case OX_CTYPE_I64:
                cvi.v.i64 = n;
                break;
            case OX_CTYPE_U8:
                cvi.v.u8 = n;
                break;
            case OX_CTYPE_U16:
                cvi.v.u16 = n;
                break;
            case OX_CTYPE_U32:
                cvi.v.u32 = n;
                break;
            case OX_CTYPE_U64:
                cvi.v.u64 = n;
                break;
            default:
                assert(0);
            }
        } else {
            cvi.v.p = cv->v.p;
        }
    } else if (kind == OX_CTYPE_U64) {
        uint64_t i = cv->v.u64;

        if (dkind & OX_CTYPE_FL_NUM) {
            if (dkind == OX_CTYPE_F32) {
                cvi.v.f32 = i;
            } else {
                cvi.v.f64 = i;
            }
        } else if (dkind & OX_CTYPE_FL_INT) {
            switch (kind) {
            case OX_CTYPE_I8:
                cvi.v.i8 = i;
                break;
            case OX_CTYPE_I16:
                cvi.v.i16 = i;
                break;
            case OX_CTYPE_I32:
                cvi.v.i32 = i;
                break;
            case OX_CTYPE_I64:
                cvi.v.i64 = i;
                break;
            case OX_CTYPE_U8:
                cvi.v.u8 = i;
                break;
            case OX_CTYPE_U16:
                cvi.v.u16 = i;
                break;
            case OX_CTYPE_U32:
                cvi.v.u32 = i;
                break;
            case OX_CTYPE_U64:
                cvi.v.u64 = i;
                break;
            default:
                assert(0);
            }
        } else {
            cvi.v.p = cv->v.p;
        }
    } else {
        int64_t i;

        switch (kind) {
        case OX_CTYPE_I8:
            i = cv->v.i8;
            break;
        case OX_CTYPE_I16:
            i = cv->v.i16;
            break;
        case OX_CTYPE_I32:
            i = cv->v.i32;
            break;
        case OX_CTYPE_I64:
            i = cv->v.i64;
            break;
        case OX_CTYPE_U8:
            i = cv->v.u8;
            break;
        case OX_CTYPE_U16:
            i = cv->v.u16;
            break;
        case OX_CTYPE_U32:
            i = cv->v.u32;
            break;
        case OX_CTYPE_U64:
            i = cv->v.u64;
            break;
        default:
            assert(0);
        }

        if (dkind & OX_CTYPE_FL_NUM) {
            if (dkind == OX_CTYPE_F32) {
                cvi.v.f32 = i;
            } else {
                cvi.v.f64 = i;
            }
        } else if (dkind & OX_CTYPE_FL_INT) {
            switch (kind) {
            case OX_CTYPE_I8:
                cvi.v.i8 = i;
                break;
            case OX_CTYPE_I16:
                cvi.v.i16 = i;
                break;
            case OX_CTYPE_I32:
                cvi.v.i32 = i;
                break;
            case OX_CTYPE_I64:
                cvi.v.i64 = i;
                break;
            case OX_CTYPE_U8:
                cvi.v.u8 = i;
                break;
            case OX_CTYPE_U16:
                cvi.v.u16 = i;
                break;
            case OX_CTYPE_U32:
                cvi.v.u32 = i;
                break;
            case OX_CTYPE_U64:
                cvi.v.u64 = i;
                break;
            default:
                assert(0);
            }
        } else {
            cvi.v.p = cv->v.p;
        }
    }

    cvi.base = NULL;
    cvi.own = OX_CPTR_NON_OWNER;

    r = ox_cvalue_new(ctxt, rv, dty, &cvi);
end:
    return r;
}

/*C.get_owner*/
static OX_Result
C_get_owner (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *v = ox_argument(ctxt, args, argc, 0);
    OX_CValue *cv;
    OX_Result r;

    if (!ox_value_is_cvalue(ctxt, v)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a C value"));
        goto end;
    }

    cv = ox_value_get_gco(ctxt, v);

    if (!ox_value_is_null(ctxt, &cv->base) && !ox_value_is_number(ctxt, &cv->base))
        ox_value_copy(ctxt, rv, &cv->base);
    else
        ox_value_set_null(ctxt, rv);

    r = OX_OK;
end:
    return r;
}

/*C.set_owner*/
static OX_Result
C_set_owner (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *v = ox_argument(ctxt, args, argc, 0);
    OX_Value *base = ox_argument(ctxt, args, argc, 1);
    OX_CValue *cv;
    OX_Result r;

    if (!ox_value_is_cvalue(ctxt, v)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a C value"));
        goto end;
    }

    if (!ox_value_is_cvalue(ctxt, base) && !ox_value_is_null(ctxt, base)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a C value"));
        goto end;
    }

    cv = ox_value_get_gco(ctxt, v);

    if (ox_value_is_null(ctxt, base)) {
        cv->o.gco.ops = (OX_GcObjectOps*)&ext_ptr_cvalue_ops;
    } else {
        cv->o.gco.ops = (OX_GcObjectOps*)&cvalue_ops;
    }
    
    ox_value_copy(ctxt, &cv->base, base);

    r = OX_OK;
end:
    return r;
}

/*Get C pointer from the value.*/
static OX_CValue*
value_get_cptr (OX_Context *ctxt, OX_Value *v)
{
    OX_CValue *cv;
    OX_CType *t;

    if (!(cv = value_get_cvalue(ctxt, v)))
        return NULL;

    t = ox_value_get_gco(ctxt, &cv->cty);
    if (t->kind != OX_CTYPE_PTR) {
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a C pointer"));
        return NULL;
    }

    return cv;
}

/*C.get_length.*/
static OX_Result
C_get_length (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *v = ox_argument(ctxt, args, argc, 0);
    OX_CValue *cv;
    OX_CPtrType *pt;

    if (!(cv = value_get_cptr(ctxt, v)))
        return OX_ERR;

    pt = ox_value_get_gco(ctxt, &cv->cty);

    ox_value_set_number(ctxt, rv, pt->len);
    return OX_OK;
}

/*C.set_length.*/
static OX_Result
C_set_length (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *v = ox_argument(ctxt, args, argc, 0);
    OX_Value *arg = ox_argument(ctxt, args, argc, 1);
    OX_VS_PUSH(ctxt, pty)
    OX_Number n = 0;
    size_t len = -1;
    OX_CValue *cv;
    OX_CPtrType *pt;
    OX_Result r;

    if (!(cv = value_get_cptr(ctxt, v))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_to_number(ctxt, arg, &n)) == OX_ERR)
        goto end;

    if (n >= 0)
        len = n;

    pt = ox_value_get_gco(ctxt, &cv->cty);
    if (pt->len != len) {
        if ((r = ox_ctype_pointer(ctxt, pty, &pt->vty, len)) == OX_ERR)
            goto end;

        ox_value_copy(ctxt, &cv->cty, pty);
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, pty)
    return r;
}

/*C.get_own.*/
static OX_Result
C_get_own (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *v = ox_argument(ctxt, args, argc, 0);
    OX_CValue *cv;
    OX_CPtrOwned own;
    OX_Result r;

    if (!(cv = value_get_cptr(ctxt, v))) {
        r = OX_ERR;
        goto end;
    }

    if (cv->o.gco.ops == (const OX_GcObjectOps*)&ptr_cvalue_ops)
        own = OX_CPTR_INTERNAL;
    else if (cv->o.gco.ops == (const OX_GcObjectOps*)&ext_ptr_cvalue_ops)
        own = OX_CPTR_EXTERNAL;
    else
        own = OX_CPTR_NON_OWNER;

    ox_value_set_number(ctxt, rv, own);
    r = OX_OK;
end:
    return r;
}

/*C.set_own.*/
static OX_Result
C_set_own (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *v = ox_argument(ctxt, args, argc, 0);
    int32_t own;
    OX_CValue *cv;
    OX_Value *cty;
    OX_CArrayInfo cai;
    OX_Result r;

    if (!(cv = value_get_cptr(ctxt, v))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_to_int32(ctxt, v, &own)) == OX_ERR)
        goto end;

    if (own & OX_CPTR_INTERNAL) {
        cty = ox_cvalue_get_ctype(ctxt, v);
        ox_ctype_get_array_info(ctxt, cty, &cai);

        if ((cai.len == -1) || (cai.isize == -1)) {
            r = ox_throw_type_error(ctxt, OX_TEXT("cannot get size of the C type"));
            goto end;
        }

        cv->o.gco.ops = (OX_GcObjectOps*)&ptr_cvalue_ops;
        ox_value_set_number(ctxt, &cv->base, cai.len * cai.isize);
    } else if (own & OX_CPTR_EXTERNAL) {
        cv->o.gco.ops = (OX_GcObjectOps*)&ext_ptr_cvalue_ops;
        ox_value_set_null(ctxt, &cv->base);
    } else {
        cv->o.gco.ops = (OX_GcObjectOps*)&cvalue_ops;
    }

    r = OX_OK;
end:
    return r;
}

/*C.get_ref*/
static OX_Result
C_get_ref (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, pty)
    OX_Value *v = ox_argument(ctxt, args, argc, 0);
    OX_Value *off = ox_argument(ctxt, args, argc, 1);
    OX_Value *len = ox_argument(ctxt, args, argc, 2);
    size_t off_idx = 0, len_idx = 0;
    ssize_t ref_len = -1;
    OX_CValue *cv;
    OX_CPtrType *pt;
    OX_CType *vt;
    OX_CValueInfo cvi;
    OX_Result r;

    if (!(cv = value_get_cptr(ctxt, v))) {
        r = OX_ERR;
        goto end;
    }

    pt = ox_value_get_gco(ctxt, &cv->cty);
    vt = ox_value_get_gco(ctxt, &pt->vty);

    if ((r = ox_to_index(ctxt, off, &off_idx)) == OX_ERR)
        goto end;
    if ((pt->len != -1) && (off_idx >= pt->len)) {
        r = ox_throw_range_error(ctxt, OX_TEXT("array offset overflow"));
        goto end;
    }

    if (!ox_value_is_null(ctxt, len)) {
        if ((r = ox_to_index(ctxt, len, &len_idx)) == OX_ERR)
            goto end;

        if ((pt->len != -1) && (off_idx +  len_idx > pt->len)) {
            r = ox_throw_range_error(ctxt, OX_TEXT("reference length overflow"));
            goto end;
        }

        ref_len = len_idx;
    } else if ((pt->len != -1) && (pt->len > off_idx)) {
        ref_len = pt->len - off_idx;
    }

    if (off_idx) {
        if (vt->size == -1) {
            r = ox_throw_type_error(ctxt, OX_TEXT("item's size is unknown"));
            goto end;
        }

        cvi.v.p = ((uint8_t*)cv->v.p) + vt->size * off_idx;
    } else {
        cvi.v.p = cv->v.p;
    }

    if ((r = ox_ctype_pointer(ctxt, pty, &pt->vty, ref_len)) == OX_ERR)
        goto end;

    cvi.base = v;
    cvi.own = OX_CPTR_NON_OWNER;

    r = ox_cvalue_new(ctxt, rv, pty, &cvi);
end:
    OX_VS_POP(ctxt, pty)
    return r;
}

/*C.copy*/
static OX_Result
C_copy (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *dst = ox_argument(ctxt, args, argc, 0);
    OX_Value *dst_pos = ox_argument(ctxt, args, argc, 1);
    OX_Value *src = ox_argument(ctxt, args, argc, 2);
    OX_Value *src_pos = ox_argument(ctxt, args, argc, 3);
    OX_Value *cpy = ox_argument(ctxt, args, argc, 4);
    size_t src_idx = 0, dst_idx = 0, cpy_num = 0, cpy_len;
    OX_CValue *src_cv, *dst_cv;
    OX_CPtrType *src_pt, *dst_pt;
    OX_CType *src_it, *dst_it;
    uint8_t *src_p, *dst_p;
    OX_Result r;

    if (!(dst_cv = value_get_cptr(ctxt, dst))) {
        r = OX_ERR;
        goto end;
    }

    if (!dst_cv->v.p) {
        r = ox_throw_null_error(ctxt, OX_TEXT("the C pointer is null"));
        goto end;
    }

    dst_pt = ox_value_get_gco(ctxt, &dst_cv->cty);

    if (!ox_value_is_null(ctxt, dst_pos)) {
        if ((r = ox_to_index(ctxt, dst_pos, &dst_idx)) == OX_ERR)
            goto end;

        if ((dst_pt->len != -1) && (dst_idx >= dst_pt->len)) {
            r = OX_OK;
            goto end;
        }
    }

    if (!ox_value_is_null(ctxt, src_pos)) {
        if ((r = ox_to_index(ctxt, src_pos, &src_idx)) == OX_ERR)
            goto end;
    }

    if (!ox_value_is_null(ctxt, cpy)) {
        if ((r = ox_to_index(ctxt, cpy, &cpy_num)) == OX_ERR)
            goto end;

        cpy_num = OX_MIN(cpy_num, dst_pt->len - dst_idx);
    } else {
        if (dst_pt->len == -1) {
            r = ox_throw_type_error(ctxt, OX_TEXT("C array's length is unknown"));
            goto end;
        }

        cpy_num = dst_pt->len - dst_idx;
    }

    if (!ox_value_is_null(ctxt, src)) {
        if (!(src_cv = value_get_cptr(ctxt, src))) {
            r = OX_ERR;
            goto end;
        }

        if (!src_cv->v.p) {
            r = ox_throw_null_error(ctxt, OX_TEXT("the C pointer is null"));
            goto end;
        }

        src_pt = ox_value_get_gco(ctxt, &src_cv->cty);
    } else {
        src_cv = dst_cv;
        src_pt = dst_pt;
    }

    if (src_idx >= src_pt->len) {
        r = OX_OK;
        goto end;
    }

    src_it = ox_value_get_gco(ctxt, &src_pt->vty);
    dst_it = ox_value_get_gco(ctxt, &dst_pt->vty);

    if ((src_it->size == -1) || (dst_it->size == -1)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("item's size is unknown"));
        goto end;
    }

    cpy_len = OX_MIN(cpy_num * src_it->size, (dst_pt->len - dst_idx) * dst_it->size);
    src_p = ((uint8_t*)src_cv->v.p) + src_idx * src_it->size;
    dst_p = ((uint8_t*)dst_cv->v.p) + dst_idx * dst_it->size;

    if (((src_p >= dst_p) && (src_p < dst_p + cpy_len))
            || ((dst_p >= src_p) && (dst_p < src_p + cpy_len))) {
        memmove(dst_p, src_p, cpy_len);
    } else {
        memcpy(dst_p, src_p, cpy_len);
    }

    r = OX_OK;
end:
    return r;
}

/*C.fill*/
static OX_Result
C_fill (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *dst = ox_argument(ctxt, args, argc, 0);
    OX_Value *v = ox_argument(ctxt, args, argc, 1);
    OX_Value *pos = ox_argument(ctxt, args, argc, 2);
    OX_Value *cpy = ox_argument(ctxt, args, argc, 3);
    OX_Number n;
    size_t idx = 0, cpy_num = 0, cpy_len;
    OX_CValue *dst_cv;
    OX_CPtrType *dst_pt;
    OX_CType *dst_it;
    uint8_t *dst_p, *dst_pe;
    OX_Result r;

    if (!(dst_cv = value_get_cptr(ctxt, dst))) {
        r = OX_ERR;
        goto end;
    }

    if (!dst_cv->v.p) {
        r = ox_throw_null_error(ctxt, OX_TEXT("the C pointer is null"));
        goto end;
    }

    dst_pt = ox_value_get_gco(ctxt, &dst_cv->cty);
    dst_it = ox_value_get_gco(ctxt, &dst_pt->vty);

    if (!(dst_it->kind & OX_CTYPE_FL_NUM)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a number buffer"));
        goto end;
    }

    if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR)
        goto end; 

    if (!ox_value_is_null(ctxt, pos)) {
        if ((r = ox_to_index(ctxt, pos, &idx)) == OX_ERR)
            goto end;
    }

    if (!ox_value_is_null(ctxt, cpy)) {
        if ((r = ox_to_index(ctxt, cpy, &cpy_num)) == OX_ERR)
            goto end;

        cpy_num = OX_MIN(cpy_num, dst_pt->len - idx);
    } else {
        if (dst_pt->len == -1) {
            r = ox_throw_type_error(ctxt, OX_TEXT("C array's length is unknown"));
            goto end;
        }

        cpy_num = dst_pt->len - idx;
    }

    cpy_len = OX_MIN(cpy_num * dst_it->size, (dst_pt->len - idx) * dst_it->size);
    dst_p = ((uint8_t*)dst_cv->v.p) + idx * dst_it->size;
    dst_pe = dst_p + dst_it->size * cpy_len;

    while (dst_p < dst_pe) {
        switch (dst_it->kind) {
        case OX_CTYPE_I8:
            *(int8_t*)dst_p = (int8_t)n;
            break;
        case OX_CTYPE_U8:
            *(uint8_t*)dst_p = (uint8_t)n;
            break;
        case OX_CTYPE_I16:
            *(int16_t*)dst_p = (int16_t)n;
            break;
        case OX_CTYPE_U16:
            *(uint16_t*)dst_p = (uint16_t)n;
            break;
        case OX_CTYPE_I32:
            *(int32_t*)dst_p = (int32_t)n;
            break;
        case OX_CTYPE_U32:
            *(uint32_t*)dst_p = (uint32_t)n;
            break;
        case OX_CTYPE_I64:
            *(int64_t*)dst_p = (int64_t)n;
            break;
        case OX_CTYPE_U64:
            *(uint64_t*)dst_p = (uint64_t)n;
            break;
        case OX_CTYPE_F32:
            *(float*)dst_p = (float)n;
            break;
        case OX_CTYPE_F64:
            *(double*)dst_p = (double)n;
            break;
        default:
            assert(0);
        }

        dst_p += dst_it->size;
    }

    r = OX_OK;
end:
    return r;
}

/*C.func_type*/
static OX_Result
C_func_type (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *rty = ox_argument(ctxt, args, argc, 0);
    OX_Value *atys, *aty;
    size_t atyc, i;
    OX_Result r;

    if (!ox_value_is_null(ctxt, rty) && !ox_value_is_ctype(ctxt, rty)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a C type"));
        goto end;
    }

    if (argc > 1) {
        atys = ox_values_item(ctxt, args, 1);
        atyc = argc - 1;

        for (i = 0; i < atyc; i ++) {
            aty = ox_values_item(ctxt, atys, i);
            if (!ox_value_is_ctype(ctxt, aty)) {
                r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a C type"));
                goto end;
            }
        }
    } else {
        atys = NULL;
        atyc = 0;
    }

    r = ox_ctype_func(ctxt, rv, rty, atys, atyc, OX_FALSE);
end:
    return r;
}

/*CPointer.$inf.$to_str*/
static OX_Result
CPointer_inf_to_str (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *pos_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *len_arg = ox_argument(ctxt, args, argc, 1);
    OX_CValue *cv;
    OX_CPtrType *pt;
    OX_CType *it;
    OX_Result r;
    size_t pos = 0;
    ssize_t len = -1;

    if (!(cv = value_get_cptr(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if (!cv->v.p) {
        ox_value_set_null(ctxt, rv);
        r = OX_OK;
        goto end;
    }

    pt = ox_value_get_gco(ctxt, &cv->cty);
    it = ox_value_get_gco(ctxt, &pt->vty);

    if ((it->kind != OX_CTYPE_I8) && (it->kind != OX_CTYPE_U8)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value cannot be converted to string"));
        goto end;
    }

    if (!ox_value_is_null(ctxt, pos_arg)) {
        if ((r = ox_to_index(ctxt, pos_arg, &pos)) == OX_ERR)
            goto end;
    }

    if (!ox_value_is_null(ctxt, len_arg)) {
        size_t idx = 0;

        if ((r = ox_to_index(ctxt, len_arg, &idx)) == OX_ERR)
            goto end;

        len = idx;

        if ((pt->len >= 0) && (pos + len > pt->len)) {
            r = ox_throw_range_error(ctxt, OX_TEXT("position overflow"));
            goto end;
        }
    } else {
        if (pt->len < 0)
            len = -1;
        else if (pos < pt->len)
            len = pt->len - pos;
        else
            len = 0;
    }

    if (len == -1) {
        r = ox_string_from_char_star(ctxt, rv, ((char*)cv->v.p) + pos);
    } else {
        r = ox_string_from_chars(ctxt, rv, ((char*)cv->v.p) + pos, len);
    }
end:
    return r;
}

/** Array iterator data.*/
typedef struct {
    OX_Value a; /**< C array value.*/
    size_t   i; /**< Current index.*/
} OX_CArrayIter;

/*Scan referenced objects in the C array iterator.*/
static void
carray_iter_scan (OX_Context *ctxt, void *p)
{
    OX_CArrayIter *ai = p;

    ox_gc_scan_value(ctxt, &ai->a);
}

/*Free the C array iterator.*/
static void
carray_iter_free (OX_Context *ctxt, void *p)
{
    OX_CArrayIter *ai = p;

    OX_DEL(ctxt, ai);
}

/*Operation functions of C array iterator.*/
static const OX_PrivateOps
carray_iter_ops = {
    carray_iter_scan,
    carray_iter_free
};

/*CPointer.$inf.$iter*/
static OX_Result
CPointer_inf_iter (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_CValue *cv;
    OX_CPtrType *pt;
    OX_CArrayIter *ai;
    OX_CType *it;
    OX_Result r;

    if (!(cv = value_get_cptr(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    pt = ox_value_get_gco(ctxt, &cv->cty);
    if (pt->len == -1) {
        r = ox_throw_type_error(ctxt, OX_TEXT("C array's length is unknown"));
        goto end;
    }

    it = ox_value_get_gco(ctxt, &pt->vty);
    if (it->size == -1) {
        r = ox_throw_type_error(ctxt, OX_TEXT("item's size is unknown"));
        goto end;
    }

    if ((r = ox_object_new(ctxt, rv, OX_OBJECT(ctxt, CArrayIterator_inf))) == OX_ERR)
        goto end;

    if (!OX_NEW(ctxt, ai)) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    ox_value_copy(ctxt, &ai->a, thiz);
    ai->i = 0;

    if ((r = ox_object_set_priv(ctxt, rv, &carray_iter_ops, ai)) == OX_ERR) {
        carray_iter_free(ctxt, ai);
        goto end;
    }

    r = OX_OK;
end:
    return r;
}

/*CPointer.$inf.$to_json*/
static OX_Result
CPointer_inf_to_json (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_CValue *cv;

    if (!(cv = value_get_cptr(ctxt, thiz)))
        return OX_ERR;

    return ox_carray_to_array(ctxt, thiz, rv);
}

/*Get the C array iterator data.*/
static OX_CArrayIter*
carray_iter_get (OX_Context *ctxt, OX_Value *v)
{
    OX_CArrayIter *ai = ox_object_get_priv(ctxt, v, &carray_iter_ops);

    if (!ai)
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a C array iterator"));

    return ai;
}

/*CArrayIterator.$inf.next*/
static OX_Result
CArrayIterator_inf_next (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_CArrayIter *ai;
    OX_CValue *cv;
    OX_CPtrType *pt;

    if (!(ai = carray_iter_get(ctxt, thiz)))
        return OX_ERR;

    cv = ox_value_get_gco(ctxt, &ai->a);
    pt = ox_value_get_gco(ctxt, &cv->cty);

    if (ai->i < pt->len)
        ai->i ++;

    return OX_OK;
}

/*CArrayIterator.$inf.end get*/
static OX_Result
CArrayIterator_inf_end_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_CArrayIter *ai;
    OX_CValue *cv;
    OX_CPtrType *pt;
    OX_Bool b;

    if (!(ai = carray_iter_get(ctxt, thiz)))
        return OX_ERR;

    cv = ox_value_get_gco(ctxt, &ai->a);
    pt = ox_value_get_gco(ctxt, &cv->cty);

    b = ai->i >= pt->len;
    ox_value_set_bool(ctxt, rv, b);
    return OX_OK;
}

/*CArrayIterator.$inf.value get*/
static OX_Result
CArrayIterator_inf_value_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_CArrayIter *ai;
    OX_CValue *cv;
    OX_CPtrType *pt;
    OX_CType *it;
    OX_Result r;

    if (!(ai = carray_iter_get(ctxt, thiz)))
        return OX_ERR;

    cv = ox_value_get_gco(ctxt, &ai->a);
    if (!cv->v.p)
        ox_throw_null_error(ctxt, OX_TEXT("the C array is freed"));

    pt = ox_value_get_gco(ctxt, &cv->cty);
    it = ox_value_get_gco(ctxt, &pt->vty);

    if (ai->i < pt->len) {
        OX_CValueInfo cvi;

        cvi.v.p = ((uint8_t*)cv->v.p) + ai->i * it->size;
        cvi.base = thiz;
        cvi.own = OX_CPTR_NON_OWNER;

        r = ox_cptr_get_value(ctxt, &pt->vty, &cvi, rv);
    } else {
        ox_value_set_null(ctxt, rv);
        r = OX_OK;
    }

    return r;
}

/*Create the C type interface.*/
static void
ctype_inf_new (OX_Context *ctxt, const char *name, OX_Value *v)
{
    OX_VS_PUSH(ctxt, nv)

    ox_not_error(ox_string_from_const_char_star(ctxt, nv, name));
    ox_not_error(ox_interface_new(ctxt, v));
    ox_not_error(ox_set(ctxt, v, OX_STRING(ctxt, _name), nv));

    OX_VS_POP(ctxt, nv)
}

/*?
 *? @lib {CType} C data type and C value.
 *?
 *? @class{ CType C data type.
 *?
 *? @roacc pointer Get the pointer type of this type.
 *?
 *? @roacc size Get the size in bytes of this type.
 *?
 *? @roacc length Get the items length of this type.
 *? If the type is an array pointer, returns the items count of the array.
 *? Otherwise returns 1.
 *?
 *? @class}
 *?
 *? @class{ Int8 8 bits integer type.
 *? @inherit {CType}
 *? @class}
 *?
 *? @class{ Int16 16 bits integer type.
 *? @inherit {CType}
 *? @class}
 *?
 *? @class{ Int32 32 bits integer type.
 *? @inherit {CType}
 *? @class}
 *?
 *? @class{ Int64 64 bits integer type.
 *? @inherit {CType}
 *? @class}
 *?
 *? @class{ UInt8 8 bits unsigned integer type.
 *? @inherit {CType}
 *? @class}
 *?
 *? @class{ UInt16 16 bits unsigned integer type.
 *? @inherit {CType}
 *? @class}
 *?
 *? @class{ UInt32 32 bits unsigned integer type.
 *? @inherit {CType}
 *? @class}
 *?
 *? @class{ UInt64 64 bits unsigned integer type.
 *? @inherit {CType}
 *? @class}
 *?
 *? @class{ Float32 32 bits float point number type.
 *? @inherit {CType}
 *? @class}
 *?
 *? @class{ Float64 64 bits float point number type.
 *? @inherit {CType}
 *? @class}
 *?
 *? @class{ Size size_t (pointer size unsigned integer number) type.
 *? @inherit {CType}
 *? @class}
 *?
 *? @class{ SSize size_t (pointer size signed integer number) type.
 *? @inherit {CType}
 *? @class}
 *?
 *? @object{ C C object.
 *?
 *? @const NON_OWNER {Number} The object is not the pointer's owner.
 *? @const INTERNAL {Number} The object is the pointer's owner, and the pointer is managed by OX's GC.
 *? @const EXTERNAL {Number} The object is the pointer's owner, and the pointer is not managed by OX's GC.
 *?
 *? @func cast Cast a C value to another type.
 *? @param v C value.
 *? @param t C data type.
 *? @return The C value with the type t.
 *?
 *? @func get_ref Get the reference to the items of a C array pointer.
 *? @param v C array pointer.
 *? @param off {Number} =0 The reference's first item's index.
 *? @param n {Number} =v.length-off The number of items in the reference array.
 *? @return The reference C array pointer.
 *?
 *? @func get_owner Get the C pointer's owner object.
 *? If a pointer is a reference to another array pointer's items,
 *? we say that the owner of this pointer is the array pointer.
 *? GC will keep the owner alive until all its references are released.
 *? @param v C value.
 *? @return The owner C value.
 *?
 *? @func set_owner Set the C pointer's owner object.
 *? If a pointer is a reference to another array pointer's items,
 *? we say that the owner of this pointer is the array pointer.
 *? GC will keep the owner alive until all its references are released.
 *? @param v C value.
 *? @param owner The owner C value.
 *?
 *? @func get_length Get the C array pointer's length.
 *? @param v C value.
 *? @return {Number} Return the items' count of the C array.
 *? If v is not a C array, return 1.
 *?
 *? @func set_length Set the C array pointer's length.
 *? @param v C value.
 *? @param len {Number} The length of the C array.
 *?
 *? @func get_own Get the own flag of the C pointer.
 *? @param v C value.
 *? @return {C.Owned} The C pointer's own flag.
 *?
 *? @func set_own Set the own flag of the C pointer.
 *? @param v C value.
 *? @param own {C.Owned} The C pointer's own flag.
 *?
 *? @func copy Copy items from a C array pointer to another one.
 *? @param dst The destination C array pointer.
 *? @param dst_pos {Number} The destination copy position.
 *? @param src =dst The source C array pointer.
 *? @param src_pos {Number} The source copy position.
 *? @param n {Number} Number of items to be copyed.
 *?
 *? @func fill Fill the C number array pointer with the value.
 *? @param buf The C number array pointer.
 *? @param v {Number} The number used to fill the buffer.
 *? @param pos {Number} =0 Fill starting position.
 *? @param n {Number} =buf.length-pos Number of items to be filled.
 *?
 *? @func func_type Create a new function type.
 *? @param rty The function's return type.
 *? @param ...atys The arguments' types.
 *? @return The new function type.
 *?
 *? @object}
 *?
 *? @class{ CNumber C number value.
 *?
 *? @func $to_num Convert the C number to an OX number value.
 *? @return {Number} The OX number value.
 *?
 *? @func $to_str Convert the C number to a string.
 *? @return {String} The result string.
 *?
 *? @acc value {Number} Get/set OX number value.
 *?
 *? @class}
 *?
 *? @class{ CPointer C pointer value.
 *?
 *? @func $to_str Convert the C pointer to string.
 *? The C pointer must be a UInt8 ot Int8 array buffer.
 *? @param start {Number} =0 The string's start position.
 *? @param len {Number} The string's length.
 *? If "len" is not present and the C buffer's length is known,
 *? the string's length is C buffer's length - pos.
 *? If "len" is not present and the C buffer's length is unknown,
 *? the string is built from the characters in C buffer from pos unitl the '0'.
 *? @return {String} The result string.
 *?
 *? @func $iter Create a new iterator to traveese the items of a C array pointer.
 *? @return {Iterator[CValue]} The iterator to traverse the items.
 *?
 *? @class} 
 */

/**
 * Initialize the C type classes.
 * @param ctxt The current running context.
 */
void
ox_ctype_class_init (OX_Context *ctxt)
{
    OX_VS_PUSH_3(ctxt, c, name, v)

    /*CType.$inf*/
    ctype_inf_new(ctxt, "CType.$inf", OX_OBJECT(ctxt, CType_inf));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, CType_inf), "pointer", CType_inf_pointer_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, CType_inf), "size", CType_inf_size_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, CType_inf), "length", CType_inf_length_get, NULL));

    /*Basic C types.*/
    ctype_class_init(ctxt, OX_OBJECT(ctxt, Int8), OX_CTYPE_I8, "Int8", sizeof(int8_t));
    ctype_class_init(ctxt, OX_OBJECT(ctxt, Int16), OX_CTYPE_I16, "Int16", sizeof(int16_t));
    ctype_class_init(ctxt, OX_OBJECT(ctxt, Int32), OX_CTYPE_I32, "Int32", sizeof(int32_t));
    ctype_class_init(ctxt, OX_OBJECT(ctxt, Int64), OX_CTYPE_I64, "Int64", sizeof(int64_t));
    ctype_class_init(ctxt, OX_OBJECT(ctxt, UInt8), OX_CTYPE_U8, "UInt8", sizeof(uint8_t));
    ctype_class_init(ctxt, OX_OBJECT(ctxt, UInt16), OX_CTYPE_U16, "UInt16", sizeof(uint16_t));
    ctype_class_init(ctxt, OX_OBJECT(ctxt, UInt32), OX_CTYPE_U32, "UInt32", sizeof(uint32_t));
    ctype_class_init(ctxt, OX_OBJECT(ctxt, UInt64), OX_CTYPE_U64, "UInt64", sizeof(uint64_t));
    ctype_class_init(ctxt, OX_OBJECT(ctxt, Float32), OX_CTYPE_F32, "Float32", sizeof(float));
    ctype_class_init(ctxt, OX_OBJECT(ctxt, Float64), OX_CTYPE_F64, "Float64", sizeof(double));
    ctype_class_init(ctxt, OX_OBJECT(ctxt, Void), OX_CTYPE_VOID, "Void", -1);

#if __SIZEOF_POINTER__  == 8
    ox_not_error(ox_object_add_const_s(ctxt, OX_OBJECT(ctxt, Global), "Size", OX_OBJECT(ctxt, UInt64)));
    ox_not_error(ox_object_add_const_s(ctxt, OX_OBJECT(ctxt, Global), "SSize", OX_OBJECT(ctxt, Int64)));
#elif __SIZEOF_POINTER__  == 4
    ox_not_error(ox_object_add_const_s(ctxt, OX_OBJECT(ctxt, Global), "Size", OX_OBJECT(ctxt, UInt32)));
    ox_not_error(ox_object_add_const_s(ctxt, OX_OBJECT(ctxt, Global), "SSize", OX_OBJECT(ctxt, Int32)));
#else
    #error illegal pointer size
#endif

    /*C*/
    ox_not_error(ox_object_new(ctxt, c, NULL));
    ox_not_error(ox_string_from_const_char_star(ctxt, name, "C"));
    ox_not_error(ox_object_add_const_s(ctxt, OX_OBJECT(ctxt, Global), "C", c));
    ox_not_error(ox_set(ctxt, c, OX_STRING(ctxt, _name), name));
    ox_not_error(ox_object_add_n_method_s(ctxt, c, "cast", C_cast));
    ox_not_error(ox_object_add_n_method_s(ctxt, c, "get_ref", C_get_ref));
    ox_not_error(ox_object_add_n_method_s(ctxt, c, "get_owner", C_get_owner));
    ox_not_error(ox_object_add_n_method_s(ctxt, c, "set_owner", C_set_owner));
    ox_not_error(ox_object_add_n_method_s(ctxt, c, "get_length", C_get_length));
    ox_not_error(ox_object_add_n_method_s(ctxt, c, "set_length", C_set_length));
    ox_not_error(ox_object_add_n_method_s(ctxt, c, "get_own", C_get_own));
    ox_not_error(ox_object_add_n_method_s(ctxt, c, "set_own", C_set_own));
    ox_not_error(ox_object_add_n_method_s(ctxt, c, "copy", C_copy));
    ox_not_error(ox_object_add_n_method_s(ctxt, c, "fill", C_fill));
    ox_not_error(ox_object_add_n_method_s(ctxt, c, "func_type", C_func_type));

    /*Owned types.*/
    ox_value_set_number(ctxt, v, OX_CPTR_NON_OWNER);
    ox_not_error(ox_object_add_const_s(ctxt, c, "NON_OWNER", v));
    ox_value_set_number(ctxt, v, OX_CPTR_INTERNAL);
    ox_not_error(ox_object_add_const_s(ctxt, c, "INTERNAL", v));
    ox_value_set_number(ctxt, v, OX_CPTR_EXTERNAL);
    ox_not_error(ox_object_add_const_s(ctxt, c, "EXTERNAL", v));

    /*CNumber.$inf*/
    ctype_inf_new(ctxt, "CNumber.$inf", OX_OBJECT(ctxt, CNumber_inf));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, CNumber_inf), "$to_num", CNumber_inf_to_num));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, CNumber_inf), "$to_str", CNumber_inf_to_str));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, CNumber_inf), "value", CNumber_inf_value_get, CNumber_inf_value_set));

    /*CPointer.$inf*/
    ctype_inf_new(ctxt, "CPointer.$inf", OX_OBJECT(ctxt, CPointer_inf));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, CPointer_inf), "$to_str", CPointer_inf_to_str));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, CPointer_inf), "$iter", CPointer_inf_iter));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, CPointer_inf), "$to_json", CPointer_inf_to_json));

    /*CArrayIterator.$inf*/
    ox_not_error(ox_interface_new(ctxt, OX_OBJECT(ctxt, CArrayIterator_inf)));
    ox_not_error(ox_interface_inherit(ctxt, OX_OBJECT(ctxt, CArrayIterator_inf), OX_OBJECT(ctxt, Iterator_inf)));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, CArrayIterator_inf), "next", CArrayIterator_inf_next));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, CArrayIterator_inf), "end", CArrayIterator_inf_end_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, CArrayIterator_inf), "value", CArrayIterator_inf_value_get, NULL));

    OX_VS_POP(ctxt, c)
}
