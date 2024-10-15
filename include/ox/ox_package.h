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
 * Package manager.
 */

#ifndef _OX_PACKAGE_H_
#define _OX_PACKAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Add a package lookup directory.
 * @param ctxt The current running context.
 * @param dir The directory to be added.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_package_add_dir (OX_Context *ctxt, const char *dir);

/**
 * Lookup the package.
 * @param ctxt The current running context.
 * @param name The name of the package.
 * @param[out] pkg Return the package information.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_package_lookup (OX_Context *ctxt, OX_Value *name, OX_Value *pkg);

/**
 * Get the packge script.
 * @param ctxt The current running context.
 * @param name The name of the package.
 * @param[out] script Return the script of the package.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_package_script (OX_Context *ctxt, OX_Value *name, OX_Value *script);

/**
 * Get the library from the package.
 * @param ctxt The current running context.
 * @param pkg The package information.
 * @param lib The library name.
 * @param path The library's pathname.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_package_get_lib (OX_Context *ctxt, OX_Value *pkg, OX_Value *lib, OX_Value *path);

/**
 * Get the executable program from the package.
 * @param ctxt The current running context.
 * @param pkg The package information.
 * @param exe The executable program's name.
 * @param path The library's pathname.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_package_get_exe (OX_Context *ctxt, OX_Value *pkg, OX_Value *exe, OX_Value *path);

#ifdef __cplusplus
}
#endif

#endif
