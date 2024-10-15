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
 * Interface.
 */

#ifndef _OX_INTERFACE_H_
#define _OX_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Check if the value is an interface.
 * @param ctxt The current running context.
 * @param v The value.
 * @retval OX_TRUE The value is an interface.
 * @retval OX_FALSE The value is not an interface.
 */
static inline OX_Bool
ox_value_is_interface (OX_Context *ctxt, OX_Value *v)
{
    return ox_value_is_gco(ctxt, v, OX_GCO_INTERFACE);
}

/**
 * Create a new interface.
 * @param ctxt The current running context.
 * @param[out] inf Return the new interface.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_interface_new (OX_Context *ctxt, OX_Value *inf);

/**
 * Inherit properties from the parent interface.
 * @param ctxt The current running context.
 * @param inf The interface.
 * @param pinf The parent interface.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_interface_inherit (OX_Context *ctxt, OX_Value *inf, OX_Value *pinf);

/**
 * Check if the interface has implement the parent interface.
 * @param ctxt The current running context.
 * @param inf The interface.
 * @param pinf The parent interface.
 * @retval OX_TRUE inf has implemented the parent interface.
 * @retval OX_FALSE inf has not implemented the parent interface.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_interface_impl (OX_Context *ctxt, OX_Value *inf, OX_Value *pinf);

#ifdef __cplusplus
}
#endif

#endif
