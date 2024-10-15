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
 * Random number.
 */

#define OX_LOG_TAG "rand"

#include "std.h"

/*Declaration index.*/
enum {
    ID_rand,
    ID_srand,
    ID_MAX
};

/*Public table.*/
static const char*
pub_tab[] = {
    "rand",
    "srand",
    NULL
};

/*Script description.*/
static const OX_ScriptDesc
script_desc = {
    NULL,
    pub_tab,
    ID_MAX
};

/*rand.*/
static OX_Result
RAND_rand (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    int i;

    i = rand();

    ox_value_set_number(ctxt, rv, i);
    return OX_OK;
}

/*srand.*/
static OX_Result
RAND_srand (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    uint32_t i;
    OX_Result r;

    if ((r = ox_to_uint32(ctxt, arg, &i)) == OX_ERR)
        return r;

    srand(i);

    return OX_OK;
}

/*Load this module.*/
OX_Result
ox_load (OX_Context *ctxt, OX_Value *s)
{
    ox_not_error(ox_script_set_desc(ctxt, s, &script_desc));
    return OX_OK;
}

/*?
 *? @lib Pseudo-random number generator.
 *?
 *? @func rand Returns a pseudo-random integer in the range 0 to 0x7fffffff inclusive.
 *? @return {Number} The pseudo-random integer number.
 *?
 *? @func srand Set the seed of the pseudo-random number generator.
 *? @param seed {Number} The seed of the pseudo-random number generator.
 */

/*Execute.*/
OX_Result
ox_exec (OX_Context *ctxt, OX_Value *f, OX_Value *s, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, v)

    /*rand*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, RAND_rand, NULL, "rand"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_rand, v));

    /*srand*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, RAND_srand, NULL, "srand"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_srand, v));

    OX_VS_POP(ctxt, v)
    return OX_OK;
}
