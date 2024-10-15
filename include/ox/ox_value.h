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
 * Value.
 */

#ifndef _OX_VALUE_H_
#define _OX_VALUE_H_

#ifdef __cplusplus
extern "C" {
#endif

/** null value tag.*/
#define OX_VALUE_TAG_NULL    0x7ff9
/** Boolean value tag.*/
#define OX_VALUE_TAG_BOOL    0x7ffa
/** GC managed object value tag.*/
#define OX_VALUE_TAG_GCO     0x7ffb

/**
 * Get the current value stack.
 * @param ctxt The current running context.
 * @return The value stack.
 */
static inline OX_ValueBuffer*
ox_get_value_stack (OX_Context *ctxt)
{
    OX_BaseContext *bctxt = (OX_BaseContext*)ctxt;

    return bctxt->v_stack;
}

/** Check if the value is in stack.*/
#define OX_VALUE_IN_STACK(v)  (OX_PTR2SIZE(v) & 1)
/** Convert the stack value pointer to index.*/
#define OX_VALUE_PTR2IDX(v)   (OX_PTR2SIZE(v) >> 1)
/** Convert the index to stack value pointer.*/
#define OX_VALUE_IDX2PTR(i)   OX_SIZE2PTR(((i) << 1) | 1)

/**
 * Get the value's real pointer.
 * @param ctxt The current running context.
 * @param v The value.
 * @return The value's real pointer.
 */
static inline OX_Value*
ox_value_get_pointer (OX_Context *ctxt, OX_Value *v)
{
    if (OX_VALUE_IN_STACK(v)) {
        /*The value is in stack.*/
        size_t idx = OX_VALUE_PTR2IDX(v);
        OX_ValueBuffer *vb = ox_get_value_stack(ctxt);

        return &ox_vector_item(vb, idx);
    } else {
        /*The value is a real pointer.*/
        return v;
    }
}

/**
 * Get the item of a value array.
 * @param ctxt The current running context.
 * @param v The first value of the array.
 * @param idx The item's index.
 * @return The item value.
 */
static inline OX_Value*
ox_values_item (OX_Context *ctxt, OX_Value *v, size_t idx)
{
    if (OX_VALUE_IN_STACK(v)) {
        /*The value array is in stack.*/
        size_t base = OX_VALUE_PTR2IDX(v);

        return OX_VALUE_IDX2PTR(base + idx);
    } else {
        /*The value array is in heap.*/
        return v + idx;
    }
}

/**
 * Get the argument value pointer.
 * @param c The current running context.
 * @param args Arguments.
 * @param argc Arguments' count.
 * @param i The argument's index.
 * @return The argument's pointer. If i >= args, return the pointer of null value.
 */
#define ox_argument(c, args, argc, i)\
    ((i) >= (argc) ? ox_value_null(c) : ox_values_item(ctxt, args, i))

/**
 * Copy the value.
 * @param ctxt The current running context.
 * @param dst The destination value.
 * @param src The source value.
 */
static inline void
ox_value_copy (OX_Context *ctxt, OX_Value *dst, OX_Value *src)
{
    dst = ox_value_get_pointer(ctxt, dst);
    src = ox_value_get_pointer(ctxt, src);
    *dst = *src;
}

/**
 * Copy the values.
 * @param ctxt The current running context.
 * @param dst The destination value.
 * @param src The source value.
 * @param n Number of values to be copyed.
 */
static inline void
ox_values_copy (OX_Context *ctxt, OX_Value *dst, OX_Value *src, size_t n)
{
    dst = ox_value_get_pointer(ctxt, dst);
    src = ox_value_get_pointer(ctxt, src);

    memcpy(dst, src, sizeof(OX_Value) * n);
}

/**
 * Get the value's tag.
 * @param v The value pointer.
 * @return The tag of the value.
 */
static inline int
ox_value_get_tag (OX_Value *v)
{
    return *v >> 48;
}

/**
 * Get the value's type.
 * @param ctxt The current running context.
 * @param v The value's pointer.
 * @return The value's type.
 */
static inline OX_ValueType
ox_value_get_type (OX_Context *ctxt, OX_Value *v)
{
    int tag;
    
    v = ox_value_get_pointer(ctxt, v);
    tag = ox_value_get_tag(v);

    switch (tag) {
    case OX_VALUE_TAG_NULL:
        return OX_VALUE_NULL;
    case OX_VALUE_TAG_BOOL:
        return OX_VALUE_BOOL;
    case OX_VALUE_TAG_GCO:
        return OX_VALUE_GCO;
    default:
        return OX_VALUE_NUMBER;
    }
}

/**
 * Set the value to null.
 * @param ctxt The current running context.
 * @param v The value to be set.
 */
static inline void
ox_value_set_null (OX_Context *ctxt, OX_Value *v)
{
    v = ox_value_get_pointer(ctxt, v);

    *v = ((OX_Value)OX_VALUE_TAG_NULL) << 48;
}

/**
 * Set the values to null.
 * @param ctxt The current running context.
 * @param v The first value to be set.
 * @param n Number of the values.
 */
static inline void
ox_values_set_null (OX_Context *ctxt, OX_Value *v, size_t n)
{
    OX_Value *lv;

    v = ox_value_get_pointer(ctxt, v);
    lv = v + n;

    while (v < lv) {
        *v = ((OX_Value)OX_VALUE_TAG_NULL) << 48;
        v ++;
    }
}

/**
 * Set the value as a boolean value.
 * @param ctxt The current running context.
 * @param v The value to be set.
 * @param b The boolean value.
 */
static inline void
ox_value_set_bool (OX_Context *ctxt, OX_Value *v, int b)
{
    v = ox_value_get_pointer(ctxt, v);

    *v = (((OX_Value)OX_VALUE_TAG_BOOL) << 48) | (b ? 1 : 0);
}

/**
 * Set the value as a number value.
 * @param ctxt The current running context.
 * @param v The value to be set.
 * @param n The number value.
 */
static inline void
ox_value_set_number (OX_Context *ctxt, OX_Value *v, OX_Number n)
{
    v = ox_value_get_pointer(ctxt, v);

    if (isnan(n))
        *(OX_Number*)v = NAN;
    else
        *(OX_Number*)v = n;
}

/**
 * Set the value as a GC managed object.
 * @param ctxt The current running context.
 * @param v The value to be set.
 * @param ptr The GC managed object pointer.
 */
static inline void
ox_value_set_gco (OX_Context *ctxt, OX_Value *v, void *ptr)
{
    v = ox_value_get_pointer(ctxt, v);

#if __SIZEOF_POINTER__  == 8
    *v = (((OX_Value)OX_VALUE_TAG_GCO) << 48) | (OX_PTR2SIZE(ptr) & 0xffffffffffffull);
#elif __SIZEOF_POINTER__  == 4
    *v = (((OX_Value)OX_VALUE_TAG_GCO) << 48) | OX_PTR2SIZE(ptr);
#else
    #error illegal pointer size
#endif
}

/**
 * Get the boolean value from a value.
 * @param ctxt The current running context.
 * @param v The value.
 * @return The boolean value.
 */
static inline OX_Bool
ox_value_get_bool (OX_Context *ctxt, OX_Value *v)
{
    v = ox_value_get_pointer(ctxt, v);

    return *v & 1;
}

/**
 * Get the number value from a value.
 * @param ctxt The current running context.
 * @param v The value.
 * @return The number value.
 */
static inline OX_Number
ox_value_get_number (OX_Context *ctxt, OX_Value *v)
{
    v = ox_value_get_pointer(ctxt, v);

    return *(OX_Number*)v;
}

/**
 * Get the GC managed object pointer from a value pointer.
 * @param v The value.
 * @return The GC managed object's pointer.
 */
static inline void*
ox_value_pointer_get_gco (OX_Value *v)
{
#if __SIZEOF_POINTER__  == 8
    if (*v & 0x800000000000ull)
        return OX_SIZE2PTR((*v & 0xffffffffffffull) | 0xffff000000000000ull);
    else
        return OX_SIZE2PTR(*v & 0xffffffffffffull);
#elif __SIZEOF_POINTER__  == 4
    return OX_SIZE2PTR(*v & 0xffffffff);
#else
    #error illegal pointer size
#endif
}

/**
 * Get the GC managed object pointer from a value.
 * @param ctxt The current running context.
 * @param v The value.
 * @return The GC managed object's pointer.
 * @retval NULL The value is not a GC managed object.
 */
static inline void*
ox_value_get_gco (OX_Context *ctxt, OX_Value *v)
{
    v = ox_value_get_pointer(ctxt, v);

    if (ox_value_get_tag(v) != OX_VALUE_TAG_GCO)
        return NULL;

    return ox_value_pointer_get_gco(v);
}

/**
 * Get the GC managed object's type.
 * @param ctxt The current running context.
 * @param v The value.
 * @return The GC managed object's type.
 * @retval -1 The value is not a GC managed object.
 */
static inline OX_GcObjectType
ox_value_get_gco_type (OX_Context *ctxt, OX_Value *v)
{
    OX_GcObject *gco;

    v = ox_value_get_pointer(ctxt, v);

    if (ox_value_get_tag(v) != OX_VALUE_TAG_GCO)
        return -1;

    gco = ox_value_pointer_get_gco(v);
    return gco->ops->type;
}

/**
 * Check if the value is null.
 * @param ctxt The current running context.
 * @param v The value.
 * @retval OX_TRUE v is null.
 * @retval OX_FALSE v is not null.
 */
static inline OX_Bool
ox_value_is_null (OX_Context *ctxt, OX_Value *v)
{
    v = ox_value_get_pointer(ctxt, v);

    return ox_value_get_tag(v) == OX_VALUE_TAG_NULL;
}

/**
 * Check if the value is a boolean value.
 * @param ctxt The current running context.
 * @param v The value.
 * @retval OX_TRUE v is a boolean value.
 * @retval OX_FALSE v is not a boolean value.
 */
static inline OX_Bool
ox_value_is_bool (OX_Context *ctxt, OX_Value *v)
{
    v = ox_value_get_pointer(ctxt, v);

    return ox_value_get_tag(v) == OX_VALUE_TAG_BOOL;
}

/**
 * Check if the value is a GC managed object.
 * @param ctxt The current running context.
 * @param v The value.
 * @param type The object type.
 * @retval OX_TRUE v is a GC managed object.
 * @retval OX_FALSE v is not a GC managed object.
 */
static inline OX_Bool
ox_value_is_gco (OX_Context *ctxt, OX_Value *v, OX_GcObjectType type)
{
    OX_GcObject *gco;

    v = ox_value_get_pointer(ctxt, v);

    if (ox_value_get_tag(v) != OX_VALUE_TAG_GCO)
        return OX_FALSE;

    if (type != -1) {
        gco = ox_value_pointer_get_gco(v);

        if (gco->ops->type != type)
            return OX_FALSE;
    }

    return OX_TRUE;
}

/**
 * Check if the value is a number.
 * @param ctxt The current running context.
 * @param v The value.
 * @retval OX_TRUE v is a number.
 * @retval OX_FALSE v is not a number.
 */
static inline OX_Bool
ox_value_is_number (OX_Context *ctxt, OX_Value *v)
{
    return ox_value_get_type(ctxt, v) == OX_VALUE_NUMBER;
}

/**
 * Get the current value stack's top.
 * @param ctxt The current running context.
 * @return The value stack's top.
 */
static inline OX_Value*
ox_value_stack_top (OX_Context *ctxt)
{
    OX_ValueBuffer *vb = ox_get_value_stack(ctxt);
    size_t idx = ox_vector_length(vb);

    return OX_VALUE_IDX2PTR(idx);
}

/**
 * Add a value variable to the current value stack.
 * @param ctxt The current running context.
 * @return The new value in the stack.
 */
extern OX_Value*
ox_value_stack_push (OX_Context *ctxt);

/**
 * Add value variables to the current value stack.
 * @param ctxt The current running context.
 * @param n Number of values to be added.
 * @return The first value in the stack.
 */
extern OX_Value*
ox_value_stack_push_n (OX_Context *ctxt, size_t n);

/** Declare a value in the stack.*/
#define OX_VS_PUSH(c, v)\
    OX_Value *v = ox_value_stack_push(c);

/** Declare 2 values in the stack.*/
#define OX_VS_PUSH_2(c, v1, v2)\
    OX_Value *v1 = ox_value_stack_push_n(c, 2);\
    OX_Value *v2 = ox_values_item(ctxt, v1, 1);

/** Declare 3 values in the stack.*/
#define OX_VS_PUSH_3(c, v1, v2, v3)\
    OX_Value *v1 = ox_value_stack_push_n(c, 3);\
    OX_Value *v2 = ox_values_item(ctxt, v1, 1);\
    OX_Value *v3 = ox_values_item(ctxt, v1, 2);

/** Declare 4 values in the stack.*/
#define OX_VS_PUSH_4(c, v1, v2, v3, v4)\
    OX_Value *v1 = ox_value_stack_push_n(c, 4);\
    OX_Value *v2 = ox_values_item(ctxt, v1, 1);\
    OX_Value *v3 = ox_values_item(ctxt, v1, 2);\
    OX_Value *v4 = ox_values_item(ctxt, v1, 3);

/** Declare 5 values in the stack.*/
#define OX_VS_PUSH_5(c, v1, v2, v3, v4, v5)\
    OX_Value *v1 = ox_value_stack_push_n(c, 5);\
    OX_Value *v2 = ox_values_item(ctxt, v1, 1);\
    OX_Value *v3 = ox_values_item(ctxt, v1, 2);\
    OX_Value *v4 = ox_values_item(ctxt, v1, 3);\
    OX_Value *v5 = ox_values_item(ctxt, v1, 4);

/** Declare 6 values in the stack.*/
#define OX_VS_PUSH_6(c, v1, v2, v3, v4, v5, v6)\
    OX_Value *v1 = ox_value_stack_push_n(c, 6);\
    OX_Value *v2 = ox_values_item(ctxt, v1, 1);\
    OX_Value *v3 = ox_values_item(ctxt, v1, 2);\
    OX_Value *v4 = ox_values_item(ctxt, v1, 3);\
    OX_Value *v5 = ox_values_item(ctxt, v1, 4);\
    OX_Value *v6 = ox_values_item(ctxt, v1, 5);

/** Declare 7 values in the stack.*/
#define OX_VS_PUSH_7(c, v1, v2, v3, v4, v5, v6, v7)\
    OX_Value *v1 = ox_value_stack_push_n(c, 7);\
    OX_Value *v2 = ox_values_item(ctxt, v1, 1);\
    OX_Value *v3 = ox_values_item(ctxt, v1, 2);\
    OX_Value *v4 = ox_values_item(ctxt, v1, 3);\
    OX_Value *v5 = ox_values_item(ctxt, v1, 4);\
    OX_Value *v6 = ox_values_item(ctxt, v1, 5);\
    OX_Value *v7 = ox_values_item(ctxt, v1, 6);

/** Declare 8 values in the stack.*/
#define OX_VS_PUSH_8(c, v1, v2, v3, v4, v5, v6, v7, v8)\
    OX_Value *v1 = ox_value_stack_push_n(c, 8);\
    OX_Value *v2 = ox_values_item(ctxt, v1, 1);\
    OX_Value *v3 = ox_values_item(ctxt, v1, 2);\
    OX_Value *v4 = ox_values_item(ctxt, v1, 3);\
    OX_Value *v5 = ox_values_item(ctxt, v1, 4);\
    OX_Value *v6 = ox_values_item(ctxt, v1, 5);\
    OX_Value *v7 = ox_values_item(ctxt, v1, 6);\
    OX_Value *v8 = ox_values_item(ctxt, v1, 7);

/** Declare 9 values in the stack.*/
#define OX_VS_PUSH_9(c, v1, v2, v3, v4, v5, v6, v7, v8, v9)\
    OX_Value *v1 = ox_value_stack_push_n(c, 9);\
    OX_Value *v2 = ox_values_item(ctxt, v1, 1);\
    OX_Value *v3 = ox_values_item(ctxt, v1, 2);\
    OX_Value *v4 = ox_values_item(ctxt, v1, 3);\
    OX_Value *v5 = ox_values_item(ctxt, v1, 4);\
    OX_Value *v6 = ox_values_item(ctxt, v1, 5);\
    OX_Value *v7 = ox_values_item(ctxt, v1, 6);\
    OX_Value *v8 = ox_values_item(ctxt, v1, 7);\
    OX_Value *v9 = ox_values_item(ctxt, v1, 8);

/** Declare 10 values in the stack.*/
#define OX_VS_PUSH_10(c, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10)\
    OX_Value *v1 = ox_value_stack_push_n(c, 10);\
    OX_Value *v2 = ox_values_item(ctxt, v1, 1);\
    OX_Value *v3 = ox_values_item(ctxt, v1, 2);\
    OX_Value *v4 = ox_values_item(ctxt, v1, 3);\
    OX_Value *v5 = ox_values_item(ctxt, v1, 4);\
    OX_Value *v6 = ox_values_item(ctxt, v1, 5);\
    OX_Value *v7 = ox_values_item(ctxt, v1, 6);\
    OX_Value *v8 = ox_values_item(ctxt, v1, 7);\
    OX_Value *v9 = ox_values_item(ctxt, v1, 8);\
    OX_Value *v10 = ox_values_item(ctxt, v1, 9);

/**
 * Pop up the top values from the current value stack.
 * @param ctxt The current running context.
 * @param v The new top of the value stack.
 */
static inline void
ox_value_stack_pop (OX_Context *ctxt, OX_Value *v)
{
    OX_ValueBuffer *vb = ox_get_value_stack(ctxt);
    size_t idx;

    assert(OX_VALUE_IN_STACK(v));

    idx = OX_VALUE_PTR2IDX(v);

    assert(ox_vector_length(vb) >= idx);

    vb->len = idx;
}

/** Pop up the values from the stack. */
#define OX_VS_POP(c, v)\
    ox_value_stack_pop(c, v);

#ifdef __cplusplus
}
#endif

#endif
