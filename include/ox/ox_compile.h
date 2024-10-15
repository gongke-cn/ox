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
 * Compile functions.
 */

#ifndef _OX_COMPILE_H_
#define _OX_COMPILE_H_

#ifdef __cplusplus
extern "C" {
#endif

/** Parse the script as an expression.*/
#define OX_COMPILE_FL_EXPR     (1 << 0)
/** The script can access the current frame stack.*/
#define OX_COMPILE_FL_CURR     (1 << 1)
/** Register the result script to the manager.*/
#define OX_COMPILE_FL_REGISTER (1 << 2)

/**
 * Compile the abstract syntax tree to script.
 * @param ctxt The current running context.
 * @param input The input.
 * @param ast The abstract syntax tree.
 * @param[out] sv The result script.
 * @param flags The compile flags.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_compile (OX_Context *ctxt, OX_Value *input, OX_Value *ast, OX_Value *sv, int flags);

/**
 * Decompile the script to readable instructions.
 * @param ctxt The current running context.
 * @param script The script value.
 * @param fp The decompile output file.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_decompile (OX_Context *ctxt, OX_Value *script, FILE *fp);

#ifdef __cplusplus
}
#endif

#endif
