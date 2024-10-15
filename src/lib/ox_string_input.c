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
 * String input.
 */

#define OX_LOG_TAG "ox_string_input"

#include "ox_internal.h"

/*Scan referenced objects in the string input.*/
static void
string_input_scan (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_StringInput *si = (OX_StringInput*)gco;

    ox_input_scan(ctxt, &si->input);

    ox_gc_scan_value(ctxt, &si->s);
}

/*Free the string input.*/
static void
string_input_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_StringInput *si = (OX_StringInput*)gco;

    ox_input_deinit(ctxt, &si->input);

    OX_DEL(ctxt, si);
}

/*Get a character from the string input.*/
static int
string_input_get_char (OX_Context *ctxt, OX_Input *input)
{
    OX_StringInput *si = (OX_StringInput*)input;
    size_t len = ox_string_length(ctxt, &si->s);
    size_t left = len - si->pos;
    const char *c = ox_string_get_char_star(ctxt, &si->s) + si->pos;
    OX_Bool error = OX_FALSE;
    int r;

    while (left) {
        r = ox_uc_from_utf8(c, &left);
        si->pos = len - left;
        if (r != -1)
            return r;

        if (!error) {
            OX_Location loc;
            
            error = OX_TRUE;
            si->input.status |= OX_INPUT_ST_ERR;

            ox_input_get_loc(input, &loc.first_line, &loc.first_column);
            ox_input_get_loc(input, &loc.last_line, &loc.last_column);

            ox_error(ctxt, input, &loc, OX_TEXT("illegal character"));
        }

        left --;
        c ++;

        while (left && ((*c & 0xc0) == 0x80)) {
            left --;
            c ++;
        }
    }

    return OX_INPUT_END;
}

/*Push back a character to the string input.*/
static void
string_input_unget_char (OX_Context *ctxt, OX_Input *input, int c)
{
    OX_StringInput *si = (OX_StringInput*)input;
    size_t n = ox_uc_utf8_length(c);

    assert(si->pos >= n);

    si->pos -= n;
}

/*Get the string input's current read position.*/
static long
string_input_tell (OX_Context *ctxt, OX_Input *input)
{
    OX_StringInput *si = (OX_StringInput*)input;

    return si->pos;
}

/*Reopen the string input.*/
static OX_Result
string_input_reopen (OX_Context *ctxt, OX_Input *input, OX_Value *v, long off)
{
    OX_StringInput *si = (OX_StringInput*)input;
    OX_StringInput *nsi;
    OX_Result r;

    if ((r = ox_string_input_new(ctxt, v, &si->s)) == OX_ERR)
        return r;

    nsi = ox_value_get_gco(ctxt, v);
    nsi->pos = off;

    return OX_OK;
}

/*String input's operation functions.*/
static const OX_InputOps
string_input_ops = {
    {
        OX_GCO_STRING_INPUT,
        string_input_scan,
        string_input_free
    },
    string_input_get_char,
    string_input_unget_char,
    string_input_tell,
    NULL,
    string_input_reopen
};

/*Build the string input's name.*/
static char*
string_input_name (OX_Context *ctxt, OX_Value *s)
{
    const char *sc;
    size_t len, left;
    char *name = NULL;
    char buf[32];
    char *dc = buf;

    sc = ox_string_get_char_star(ctxt, s);
    len = ox_string_length(ctxt, s);
    left = sizeof(buf) - 1;

    while ((len > 0) && (left > 0)) {
        switch (*sc) {
        case '\t':
            if (left < 2)
                goto end;
            dc[0] = '\\';
            dc[1] = 't';
            dc += 2;
            left -= 2;
            break;
        case '\n':
            if (left < 2)
                goto end;
            dc[0] = '\\';
            dc[1] = 'n';
            dc += 2;
            left -= 2;
            break;
        case '\v':
            if (left < 2)
                goto end;
            dc[0] = '\\';
            dc[1] = 'v';
            dc += 2;
            left -= 2;
            break;
        case '\f':
            if (left < 2)
                goto end;
            dc[0] = '\\';
            dc[1] = 'f';
            dc += 2;
            left -= 2;
            break;
        case '\r':
            if (left < 2)
                goto end;
            dc[0] = '\\';
            dc[1] = 'r';
            dc += 2;
            left -= 2;
            break;
        case '\a':
            if (left < 2)
                goto end;
            dc[0] = '\\';
            dc[1] = 'a';
            dc += 2;
            left -= 2;
            break;
        case '\b':
            if (left < 2)
                goto end;
            dc[0] = '\\';
            dc[1] = 'b';
            dc += 2;
            left -= 2;
            break;
        case '\"':
            if (left < 2)
                goto end;
            dc[0] = '\\';
            dc[1] = '\"';
            dc += 2;
            left -= 2;
            break;
        case '\\':
            if (left < 2)
                goto end;
            dc[0] = '\\';
            dc[1] = '\\';
            dc += 2;
            left -= 2;
            break;
        default:
            if (ox_char_is_graph(*sc) || (*sc == ' ')) {
                *dc = *sc;
                dc ++;
                left --;
            } else {
                uint8_t c = *sc;

                if (left < 4)
                    goto end;

                dc[0] = '\\';
                dc[1] = 'x';
                dc[2] = ox_hex_char_from_num(c >> 4);
                dc[3] = ox_hex_char_from_num(c & 0xf);

                dc += 4;
                left -= 4;
            }
        }

        sc ++;
        len --;
    }

end:
    if (len) {
        if (left < 3) {
            dc -= 3 - left;
        }

        *dc ++ = '.';
        *dc ++ = '.';
        *dc ++ = '.';
    }

    *dc = 0;

    name = ox_strdup(ctxt, buf);

    return name;
}

/**
 * Create a new string input.
 * @param ctxt The current running context.
 * @param[out] inputv Return the new input value.
 * @param s The input string.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_string_input_new (OX_Context *ctxt, OX_Value *inputv, OX_Value *s)
{
    OX_StringInput *si;
    char *name;

    assert(ctxt && inputv && s);
    assert(ox_value_is_string(ctxt, s));

    name = string_input_name(ctxt, s);

    if (!OX_NEW(ctxt, si)) {
        if (name)
            ox_strfree(ctxt, name);
        return ox_throw_no_mem_error(ctxt);
    }

    si->input.gco.ops = (OX_GcObjectOps*)&string_input_ops;

    ox_input_init(ctxt, &si->input);

    si->input.name = name;
    ox_value_copy(ctxt, &si->s, s);
    si->pos = 0;

    ox_value_set_gco(ctxt, inputv, si);
    ox_gc_add(ctxt, si);

    return OX_OK;
}