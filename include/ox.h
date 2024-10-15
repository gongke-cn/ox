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
 * OX language.
 */

#ifndef _OX_H_
#define _OX_H_

#include <inttypes.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <ffi.h>
#include <assert.h>

#include "ox/ox_macros.h"
#include "ox/ox_types.h"
#include "ox/ox_log.h"
#include "ox/ox_mem.h"
#include "ox/ox_vector.h"
#include "ox/ox_char_buffer.h"
#include "ox/ox_list.h"
#include "ox/ox_hash.h"
#include "ox/ox_value.h"
#include "ox/ox_gc.h"
#include "ox/ox_error.h"
#include "ox/ox_string.h"
#include "ox/ox_array.h"
#include "ox/ox_function.h"
#include "ox/ox_interface.h"
#include "ox/ox_object.h"
#include "ox/ox_class.h"
#include "ox/ox_operation.h"
#include "ox/ox_iterator.h"
#include "ox/ox_input.h"
#include "ox/ox_prompt.h"
#include "ox/ox_char.h"
#include "ox/ox_parser.h"
#include "ox/ox_compile.h"
#include "ox/ox_package.h"
#include "ox/ox_script.h"
#include "ox/ox_enum.h"
#include "ox/ox_re.h"
#include "ox/ox_ctype.h"
#include "ox/ox_vm.h"
#include "ox/ox_context.h"
#include "ox/ox_json.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif
