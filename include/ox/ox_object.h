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
 * Object.
 */

#ifndef _OX_OBJECT_H_
#define _OX_OBJECT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Check if the value is an object.
 * @param ctxt The current running context.
 * @param v The value.
 * @retval OX_TRUE The value is an object.
 * @retval OX_FALSE The value is not an object.
 */
static inline OX_Bool
ox_value_is_object (OX_Context *ctxt, OX_Value *v)
{
    OX_GcObjectType type = ox_value_get_gco_type(ctxt, v);

    if (type == -1)
        return OX_FALSE;

    return (type & OX_GCO_FL_OBJECT) ? OX_TRUE : OX_FALSE;
}

/**
 * Get the object's interface.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param[out] inf Return the interface value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_object_get_interface (OX_Context *ctxt, OX_Value *o, OX_Value *inf)
{
    OX_Object *op;

    assert(ox_value_is_object(ctxt, o));

    op = ox_value_get_gco(ctxt, o);

    ox_value_copy(ctxt, inf, &op->inf);
    return OX_OK;
}

/**
 * Set the object's interface.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param inf The interface value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_object_set_interface (OX_Context *ctxt, OX_Value *o, OX_Value *inf)
{
    OX_Object *op;

    assert(ox_value_is_object(ctxt, o));
    assert(ox_value_is_null(ctxt, inf) || ox_value_is_interface(ctxt, inf));

    op = ox_value_get_gco(ctxt, o);

    ox_value_copy(ctxt, &op->inf, inf);
    return OX_OK;
}

/**
 * Set the object's operation functions.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param ops The operation functions.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
static inline OX_Result
ox_object_set_ops (OX_Context *ctxt, OX_Value *o, const OX_ObjectOps *ops)
{
    OX_Object *op;

    assert(ox_value_is_object(ctxt, o));

    op = ox_value_get_gco(ctxt, o);

    op->gco.ops = (OX_GcObjectOps*)ops;
    return OX_OK;
}

/**
 * Initialize an object.
 * @param ctxt The current running context.
 * @param o The object to be initialized.
 * @param inf The interface of the object.
 */
extern void
ox_object_init (OX_Context *ctxt, OX_Object *o, OX_Value *inf);

/**
 * Release the object.
 * @param ctxt The current running context.
 * @param o The object to released.
 */
extern void
ox_object_deinit (OX_Context *ctxt, OX_Object *o);

/**
 * Scan referenced objects in the object.
 * @param ctxt The current running context.
 * @param gco The object to be scanned.
 */
extern void
ox_object_scan (OX_Context *ctxt, OX_GcObject *gco);

/**
 * Scan referenced objects in the object.
 * @param ctxt The current running context.
 * @param gco The object to be scanned.
 */
extern void
ox_object_free (OX_Context *ctxt, OX_GcObject *gco);

/**
 * Lookup the owned property in the object.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param p The property's key.
 * @param[out] v Return the property value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_object_lookup (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v);

/**
 * Get the property value of an object.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param p The property's key.
 * @param[out] v Return the property value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_object_get (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v);

/**
 * Get the property value of an object with this argument.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param p The property's key.
 * @param[out] v Return the property value.
 * @param thiz This argument.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_object_get_t (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v, OX_Value *thiz);

/**
 * Set the property value of an object.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param p The property's key.
 * @param v The property's new value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_object_set (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v);

/**
 * Set the property value of an object with this argument.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param p The property's key.
 * @param v The property's new value.
 * @param thiz This argument.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_object_set_t (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v, OX_Value *thiz);

/**
 * Call the object value.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param thiz This argument.
 * @param args Arguments.
 * @param argc Count of the arguments.
 * @param[out] rv Return value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_object_call (OX_Context *ctxt, OX_Value *o, OX_Value *thiz, OX_Value *args,
        size_t argc, OX_Value *rv);

/**
 * Create a new object.
 * @param ctxt The current running context.
 * @param[out] o Return the new object.
 * @param inf The interface of the object.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_object_new (OX_Context *ctxt, OX_Value *o, OX_Value *inf);

/**
 * Create a new object from a class.
 * @param ctxt The current running context.
 * @param[out] o Return the new object.
 * @param c The class.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_object_from_class (OX_Context *ctxt, OX_Value *o, OX_Value *c);

/**
 * Set the object's private data.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param ops The private data's operation functions.
 * @param data The private data.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_object_set_priv (OX_Context *ctxt, OX_Value *o, const OX_PrivateOps *ops, void *data);

/**
 * Get the object's private data.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param ops The private data's operation functions.
 * @return The private data.
 * @retval NULL The private data is not set.
 */
extern void*
ox_object_get_priv (OX_Context *ctxt, OX_Value *o, const OX_PrivateOps *ops);

/**
 * Get the object's private data operations pointer.
 * @param ctxt The current running context.
 * @param o The object value.
 * @return The private operation pointer of the object.
 */
static inline const OX_PrivateOps*
ox_object_get_priv_ops (OX_Context *ctxt, OX_Value *o)
{
    OX_Object *op;

    assert(ctxt && o);
    
    if (!ox_value_is_object(ctxt, o))
        return NULL;

    op = ox_value_get_gco(ctxt, o);
    return op->priv_ops;
}

/**
 * Set the object's scope.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param scope The scope object value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_object_set_scope (OX_Context *ctxt, OX_Value *o, OX_Value *scope);

/**
 * Set the object's name.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param name The name value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_object_set_name (OX_Context *ctxt, OX_Value *o, OX_Value *name);

/**
 * Set the object's name.
 * name is a constant C string.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param name The name value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_object_set_name_s (OX_Context *ctxt, OX_Value *o, const char *name);

/**
 * Add a property to the object.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param type The property's type.
 * @param name The name of the property.
 * @param v1 Value 1 of the property.
 * @param v2 Value 2 of the property.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_object_add_prop (OX_Context *ctxt, OX_Value *o, OX_PropertyType type,
        OX_Value *name, OX_Value *v1, OX_Value *v2);

/**
 * Add a property to the object.
 * name is a constant C string.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param type The property's type.
 * @param name The name of the property.
 * @param v1 Value 1 of the property.
 * @param v2 Value 2 of the property.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_object_add_prop_s (OX_Context *ctxt, OX_Value *o, OX_PropertyType type,
        const char *name, OX_Value *v1, OX_Value *v2);

/**
 * Add a native method property to the object.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param name The name of the property.
 * @param cf The C function's pointer.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_object_add_n_method (OX_Context *ctxt, OX_Value *o,
        OX_Value *name, OX_CFunc cf);

/**
 * Add a native method property to the object.
 * name is a constant C string.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param name The name of the property.
 * @param cf The C function's pointer.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_object_add_n_method_s (OX_Context *ctxt, OX_Value *o,
        const char *name, OX_CFunc cf);

/**
 * Add a native accessor property to the object.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param name The name of the property.
 * @param get The C getter function's pointer.
 * @param set The C setter function's pointer.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_object_add_n_accessor (OX_Context *ctxt, OX_Value *o,
        OX_Value *name, OX_CFunc get, OX_CFunc set);

/**
 * Add a native accessor property to the object.
 * name is a constant C string.
 * @param ctxt The current running context.
 * @param o The object value.
 * @param name The name of the property.
 * @param get The C getter function's pointer.
 * @param set The C setter function's pointer.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_object_add_n_accessor_s (OX_Context *ctxt, OX_Value *o,
        const char *name, OX_CFunc get, OX_CFunc set);

/**
 * Add a variable property to the object.
 * @param c The current running context.
 * @param o The object value.
 * @param n The name of the property.
 * @param v The value of the property.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
#define ox_object_add_var(c, o, n, v)\
    ox_object_add_prop(c, o, OX_PROPERTY_VAR, n, v, NULL)

/**
 * Add a constant property to the object.
 * @param c The current running context.
 * @param o The object value.
 * @param n The name of the property.
 * @param v The value of the property.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
#define ox_object_add_const(c, o, n, v)\
    ox_object_add_prop(c, o, OX_PROPERTY_CONST, n, v, NULL)

/**
 * Add an accessor property to the object.
 * @param c The current running context.
 * @param o The object value.
 * @param n The name of the property.
 * @param g The getter of the accessor.
 * @param s the setter of the accessor.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
#define ox_object_add_accessor(c, o, n, g, s)\
    ox_object_add_prop(c, o, OX_PROPERTY_ACCESSOR, n, g, s)

/**
 * Add a variable property to the object.
 * n is a constant C string.
 * @param c The current running context.
 * @param o The object value.
 * @param n The name of the property.
 * @param v The value of the property.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
#define ox_object_add_var_s(c, o, n, v)\
    ox_object_add_prop_s(c, o, OX_PROPERTY_VAR, n, v, NULL)

/**
 * Add a constant property to the object.
 * n is a constant C string.
 * @param c The current running context.
 * @param o The object value.
 * @param n The name of the property.
 * @param v The value of the property.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
#define ox_object_add_const_s(c, o, n, v)\
    ox_object_add_prop_s(c, o, OX_PROPERTY_CONST, n, v, NULL)

/**
 * Add an accessor property to the object.
 * n is a constant C string.
 * @param c The current running context.
 * @param o The object value.
 * @param n The name of the property.
 * @param g The getter of the accessor.
 * @param s the setter of the accessor.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
#define ox_object_add_accessor_s(c, o, n, g, s)\
    ox_object_add_prop_s(c, o, OX_PROPERTY_ACCESSOR, n, g, s)

/**
 * Create a new object iterator.
 * @param ctxt The current running context.
 * @param[out] iter Return the new iterator,
 * @param o The object value.
 * @param type The iterator's type.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_object_iter_new (OX_Context *ctxt, OX_Value *iter, OX_Value *o, OX_ObjectIterType type);

/**
 * Delete a property of the object.
 * @param ctxt The current running context.
 * @param o The object.
 * @param p The property name.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_object_del (OX_Context *ctxt, OX_Value *o, OX_Value *p);

/**
 * Get the keys of the object.
 * @param ctxt The current running context.
 * @param o The object.
 * @param[out] keys Return the property keys array.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_object_keys (OX_Context *ctxt, OX_Value *o, OX_Value *keys);

/**
 * Set the keys of an object.
 * @param ctxt The current running context.
 * @param o The object.
 * @param keys The keys array.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
extern OX_Result
ox_object_set_keys (OX_Context *ctxt, OX_Value *o, const char **keys);

#ifdef __cplusplus
}
#endif

#endif
