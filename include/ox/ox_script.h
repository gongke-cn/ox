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
 * Script.
 */

#ifndef _OX_SCRIPT_H_
#define _OX_SCRIPT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Set the script's description.
 * @param ctxt The current running context.
 * @param s The script.
 * @param desc The script's description.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_script_set_desc (OX_Context *ctxt, OX_Value *s, const OX_ScriptDesc *desc);

/**
 * Set the script's internal value by its index.
 * @param ctxt The current running context.
 * @param s The script.
 * @param id The index of the value.
 * @param v The new value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_script_set_value (OX_Context *ctxt, OX_Value *s, int id, OX_Value *v);

/**
 * Get the script's internal value's pointer.
 * @param ctxt The current running context.
 * @param s The script.
 * @param id The index of the value.
 * @return The internal value's pointer.
 */
extern OX_Value*
ox_script_get_value (OX_Context *ctxt, OX_Value *s, int id);

/**
 * Get the current OX script.
 * @param ctxt The current running context.
 * @param[out] s Return the current script.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_script_current (OX_Context *ctxt, OX_Value *s);

/**
 * Load the script by its name.
 * @param ctxt The current running context.
 * @param base The name script try to load the script.
 * @param name The name of the script.
 * If name is starting with ".", ".." or "/", it means the pathname of the script.
 * Else the name is "PACKAGE/FILE".
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_script_load (OX_Context *ctxt, OX_Value *s, OX_Value *base, OX_Value *name);

/**
 * Set the script's text domain name.
 * @param ctxt The current running context.
 * @param s The script.
 * @param td The text domain name.
 * @param dir The base directory for message catalogs belonging to domain.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_script_set_text_domain (OX_Context *ctxt, OX_Value *s, OX_Value *td, const char *dir);

#ifdef __cplusplus
}
#endif

#endif
