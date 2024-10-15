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
 * Memory allocate and free functions.
 */

#define OX_LOG_TAG "ox_mem"

#include "ox_internal.h"

/**
 * Resize a member buffer.
 * @param ctxt The current running context.
 * @param optr The old buffer's pointer.
 * @param osize The old buffer's size.
 * @param nsize The new buffer's size.
 * @return The new buffer's pointer.
 * @retval NULL Cannot allocate the new buffer if nsize is not 0.
 */
void*
ox_realloc (OX_Context *ctxt, void *optr, size_t osize, size_t nsize)
{
    OX_VM *vm = ox_vm_get(ctxt);
    void *nptr;

    if (nsize) {
        nptr = realloc(optr, nsize);
        if (nptr) {
            vm->mem_allocted += nsize - osize;
            vm->mem_max_allocated = OX_MAX(vm->mem_max_allocated, vm->mem_allocted);
        } else {
            OX_LOG_F(ctxt, "realloc %"PRIdPTR" -> %"PRIdPTR" failed",
                    osize, nsize);
        }
    } else {
        free(optr);
        vm->mem_allocted -= osize;
        nptr = NULL;
    }

    return nptr;
}

/**
 * Initialize the memory manager data in the context.
 * @param ctxt The running context.
 */
void
ox_mem_init (OX_Context *ctxt)
{
    OX_VM *vm = ox_vm_get(ctxt);

    vm->mem_allocted = 0;
    vm->mem_max_allocated = 0;
}

/**
 * Release the memry manager data in the context.
 * @param ctxt The running context.
 */
void
ox_mem_deinit (OX_Context *ctxt)
{
    OX_VM *vm = ox_vm_get(ctxt);

    OX_LOG_D(ctxt, "maximum allocated memory: %"PRIdPTR"B",
            vm->mem_max_allocated);

    if (vm->mem_allocted) {
        OX_LOG_E(ctxt, "unfreed memory: %"PRIdPTR"B",
                vm->mem_allocted);
    }
}
