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
 * Enumeration.
 */

#ifndef _OX_ENUM_H_
#define _OX_ENUM_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a new enumeration.
 * @param ctxt The current running context.
 * @param[out] e Return the enumeration object.
 * @param type The enumeration's type.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_enum_new (OX_Context *ctxt, OX_Value *e, OX_EnumType type);

/**
 * Add an enumeration item.
 * @param ctxt The current running context.
 * @param e The enumeration object.
 * @param c The container class.
 * @param name The name of the enumeration.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_enum_add_item (OX_Context *ctxt, OX_Value *e, OX_Value *c, OX_Value *name, int v);

/**
 * Add an enumeration item.
 * The name is a null terminated characters string.
 * @param ctxt The current running context.
 * @param e The enumeration object.
 * @param c The container class.
 * @param name The name of the enumeration.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_enum_add_item_s (OX_Context *ctxt, OX_Value *e, OX_Value *c, const char *name, int v);

#ifdef __cplusplus
}
#endif

#endif
