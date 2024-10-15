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
 * JSON functions.
 */

#ifndef _OX_JSON_H_
#define _OX_JSON_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Parse the JSON input.
 * @param ctxt The current running context.
 * @param inputv The JSON input.
 * @param sched Schedule when read from input.
 * @param[out] v Return the result value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_json_parse (OX_Context *ctxt, OX_Value *input, OX_Bool sched, OX_Value *v);

/**
 * Convert the value to JSON string.
 * @param ctxt The current running context.
 * @param v The value.
 * @param indent Indent string.
 * @param filter The filter function.
 * @param map The map function.
 * @param[out] str Return the JSON string.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_json_to_str (OX_Context *ctxt, OX_Value *v, OX_Value *indent, OX_Value *filter,
        OX_Value *map, OX_Value *str);

#ifdef __cplusplus
}
#endif

#endif
