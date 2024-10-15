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
 * Function.
 */

#ifndef _OX_FUNCTION_H_
#define _OX_FUNCTION_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Check if the value is a function.
 * @param ctxt The current running context.
 * @param f The function.
 * @retval OX_TRUE The value is a function.
 * @retval OX_FALSE The value is not a function.
 */
static inline OX_Bool
ox_value_is_function (OX_Context *ctxt, OX_Value *f)
{
    OX_GcObjectType type;

    type = ox_value_get_gco_type(ctxt, f);
    if (type == -1)
        return OX_FALSE;

    return (type & OX_GCO_FL_FUNC) ? OX_TRUE : OX_FALSE;
}

/**
 * Create a new native function.
 * @param ctxt The current running context.
 * @param[out] f Return the native function.
 * @param cf The C function.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_native_func_new (OX_Context *ctxt, OX_Value *f, OX_CFunc cf);

/**
 * Get the script contains the native function.
 * @param ctxt The current running context.
 * @param f The native function.
 * @param[out] s Return the native script contains the function.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_native_func_get_script (OX_Context *ctxt, OX_Value *f, OX_Value *s);

/**
 * Create a new native function with scope and name.
 * @param ctxt The current running context.
 * @param[out] f Return the native function.
 * @param cf The C function.
 * @param scope The scope object.
 * @param name The name of the function.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_named_native_func_new (OX_Context *ctxt, OX_Value *f, OX_CFunc cf,
        OX_Value *scope, OX_Value *name);

/**
 * Create a new native function with scope and name.
 * name is a constant C string.
 * @param ctxt The current running context.
 * @param[out] f Return the native function.
 * @param cf The C function.
 * @param scope The scope object.
 * @param name The name of the function.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_named_native_func_new_s (OX_Context *ctxt, OX_Value *f, OX_CFunc cf,
        OX_Value *scope, const char *name);

#ifdef __cplusplus
}
#endif

#endif
