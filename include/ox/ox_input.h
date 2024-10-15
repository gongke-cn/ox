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
 * Character input.
 */

#ifndef _OX_INPUT_H_
#define _OX_INPUT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Check if the input is an input.
 * @param ctxt The current running context.
 * @param v The value.
 * @retval OX_TRUE The value is an input.
 * @retval OX_FALSE The value is not an input.
 */
static inline OX_Bool
ox_value_is_input (OX_Context *ctxt, OX_Value *v)
{
    OX_GcObjectType type = ox_value_get_gco_type(ctxt, v);

    if (type == -1)
        return OX_FALSE;

    return (type & OX_GCO_FL_INPUT) ? OX_TRUE : OX_FALSE;
}

/**
 * Check if the input has error.
 * @param input The input.
 * @retval OX_TRUE The input has error.
 * @retval OX_FALSE The input has not error.
 */
static inline OX_Bool
ox_input_error (OX_Input *input)
{
    return (input->status & OX_INPUT_ST_ERR) ? OX_TRUE : OX_FALSE;
}

/**
 * Initialize a new input.
 * @param ctxt The current running context.
 * @param input The input to be initialized.
 */
static inline void
ox_input_init (OX_Context *ctxt, OX_Input *input)
{
    input->name = NULL;
    input->status = 0;
    input->line = 1;
    input->column = 0;
    input->counter = 0;

    ox_vector_init(&input->loc_stubs);
}

/**
 * Release the input.
 * @param ctxt The current running context.
 * @param input The input to be released.
 */
static inline void
ox_input_deinit (OX_Context *ctxt, OX_Input *input)
{
    if (input->name)
        ox_strfree(ctxt, input->name);

    ox_vector_deinit(ctxt, &input->loc_stubs);
}

/**
 * Scan referenced objects in the input.
 * @param ctxt The current running context.
 * @param input The input to be scanned.
 */
static inline void
ox_input_scan (OX_Context *ctxt, OX_Input *input)
{
}

/**
 * Get the current read position of the input.
 * @param input The input.
 * @param[out] line Return the current line number.
 * @param[out] col Return the current column number.
 */
static inline void
ox_input_get_loc (OX_Input *input, int *line, int *col)
{
    if (line)
        *line = input->line;
    if (col)
        *col = input->column;
}

/**
 * Get a character from the input.
 * @param ctxt The current running context.
 * @param input The input.
 * @return Return an unicode character read from input.
 * @retval OX_INPUT_END The input is end.
 */
extern int
ox_input_get_char (OX_Context *ctxt, OX_Input *input);

/**
 * Push back a character to the input.
 * @param ctxt The current running context.
 * @param input The input.
 */
extern void
ox_input_unget_char (OX_Context *ctxt, OX_Input *input, int c);

/**
 * Close the input.
 * @param ctxt The current running context.
 * @param inputv The input value to be closed.
 */
extern void
ox_input_close (OX_Context *ctxt, OX_Value *inputv);

/**
 * Create a new string input.
 * @param ctxt The current running context.
 * @param[out] inputv Return the new input value.
 * @param s The input string.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_string_input_new (OX_Context *ctxt, OX_Value *inputv, OX_Value *s);

/**
 * Create a new file input.
 * @param ctxt The current running context.
 * @param[out] inputv Return the new input value.
 * @param filename The filename.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_file_input_new (OX_Context *ctxt, OX_Value *inputv, const char *filename);

#ifdef __cplusplus
}
#endif

#endif
