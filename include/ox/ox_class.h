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
 * Class.
 */

#ifndef _OX_CLASS_H_
#define _OX_CLASS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Check if the value is a class.
 * @param ctxt The current running context.
 * @param v The value.
 * @retval OX_TRUE The value is a class.
 * @retval OX_FALSE The value is not a class.
 */
static inline OX_Bool
ox_value_is_class (OX_Context *ctxt, OX_Value *v)
{
    return ox_value_is_gco(ctxt, v, OX_GCO_CLASS);
}

/**
 * Create a new class.
 * @param ctxt The current running context.
 * @param[out] c Return the new class object.
 * @param[out] inf Return the new interface object.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_class_new (OX_Context *ctxt, OX_Value *c, OX_Value *inf);

/**
 * Set the class's object allocate function.
 * @param ctxt The current running context.
 * @param c The class.
 * @param alloc The object allocate function.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_class_set_alloc_func (OX_Context *ctxt, OX_Value *c, OX_AllocObjectFunc alloc);

/**
 * Create a new class with its scope and name.
 * @param ctxt The current running context.
 * @param[out] c Return the new class object.
 * @param[out] inf Return the new interface object.
 * @param scope The scope of the class.
 * @param name The name of the class.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_named_class_new (OX_Context *ctxt, OX_Value *c, OX_Value *inf,
        OX_Value *scope, OX_Value *name);

/**
 * Create a new class with its scope and name.
 * name is a constant C string.
 * @param ctxt The current running context.
 * @param[out] c Return the new class object.
 * @param[out] inf Return the new interface object.
 * @param scope The scope of the class.
 * @param name The name of the class.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_named_class_new_s (OX_Context *ctxt, OX_Value *c, OX_Value *inf,
        OX_Value *scope, const char *name);

/**
 * Inherit the properties from the parent class.
 * @param ctxt The current running context.
 * @param c The class value.
 * @param pc The parent class value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_class_inherit (OX_Context *ctxt, OX_Value *c, OX_Value *pc);

#ifdef __cplusplus
}
#endif

#endif
