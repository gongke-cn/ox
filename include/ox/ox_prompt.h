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
 * Prompt message.
 */

#ifndef _OX_PROMPT_H_
#define _OX_PROMPT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Output prompt message.
 * @param ctxt The current running context.
 * @param input The input generate the message.
 * @param loc Location where generate the message.
 * @param type Prompt message type.
 * @param fmt Output format pattern.
 * @param ... Arguments.
 */
extern void
ox_prompt (OX_Context *ctxt, OX_Input *input, OX_Location *loc,
        OX_PromptType type, const char *fmt, ...);

/**
 * Output prompt message with va_list argument.
 * @param ctxt The current running context.
 * @param input The input generate the message.
 * @param loc Location where generate the message.
 * @param type Prompt message type.
 * @param fmt Output format pattern.
 * @param ap Variable argument list.
 */
extern void
ox_prompt_v (OX_Context *ctxt, OX_Input *input, OX_Location *loc,
        OX_PromptType type, const char *fmt, va_list ap);

/**
 * Output note prompt message.
 * @param ctxt The current running context.
 * @param input The input generate the message.
 * @param loc Location where generate the message.
 * @param fmt Output format pattern.
 * @param ... Arguments.
 */
extern void
ox_note (OX_Context *ctxt, OX_Input *input, OX_Location *loc,
        const char *fmt, ...);

/**
 * Output warning prompt message.
 * @param ctxt The current running context.
 * @param input The input generate the message.
 * @param loc Location where generate the message.
 * @param fmt Output format pattern.
 * @param ... Arguments.
 */
extern void
ox_warning (OX_Context *ctxt, OX_Input *input, OX_Location *loc,
        const char *fmt, ...);

/**
 * Output error prompt message.
 * @param ctxt The current running context.
 * @param input The input generate the message.
 * @param loc Location where generate the message.
 * @param fmt Output format pattern.
 * @param ... Arguments.
 */
extern void
ox_error (OX_Context *ctxt, OX_Input *input, OX_Location *loc,
        const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
