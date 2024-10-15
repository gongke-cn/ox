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
 * ASCII character functions.
 */

#define OX_LOG_TAG "char"

#include <ctype.h>
#include "std.h"

#define OX_FOREACH_CFUNC(c)\
    c(isalnum)\
    c(isalpha)\
    c(iscntrl)\
    c(isdigit)\
    c(isgraph)\
    c(islower)\
    c(isprint)\
    c(ispunct)\
    c(isspace)\
    c(isupper)\
    c(isxdigit)\
    c(isascii)\
    c(isblank)

/*Declaration index.*/
enum {
#define OX_CFUNC_ID(c) ID_##c,
    OX_FOREACH_CFUNC(OX_CFUNC_ID)
    ID_MAX
};

/*Public table.*/
static const char*
pub_tab[] = {
#define OX_CFUNC_NAME(c) #c,
    OX_FOREACH_CFUNC(OX_CFUNC_NAME)
    NULL
};

/*Script description.*/
static const OX_ScriptDesc
script_desc = {
    NULL,
    pub_tab,
    ID_MAX
};

/*Load this module.*/
OX_Result
ox_load (OX_Context *ctxt, OX_Value *s)
{
    ox_not_error(ox_script_set_desc(ctxt, s, &script_desc));
    return OX_OK;
}

/*?
 *? @lib Character classification functions.
 *?
 *? @func isalnum Check if the character is a letter or a digital number character.
 *? @param c {Number} The character.
 *? @return {Bool} The character is a letter, a digital number character or not.
 *?
 *? @func isalpha Check if the character is a letter.
 *? @param c {Number} The character.
 *? @return {Bool} The character is a letter or not.
 *?
 *? @func iscntrl Check if the character is a control character.
 *? @param c {Number} The character.
 *? @return {Bool} The character is a control character or not.
 *?
 *? @func isdigit Check if the character is a digital character (0 ~ 9).
 *? @param c {Number} The character.
 *? @return {Bool} The character is a digital character or not.
 *?
 *? @func isxdigit Check if the character is a hexadecimal digital character (0 ~ 9, a - f, A - F).
 *? @param c {Number} The character.
 *? @return {Bool} The character is a hexadecimal digital character or not.
 *?
 *? @func isgraph Check if the character is a graphic character (printable character except space).
 *? @param c {Number} The character.
 *? @return {Bool} The character is a graphic character or not.
 *?
 *? @func islower Check if the character is a lower letter.
 *? @param c {Number} The character.
 *? @return {Bool} The character is a lower letter or not.
 *?
 *? @func isupper Check if the character is an upper letter.
 *? @param c {Number} The character.
 *? @return {Bool} The character is an upper letter or not.
 *?
 *? @func isprint Check if the character is a printable letter.
 *? @param c {Number} The character.
 *? @return {Bool} The character is a printable letter or not.
 *?
 *? @func ispunct Check if the character is a punctuation letter.
 *? @param c {Number} The character.
 *? @return {Bool} The character is a punctuation letter or not.
 *?
 *? @func isspace Check if the character is a space character(\n, \r, \t, \v, \f, space).
 *? @param c {Number} The character.
 *? @return {Bool} The character is a space character or not.
 *?
 *? @func isblank Check if the character is a blank character(\t or space).
 *? @param c {Number} The character.
 *? @return {Bool} The character is a blank character or not.
 *?
 *? @func isascii Check if the character is an ASCII character.
 *? @param c {Number} The character.
 *? @return {Bool} The character is an ASCII character or not.
 */

#define OX_CFUNC_IMPL(cf)\
static OX_Result \
cfunc_##cf (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)\
{\
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);\
    int32_t c;\
    int ret;\
    OX_Result r;\
    if ((r = ox_to_int32(ctxt, arg, &c)) == OX_ERR)\
        return r;\
    if ((c >= 0) && (c <= UCHAR_MAX))\
        ret = cf(c);\
    else\
        ret = OX_FALSE;\
    ox_value_set_bool(ctxt, rv, ret ? OX_TRUE : OX_FALSE);\
    return OX_OK;\
}

OX_FOREACH_CFUNC(OX_CFUNC_IMPL)

/*Execute.*/
OX_Result
ox_exec (OX_Context *ctxt, OX_Value *f, OX_Value *s, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, v)

#define OX_CFUNC_ADD(c)\
    ox_not_error(ox_named_native_func_new_s(ctxt, v, cfunc_##c, NULL, #c));\
    ox_not_error(ox_script_set_value(ctxt, s, ID_##c, v));

    OX_FOREACH_CFUNC(OX_CFUNC_ADD)

    OX_VS_POP(ctxt, v)
    return OX_OK;
}
