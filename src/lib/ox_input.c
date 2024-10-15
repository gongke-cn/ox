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
 * Input.
 */

#define OX_LOG_TAG "ox_input"

#include "ox_internal.h"

#define OX_TEXT_LINE_WIDTH 72

/*Add a location stub.*/
static void
input_add_loc_stub (OX_Context *ctxt, OX_Input *input, long off)
{
    OX_InputLocStub stub;
    int line, col;
    size_t len;

    line = input->line;
    col = input->column;

    if ((len = ox_vector_length(&input->loc_stubs))) {
        OX_InputLocStub *old = &ox_vector_item(&input->loc_stubs, len - 1);

        if ((old->line == line) && (old->column == col))
            return;
    }

    stub.line = line;
    stub.column = col;
    stub.offset = off;

    ox_not_error(ox_vector_append(ctxt, &input->loc_stubs, stub));
}

/*Lookup the location stub.*/
static void
input_lookup_loc_stub (OX_Context *ctxt, OX_Input *input, int line, int col,
        int *pline, int *pcol, long *poff)
{
    size_t min, max, mid;
    OX_InputLocStub *stub;

    min = 0;
    max = ox_vector_length(&input->loc_stubs);

    if (max == 0) {
        *pline = 1;
        *pcol = 1;
        *poff = 0;
        return;
    }

    while (max > min + 1) {
        
        mid = (min + max) / 2;

        stub = &ox_vector_item(&input->loc_stubs, mid);

        if (stub->line > line) {
            max = mid;
        } else if (stub->line < line) {
            min = mid;
        } else if (stub->column > col) {
            max = mid;
        } else if (stub->column < col) {
            min = mid;
        } else {
            *pline = stub->line;
            *pcol = stub->column;
            *poff = stub->offset;
            return;
        }
    }

    stub = &ox_vector_item(&input->loc_stubs, min);

    if ((stub->line < line) || ((stub->line == line) && (stub->column <= col))) {
        *pline = stub->line;
        *pcol = stub->column;
        *poff = stub->offset;
    } else {
        *pline = 1;
        *pcol = 0;
        *poff = 0;
    }
}

/**
 * Get a character from the input.
 * @param ctxt The current running context.
 * @param input The input.
 * @return Return an unicode character read from input.
 * @retval OX_INPUT_END The input is end.
 */
int
ox_input_get_char (OX_Context *ctxt, OX_Input *input)
{
    OX_InputOps *ops;
    int c;
    long off = 0;

    assert(!(input->status & OX_INPUT_ST_CLOSED));

    if (input->status & OX_INPUT_ST_LF) {
        input->status &= ~OX_INPUT_ST_LF;
        input->line ++;
        input->column = 0;
    }

    ops = (OX_InputOps*)input->gco.ops;

    if (((input->counter + 1) & 0xfff) == 0)
        off = ops->tell(ctxt, input);

    c = ops->get_char(ctxt, input);

    if (c >= 0) {
        input->column ++;
        input->counter ++;

        if (off)
            input_add_loc_stub(ctxt, input, off);

        if (c == '\n')
            input->status |= OX_INPUT_ST_LF;
    }

    return c;
}

/**
 * Push back a character to the input.
 * @param ctxt The current running context.
 * @param input The input.
 */
void
ox_input_unget_char (OX_Context *ctxt, OX_Input *input, int c)
{
    assert(!(input->status & OX_INPUT_ST_CLOSED));
    
    if (c >= 0) {
        OX_InputOps *ops;

        input->status &= ~OX_INPUT_ST_LF;
        input->column --;
        input->counter --;

        ops = (OX_InputOps*)input->gco.ops;

        ops->unget_char(ctxt, input, c);
    }
}

/**
 * Close the input.
 * @param ctxt The current running context.
 * @param inputv The input value to be closed.
 */
void
ox_input_close (OX_Context *ctxt, OX_Value *inputv)
{
    OX_Input *input;

    assert(ctxt && inputv);
    assert(ox_value_is_input(ctxt, inputv));

    input = ox_value_get_gco(ctxt, inputv);

    if (!(input->status & OX_INPUT_ST_CLOSED)) {
        OX_InputOps *ops;

        input->status |= OX_INPUT_ST_CLOSED;

        ops = (OX_InputOps*)input->gco.ops;

        if (ops->close)
            ops->close(ctxt, input);
    }
}

/**
 * Show the input's text of the location.
 * @param ctxt The current running context.
 * @param input The input.
 * @param loc Location of the text.
 * @param col Color of the text.
 */
void
ox_input_show_text (OX_Context *ctxt, OX_Input *input, OX_Location *loc, const char *col)
{
    int sline, scol, cline, ccol, cstart, cend;
    long off;
    int c;
    OX_InputOps *ops;
    OX_Input *ninput = NULL;
    OX_VS_PUSH(ctxt, inputv)
    OX_CharBuffer text, cursor;
    const char *tcstr, *ccstr;
    OX_Bool cursor_start = OX_FALSE;
    OX_Bool cursor_end = OX_FALSE;
    int width = 0, start = 0;
    OX_Bool has_cursor = loc->first_column ? OX_TRUE : OX_FALSE;

    ox_char_buffer_init(&text);
    ox_char_buffer_init(&cursor);

    sline = loc->first_line;
    scol = has_cursor ? loc->first_column : 1;

    if (has_cursor) {
        if (loc->first_line == loc->last_line) {
            if (loc->last_column - loc->first_column >= OX_TEXT_LINE_WIDTH)
                scol = loc->first_line;
            else if (loc->last_column > OX_TEXT_LINE_WIDTH)
                scol = loc->last_column - OX_TEXT_LINE_WIDTH;
            else
                scol = 1;
        } else {
            if (scol > OX_TEXT_LINE_WIDTH)
                scol -= OX_TEXT_LINE_WIDTH;
            else
                scol = 1;
        }
    }

    /*Lookup the location.*/
    input_lookup_loc_stub(ctxt, input, sline, scol, &cline, &ccol, &off);

    ops = (OX_InputOps*)input->gco.ops;

    /*Open a new input.*/
    if ((ops->reopen(ctxt, input, inputv, off) == OX_ERR))
        goto end;

    ninput = ox_value_get_gco(ctxt, inputv);

    /*Read the line.*/
    cstart = loc->first_column;
    cend = (loc->first_line == loc->last_line) ? loc->last_column : INT_MAX;

    while (1) {
        c = ox_input_get_char(ctxt, ninput);

        if ((cline < sline) || (ccol < scol)) {
            if (c == OX_INPUT_END)
                goto end;

            if (c == '\n') {
                cline ++;
                ccol = 1;
            } else {
                ccol ++;
            }

            continue;
        }

        if (c != OX_INPUT_END) {
            if (ox_char_is_space(c)) {
                ox_not_error(ox_char_buffer_append_char(ctxt, &text, ' '));
            } else if (ox_char_is_graph(c)) {
                ox_not_error(ox_char_buffer_append_char(ctxt, &text, c));
            } else {
                ox_not_error(ox_char_buffer_append_char(ctxt, &text, 'X'));
            }
        }

        if (has_cursor) {
            if ((ccol >= cstart) && (ccol <= cend)) {
                if (!cursor_start) {
                    cursor_start = OX_TRUE;
                    if (col) {
                        ox_not_error(ox_char_buffer_append_char_star(ctxt, &cursor, col));
                    }
                }
                ox_not_error(ox_char_buffer_append_char(ctxt, &cursor, '~'));
            } else {
                if (cursor_start && !cursor_end) {
                    cursor_end = OX_TRUE;
                    if (col) {
                        ox_not_error(ox_char_buffer_append_char_star(ctxt, &cursor, "\033[0m"));
                    }
                }
                ox_not_error(ox_char_buffer_append_char(ctxt, &cursor, ' '));
            }
        }

        width ++;
        if (width > OX_TEXT_LINE_WIDTH) {
            start ++;
            width --;
        }

        if ((c == OX_INPUT_END) || (c == '\n'))
            break;

        ccol ++;

        if (ccol - cstart >= OX_TEXT_LINE_WIDTH)
            break;
    }

    if (cursor_start && !cursor_end) {
        if (col) {
            ox_not_error(ox_char_buffer_append_char_star(ctxt, &cursor, "\033[0m"));
        }
    }

    ox_not_null(tcstr = ox_char_buffer_get_char_star(ctxt, &text));
    fprintf(stderr, "%5d | %s\n", sline, tcstr + start);

    if (has_cursor) {
        ox_not_null(ccstr = ox_char_buffer_get_char_star(ctxt, &cursor));
        fprintf(stderr, "      | %s\n", ccstr + start);
    }
end:
    if (ninput)
        ox_input_close(ctxt, inputv);

    ox_char_buffer_deinit(ctxt, &text);
    ox_char_buffer_deinit(ctxt, &cursor);
    OX_VS_POP(ctxt, inputv)
}