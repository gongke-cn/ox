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
 * Instruction functions.
 */

/*Duplicate.*/
static inline OX_Result
do_dup (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s, OX_Value *d)
{
    ox_value_copy(ctxt, d, s);
    return OX_OK;
}

/*Logic not.*/
static inline OX_Result
do_not (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s, OX_Value *d)
{
    OX_Bool b = ox_to_bool(ctxt, s);

    ox_value_set_bool(ctxt, d, !b);
    return OX_OK;
}

/*Convert value to number.*/
static inline OX_Result
do_to_num (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s, OX_Value *d)
{
    OX_Number n;
    OX_Result r;

    if (ox_value_is_int64(ctxt, s)) {
        ox_value_copy(ctxt, d, s);
    } else {
        if ((r = ox_to_number(ctxt, s, &n)) == OX_ERR)
            return r;

        ox_value_set_number(ctxt, d, n);
    }

    return OX_OK;
}

/*Convert the value to 32 bits integer for bitwise operation.*/
static OX_Result
to_bit_int32 (OX_Context *ctxt, OX_Value *v, uint32_t *pi)
{
    OX_Number n;
    OX_Result r;

    if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR) {
        *pi = 0;
        return r;
    }

    if ((n < INT32_MIN) || (n > UINT32_MAX)) {
        *pi = 0;
        return ox_throw_range_error(ctxt, OX_TEXT("number value overflow"));
    }

    *pi = n;
    return OX_OK;
}

/*Bit reverse.*/
static inline OX_Result
do_rev (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s, OX_Value *d)
{
    OX_Result r;

    if (ox_value_is_int64(ctxt, s)) {
        uint64_t i = ox_value_get_uint64(ctxt, s);

        r = ox_value_set_uint64(ctxt, d, ~i);
    } else {
        uint32_t i;

        if ((r = to_bit_int32(ctxt, s, &i)) == OX_ERR)
            return r;

        ox_value_set_number(ctxt, d, ~i);
    }

    return r;
}

/*Negative.*/
static inline OX_Result
do_neg (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s, OX_Value *d)
{
    OX_Result r;

    if (ox_value_is_int64(ctxt, s)) {
        int64_t i = ox_value_get_int64(ctxt, s);

        r = ox_value_set_int64(ctxt, d, -i);
    } else {
        OX_Number n;

        if ((r = ox_to_number(ctxt, s, &n)) == OX_ERR)
            return r;

        ox_value_set_number(ctxt, d, -n);
    }

    return r;
}

/*Type of.*/
static inline OX_Result
do_typeof (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s, OX_Value *d)
{
    return ox_type_of(ctxt, s, d);
}

/*Not null.*/
static inline OX_Result
do_not_null (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s, OX_Value *d)
{
    OX_Bool b = !ox_value_is_null(ctxt, s);

    ox_value_set_bool(ctxt, d, b);
    return OX_OK;
}

/*Add global reference.*/
static inline OX_Result
do_global (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s)
{
    OX_Result r;

    if (ox_value_is_gco(ctxt, s, -1)) {
        if ((r = ox_global_ref(ctxt, s)) == OX_ERR)
            return r;
    }

    return OX_OK;
}

/*Add child to owned object.*/
static inline OX_Result
do_owned (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *c, OX_Value *s)
{
    OX_VS_PUSH(ctxt, owned)
    OX_Stack *se = NULL;
    OX_Stack *se_top, *se_bot;
    OX_Result r;

    se_top = ox_stack_top(ctxt);
    se_bot = se_top - ctxt->s_stack->len;
    while (se_top >= se_bot) {
        if ((se_top->type == OX_STACK_ARRAY) || (se_top->type == OX_STACK_OBJECT)) {
            se = se_top;
            break;
        }
        se_top --;
    }

    assert(se);

    if ((r = ox_get(ctxt, se->s.o.o, OX_STRING(ctxt, _owned), owned)) == OX_ERR)
        goto end;

    if (ox_value_is_null(ctxt, owned)) {
        if ((r = ox_object_new(ctxt, owned, NULL)) == OX_ERR)
            goto end;

        if ((r = ox_set(ctxt, se->s.o.o, OX_STRING(ctxt, _owned), owned)) == OX_ERR)
            goto end;
    }

    if ((r = ox_set(ctxt, se->s.o.o, c, s)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    OX_VS_POP(ctxt, owned)
    return r;
}

/*Get current object.*/
static inline OX_Result
do_curr (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *d)
{
    OX_Stack *se = NULL;
    OX_Stack *se_top, *se_bot;

    se_top = ox_stack_top(ctxt);
    se_bot = se_top - ctxt->s_stack->len;
    while (se_top >= se_bot) {
        if ((se_top->type == OX_STACK_ARRAY) || (se_top->type == OX_STACK_OBJECT)) {
            se = se_top;
            break;
        }
        se_top --;
    }

    assert(se);

    ox_value_copy(ctxt, d, se->s.o.o);
    return OX_OK;
}

/*Get pointer.*/
static inline OX_Result
do_get_ptr (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s, OX_Value *d)
{
    OX_VS_PUSH(ctxt, pty)
    OX_Value *ty;
    OX_CValue *cv;
    OX_CValueInfo cvi;
    OX_CArrayInfo ai;
    OX_Result r;

    if (!ox_value_is_cvalue(ctxt, s)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a C value"));
        goto end;
    }

    ty = ox_cvalue_get_ctype(ctxt, s);

    ox_ctype_get_array_info(ctxt, ty, &ai);

    if ((r = ox_ctype_pointer(ctxt, pty, ty, ai.len)) == OX_ERR)
        goto end;

    cv = ox_value_get_gco(ctxt, s);

    cvi.v.p = &cv->v;
    cvi.base = s;

    r = ox_cvalue_new(ctxt, d, pty, &cvi);
end:
    OX_VS_POP(ctxt, pty)
    return r;
}

/*Get the pointed value.*/
static inline OX_Result
do_get_value (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s, OX_Value *d)
{
    OX_Value *ty;
    OX_CValue *cv;
    OX_CTypeKind kind;
    OX_CValueInfo cvi;
    OX_Result r;

    if (!ox_value_is_cvalue(ctxt, s)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a C value"));
        goto end;
    }

    cv = ox_value_get_gco(ctxt, s);
    kind = ox_ctype_get_kind(ctxt, &cv->cty);

    if (kind == OX_CTYPE_FUNC) {
        r = ox_throw_type_error(ctxt, OX_TEXT("cannot get value of a C function"));
        goto end;
    } else if (kind & OX_CTYPE_FL_NUM) {
        cvi.v.p = &cv->v;
        ty = &cv->cty;
    } else {
        OX_CPtrType *pt = ox_value_get_gco(ctxt, &cv->cty);

        cvi.v.p = cv->v.p;
        ty = &pt->vty;
    }

    r = ox_cptr_get_value(ctxt, ty, &cvi, d);
end:
    return r;
}

/*Set the pointed value.*/
static inline OX_Result
do_set_value (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1)
{
    OX_Value *ty;
    OX_CValue *cv;
    OX_CTypeKind kind;
    OX_CValueInfo cvi;
    OX_Result r;

    if (!ox_value_is_cvalue(ctxt, s0)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a C value"));
        goto end;
    }

    cv = ox_value_get_gco(ctxt, s0);
    kind = ox_ctype_get_kind(ctxt, &cv->cty);

    if (kind == OX_CTYPE_FUNC) {
        r = ox_throw_type_error(ctxt, OX_TEXT("cannot set value of a C function"));
        goto end;
    } else if (kind & OX_CTYPE_FL_NUM) {
        cvi.v.p = &cv->v;
        ty = &cv->cty;
    } else {
        OX_CPtrType *pt = ox_value_get_gco(ctxt, &cv->cty);

        cvi.v.p = cv->v.p;
        ty = &pt->vty;
    }

    r = ox_cptr_set_value(ctxt, ty, &cvi, s1);
end:
    return r;
}

/*Convert the value to 64 bits integer number.*/
static OX_Result
to_int64 (OX_Context *ctxt, OX_Value *v, int64_t *pi)
{
    OX_Result r;

    if (ox_value_is_int64(ctxt, v)) {
        *pi = ox_value_get_int64(ctxt, v);
    } else {
        OX_Number n;

        if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR)
            return r;

        *pi = n;
    }

    return OX_OK;
}

/*Convert the value to 64 bits unsigned integer number.*/
static OX_Result
to_uint64 (OX_Context *ctxt, OX_Value *v, uint64_t *pi)
{
    OX_Result r;

    if (ox_value_is_int64(ctxt, v)) {
        *pi = ox_value_get_uint64(ctxt, v);
    } else {
        OX_Number n;

        if ((r = ox_to_number(ctxt, v, &n)) == OX_ERR)
            return r;

        *pi = n;
    }

    return OX_OK;
}

/*Check if the operation is a binary 64 bits integer operation.*/
static OX_Bool
is_int64_binary (OX_Context *ctxt, OX_Value *v0, OX_Value *v1, OX_CTypeKind *pk0, OX_CTypeKind *pk1)
{
    *pk0 = ox_value_is_int64(ctxt, v0);
    *pk1 = ox_value_is_int64(ctxt, v1);

    if (*pk0 || *pk1)
        return OX_TRUE;

    return OX_FALSE;
}

/*Add.*/
static inline OX_Result
do_add (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *d)
{
    OX_Result r;
    OX_CTypeKind k0, k1;

    if (ox_value_is_string(ctxt, s0) || ox_value_is_string(ctxt, s1)) {
        OX_VS_PUSH_2(ctxt, str0, str1)

        if ((r = ox_to_string(ctxt, s0, str0)) == OX_ERR)
            return r;
        else if ((r = ox_to_string(ctxt, s1, str1)) == OX_ERR)
            return r;
        else if ((r = ox_string_concat(ctxt, str0, str1, d)) == OX_ERR)
            return r;

        OX_VS_POP(ctxt, str0)
    } else if (is_int64_binary(ctxt, s0, s1, &k0, &k1)) {
        if ((k0 == OX_CTYPE_U64) || (k1 == OX_CTYPE_U64)) {
            uint64_t i0, i1;

            if ((r = to_uint64(ctxt, s0, &i0)) == OX_ERR)
                return r;
            if ((r = to_uint64(ctxt, s1, &i1)) == OX_ERR)
                return r;
            if ((r = ox_value_set_uint64(ctxt, d, i0 + i1)) == OX_ERR)
                return r;
        } else {
            int64_t i0, i1;

            if ((r = to_int64(ctxt, s0, &i0)) == OX_ERR)
                return r;
            if ((r = to_int64(ctxt, s1, &i1)) == OX_ERR)
                return r;
            if ((r = ox_value_set_int64(ctxt, d, i0 + i1)) == OX_ERR)
                return r;
        }
    } else {
        OX_Number n0, n1;

        if ((r = ox_to_number(ctxt, s0, &n0)) == OX_ERR)
            return r;

        if ((r = ox_to_number(ctxt, s1, &n1)) == OX_ERR)
            return r;

        ox_value_set_number(ctxt, d, n0 + n1);
    }

    return OX_OK;
}

/*Substract.*/
static inline OX_Result
do_sub (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *d)
{
    OX_CTypeKind k0, k1;
    OX_Result r;

    if (is_int64_binary(ctxt, s0, s1, &k0, &k1)) {
        if ((k0 == OX_CTYPE_U64) || (k1 == OX_CTYPE_U64)) {
            uint64_t i0, i1;

            if ((r = to_uint64(ctxt, s0, &i0)) == OX_ERR)
                return r;
            if ((r = to_uint64(ctxt, s1, &i1)) == OX_ERR)
                return r;
            if ((r = ox_value_set_uint64(ctxt, d, i0 - i1)) == OX_ERR)
                return r;
        } else {
            int64_t i0, i1;

            if ((r = to_int64(ctxt, s0, &i0)) == OX_ERR)
                return r;
            if ((r = to_int64(ctxt, s1, &i1)) == OX_ERR)
                return r;
            if ((r = ox_value_set_int64(ctxt, d, i0 - i1)) == OX_ERR)
                return r;
        }
    } else {
        OX_Number n0, n1;

        if ((r = ox_to_number(ctxt, s0, &n0)) == OX_ERR)
            return r;
        if ((r = ox_to_number(ctxt, s1, &n1)) == OX_ERR)
            return r;

        ox_value_set_number(ctxt, d, n0 - n1);
    }

    return OX_OK;
}

/*Match.*/
static inline OX_Result
do_match (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *d)
{
    OX_VS_PUSH(ctxt, mr)
    OX_Result r;

    r = ox_call_method(ctxt, s0, OX_STRING(ctxt, match), s1, 1, mr);
    if (r == OX_OK) {
        if (!ox_value_is_null(ctxt, mr))
            r = ox_to_string(ctxt, mr, d);
        else
            ox_value_set_null(ctxt, d);
    }

    OX_VS_POP(ctxt, mr)
    return r;
}

/*Power.*/
static inline OX_Result
do_exp (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *d)
{
    OX_Number n0, n1;
    OX_Result r;

    if ((r = ox_to_number(ctxt, s0, &n0)) == OX_ERR)
        return r;

    if ((r = ox_to_number(ctxt, s1, &n1)) == OX_ERR)
        return r;

    ox_value_set_number(ctxt, d, pow(n0, n1));
    return OX_OK;
}

/*Multiply.*/
static inline OX_Result
do_mul (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *d)
{
    OX_CTypeKind k0, k1;
    OX_Result r;

    if (is_int64_binary(ctxt, s0, s1, &k0, &k1)) {
        if ((k0 == OX_CTYPE_U64) || (k1 == OX_CTYPE_U64)) {
            uint64_t i0, i1;

            if ((r = to_uint64(ctxt, s0, &i0)) == OX_ERR)
                return r;
            if ((r = to_uint64(ctxt, s1, &i1)) == OX_ERR)
                return r;
            if ((r = ox_value_set_uint64(ctxt, d, i0 * i1)) == OX_ERR)
                return r;
        } else {
            int64_t i0, i1;

            if ((r = to_int64(ctxt, s0, &i0)) == OX_ERR)
                return r;
            if ((r = to_int64(ctxt, s1, &i1)) == OX_ERR)
                return r;
            if ((r = ox_value_set_int64(ctxt, d, i0 * i1)) == OX_ERR)
                return r;
        }
    } else {
        OX_Number n0, n1;

        if ((r = ox_to_number(ctxt, s0, &n0)) == OX_ERR)
            return r;
        if ((r = ox_to_number(ctxt, s1, &n1)) == OX_ERR)
            return r;

        ox_value_set_number(ctxt, d, n0 * n1);
    }

    return OX_OK;
}

/*Divide.*/
static inline OX_Result
do_div (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *d)
{
    OX_CTypeKind k0, k1;
    OX_Result r;

    if (is_int64_binary(ctxt, s0, s1, &k0, &k1)) {
        if ((k0 == OX_CTYPE_U64) || (k1 == OX_CTYPE_U64)) {
            uint64_t i0, i1;

            if ((r = to_uint64(ctxt, s0, &i0)) == OX_ERR)
                return r;
            if ((r = to_uint64(ctxt, s1, &i1)) == OX_ERR)
                return r;
            if ((r = ox_value_set_uint64(ctxt, d, i0 / i1)) == OX_ERR)
                return r;
        } else {
            int64_t i0, i1;

            if ((r = to_int64(ctxt, s0, &i0)) == OX_ERR)
                return r;
            if ((r = to_int64(ctxt, s1, &i1)) == OX_ERR)
                return r;
            if ((r = ox_value_set_int64(ctxt, d, i0 / i1)) == OX_ERR)
                return r;
        }
    } else {
        OX_Number n0, n1;

        if ((r = ox_to_number(ctxt, s0, &n0)) == OX_ERR)
            return r;
        if ((r = ox_to_number(ctxt, s1, &n1)) == OX_ERR)
            return r;

        ox_value_set_number(ctxt, d, n0 / n1);
    }

    return OX_OK;
}

/*Modulo.*/
static inline OX_Result
do_mod (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *d)
{
    OX_CTypeKind k0, k1;
    OX_Result r;

    if (is_int64_binary(ctxt, s0, s1, &k0, &k1)) {
        if ((k0 == OX_CTYPE_U64) || (k1 == OX_CTYPE_U64)) {
            uint64_t i0, i1;

            if ((r = to_uint64(ctxt, s0, &i0)) == OX_ERR)
                return r;
            if ((r = to_uint64(ctxt, s1, &i1)) == OX_ERR)
                return r;
            if ((r = ox_value_set_uint64(ctxt, d, i0 % i1)) == OX_ERR)
                return r;
        } else {
            int64_t i0, i1;

            if ((r = to_int64(ctxt, s0, &i0)) == OX_ERR)
                return r;
            if ((r = to_int64(ctxt, s1, &i1)) == OX_ERR)
                return r;
            if ((r = ox_value_set_int64(ctxt, d, i0 % i1)) == OX_ERR)
                return r;
        }
    } else {
        OX_Number n0, n1;

        if ((r = ox_to_number(ctxt, s0, &n0)) == OX_ERR)
            return r;
        if ((r = ox_to_number(ctxt, s1, &n1)) == OX_ERR)
            return r;

        ox_value_set_number(ctxt, d, fmod(n0, n1));
    }

    return OX_OK;
}

/*Shift left.*/
static inline OX_Result
do_shl (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *d)
{
    OX_CTypeKind kind;
    OX_Result r;

    if ((kind = ox_value_is_int64(ctxt, s0))) {
        uint64_t i0 = ox_value_get_uint64(ctxt, s0);
        uint32_t i1;

        if ((r = ox_to_uint32(ctxt, s1, &i1)) == OX_ERR)
            return r;
        if (kind == OX_CTYPE_I64)
            r = ox_value_set_int64(ctxt, d, i0 << i1);
        else
            r = ox_value_set_uint64(ctxt, d, i0 << i1);
        if (r == OX_ERR)
            return r;
    } else {
        uint32_t i0, i1;

        if ((r = to_bit_int32(ctxt, s0, &i0)) == OX_ERR)
            return r;
        if ((r = ox_to_uint32(ctxt, s1, &i1)) == OX_ERR)
            return r;

        ox_value_set_number(ctxt, d, i0 << i1);
    }

    return OX_OK;
}

/*Shift right.*/
static inline OX_Result
do_shr (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *d)
{
    OX_CTypeKind kind;
    OX_Result r;

    if ((kind = ox_value_is_int64(ctxt, s0))) {
        int64_t i0 = ox_value_get_int64(ctxt, s0);
        uint32_t i1;

        if ((r = ox_to_uint32(ctxt, s1, &i1)) == OX_ERR)
            return r;
        if (kind == OX_CTYPE_I64)
            r = ox_value_set_int64(ctxt, d, i0 >> i1);
        else
            r = ox_value_set_uint64(ctxt, d, i0 >> i1);
        if (r == OX_ERR)
            return r;
    } else {
        int32_t i0;
        uint32_t i1;

        if ((r = to_bit_int32(ctxt, s0, (uint32_t*)&i0)) == OX_ERR)
            return r;

        if ((r = ox_to_uint32(ctxt, s1, &i1)) == OX_ERR)
            return r;

        ox_value_set_number(ctxt, d, i0 >> i1);
    }

    return OX_OK;
}

/*Unsigned shift right.*/
static inline OX_Result
do_ushr (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *d)
{
    OX_CTypeKind kind;
    OX_Result r;

    if ((kind = ox_value_is_int64(ctxt, s0))) {
        uint64_t i0 = ox_value_get_uint64(ctxt, s0);
        uint32_t i1;

        if ((r = ox_to_uint32(ctxt, s1, &i1)) == OX_ERR)
            return r;
        if (kind == OX_CTYPE_I64)
            r = ox_value_set_int64(ctxt, d, i0 >> i1);
        else
            r = ox_value_set_uint64(ctxt, d, i0 >> i1);
        if (r == OX_ERR)
            return r;
    } else {
        uint32_t i0, i1;
        OX_Result r;

        if ((r = to_bit_int32(ctxt, s0, &i0)) == OX_ERR)
            return r;

        if ((r = ox_to_uint32(ctxt, s1, &i1)) == OX_ERR)
            return r;

        ox_value_set_number(ctxt, d, i0 >> i1);
    }

    return OX_OK;
}

/*Less than.*/
static inline OX_Result
do_lt (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *d)
{
    OX_CTypeKind k0, k1;
    OX_Result r;

    if (ox_value_is_string(ctxt, s0) || ox_value_is_string(ctxt, s1)) {
        OX_VS_PUSH_2(ctxt, str0, str1)

        if ((r = ox_to_string(ctxt, s0, str0)) == OX_ERR)
            return r;
        else if ((r = ox_to_string(ctxt, s1, str1)) == OX_ERR)
            return r;
        else {
            int v = ox_string_compare(ctxt, str0, str1);

            ox_value_set_bool(ctxt, d, v < 0);
        }

        OX_VS_POP(ctxt, str0)
    } else if (is_int64_binary(ctxt, s0, s1, &k0, &k1)) {
        if ((k0 == OX_CTYPE_U64) || (k1 == OX_CTYPE_U64)) {
            uint64_t i0, i1;

            if ((r = to_uint64(ctxt, s0, &i0)) == OX_ERR)
                return r;
            if ((r = to_uint64(ctxt, s1, &i1)) == OX_ERR)
                return r;
            ox_value_set_bool(ctxt, d, i0 < i1);
        } else {
            int64_t i0, i1;

            if ((r = to_int64(ctxt, s0, &i0)) == OX_ERR)
                return r;
            if ((r = to_int64(ctxt, s1, &i1)) == OX_ERR)
                return r;
            ox_value_set_bool(ctxt, d, i0 < i1);
        }
    } else {
        OX_Number n0, n1;

        if ((r = ox_to_number(ctxt, s0, &n0)) == OX_ERR)
            return r;

        if ((r = ox_to_number(ctxt, s1, &n1)) == OX_ERR)
            return r;

        ox_value_set_bool(ctxt, d, n0 < n1);
    }

    return OX_OK;
}

/*Greater than.*/
static inline OX_Result
do_gt (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *d)
{
    OX_CTypeKind k0, k1;
    OX_Result r;

    if (ox_value_is_string(ctxt, s0) || ox_value_is_string(ctxt, s1)) {
        OX_VS_PUSH_2(ctxt, str0, str1)

        if ((r = ox_to_string(ctxt, s0, str0)) == OX_ERR)
            return r;
        else if ((r = ox_to_string(ctxt, s1, str1)) == OX_ERR)
            return r;
        else {
            int v = ox_string_compare(ctxt, str0, str1);

            ox_value_set_bool(ctxt, d, v > 0);
        }

        OX_VS_POP(ctxt, str0)
    } else if (is_int64_binary(ctxt, s0, s1, &k0, &k1)) {
        if ((k0 == OX_CTYPE_U64) || (k1 == OX_CTYPE_U64)) {
            uint64_t i0, i1;

            if ((r = to_uint64(ctxt, s0, &i0)) == OX_ERR)
                return r;
            if ((r = to_uint64(ctxt, s1, &i1)) == OX_ERR)
                return r;
            ox_value_set_bool(ctxt, d, i0 > i1);
        } else {
            int64_t i0, i1;

            if ((r = to_int64(ctxt, s0, &i0)) == OX_ERR)
                return r;
            if ((r = to_int64(ctxt, s1, &i1)) == OX_ERR)
                return r;
            ox_value_set_bool(ctxt, d, i0 > i1);
        }
    } else {
        OX_Number n0, n1;

        if ((r = ox_to_number(ctxt, s0, &n0)) == OX_ERR)
            return r;

        if ((r = ox_to_number(ctxt, s1, &n1)) == OX_ERR)
            return r;

        ox_value_set_bool(ctxt, d, n0 > n1);
    }

    return OX_OK;
}

/*Less than or equal to.*/
static inline OX_Result
do_le (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *d)
{
    OX_CTypeKind k0, k1;
    OX_Result r;

    if (ox_value_is_string(ctxt, s0) || ox_value_is_string(ctxt, s1)) {
        OX_VS_PUSH_2(ctxt, str0, str1)

        if ((r = ox_to_string(ctxt, s0, str0)) == OX_ERR)
            return r;
        else if ((r = ox_to_string(ctxt, s1, str1)) == OX_ERR)
            return r;
        else {
            int v = ox_string_compare(ctxt, str0, str1);

            ox_value_set_bool(ctxt, d, v <= 0);
        }

        OX_VS_POP(ctxt, str0)
    } else if (is_int64_binary(ctxt, s0, s1, &k0, &k1)) {
        if ((k0 == OX_CTYPE_U64) || (k1 == OX_CTYPE_U64)) {
            uint64_t i0, i1;

            if ((r = to_uint64(ctxt, s0, &i0)) == OX_ERR)
                return r;
            if ((r = to_uint64(ctxt, s1, &i1)) == OX_ERR)
                return r;
            ox_value_set_bool(ctxt, d, i0 <= i1);
        } else {
            int64_t i0, i1;

            if ((r = to_int64(ctxt, s0, &i0)) == OX_ERR)
                return r;
            if ((r = to_int64(ctxt, s1, &i1)) == OX_ERR)
                return r;
            ox_value_set_bool(ctxt, d, i0 <= i1);
        }
    } else {
        OX_Number n0, n1;

        if ((r = ox_to_number(ctxt, s0, &n0)) == OX_ERR)
            return r;

        if ((r = ox_to_number(ctxt, s1, &n1)) == OX_ERR)
            return r;

        ox_value_set_bool(ctxt, d, n0 <= n1);
    }

    return OX_OK;
}

/*Greater than or equal to.*/
static inline OX_Result
do_ge (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *d)
{
    OX_CTypeKind k0, k1;
    OX_Result r;

    if (ox_value_is_string(ctxt, s0) || ox_value_is_string(ctxt, s1)) {
        OX_VS_PUSH_2(ctxt, str0, str1)

        if ((r = ox_to_string(ctxt, s0, str0)) == OX_ERR)
            return r;
        else if ((r = ox_to_string(ctxt, s1, str1)) == OX_ERR)
            return r;
        else {
            int v = ox_string_compare(ctxt, str0, str1);

            ox_value_set_bool(ctxt, d, v >= 0);
        }

        OX_VS_POP(ctxt, str0)
    } else if (is_int64_binary(ctxt, s0, s1, &k0, &k1)) {
        if ((k0 == OX_CTYPE_U64) || (k1 == OX_CTYPE_U64)) {
            uint64_t i0, i1;

            if ((r = to_uint64(ctxt, s0, &i0)) == OX_ERR)
                return r;
            if ((r = to_uint64(ctxt, s1, &i1)) == OX_ERR)
                return r;
            ox_value_set_bool(ctxt, d, i0 >= i1);
        } else {
            int64_t i0, i1;

            if ((r = to_int64(ctxt, s0, &i0)) == OX_ERR)
                return r;
            if ((r = to_int64(ctxt, s1, &i1)) == OX_ERR)
                return r;
            ox_value_set_bool(ctxt, d, i0 >= i1);
        }
    } else {
        OX_Number n0, n1;

        if ((r = ox_to_number(ctxt, s0, &n0)) == OX_ERR)
            return r;

        if ((r = ox_to_number(ctxt, s1, &n1)) == OX_ERR)
            return r;

        ox_value_set_bool(ctxt, d, n0 >= n1);
    }

    return OX_OK;
}

/*Instance of.*/
static inline OX_Result
do_instof (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *d)
{
    OX_Result r;

    if ((r = ox_instance_of(ctxt, s0, s1)) == OX_ERR)
        return r;

    ox_value_set_bool(ctxt, d, r);
    return OX_OK;
}

/*Equal to.*/
static inline OX_Result
do_eq (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *d)
{
    OX_Bool b;

    b = ox_equal(ctxt, s0, s1);

    ox_value_set_bool(ctxt, d, b);
    return OX_OK;
}

/*Not equal to.*/
static inline OX_Result
do_ne (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *d)
{
    OX_Bool b;

    b = ox_equal(ctxt, s0, s1);

    ox_value_set_bool(ctxt, d, !b);
    return OX_OK;
}

/*Bit and.*/
static inline OX_Result
do_and (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *d)
{
    OX_CTypeKind k0, k1;
    OX_Result r;

    if (is_int64_binary(ctxt, s0, s1, &k0, &k1)) {
        if ((k0 == OX_CTYPE_U64) || (k1 == OX_CTYPE_U64)) {
            uint64_t i0, i1;

            if ((r = to_uint64(ctxt, s0, &i0)) == OX_ERR)
                return r;
            if ((r = to_uint64(ctxt, s1, &i1)) == OX_ERR)
                return r;
            if ((r = ox_value_set_uint64(ctxt, d, i0 & i1)) == OX_ERR)
                return r;
        } else {
            int64_t i0, i1;

            if ((r = to_int64(ctxt, s0, &i0)) == OX_ERR)
                return r;
            if ((r = to_int64(ctxt, s1, &i1)) == OX_ERR)
                return r;
            if ((r = ox_value_set_int64(ctxt, d, i0 & i1)) == OX_ERR)
                return r;
        }
    } else {
        uint32_t i0, i1;

        if ((r = to_bit_int32(ctxt, s0, &i0)) == OX_ERR)
            return r;
        if ((r = to_bit_int32(ctxt, s1, &i1)) == OX_ERR)
            return r;

        ox_value_set_number(ctxt, d, i0 & i1);
    }

    return OX_OK;
}

/*Bit xor.*/
static inline OX_Result
do_xor (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *d)
{
    OX_CTypeKind k0, k1;
    OX_Result r;

    if (is_int64_binary(ctxt, s0, s1, &k0, &k1)) {
        if ((k0 == OX_CTYPE_U64) || (k1 == OX_CTYPE_U64)) {
            uint64_t i0, i1;

            if ((r = to_uint64(ctxt, s0, &i0)) == OX_ERR)
                return r;
            if ((r = to_uint64(ctxt, s1, &i1)) == OX_ERR)
                return r;
            if ((r = ox_value_set_uint64(ctxt, d, i0 ^ i1)) == OX_ERR)
                return r;
        } else {
            int64_t i0, i1;

            if ((r = to_int64(ctxt, s0, &i0)) == OX_ERR)
                return r;
            if ((r = to_int64(ctxt, s1, &i1)) == OX_ERR)
                return r;
            if ((r = ox_value_set_int64(ctxt, d, i0 ^ i1)) == OX_ERR)
                return r;
        }
    } else {
        uint32_t i0, i1;

        if ((r = to_bit_int32(ctxt, s0, &i0)) == OX_ERR)
            return r;
        if ((r = to_bit_int32(ctxt, s1, &i1)) == OX_ERR)
            return r;

        ox_value_set_number(ctxt, d, i0 ^ i1);
    }

    return OX_OK;
}

/*Bit or.*/
static inline OX_Result
do_or (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *d)
{
    OX_CTypeKind k0, k1;
    OX_Result r;

    if (is_int64_binary(ctxt, s0, s1, &k0, &k1)) {
        if ((k0 == OX_CTYPE_U64) || (k1 == OX_CTYPE_U64)) {
            uint64_t i0, i1;

            if ((r = to_uint64(ctxt, s0, &i0)) == OX_ERR)
                return r;
            if ((r = to_uint64(ctxt, s1, &i1)) == OX_ERR)
                return r;
            if ((r = ox_value_set_uint64(ctxt, d, i0 | i1)) == OX_ERR)
                return r;
        } else {
            int64_t i0, i1;

            if ((r = to_int64(ctxt, s0, &i0)) == OX_ERR)
                return r;
            if ((r = to_int64(ctxt, s1, &i1)) == OX_ERR)
                return r;
            if ((r = ox_value_set_int64(ctxt, d, i0 | i1)) == OX_ERR)
                return r;
        }
    } else {
        uint32_t i0, i1;

        if ((r = to_bit_int32(ctxt, s0, &i0)) == OX_ERR)
            return r;

        if ((r = to_bit_int32(ctxt, s1, &i1)) == OX_ERR)
            return r;

        ox_value_set_number(ctxt, d, i0 | i1);
    }

    return OX_OK;
}

/*Load null.*/
static inline OX_Result
do_load_null (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *d)
{
    ox_value_set_null(ctxt, d);
    return OX_OK;
}

/*Load true.*/
static inline OX_Result
do_load_true (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *d)
{
    ox_value_set_bool(ctxt, d, OX_TRUE);
    return OX_OK;
}

/*Load false.*/
static inline OX_Result
do_load_false (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *d)
{
    ox_value_set_bool(ctxt, d, OX_FALSE);
    return OX_OK;
}

/*Load this argument.*/
static inline OX_Result
do_this (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *d)
{
    ox_value_copy(ctxt, d, &rs->frame->thiz);
    return OX_OK;
}

/*Load bottom frame's this argument.*/
static inline OX_Result
do_this_b (OX_Context *ctxt, OX_RunStatus *rs, int off, OX_Value *d)
{
    ox_value_copy(ctxt, d, &rs->f->frames[off]->thiz);
    return OX_OK;
}

/*Get arguments vector.*/
static inline OX_Result
do_argv (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *d)
{
    if (ox_value_is_null(ctxt, rs->argv)) {
        OX_Result r;

        if ((r = ox_array_new(ctxt, rs->argv, rs->argc)) == OX_ERR)
            return r;

        if (rs->argc) {
            if ((r = ox_array_set_items(ctxt, rs->argv, 0, rs->args, rs->argc)) == OX_ERR)
                return r;
        }
    }
    
    ox_value_copy(ctxt, d, rs->argv);

    return OX_OK;
}

/*Get constant value.*/
static inline OX_Result
do_get_cv (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *c, OX_Value *d)
{
    ox_value_copy(ctxt, d, c);
    return OX_OK;
}

/*Get private property name.*/
static inline OX_Result
do_get_pp (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *p, OX_Value *d)
{
    ox_value_copy(ctxt, d, p);
    return OX_OK;
}

/*Get localized text string.*/
static inline OX_Result
do_get_lt (OX_Context *ctxt, OX_RunStatus *rs, uint16_t t, OX_Value *d)
{
    OX_Value *tv = &rs->s->ts[t];
    OX_Result r;

    if (ox_value_is_null(ctxt, &rs->s->script.text_domain)) {
        ox_value_copy(ctxt, d, tv);
    } else {
        OX_Value *ltv = &rs->s->lts[t];

        if (ox_value_is_null(ctxt, ltv)) {
            const char *domain = ox_string_get_char_star(ctxt, &rs->s->script.text_domain);
            const char *cstr = ox_string_get_char_star(ctxt, tv);
            const char *lcstr;

            lcstr = dgettext(domain, cstr);

            if ((r = ox_string_from_const_char_star(ctxt, ltv, lcstr)) == OX_ERR)
                return r;
        }

        ox_value_copy(ctxt, d, ltv);
    }

    return OX_OK;
}

/*Get localized string template.*/
static inline OX_Result
do_get_ltt (OX_Context *ctxt, OX_RunStatus *rs, uint16_t t, OX_Value *d)
{
    OX_Value *s = &rs->s->tts[t];
    OX_Value *templ = &rs->s->ltts[t];
    OX_VS_PUSH(ctxt, ls)
    OX_Result r;

    if (ox_value_is_null(ctxt, templ)) {
        if (ox_value_is_null(ctxt, &rs->s->script.text_domain)) {
            if ((r = ox_str_templ_from_str(ctxt, templ, s)) == OX_ERR)
                goto end;
        } else {
            const char *domain = ox_string_get_char_star(ctxt, &rs->s->script.text_domain);
            const char *cstr = ox_string_get_char_star(ctxt, s);
            const char *lcstr;

            lcstr = dgettext(domain, cstr);

            if ((r = ox_string_from_const_char_star(ctxt, ls, lcstr)) == OX_ERR)
                goto end;

            if ((r = ox_str_templ_from_str(ctxt, templ, ls)) == OX_ERR)
                goto end;
        }
    }

    ox_value_copy(ctxt, d, templ);
    r = OX_OK;
end:
    OX_VS_POP(ctxt, ls);
    return r;
}

/*Get local declaration.*/
static inline OX_Result
do_get_t (OX_Context *ctxt, OX_RunStatus *rs, int id, OX_Value *d)
{
    OX_Frame *f = ctxt->frames;

    ox_value_copy(ctxt, d, &f->v[id]);
    return OX_OK;
}

/*Set local declaration.*/
static inline OX_Result
do_set_t (OX_Context *ctxt, OX_RunStatus *rs, int id, OX_Value *s)
{
    OX_Frame *f = ctxt->frames;

    ox_value_copy(ctxt, &f->v[id], s);
    return OX_OK;
}

/*Set local auto closed declaration.*/
static inline OX_Result
do_set_t_ac (OX_Context *ctxt, OX_RunStatus *rs, int id, OX_Value *s)
{
    OX_Frame *f = ctxt->frames;
    OX_Result r;

    if (!ox_equal(ctxt, &f->v[id], s)) {
        if ((r = auto_close(ctxt, &f->v[id])) == OX_ERR)
            return r;

        ox_value_copy(ctxt, &f->v[id], s);
    }

    return OX_OK;
}

/*Get local declaration in bottom frame.*/
static inline OX_Result
do_get_t_b (OX_Context *ctxt, OX_RunStatus *rs, int off, int id, OX_Value *d)
{
    ox_value_copy(ctxt, d, &rs->f->frames[off]->v[id]);
    return OX_OK;
}

/*Set local declaration in bottom frame.*/
static inline OX_Result
do_set_t_b (OX_Context *ctxt, OX_RunStatus *rs, int off, int id, OX_Value *s)
{
    ox_value_copy(ctxt, &rs->f->frames[off]->v[id], s);
    return OX_OK;
}

/*Set local auto closed declaration in bottom frame.*/
static inline OX_Result
do_set_t_b_ac (OX_Context *ctxt, OX_RunStatus *rs, int off, int id, OX_Value *s)
{
    OX_Result r;

    if (!ox_equal(ctxt, &rs->f->frames[off]->v[id], s)) {
        if ((r = auto_close(ctxt, &rs->f->frames[off]->v[id])) == OX_ERR)
            return r;

        ox_value_copy(ctxt, &rs->f->frames[off]->v[id], s);
    }

    return OX_OK;
}

/*Get declaration by name.*/
static inline OX_Result
do_get_n (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *c, OX_Value *d)
{
    OX_String *k;
    OX_ScriptFunc *sf = rs->s->sfuncs;
    OX_ScriptDecl *sdecl;
    OX_Result r;
    uint8_t *bc;

    if ((r = ox_string_singleton(ctxt, c)) == OX_ERR)
        return r;

    k = ox_value_get_gco(ctxt, c);

    bc = &rs->s->bc[rs->sf->bc_start + rs->frame->ip];

    sdecl = ox_hash_lookup_c(ctxt, &sf->decl_hash, k, NULL, OX_ScriptDecl, he);
    if (sdecl) {
        bc[0] = OX_BC_get_r;
        bc[1] = sdecl->id >> 8;
        bc[2] = sdecl->id & 0xff;

        ox_value_copy(ctxt, d, &rs->s->script.frame->v[sdecl->id]);
        r = OX_OK;
    } else {
        bc[0] = OX_BC_get_g;

        r = ox_get_throw(ctxt, OX_OBJECT(ctxt, Global), c, d);
    }

    return r;
}

/*Get global declaration.*/
static inline OX_Result
do_get_g (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *c, OX_Value *d)
{
    return ox_get_throw(ctxt, OX_OBJECT(ctxt, Global), c, d);
}

/*Get referenced declaration.*/
static inline OX_Result
do_get_r (OX_Context *ctxt, OX_RunStatus *rs, int id, OX_Value *d)
{
    ox_value_copy(ctxt, d, &rs->s->script.frame->v[id]);
    return OX_OK;
}

/*Get property.*/
static inline OX_Result
do_get_p (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *d)
{
    return ox_get(ctxt, s0, s1, d);
}

/*Lookup property.*/
static inline OX_Result
do_lookup_p (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *d)
{
    return ox_lookup(ctxt, s0, s1, d);
}

/*Set property.*/
static inline OX_Result
do_set_p (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *s2)
{
    return ox_set(ctxt, s0, s1, s2);
}

/*Get an argument.*/
static inline OX_Result
do_get_a (OX_Context *ctxt, OX_RunStatus *rs, int id, OX_Value *d)
{
    if (id >= rs->argc)
        ox_value_set_null(ctxt, d);
    else
        ox_value_copy(ctxt, d, ox_values_item(ctxt, rs->args, id));

    return OX_OK;
}

/*Throw an error.*/
static inline OX_Result
do_throw (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s)
{
    return ox_throw(ctxt, s);
}

/*Return from the function.*/
static inline OX_Result
do_ret (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s)
{
    ox_value_copy(ctxt, rs->rv, s);
    return OX_RETURN;
}

/*Jump.*/
static inline OX_Result
do_jmp (OX_Context *ctxt, OX_RunStatus *rs, int l)
{
    rs->frame->ip = l;
    return OX_JUMP;
}

/*Deep jump.*/
static inline OX_Result
do_deep_jmp (OX_Context *ctxt, OX_RunStatus *rs, int off, int l)
{
    rs->jmp_ip = l;
    rs->jmp_sp = ctxt->s_stack->len - off;
    return OX_DEEP_JUMP;
}

/*Jump if true.*/
static inline OX_Result
do_jt (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s, int l)
{
    OX_Bool b = ox_to_bool(ctxt, s);

    if (b) {
        rs->frame->ip = l;
        return OX_JUMP;
    }

    return OX_TRUE;
}

/*Jump if false.*/
static inline OX_Result
do_jf (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s, int l)
{
    OX_Bool b = ox_to_bool(ctxt, s);

    if (!b) {
        rs->frame->ip = l;
        return OX_JUMP;
    }

    return OX_TRUE;
}

/*Jump if the value is not null.*/
static inline OX_Result
do_jnn (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s, int l)
{
    OX_Bool b = ox_value_is_null(ctxt, s);

    if (!b) {
        rs->frame->ip = l;
        return OX_JUMP;
    }

    return OX_TRUE;
}

/*Multipart string start.*/
static inline OX_Result
do_str_start (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s)
{
    OX_Stack *se = ox_stack_push(ctxt, OX_STACK_STR);

    se->s.s.f = ox_value_stack_top(ctxt);
    se->s.s.args = ox_value_stack_push(ctxt);
    se->s.s.argc = 1;

    ox_value_copy(ctxt, se->s.s.args, s);

    return OX_OK;
}

/*Multipart string start with template function.*/
static inline OX_Result
do_str_start_t (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1)
{
    OX_Stack *se = ox_stack_push(ctxt, OX_STACK_STR);

    se->s.s.f = ox_value_stack_push_n(ctxt, 2);
    se->s.s.args = ox_values_item(ctxt, se->s.s.f, 1);
    se->s.s.argc = 1;

    ox_value_copy(ctxt, se->s.s.f, s1);
    ox_value_copy(ctxt, se->s.s.args, s0);

    return OX_OK;
}

/*Add a multipart string's item.*/
static inline OX_Result
do_str_item (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s)
{
    OX_Stack *se = ox_stack_top(ctxt);
    OX_Value *arg = ox_value_stack_push(ctxt);
    OX_Result r;

    se->s.s.argc ++;

    if ((r = ox_to_string(ctxt, s, arg)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    return r;
}

/*Add a multipart string's item with flags.*/
static inline OX_Result
do_str_item_f (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *c, OX_Value *s)
{
    OX_Stack *se = ox_stack_top(ctxt);
    OX_Value *arg = ox_value_stack_push(ctxt);
    int flags;

    se->s.s.argc ++;

    flags = ox_value_get_number(ctxt, c);

    return ox_to_string_format(ctxt, s, arg, flags);
}

/*Concatente the strings.*/
static OX_Result
str_concat (OX_Context *ctxt, OX_Stack *se, OX_Value *d)
{
    OX_Value *item = ox_value_stack_push(ctxt);
    OX_Value *templ;
    OX_CharBuffer cb;
    size_t len, i;
    OX_Result r;

    ox_char_buffer_init(&cb);

    templ = se->s.s.args;

    len = ox_array_length(ctxt, templ);

    for (i = 0; i < len; i ++) {
        if ((r = ox_array_get_item(ctxt, templ, i, item)) == OX_ERR)
            goto end;
        if (!ox_value_is_null(ctxt, item)) {
            if ((r = ox_char_buffer_append_string(ctxt, &cb, item)) == OX_ERR)
                goto end;
        }

        if (i != len - 1) {
            OX_Value *arg = ox_values_item(ctxt, se->s.s.args, i + 1);

            if ((r = ox_char_buffer_append_string(ctxt, &cb, arg)) == OX_ERR)
                goto end;
        }
    }

    if ((r = ox_char_buffer_get_string(ctxt, &cb, d)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    ox_char_buffer_deinit(ctxt, &cb);
    return r;
}

/*Multipart string end.*/
static inline OX_Result
do_str_end (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *d)
{
    OX_Stack *se = ox_stack_top(ctxt);
    OX_Result r;

    if (se->s.s.f == se->s.s.args) {
        r = str_concat(ctxt, se, d);
    } else {
        r = ox_call(ctxt, se->s.s.f, ox_value_null(ctxt), se->s.s.args, se->s.s.argc, d);
    }

    if (r == OX_OK)
        ox_stack_pop(ctxt);

    return r;
}

/*Call start.*/
static inline OX_Result
do_call_start (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1)
{
    OX_Stack *se = ox_stack_push(ctxt, OX_STACK_CALL);

    se->s.c.f = ox_value_stack_push_n(ctxt, 3);
    se->s.c.thiz = ox_values_item(ctxt, se->s.c.f, 1);
    se->s.c.iter = ox_values_item(ctxt, se->s.c.f, 2);
    se->s.c.args = ox_value_stack_top(ctxt);
    se->s.c.argc = 0;

    ox_value_copy(ctxt, se->s.c.f, s0);
    ox_value_copy(ctxt, se->s.c.thiz, s1);

    return OX_OK;
}

/*Add an argument.*/
static inline OX_Result
do_arg (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s)
{
    OX_Stack *se = ox_stack_top(ctxt);
    OX_Value *dst = ox_value_stack_push(ctxt);

    ox_value_copy(ctxt, dst, s);
    se->s.c.argc ++;

    return OX_OK;
}

/*Add arguments from iterator.*/
static inline OX_Result
do_arg_spread (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s)
{
    OX_Stack *se = ox_stack_top(ctxt);
    OX_Bool allocted = OX_FALSE;
    OX_Value *arg;
    OX_Result r;

    if ((r = ox_iterator_new(ctxt, se->s.c.iter, s)) == OX_ERR)
        goto end;
    allocted = OX_TRUE;

    while (1) {
        if ((r = ox_iterator_end(ctxt, se->s.c.iter)) == OX_ERR)
            goto end;

        if (r)
            break;

        arg = ox_value_stack_push(ctxt);
        if ((r = ox_iterator_value(ctxt, se->s.c.iter, arg)) == OX_ERR)
            goto end;

        se->s.c.argc ++;

        if ((r = ox_iterator_next(ctxt, se->s.c.iter)) == OX_ERR)
            goto end;
    }
end:
    if (allocted)
        r |= ox_close(ctxt, se->s.c.iter);
    return r;
}

/*Call a function.*/
static inline OX_Result
call_func (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Result r;

    if (ox_value_is_gco(ctxt, f, OX_GCO_FUNCTION)) {
        OX_Stack *ret;
        OX_Frame *frame;
        OX_Function *func;

        ret = ox_stack_push(ctxt, OX_STACK_RETURN);

        run_status_to_rec(ctxt, rs, &ret->s.r);

        ret->s.r.frame = ctxt->frames;
        ret->s.r.args = rs->args;
        ret->s.r.argc = rs->argc;
        ret->s.r.rv = rs->rv;
        ret->s.r.vp = OX_VALUE_PTR2IDX(rs->regs);
        ret->s.r.sp = rs->sp;

        func = ox_value_get_gco(ctxt, f);
        frame = ox_frame_push(ctxt, f, func->sfunc->decl_hash.e_num);

        rs->f = ox_value_get_gco(ctxt, f);
        rs->thiz = thiz;
        rs->args = args;
        rs->argc = argc;
        rs->rv = rv;
        rs->frame = frame;
        rs->frame->ip = 0;
        rs->sf = rs->f->sfunc;
        rs->s = rs->sf->script;
        rs->regs = ox_value_stack_push_n(ctxt, rs->sf->reg_num + 1);
        rs->argv = ox_values_item(ctxt, rs->regs, rs->sf->reg_num);
        rs->sp = ctxt->s_stack->len;

        ox_value_copy(ctxt, &rs->frame->thiz, thiz);

        return OX_JUMP;
    } else {
        if ((r = ox_call(ctxt, f, thiz, args, argc, rv)) == OX_ERR)
            return r;

        if ((r = ox_stack_pop(ctxt)) == OX_ERR)
            return r;

        return OX_OK;
    }
}

/*Call end.*/
static inline OX_Result
do_call_end (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *d)
{
    OX_Stack *se = ox_stack_top(ctxt);

    return call_func(ctxt, rs, se->s.c.f, se->s.c.thiz, se->s.c.args, se->s.c.argc, d);
}

/*Call end tail.*/
static inline OX_Result
do_call_end_tail (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *d)
{
    OX_Stack *se = ox_stack_top(ctxt);

    if (ox_equal(ctxt, se->s.c.f, &rs->frame->func)) {
        size_t base = OX_VALUE_PTR2IDX(rs->argv);
        size_t src, dst;

        src = OX_VALUE_PTR2IDX(se->s.c.thiz);
        dst = base + 1;
        rs->thiz = OX_VALUE_IDX2PTR(dst);
        if (src != dst)
            memmove(ctxt->base.v_stack->items + dst, ctxt->base.v_stack->items + src, sizeof(OX_Value));

        src = OX_VALUE_PTR2IDX(se->s.c.args);
        dst ++;
        rs->args = OX_VALUE_IDX2PTR(dst);
        rs->argc = se->s.c.argc;
        if ((src != dst) && rs->argc)
            memmove(ctxt->base.v_stack->items + dst, ctxt->base.v_stack->items + src, sizeof(OX_Value) * rs->argc);

        while (ctxt->s_stack->len > rs->sp)
            ox_stack_pop(ctxt);

        rs->frame->ip = 0;
        ctxt->base.v_stack->len = dst + rs->argc;

        ox_value_set_null(ctxt, rs->argv);
        ox_values_set_null(ctxt, rs->regs, rs->sf->reg_num);

        return OX_JUMP;
    }
    
    return call_func(ctxt, rs, se->s.c.f, se->s.c.thiz, se->s.c.args, se->s.c.argc, d);
}

/*Try start.*/
static inline OX_Result
do_try_start (OX_Context *ctxt, OX_RunStatus *rs, int l0, int l1)
{
    OX_Stack *se = ox_stack_push(ctxt, OX_STACK_TRY);

    se->s.t.state = OX_TRY_STATE_TRY;
    se->s.t.r = OX_OK;
    se->s.t.catch_label = l0;
    se->s.t.finally_label = l1;
    se->s.t.jmp_ip = 0;
    se->s.t.jmp_sp = 0;

    return OX_OK;
}

/*Try end.*/
static inline OX_Result
do_try_end (OX_Context *ctxt, OX_RunStatus *rs)
{
    OX_Stack *se = ox_stack_top(ctxt);

    se->s.t.state = OX_TRY_STATE_FINALLY;
    rs->frame->ip = se->s.t.finally_label;

    return OX_JUMP;
}

/*Catch an error.*/
static inline OX_Result
do_catch (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *d)
{
    OX_Stack *se = ox_stack_top(ctxt);

    se->s.t.r = OX_OK;
    ox_catch(ctxt, d);

    return OX_OK;
}

/*Catch end.*/
static inline OX_Result
do_catch_end (OX_Context *ctxt, OX_RunStatus *rs)
{
    OX_Stack *se = ox_stack_top(ctxt);

    se->s.t.state = OX_TRY_STATE_FINALLY;

    return OX_OK;
}

/*Finally.*/
static inline OX_Result
do_finally (OX_Context *ctxt, OX_RunStatus *rs)
{
    OX_Stack *se = ox_stack_top(ctxt);
    OX_Result r = se->s.t.r;

    rs->jmp_ip = se->s.t.jmp_ip;
    rs->jmp_sp = se->s.t.jmp_sp;

    ox_stack_pop(ctxt);

    return r;
}

/*Schedule.*/
static inline OX_Result
do_sched (OX_Context *ctxt, OX_RunStatus *rs)
{
    ox_unlock(ctxt);
    ox_thread_yield();
    ox_lock(ctxt);

    return OX_OK;
}

/*Schedule start.*/
static inline OX_Result
do_sched_start (OX_Context *ctxt, OX_RunStatus *rs)
{
    ox_stack_push(ctxt, OX_STACK_SCHED);
    ctxt->base.sched_cnt ++;
    return OX_OK;
}

/*Yield.*/
static inline OX_Result
do_yield (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s, OX_Value *d)
{
    OX_Fiber *fiber = rs->fiber;

    if (!fiber)
        return ox_throw_reference_error(ctxt, OX_TEXT("\"yield\" must be used in fiber"));

    fiber->yr = d;
    ox_value_copy(ctxt, &fiber->rv, s);

    return OX_YIELD;
}

/*Pop a state entry from the stack*/
static inline OX_Result
do_s_pop (OX_Context *ctxt, OX_RunStatus *rs)
{
    return ox_stack_pop(ctxt);
}

/*Iterator start.*/
static inline OX_Result
do_iter_start (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s)
{
    OX_Stack *se = ox_stack_push(ctxt, OX_STACK_ITER);
    OX_Result r;

    se->s.i.iter = ox_value_stack_push(ctxt);

    if ((r = ox_iterator_new(ctxt, se->s.i.iter, s)) == OX_ERR) {
        ctxt->s_stack->len --;
        return r;
    }

    return OX_OK;
}

/*Iterator step.*/
static inline OX_Result
do_iter_step (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *d, int l)
{
    OX_Stack *se = ox_stack_top(ctxt);
    OX_Result r;

    if ((r = ox_iterator_end(ctxt, se->s.i.iter)) == OX_ERR)
        return r;

    if (r) {
        if ((r = ox_stack_pop(ctxt)) == OX_ERR)
            return r;

        rs->frame->ip = l;
        return OX_JUMP;
    }

    if ((r = ox_iterator_value(ctxt, se->s.i.iter, d)) == OX_ERR)
        return r;

    if ((r = ox_iterator_next(ctxt, se->s.i.iter)) == OX_ERR)
        return r;

    return OX_OK;
}

/*Array pattern start.*/
static inline OX_Result
do_apat_start (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s)
{
    OX_Stack *se = ox_stack_push(ctxt, OX_STACK_APAT);

    se->s.a.a = ox_value_stack_push(ctxt);
    se->s.a.id = 0;

    ox_value_copy(ctxt, se->s.a.a, s);

    return OX_OK;
}

/*Array pattern move to next item.*/
static inline OX_Result
do_apat_next (OX_Context *ctxt, OX_RunStatus *rs)
{
    OX_Stack *se = ox_stack_top(ctxt);

    se->s.a.id ++;

    return OX_OK;
}

/*Get item from the array pattern.*/
static inline OX_Result
do_apat_get (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *d)
{
    OX_Stack *se = ox_stack_top(ctxt);
    OX_VS_PUSH(ctxt, k)
    OX_Result r;

    ox_value_set_number(ctxt, k, se->s.a.id);

    if ((r = ox_get(ctxt, se->s.a.a, k, d)) == OX_ERR)
        goto end;

    se->s.a.id ++;

    r = OX_OK;
end:
    OX_VS_POP(ctxt, k)
    return r;
}

/*Get rest items of the array pattern.*/
static inline OX_Result
do_apat_rest (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *d)
{
    OX_Stack *se = ox_stack_top(ctxt);
    OX_VS_PUSH_2(ctxt, k, lv)
    OX_Number n;
    size_t len, alen;
    OX_Result r;

    if ((r = ox_get(ctxt, se->s.a.a, OX_STRING(ctxt, length), lv)) == OX_ERR)
        goto end;

    if ((r = ox_to_number(ctxt, lv, &n)) == OX_ERR)
        goto end;

    len = n;

    if (se->s.a.id < len)
        alen = len - se->s.a.id;
    else
        alen = 0;

    if ((r = ox_array_new(ctxt, d, alen)) == OX_ERR)
        goto end;

    while (se->s.a.id < len) {
        ox_value_set_number(ctxt, k, se->s.a.id);

        if ((r = ox_get(ctxt, se->s.a.a, k, d)) == OX_ERR)
            goto end;

        se->s.a.id ++;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, k)
    return r;
}

/*Object pattern start.*/
static inline OX_Result
do_opat_start (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s)
{
    OX_Stack *se = ox_stack_push(ctxt, OX_STACK_OPAT);

    se->s.o.o = ox_value_stack_push(ctxt);
    ox_size_hash_init(&se->s.o.p_hash);

    ox_value_copy(ctxt, se->s.o.o, s);

    return OX_OK;
}

/*Object pattern get a property.*/
static inline OX_Result
do_opat_get (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s, OX_Value *d)
{
    OX_Stack *se = ox_stack_top(ctxt);
    OX_ValueEntry *e;
    OX_HashEntry **pe;
    OX_Result r;

    if ((r = ox_get(ctxt, se->s.o.o, s, d)) == OX_ERR)
        return r;

    e = ox_hash_lookup_c(ctxt, &se->s.o.p_hash, s, &pe, OX_ValueEntry, he);
    if (!e) {
        if (!OX_NEW(ctxt, e))
            return ox_throw_no_mem_error(ctxt);

        ox_value_copy(ctxt, &e->k, s);
        ox_value_copy(ctxt, &e->v, d);

        if ((r = ox_hash_insert(ctxt, &se->s.o.p_hash, &e->k, &e->he, pe)) == OX_ERR)
            return r;
    }

    return OX_OK;
}

/*Create object from rest properties of the object pattern.*/
static inline OX_Result
do_opat_rest (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *d)
{
    OX_Stack *se = ox_stack_top(ctxt);
    OX_VS_PUSH_4(ctxt, iter, e, k, v)
    OX_Bool allocated = OX_FALSE;
    OX_Result r;

    if (!ox_value_is_object(ctxt, se->s.o.o)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not an object"));
        goto end;
    }

    if ((r = ox_object_new(ctxt, d, NULL)) == OX_ERR)
        goto end;

    if ((r = ox_object_iter_new(ctxt, iter, se->s.o.o, OX_OBJECT_ITER_KEY_VALUE)) == OX_ERR)
        goto end;
    allocated = OX_TRUE;

    while (1) {
        if ((r = ox_iterator_end(ctxt, iter)) == OX_ERR)
            goto end;

        if (r)
            break;

        if ((r = ox_iterator_value(ctxt, iter, e)) == OX_ERR)
            goto end;

        if ((r = ox_array_get_item(ctxt, e, 0, k)) == OX_ERR)
            goto end;

        if (!ox_hash_lookup(ctxt, &se->s.o.p_hash, k, NULL)) {
            if ((r = ox_array_get_item(ctxt, e, 1, v)) == OX_ERR)
                goto end;

            if ((r = ox_set(ctxt, d, k, v)) == OX_ERR)
                goto end;
        }

        if ((r = ox_iterator_next(ctxt, iter)) == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    if (allocated)
        r |= ox_close(ctxt, iter);
    OX_VS_POP(ctxt, iter)
    return r;
}

/*Create a new array.*/
static inline OX_Result
do_a_new (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *d)
{
    return ox_array_new(ctxt, d, 0);
}

/*Start array setting*/
static inline OX_Result
do_a_start (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s)
{
    OX_Stack *se = ox_stack_push(ctxt, OX_STACK_ARRAY);
    OX_Value *v;
    size_t id = 0;
    OX_Result r;

    se->s.a.a = ox_value_stack_push_n(ctxt, 2);
    v = ox_values_item(ctxt, se->s.a.a, 1);
    ox_value_copy(ctxt, se->s.a.a, s);

    if ((r = ox_get(ctxt, s, OX_STRING(ctxt, length), v)) == OX_ERR)
        goto end;

    if ((r = ox_to_index(ctxt, v, &id)) == OX_ERR)
        goto end;

    se->s.a.id = id;
end:
    ox_value_stack_pop(ctxt, v);
    return r;
}

/*Move array item index to the next position.*/
static inline OX_Result
do_a_next (OX_Context *ctxt, OX_RunStatus *rs)
{
    OX_Stack *se = ox_stack_top(ctxt);

    se->s.a.id ++;

    return OX_OK;
}

/*Add an item to the array.*/
static inline OX_Result
do_a_item (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s)
{
    OX_Stack *se = ox_stack_top(ctxt);
    OX_VS_PUSH(ctxt, k)
    OX_Result r;

    ox_value_set_number(ctxt, k, se->s.a.id);

    if ((r = ox_set(ctxt, se->s.a.a, k, s)) == OX_ERR)
        goto end;

    se->s.a.id ++;
    r = OX_OK;
end:
    OX_VS_POP(ctxt, k)
    return r;
}

/*Add items to the array.*/
static inline OX_Result
do_a_spread (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s)
{
    OX_Stack *se = ox_stack_top(ctxt);
    OX_VS_PUSH_3(ctxt, iter, k, v)
    OX_Bool allocated = OX_FALSE;
    OX_Result r;

    if ((r = ox_iterator_new(ctxt, iter, s)) == OX_ERR)
        goto end;

    while (1) {
        if ((r = ox_iterator_end(ctxt, iter)) == OX_ERR)
            goto end;

        if (r)
            break;

        if ((r = ox_iterator_value(ctxt, iter, v)) == OX_ERR)
            goto end;

        ox_value_set_number(ctxt, k, se->s.a.id);

        if ((r = ox_set(ctxt, se->s.a.a, k, v)) == OX_ERR)
            goto end;

        se->s.a.id ++;

        if ((r = ox_iterator_next(ctxt, iter)) == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    if (allocated)
        r |= ox_close(ctxt, iter);
    OX_VS_POP(ctxt, iter)
    return r;
}

/*Array setting end.*/
static inline OX_Result
do_a_end (OX_Context *ctxt, OX_RunStatus *rs)
{
    OX_Stack *se = ox_stack_top(ctxt);
    OX_Result r;

    if (ox_value_is_array(ctxt, se->s.a.a)) {
        if ((r = ox_array_set_length(ctxt, se->s.a.a, se->s.a.id)) == OX_ERR)
            return r;
    }

    return ox_stack_pop(ctxt);
}

/*Create a new object.*/
static inline OX_Result
do_o_new (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *d)
{
    return ox_object_new(ctxt, d, NULL);
}

/*Start an object setting.*/
static inline OX_Result
do_o_start (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s)
{
    OX_Stack *se = ox_stack_push(ctxt, OX_STACK_OBJECT);

    se->s.o.o = ox_value_stack_push(ctxt);

    ox_value_copy(ctxt, se->s.o.o, s);
    ox_size_hash_init(&se->s.o.p_hash);

    return OX_OK;
}

/*Set the property of the object.*/
static inline OX_Result
do_o_prop (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1)
{
    OX_Stack *se = ox_stack_top(ctxt);

    return ox_set(ctxt, se->s.o.o, s0, s1);
}

/*Set the properties of the object.*/
static inline OX_Result
do_o_spread (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s)
{
    OX_Stack *se = ox_stack_top(ctxt);
    OX_VS_PUSH_4(ctxt, iter, e, k, v)
    OX_Bool allocated = OX_FALSE;
    OX_Result r;

    if (!ox_value_is_object(ctxt, s)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not an object"));
        goto end;
    }

    if ((r = ox_object_iter_new(ctxt, iter, s, OX_OBJECT_ITER_KEY_VALUE)) == OX_ERR)
        goto end;
    allocated = OX_TRUE;

    while (1) {
        if ((r = ox_iterator_end(ctxt, iter)) == OX_ERR)
            goto end;

        if (r)
            break;

        if ((r = ox_iterator_value(ctxt, iter, e)) == OX_ERR)
            goto end;

        if ((r = ox_array_get_item(ctxt, e, 0, k)) == OX_ERR)
            goto end;

        if ((r = ox_array_get_item(ctxt, e, 1, v)) == OX_ERR)
            goto end;

        if ((r = ox_set(ctxt, se->s.o.o, k, v)) == OX_ERR)
            goto end;

        if ((r = ox_iterator_next(ctxt, iter)) == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    if (allocated)
        r |= ox_close(ctxt, iter);
    OX_VS_POP(ctxt, iter)
    return r;
}

/*Start parameters setting.*/
static inline OX_Result
do_p_start (OX_Context *ctxt, OX_RunStatus *rs)
{
    OX_Stack *se = ox_stack_push(ctxt, OX_STACK_PARAM);

    se->s.p.id = 0;

    return OX_OK;
}

/*Get a parameter.*/
static inline OX_Result
do_p_get (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *d)
{
    OX_Stack *se = ox_stack_top(ctxt);

    if (se->s.p.id >= rs->argc) {
        ox_value_set_null(ctxt, d);
    } else {
        ox_value_copy(ctxt, d, ox_values_item(ctxt, rs->args, se->s.p.id));

        se->s.p.id ++;
    }

    return OX_OK;
}

/*Get the rest parameters.*/
static inline OX_Result
do_p_rest (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *d)
{
    OX_Stack *se = ox_stack_top(ctxt);
    size_t len, id;
    OX_Result r;

    if (se->s.p.id >= rs->argc)
        len = 0;
    else
        len = rs->argc - se->s.p.id;

    if ((r = ox_array_new(ctxt, d, len)) == OX_ERR)
        return r;

    id = 0;

    while (se->s.p.id < rs->argc) {
        if ((r = ox_array_set_item(ctxt, d, id, ox_values_item(ctxt, rs->args, se->s.p.id))) == OX_ERR)
            return r;

        se->s.p.id ++;
        id ++;
    }

    return OX_OK;
}

/*Create a new function.*/
static inline OX_Result
do_f_new (OX_Context *ctxt, OX_RunStatus *rs, int fid, OX_Value *d)
{
    OX_ScriptFunc *sf = &rs->s->sfuncs[fid];

    return ox_function_new(ctxt, d, sf);
}

/*Create a new class.*/
static inline OX_Result
do_c_new (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *d0, OX_Value *d1)
{
    return ox_class_new(ctxt, d0, d1);
}

/*Add a parent class.*/
static inline OX_Result
do_c_parent (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1)
{
    return ox_class_inherit(ctxt, s0, s1);
}

/*Add a constant property to the class.*/
static inline OX_Result
do_c_const (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *s2)
{
    OX_VS_PUSH(ctxt, k)
    OX_Result r;

    if ((r = ox_to_string(ctxt, s1, k)) == OX_ERR)
        goto end;

    r = ox_object_add_const(ctxt, s0, k, s2);
end:
    OX_VS_POP(ctxt, k)
    return r;
}

/*Add a variable property to the class.*/
static inline OX_Result
do_c_var (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *s2)
{
    OX_VS_PUSH(ctxt, k)
    OX_Result r;

    if ((r = ox_to_string(ctxt, s1, k)) == OX_ERR)
        goto end;

    r = ox_object_add_var(ctxt, s0, k, s2);
end:
    OX_VS_POP(ctxt, k)
    return r;
}

/*Add an accessor property to the class.*/
static inline OX_Result
do_c_acce (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *s2, OX_Value *s3)
{
    OX_VS_PUSH(ctxt, k)
    OX_Result r;

    if ((r = ox_to_string(ctxt, s1, k)) == OX_ERR)
        goto end;

    r = ox_object_add_accessor(ctxt, s0, k, s2, s3);
end:
    OX_VS_POP(ctxt, k)
    return r;
}

/*Add a readonly accessor property to the class.*/
static inline OX_Result
do_c_ro_acce (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1, OX_Value *s2)
{
    OX_VS_PUSH(ctxt, k)
    OX_Result r;

    if ((r = ox_to_string(ctxt, s1, k)) == OX_ERR)
        goto end;

    r = ox_object_add_accessor(ctxt, s0, k, s2, NULL);
end:
    OX_VS_POP(ctxt, k)
    return r;
}

/*Enumeration start.*/
static inline OX_Result
do_e_start (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s)
{
    OX_Stack *se = ox_stack_push(ctxt, OX_STACK_ENUM);
    OX_Result r;

    se->s.e.c = ox_value_stack_push_n(ctxt, 2);
    se->s.e.e = ox_values_item(ctxt, se->s.e.c, 1);

    ox_value_copy(ctxt, se->s.e.c, s);
    ox_value_set_null(ctxt, se->s.e.e);
    se->s.e.v = 0;

    if ((r = ox_enum_new(ctxt, se->s.e.e, OX_ENUM_ENUM)) == OX_ERR)
        return r;

    return OX_OK;
}

/*Enumeration start with object name.*/
static inline OX_Result
do_e_start_n (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *c, OX_Value *s)
{
    OX_Stack *se = ox_stack_push(ctxt, OX_STACK_ENUM);
    OX_Result r;

    se->s.e.c = ox_value_stack_push_n(ctxt, 2);
    se->s.e.e = ox_values_item(ctxt, se->s.e.c, 1);

    ox_value_copy(ctxt, se->s.e.c, s);
    ox_value_set_null(ctxt, se->s.e.e);
    se->s.e.v = 0;

    if ((r = ox_enum_new(ctxt, se->s.e.e, OX_ENUM_ENUM)) == OX_ERR)
        return r;

    if ((r = ox_object_add_const(ctxt, s, c, se->s.e.e)) == OX_ERR)
        return r;

    return OX_OK;
}

/*Add an enumeration item.*/
static inline OX_Result
do_e_item (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *c)
{
    OX_Stack *se = ox_stack_top(ctxt);
    OX_Result r;

    if (!ox_value_is_null(ctxt, se->s.e.e)) {
        if ((r = ox_enum_add_item(ctxt, se->s.e.e, se->s.e.c, c, se->s.e.v)) == OX_ERR)
            return r;
    }

    se->s.e.v ++;
    return OX_OK;
}

/*Bitfield start.*/
static inline OX_Result
do_b_start (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s)
{
    OX_Stack *se = ox_stack_push(ctxt, OX_STACK_BITFIELD);
    OX_Result r;

    se->s.b.c = ox_value_stack_push_n(ctxt, 2);
    se->s.b.b = ox_values_item(ctxt, se->s.b.c, 1);

    ox_value_copy(ctxt, se->s.b.c, s);
    ox_value_set_null(ctxt, se->s.b.b);
    se->s.b.v = 0;

    if ((r = ox_enum_new(ctxt, se->s.b.b, OX_ENUM_BITFIELD)) == OX_ERR)
        return r;

    return OX_OK;
}

/*Bitfield start with object name.*/
static inline OX_Result
do_b_start_n (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *c, OX_Value *s)
{
    OX_Stack *se = ox_stack_push(ctxt, OX_STACK_BITFIELD);
    OX_Result r;

    se->s.b.c = ox_value_stack_push_n(ctxt, 2);
    se->s.b.b = ox_values_item(ctxt, se->s.b.c, 1);

    ox_value_copy(ctxt, se->s.b.c, s);
    ox_value_set_null(ctxt, se->s.b.b);
    se->s.b.v = 0;

    if ((r = ox_enum_new(ctxt, se->s.b.b, OX_ENUM_BITFIELD)) == OX_ERR)
        return r;

    if ((r = ox_object_add_const(ctxt, s, c, se->s.b.b)) == OX_ERR)
        return r;

    return OX_OK;
}

/*Add a bitfield item.*/
static inline OX_Result
do_b_item (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *c)
{
    OX_Stack *se = ox_stack_top(ctxt);
    int v;
    OX_Result r;

    v = 1 << se->s.b.v;

    if (!ox_value_is_null(ctxt, se->s.b.b)) {
        if ((r = ox_enum_add_item(ctxt, se->s.b.b, se->s.b.c, c, v)) == OX_ERR)
            return r;
    }

    se->s.b.v ++;
    return OX_OK;
}

/*Set the name of the object.*/
static inline OX_Result
do_set_name (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1)
{
    return ox_object_set_name(ctxt, s0, s1);
}

/*Set the name of the accessor's getter.*/
static inline OX_Result
do_set_name_g (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1)
{
    OX_VS_PUSH(ctxt, n)
    OX_CharBuffer cb;
    OX_Result r;

    ox_char_buffer_init(&cb);

    if ((r = ox_char_buffer_append_string(ctxt, &cb, s1)) == OX_ERR)
        goto end;

    if ((r = ox_char_buffer_append_chars(ctxt, &cb, ":get", 4)) == OX_ERR)
        goto end;

    if ((r = ox_char_buffer_get_string(ctxt, &cb, n)) == OX_ERR)
        goto end;

    r = ox_object_set_name(ctxt, s0, n);

end:
    ox_char_buffer_deinit(ctxt, &cb);
    OX_VS_POP(ctxt, n)
    return r;
}

/*Set the name of the accessor's setter.*/
static inline OX_Result
do_set_name_s (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1)
{
    OX_VS_PUSH(ctxt, n)
    OX_CharBuffer cb;
    OX_Result r;

    ox_char_buffer_init(&cb);

    if ((r = ox_char_buffer_append_string(ctxt, &cb, s1)) == OX_ERR)
        goto end;

    if ((r = ox_char_buffer_append_chars(ctxt, &cb, ":set", 4)) == OX_ERR)
        goto end;

    if ((r = ox_char_buffer_get_string(ctxt, &cb, n)) == OX_ERR)
        goto end;

    r = ox_object_set_name(ctxt, s0, n);

end:
    ox_char_buffer_deinit(ctxt, &cb);
    OX_VS_POP(ctxt, n)
    return r;
}

/*Set the scope of the object.*/
static inline OX_Result
do_set_scope (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *s0, OX_Value *s1)
{
    return ox_object_set_scope(ctxt, s0, s1);
}

/*Check if the value of the name is not null.*/
static inline OX_Result
do_name_nn (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *c, OX_Value *s)
{
    if (ox_value_is_null(ctxt, s))
        return ox_throw_null_error(ctxt,
                OX_TEXT("value of \"%s\" is null"),
                ox_string_get_char_star(ctxt, c));

    return OX_OK;
}

/*Check if the value of the property is not null.*/
static inline OX_Result
do_prop_nn (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *c, OX_Value *s)
{
    if (ox_value_is_null(ctxt, s)) {
        OX_VS_PUSH(ctxt, str)
        OX_Result r;

        r = ox_to_string(ctxt, c, str);

        if (r == OX_OK)
            ox_throw_null_error(ctxt,
                    OX_TEXT("value of property \"%s\" is null"),
                    ox_string_get_char_star(ctxt, str));

        OX_VS_POP(ctxt, str)
        return OX_ERR;
    }

    return OX_OK;
}

/*Check if the value of the private property is not null.*/
static inline OX_Result
do_pprop_nn (OX_Context *ctxt, OX_RunStatus *rs, OX_Value *p, OX_Value *s)
{
    if (ox_value_is_null(ctxt, s)) {
        OX_VS_PUSH(ctxt, str)
        OX_Result r;

        r = ox_to_string(ctxt, p, str);

        if (r == OX_OK)
            ox_throw_null_error(ctxt,
                    OX_TEXT("value of property \"%s\" is null"),
                    ox_string_get_char_star(ctxt, str));

        OX_VS_POP(ctxt, str)
        return OX_ERR;
    }

    return OX_OK;
}


/*No operation.*/
static inline OX_Result
do_nop (OX_Context *ctxt, OX_RunStatus *rs)
{
    return OX_OK;
}
