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
 * Error.
 */

#ifndef _OX_ERROR_H_
#define _OX_ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Throw an error.
 * @param ctxt The current running context.
 * @param e The error value.
 * @return OX_ERR.
 */
extern OX_Result
ox_throw (OX_Context *ctxt, OX_Value *e);

/**
 * Catch the error.
 * @param ctxt The current running context.
 * @param[out] e Return the error value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_catch (OX_Context *ctxt, OX_Value *e);

/**
 * Throw a system error.
 * @param ctxt The current running context.
 * @param fmt Output format pattern.
 * @param ... Arguments.
 * @return OX_ERR.
 */
extern OX_Result
ox_throw_system_error (OX_Context *ctxt, const char *fmt, ...);

/**
 * Throw a no memory error.
 * @param ctxt The current running context.
 * @return OX_ERR.
 */
extern OX_Result
ox_throw_no_mem_error (OX_Context *ctxt);

/**
 * Throw a null error.
 * @param ctxt The current running context.
 * @param fmt Output format pattern.
 * @param ... Arguments.
 * @return OX_ERR.
 */
extern OX_Result
ox_throw_null_error (OX_Context *ctxt, const char *fmt, ...);

/**
 * Throw a value range error.
 * @param ctxt The current running context.
 * @param fmt Output format pattern.
 * @param ... Arguments.
 * @return OX_ERR.
 */
extern OX_Result
ox_throw_range_error (OX_Context *ctxt, const char *fmt, ...);

/**
 * Throw an access error.
 * @param ctxt The current running context.
 * @param fmt Output format pattern.
 * @param ... Arguments.
 * @return OX_ERR.
 */
extern OX_Result
ox_throw_access_error (OX_Context *ctxt, const char *fmt, ...);

/**
 * Throw a type error.
 * @param ctxt The current running context.
 * @param fmt Output format pattern.
 * @param ... Arguments.
 * @return OX_ERR.
 */
extern OX_Result
ox_throw_type_error (OX_Context *ctxt, const char *fmt, ...);

/**
 * Throw a syntax error.
 * @param ctxt The current running context.
 * @param fmt Output format pattern.
 * @param ... Arguments.
 * @return OX_ERR.
 */
extern OX_Result
ox_throw_syntax_error (OX_Context *ctxt, const char *fmt, ...);

/**
 * Throw a reference error.
 * @param ctxt The current running context.
 * @param fmt Output format pattern.
 * @param ... Arguments.
 * @return OX_ERR.
 */
extern OX_Result
ox_throw_reference_error (OX_Context *ctxt, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
