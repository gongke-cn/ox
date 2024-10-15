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
 * GIConstantInfo.
 */

/*Convert the constant information to value.*/
static OX_Result
giconstantinfo_to_value (OX_Context *ctxt, OX_GIRepository *repo, GIConstantInfo *ci, OX_Value *v)
{
    GIArgument arg;
    OX_GICtxt gic;
    OX_GIOwn own = OX_GI_OWN_NOTHING;
    GITypeInfo *ti = gi_constant_info_get_type_info(ci);
    OX_Result r;

    gi_constant_info_get_value(ci, &arg);

    gictxt_init(&gic, repo, NULL, NULL, NULL);

    r = giargument_to_value(ctxt, &gic, &arg, &own, ti, v);

    gi_constant_info_free_value(ci, &arg);
    gi_base_info_unref(ti);

    return r;
}
