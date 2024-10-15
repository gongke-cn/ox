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
 * Source code parser.
 */

#ifndef _OX_PARSER_H_
#define _OX_PARSER_H_

#ifdef __cplusplus
extern "C" {
#endif

/** "return" statement can be used in bottom block.*/
#define OX_PARSE_FL_RETURN    (1 << 0)
/** Do not report error/warning prompt message.*/
#define OX_PARSE_FL_NO_PROMPT (1 << 1)
/** Parse document data in the file.*/
#define OX_PARSE_FL_DOC       (1 << 2)

/**
 * Parse a script to abstract syntax tree.
 * @param ctxt The current running context.
 * @param input The input value.
 * @param[out] ast Return the result AST value.
 * @param flags Parse flags.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_parse (OX_Context *ctxt, OX_Value *input, OX_Value *ast, int flags);

/**
 * Convert the abstract syntax tree to string.
 * @param ctxt The current running context.
 * @param ast The abstract syntax tree.
 * @param[out] s The result string.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_ast_to_string (OX_Context *ctxt, OX_Value *ast, OX_Value *s);

#ifdef __cplusplus
}
#endif

#endif
