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
 * Prompt message functions.
 */

#define OX_LOG_TAG "ox_prompt"

#include "ox_internal.h"

/**
 * Output prompt message.
 * @param ctxt The current running context.
 * @param input The input generate the message.
 * @param loc Location where generate the message.
 * @param type Prompt message type.
 * @param fmt Output format pattern.
 * @param ... Arguments.
 */
void
ox_prompt (OX_Context *ctxt, OX_Input *input, OX_Location *loc,
        OX_PromptType type, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    ox_prompt_v(ctxt, input, loc, type, fmt, ap);

    va_end(ap);
}

/**
 * Output prompt message with va_list argument.
 * @param ctxt The current running context.
 * @param input The input generate the message.
 * @param loc Location where generate the message.
 * @param type Prompt message type.
 * @param fmt Output format pattern.
 * @param ap Variable argument list.
 */
void
ox_prompt_v (OX_Context *ctxt, OX_Input *input, OX_Location *loc,
        OX_PromptType type, const char *fmt, va_list ap)
{
    char *tag, *col;

    if (input && input->name) {
        fprintf(stderr, "\"%s\": ", input->name);
    }

    if (loc) {
        if (loc->first_line == loc->last_line) {
            if (loc->first_column == loc->last_column) {
                fprintf(stderr, "%d.%d: ",
                        loc->first_line, loc->first_column);
            } else {
                fprintf(stderr, "%d.%d-%d: ",
                        loc->first_line, loc->first_column, loc->last_column);
            }
        } else {
            fprintf(stderr, "%d.%d-%d.%d: ",
                    loc->first_line, loc->first_column,
                    loc->last_line, loc->last_column);
        }
    }

    switch (type) {
    case OX_PROMPT_NOTE:
        tag = OX_TEXT("note");
        col = "\033[36;1m";
        break;
    case OX_PROMPT_WARNING:
        tag = OX_TEXT("warning");
        col = "\033[35;1m";
        break;
    case OX_PROMPT_ERROR:
        tag = OX_TEXT("error");
        col = "\033[31;1m";
        break;
    default:
        assert(0);
    }

#ifdef OX_SUPPORT_COLOR_TTY
    if (!isatty(2))
#endif /*OX_SUPPORT_COLOR_TTY*/
        col = NULL;

    fprintf(stderr, "%s%s%s: ",
            col ? col : "",
            tag,
            col ? "\033[0m" : "");

    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");

    if (input && !(input->status & OX_INPUT_ST_NOT_SHOW) && loc) {
        ox_input_show_text(ctxt, input, loc, col);
    }
}

/**
 * Output note prompt message.
 * @param ctxt The current running context.
 * @param input The input generate the message.
 * @param loc Location where generate the message.
 * @param fmt Output format pattern.
 * @param ... Arguments.
 */
void
ox_note (OX_Context *ctxt, OX_Input *input, OX_Location *loc,
        const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    ox_prompt_v(ctxt, input, loc, OX_PROMPT_NOTE, fmt, ap);

    va_end(ap);
}

/**
 * Output warning prompt message.
 * @param ctxt The current running context.
 * @param input The input generate the message.
 * @param loc Location where generate the message.
 * @param fmt Output format pattern.
 * @param ... Arguments.
 */
void
ox_warning (OX_Context *ctxt, OX_Input *input, OX_Location *loc,
        const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    ox_prompt_v(ctxt, input, loc, OX_PROMPT_WARNING, fmt, ap);

    va_end(ap);
}

/**
 * Output error prompt message.
 * @param ctxt The current running context.
 * @param input The input generate the message.
 * @param loc Location where generate the message.
 * @param fmt Output format pattern.
 * @param ... Arguments.
 */
void
ox_error (OX_Context *ctxt, OX_Input *input, OX_Location *loc,
        const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    ox_prompt_v(ctxt, input, loc, OX_PROMPT_ERROR, fmt, ap);

    va_end(ap);
}

