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
 * Class.
 */

#define OX_LOG_TAG "ox_class"

#include "ox_internal.h"

/*Free the class.*/
static void
class_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_Class *c = (OX_Class*)gco;

    ox_object_deinit(ctxt, &c->o);
    OX_DEL(ctxt, c);
}

/*Call a generic class.*/
static OX_Result
gen_class_call (OX_Context *ctxt, OX_Value *o, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH_2(ctxt, inf, tv)
    OX_Class *c = ox_value_get_gco(ctxt, o);
    OX_Result r;

    r = ox_get_throw(ctxt, o, OX_STRING(ctxt, _inf), inf);
    if (r == OX_OK) {
        if (c->alloc)
            r = c->alloc(ctxt, rv, inf);
        else
            r = ox_object_new(ctxt, rv, inf);

        if (r == OX_OK)
            r = ox_try_call_method(ctxt, rv, OX_STRING(ctxt, _init), args, argc, tv);
    }

    OX_VS_POP(ctxt, inf)
    return r;
}

/*Call the class.*/
static OX_Result
class_call (OX_Context *ctxt, OX_Value *o, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Result r;

    if (ox_equal(ctxt, o, OX_OBJECT(ctxt, Bool)))
        r = ox_bool_class_call(ctxt, o, thiz, args, argc, rv);
    else if (ox_equal(ctxt, o, OX_OBJECT(ctxt, Number)))
        r = ox_number_class_call(ctxt, o, thiz, args, argc, rv);
    else if (ox_equal(ctxt, o, OX_OBJECT(ctxt, String)))
        r = ox_string_class_call(ctxt, o, thiz, args, argc, rv);
    else if (ox_equal(ctxt, o, OX_OBJECT(ctxt, Function)))
        r = ox_function_class_call(ctxt, o, thiz, args, argc, rv);
    else
        r = gen_class_call(ctxt, o, thiz, args, argc, rv);

    return r;
}

/*Class's operation functions.*/
static const OX_ObjectOps
class_ops = {
    {
        OX_GCO_CLASS,
        ox_object_scan,
        class_free
    },
    ox_object_keys,
    ox_object_lookup,
    ox_object_get,
    ox_object_set,
    ox_object_del,
    class_call
};

/**
 * Create a new class.
 * @param ctxt The current running context.
 * @param[out] c Return the new class object.
 * @param[out] inf Return the new interface object.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_class_new (OX_Context *ctxt, OX_Value *c, OX_Value *inf)
{
    OX_Value *infv = NULL;
    OX_Class *cp;
    OX_Result r;

    assert(ctxt && c);

    if (!inf) {
        infv = ox_value_stack_push(ctxt);
        inf = infv;
    }

    if (!OX_NEW(ctxt, cp)) {
        r = OX_ERR;
        goto end;
    }

    ox_object_init(ctxt, &cp->o, NULL);
    cp->o.gco.ops = (OX_GcObjectOps*)&class_ops;
    cp->alloc = NULL;

    ox_value_set_gco(ctxt, c, cp);
    ox_gc_add(ctxt, cp);

    if ((r = ox_interface_new(ctxt, inf)) == OX_ERR)
        goto end;

    if ((r = ox_object_add_const(ctxt, c, OX_STRING(ctxt, _inf), inf)) == OX_ERR)
        goto end;

    if ((r = ox_object_add_const(ctxt, inf, OX_STRING(ctxt, _class), c)) == OX_ERR)
        goto end;

    if ((r = ox_object_set_scope(ctxt, inf, c)) == OX_ERR)
        return r;

    if ((r = ox_object_set_name(ctxt, inf, OX_STRING(ctxt, _inf)) == OX_ERR))
        return r;

    r = OX_OK;
end:
    if (infv)
        ox_value_stack_pop(ctxt, infv);
    return r;
}

/**
 * Set the class's object allocate function.
 * @param ctxt The current running context.
 * @param c The class.
 * @param alloc The object allocate function.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_class_set_alloc_func (OX_Context *ctxt, OX_Value *c, OX_AllocObjectFunc alloc)
{
    OX_Class *cp;

    assert(ctxt && c && alloc);
    assert(ox_value_is_class(ctxt, c));

    cp = ox_value_get_gco(ctxt, c);

    cp->alloc = alloc;
    return OX_OK;
}

/**
 * Create a new class with its scope and name.
 * @param ctxt The current running context.
 * @param[out] c Return the new class object.
 * @param[out] inf Return the new interface object.
 * @param scope The scope of the class.
 * @param name The name of the class.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_named_class_new (OX_Context *ctxt, OX_Value *c, OX_Value *inf,
        OX_Value *scope, OX_Value *name)
{
    OX_Result r;

    if ((r = ox_class_new(ctxt, c, inf)) == OX_ERR)
        return r;

    if (scope && !ox_value_is_null(ctxt, scope))
        if ((r = ox_object_set_scope(ctxt, c, scope)) == OX_ERR)
            return r;

    if (name && !ox_value_is_null(ctxt, name))
        if ((r = ox_object_set_name(ctxt, c, name)) == OX_ERR)
            return r;

    return OX_OK;
}

/**
 * Create a new class with its scope and name.
 * name is a constant C string.
 * @param ctxt The current running context.
 * @param[out] c Return the new class object.
 * @param[out] inf Return the new interface object.
 * @param scope The scope of the class.
 * @param name The name of the class.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_named_class_new_s (OX_Context *ctxt, OX_Value *c, OX_Value *inf,
        OX_Value *scope, const char *name)
{
    OX_VS_PUSH(ctxt, nv)
    OX_Result r;

    assert(name);

    r = ox_string_from_const_char_star(ctxt, nv, name);

    if (r == OX_OK)
        r = ox_named_class_new(ctxt, c, inf, scope, nv);

    OX_VS_POP(ctxt, nv)
    return r;
}

/**
 * Inherit the properties from the parent class.
 * @param ctxt The current running context.
 * @param c The class value.
 * @param pc The parent class value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_class_inherit (OX_Context *ctxt, OX_Value *c, OX_Value *pc)
{
    OX_VS_PUSH_3(ctxt, pn, inf, pinf)
    OX_Class *cp;
    OX_Result r;

    assert(ctxt && c && pc);
    assert(ox_value_is_class(ctxt, c));

    /*Set allocate function.*/
    cp = ox_value_get_gco(ctxt, c);

    if (ox_value_is_class(ctxt, pc)) {
        OX_Class *pp = ox_value_get_gco(ctxt, pc);

        if (pp->alloc) {
            if (cp->alloc && (cp->alloc != pp->alloc)) {
                r = ox_throw_type_error(ctxt,
                        OX_TEXT("class contains multiple parents with private allocation functions"));
                goto end;
            }
            cp->alloc = pp->alloc;
        }
    }

    if ((r = ox_get_throw(ctxt, c, OX_STRING(ctxt, _inf), inf)) == OX_ERR)
        goto end;

    if (!ox_value_is_interface(ctxt, inf)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("property \"$inf\" is not an interface"));
        goto end;
    }

    if ((r = ox_get_throw(ctxt, pc, OX_STRING(ctxt, _inf), pinf)) == OX_ERR)
        goto end;

    if (!ox_value_is_interface(ctxt, pinf)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("property \"$inf\" is not an interface"));
        goto end;
    }

    r = ox_interface_inherit(ctxt, inf, pinf);
end:
    OX_VS_POP(ctxt, pn)
    return r;
}