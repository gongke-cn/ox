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

#ifndef _OX_CTYPE_H_
#define _OX_CTYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Check if the value is a C type.
 * @param ctxt The current running context.
 * @param v The value.
 * @retval OX_TRUE The value is a C type.
 * @retval OX_FALSE The value is not a C type.
 */
static inline OX_Bool
ox_value_is_ctype (OX_Context *ctxt, OX_Value *v)
{
    return ox_value_is_gco(ctxt, v, OX_GCO_CTYPE);
}

/**
 * Check if the value is a C value.
 * @param ctxt The current running context.
 * @param v The value.
 * @retval OX_TRUE The value is a C value.
 * @retval OX_FALSE The value is not a C value.
 */
static inline OX_Bool
ox_value_is_cvalue (OX_Context *ctxt, OX_Value *v)
{
    return ox_value_is_gco(ctxt, v, OX_GCO_CVALUE);
}

/**
 * Get the basic C type value.
 * @param ctxt The current running context.
 * @param kind The kind of the C type.
 * @return The C type value.
 */
extern OX_Value*
ox_ctype_get (OX_Context *ctxt, OX_CTypeKind kind);

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
extern OX_Result
ox_ctype_pointer (OX_Context *ctxt, OX_Value *pty, OX_Value *vty, ssize_t len);

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
extern OX_Result
ox_ctype_array (OX_Context *ctxt, OX_Value *aty, OX_Value *ity, ssize_t len);

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
extern OX_Result
ox_ctype_func (OX_Context *ctxt, OX_Value *fty, OX_Value *rty, OX_Value *atys, size_t argc, OX_Bool vaarg);

/**
 * Create a new C structure type.
 * @param ctxt The current running context.
 * @param[out] sty Return the new structure type.
 * @param[out] inf Return the interface of the structure.
 * @param size Size of the structure in bytes.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_ctype_struct (OX_Context *ctxt, OX_Value *sty, OX_Value *inf, ssize_t size);

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
extern OX_Result
ox_named_ctype_struct (OX_Context *ctxt, OX_Value *sty, OX_Value *inf, size_t size,
        OX_Value *scope, OX_Value *name);

/**
 * Create a new C structure type with its scope and name.
 * name is a 0 terminated characters string.
 * @param ctxt The current running context.
 * @param[out] sty Return the new structure type.
 * @param[out] inf Return the interface of the structure.
 * @param size Size of the structure in bytes.
 * @param scope The scope of the structure.
 * @param name The name of the structure.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_named_ctype_struct_s (OX_Context *ctxt, OX_Value *sty, OX_Value *inf, size_t size,
        OX_Value *scope, const char *name);

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
extern OX_Result
ox_cstruct_add_field (OX_Context *ctxt, OX_Value *sty, OX_Value *cty, OX_Value *name, size_t off);

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
extern OX_Result
ox_cstruct_add_field_s (OX_Context *ctxt, OX_Value *sty, OX_Value *cty, const char *name, size_t off);

/**
 * Create a new C value.
 * @param ctxt The current running context.
 * @param cv Return the new C value.
 * @param cty The C type of the value.
 * @param cvi The value's information.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_cvalue_new (OX_Context *ctxt, OX_Value *cv, OX_Value *cty, OX_CValueInfo *cvi);

/**
 * Get the C type of the C value.
 * @param ctxt The current running context.
 * @param cv The C value.
 * @return The C type's pointer.
 */
extern OX_Value*
ox_cvalue_get_ctype (OX_Context *ctxt, OX_Value *cv);

/**
 * Get the C value's pointer.
 * @param ctxt The current running context.
 * @param cv The C value.
 * @return The pointer of the C value.
 */
extern void*
ox_cvalue_get_pointer (OX_Context *ctxt, OX_Value *cv);

/**
 * Set the C pointer value to NULL.
 * @param ctxt The current running context.
 * @param cv The C value.
 */
extern void
ox_cvalue_set_null (OX_Context *ctxt, OX_Value *cv);

/**
 * Get the kind of the C type.
 * @param ctxt The current running context.
 * @param ct The C type.
 * @return The kind of the C type.
 */
static inline OX_CTypeKind
ox_ctype_get_kind (OX_Context *ctxt, OX_Value *ct)
{
    OX_CType *t;

    assert(ox_value_is_ctype(ctxt, ct));

    t = ox_value_get_gco(ctxt, ct);

    return t->kind;
}

/**
 * Get the size of the C type.
 * @param ctxt The current running context.
 * @param ct The C type.
 * @return The kind of the C type.
 */
static inline size_t
ox_ctype_get_size (OX_Context *ctxt, OX_Value *ct)
{
    OX_CType *t;

    assert(ox_value_is_ctype(ctxt, ct));

    t = ox_value_get_gco(ctxt, ct);

    return t->size;
}

/**
 * Get the pointer type's pointed value's type.
 * @param ctxt The current running context.
 * @param cty The C type.
 * @return The pointed value's type.
 * @retval NULL The type is not a pointer type.
 */
static inline OX_Value*
ox_ctype_get_value_type (OX_Context *ctxt, OX_Value *cty)
{
    OX_CPtrType *pty;

    assert(ctxt && cty);

    if (ox_ctype_get_kind(ctxt, cty) != OX_CTYPE_PTR)
        return NULL;

    pty = ox_value_get_gco(ctxt, cty);

    return &pty->vty;
}

/**
 * Get the array type's item's type.
 * @param ctxt The current running context.
 * @param cty The C type.
 * @return The item's type.
 * @retval NULL The type is not an array type.
 */
static inline OX_Value*
ox_ctype_get_item_type (OX_Context *ctxt, OX_Value *cty)
{
    OX_CArrayType *aty;

    assert(ctxt && cty);

    if (ox_ctype_get_kind(ctxt, cty) != OX_CTYPE_ARRAY)
        return NULL;

    aty = ox_value_get_gco(ctxt, cty);

    return &aty->ity;
}

/**
 * Get the C array type's information.
 * @param ctxt The current running context.
 * @param ct The C type.
 * @param[out] ai Return the array information.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_ctype_get_array_info (OX_Context *ctxt, OX_Value *ct, OX_CArrayInfo *ai)
{
    OX_CType *t;

    assert(ox_value_is_ctype(ctxt, ct));

    t = ox_value_get_gco(ctxt, ct);

    if (t->kind == OX_CTYPE_ARRAY) {
        OX_CArrayType *at = (OX_CArrayType*)t;
        OX_CType *it = ox_value_get_gco(ctxt, &at->ity);

        ai->len = at->len;
        ai->isize = it->size;
    } else if (t->kind == OX_CTYPE_PTR) {
        OX_CPtrType *pt = (OX_CPtrType*)t;
        OX_CType *it = ox_value_get_gco(ctxt, &pt->vty);

        ai->len = pt->len;
        ai->isize = it->size;
    } else {
        ai->len = 1;
        ai->isize = t->size;
    }

    return OX_OK;
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
extern OX_Result
ox_function_get_cptr (OX_Context *ctxt, OX_Value *f, OX_Value *cty, void **p);

/**
 * Get the C value from a pointer.
 * @param ctxt The current running context.
 * @param cty The type of the pointed value.
 * @param cvi The C pointer's information.
 * @param[out] v Return the pointed C value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_cptr_get_value (OX_Context *ctxt, OX_Value *cty, OX_CValueInfo *cvi, OX_Value *v);

/**
 * Set the value to the C pointer.
 * @param ctxt The current running context.
 * @param cty The type of the pointed value.
 * @param cvi The C pointer's information.
 * @param v The new value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_cptr_set_value (OX_Context *ctxt, OX_Value *cty, OX_CValueInfo *cvi, OX_Value *v);

/**
 * Get the C arrray's length.
 * @param ctxt The current running context.
 * @param cv The C array value.
 * @return The length of the C array.
 */
static inline ssize_t
ox_carray_length (OX_Context *ctxt, OX_Value *cv)
{
    OX_Value *t;
    OX_CPtrType *pt;

    assert(ox_value_is_cvalue(ctxt, cv));

    t = ox_cvalue_get_ctype(ctxt, cv);
    assert(ox_ctype_get_kind(ctxt, t) == OX_CTYPE_PTR);

    pt = ox_value_get_gco(ctxt, t);
    return pt->len;
}

/**
 * Convert the C array to json.
 * @param ctxt The current running context.
 * @param cv The C array value.
 * @param[out] a Return the OX array object.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_carray_to_array (OX_Context *ctxt, OX_Value *cv, OX_Value *a);

/**
 * Check if the value is a C pointer of a structure.
 * @param ctxt The current running context.
 * @param v The value.
 * @retval OX_TRUE The value is a C pointer of a structure.
 * @retval OX_FALSE The value is not a C pointer of a structure.
 */
static inline OX_Bool
ox_value_is_struct_cptr (OX_Context *ctxt, OX_Value *v)
{
    OX_Value *t;
    OX_CPtrType *pt;

    if (!ox_value_is_cvalue(ctxt, v))
        return OX_FALSE;

    t = ox_cvalue_get_ctype(ctxt, v);
    if (ox_ctype_get_kind(ctxt, t) != OX_CTYPE_PTR)
        return OX_FALSE;

    pt = ox_value_get_gco(ctxt, t);
    if (ox_ctype_get_kind(ctxt, &pt->vty) != OX_CTYPE_STRUCT)
        return OX_FALSE;

    return OX_TRUE;
}

/**
 * Check if the value is a C pointer.
 * @param ctxt The current running context.
 * @param v The value.
 * @param vty The value's type or NULL.
 * @retval OX_TRUE The value is a C pointer of value which type is vty.
 * @retval OX_FALSE The value is not a C pointer of value which type is vty.
 */
static inline OX_Bool
ox_value_is_cptr (OX_Context *ctxt, OX_Value *v, OX_Value *vty)
{
    OX_Value *t;

    if (!ox_value_is_cvalue(ctxt, v))
        return OX_FALSE;

    t = ox_cvalue_get_ctype(ctxt, v);
    if (ox_ctype_get_kind(ctxt, t) != OX_CTYPE_PTR)
        return OX_FALSE;

    if (vty) {
        OX_CPtrType *pt = ox_value_get_gco(ctxt, t);

        if (!ox_equal(ctxt, vty, &pt->vty))
            return OX_FALSE;
    }

    return OX_TRUE;
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
extern OX_Result
ox_cfunc_new (OX_Context *ctxt, OX_Value *v, OX_Value *fty, void *p);

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
extern OX_Result
ox_named_cfunc_new (OX_Context *ctxt, OX_Value *v, OX_Value *fty, void *p,
        OX_Value *scope, OX_Value *name);

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
extern OX_Result
ox_named_cfunc_new_s (OX_Context *ctxt, OX_Value *v, OX_Value *fty, void *p,
        OX_Value *scope, const char *name);

/**
 * Check if the value is a 64 bits singed or unsigned integer.
 * @param ctxt The current running context.
 * @param v The value.
 * @retval OX_CTYPE_I64 The value is a 64 bits signed integer.
 * @retval OX_CTYPE_U64 The value is a 64 bits unsigned integer.
 * @retval OX_FALSE The value is not a 64 bits signed or unsigned integer.
 */
static inline int
ox_value_is_int64 (OX_Context *ctxt, OX_Value *v)
{
    OX_Value *cty;
    OX_CTypeKind kind;

    if (!ox_value_is_cvalue(ctxt, v))
        return OX_FALSE;

    cty = ox_cvalue_get_ctype(ctxt, v);
    kind = ox_ctype_get_kind(ctxt, cty);

    if ((kind == OX_CTYPE_I64) || (kind == OX_CTYPE_U64))
        return kind;

    return OX_FALSE;
}

/**
 * Get the 64 bits integer number from the value.
 * @param ctxt The current running context.
 * @param v The value.
 * @return The 64 bits integer number.
 */
static inline int64_t
ox_value_get_int64 (OX_Context *ctxt, OX_Value *v)
{
    OX_CValue *cv = ox_value_get_gco(ctxt, v);

    return cv->v.i64;
}

/**
 * Get the 64 bits unsigned integer number from the value.
 * @param ctxt The current running context.
 * @param v The value.
 * @return The 64 bits unsigned integer number.
 */
static inline uint64_t
ox_value_get_uint64 (OX_Context *ctxt, OX_Value *v)
{
    OX_CValue *cv = ox_value_get_gco(ctxt, v);

    return cv->v.u64;
}

/**
 * Set the value as 64 bits integer number.
 * @param ctxt The current runing context.
 * @param v The value to be set.
 * @param i 64 bits integer number value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_value_set_int64 (OX_Context *ctxt, OX_Value *v, int64_t i)
{
    OX_CValueInfo cvi;

    cvi.v.i64 = i;
    cvi.base = NULL;

    return ox_cvalue_new(ctxt, v, ox_ctype_get(ctxt, OX_CTYPE_I64), &cvi);
}

/**
 * Set the value as 64 bits unsigned integer number.
 * @param ctxt The current runing context.
 * @param v The value to be set.
 * @param i 64 bits unsigned integer number value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_value_set_uint64 (OX_Context *ctxt, OX_Value *v, uint64_t i)
{
    OX_CValueInfo cvi;

    cvi.v.u64 = i;
    cvi.base = NULL;

    return ox_cvalue_new(ctxt, v, ox_ctype_get(ctxt, OX_CTYPE_U64), &cvi);
}

/**
 * Check if the value is a C number.
 * @param ctxt The current runing context.
 * @param v The value.
 * @retval OX_FALSE The value is not a C number.
 * @return The C number's kind.
 */
static inline OX_CTypeKind
ox_value_is_c_number (OX_Context *ctxt, OX_Value *v)
{
    OX_Value *t;
    OX_CTypeKind kind;

    if (!ox_value_is_cvalue(ctxt, v))
        return OX_FALSE;

    t = ox_cvalue_get_ctype(ctxt, v);
    kind = ox_ctype_get_kind(ctxt, t);

    return (kind & OX_CTYPE_FL_NUM) ? kind : OX_FALSE;
}

/**
 * Check if the value is a C string.
 * @param ctxt The current runing context.
 * @param v The value.
 * @retval OX_FALSE The value is not a C string.
 * @retval OX_CTYPE_I8 The string's items' types are int8_t.
 * @retval OX_CTYPE_U8 The string's items' types atr uint8_t.
 */
static inline OX_CTypeKind
ox_value_is_c_string (OX_Context *ctxt, OX_Value *v)
{
    OX_Value *t;
    OX_CTypeKind kind;
    OX_CValue *cv;
    OX_CPtrType *pty;

    if (!ox_value_is_cvalue(ctxt, v))
        return OX_FALSE;

    t = ox_cvalue_get_ctype(ctxt, v);
    kind = ox_ctype_get_kind(ctxt, t);
    if (kind != OX_CTYPE_PTR)
        return OX_FALSE;

    cv = ox_value_get_gco(ctxt, v);
    if (cv->v.p == NULL)
        return OX_FALSE;

    pty = ox_value_get_gco(ctxt, t);
    kind = ox_ctype_get_kind(ctxt, &pty->vty);

    if ((kind == OX_CTYPE_I8) || (kind == OX_CTYPE_U8))
        return kind;

    return OX_FALSE;
}

#ifdef __cplusplus
}
#endif

#endif
