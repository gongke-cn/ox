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
 * JSON.
 */

#define OX_LOG_TAG "json"

#include <ox_internal.h>

/*Declaration index.*/
enum {
    ID_JSON,
    ID_MAX
};

/*Public table.*/
static const char*
pub_tab[] = {
    "JSON",
    NULL
};

/*Script descrition.*/
static const OX_ScriptDesc
script_desc = {
    NULL,
    pub_tab,
    ID_MAX
};

/*json_from_file.*/
static OX_Result
json_from_file (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *fn = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH_2(ctxt, fn_str, input)
    const char *fn_cstr;
    OX_Result r;

    if ((r = ox_to_string(ctxt, fn, fn_str)) == OX_ERR)
        goto end;

    fn_cstr = ox_string_get_char_star(ctxt, fn_str);
    if ((r = ox_file_input_new(ctxt, input, fn_cstr)) == OX_ERR)
        goto end;

    r = ox_json_parse(ctxt, input, OX_TRUE, rv);
end:
    if (!ox_value_is_null(ctxt, input))
        ox_input_close(ctxt, input);
    OX_VS_POP(ctxt, fn_str)
    return r;
}

/*json_from_str.*/
static OX_Result
json_from_str (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH_2(ctxt, str, input)
    OX_Result r;

    if ((r = ox_to_string(ctxt, arg, str)) == OX_ERR)
        goto end;

    if ((r = ox_string_input_new(ctxt, input, str)) == OX_ERR)
        goto end;

    r = ox_json_parse(ctxt, input, OX_FALSE, rv);
end:
    if (!ox_value_is_null(ctxt, input))
        ox_input_close(ctxt, input);
    OX_VS_POP(ctxt, str)
    return r;
}

/*json_to_str.*/
static OX_Result
json_to_str (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *v = ox_argument(ctxt, args, argc, 0);
    OX_Value *indent_arg = ox_argument(ctxt, args, argc, 1);
    OX_Value *filter = ox_argument(ctxt, args, argc, 2);
    OX_Value *map = ox_argument(ctxt, args, argc, 3);
    OX_VS_PUSH(ctxt, indent)
    OX_Result r;

    if (!ox_value_is_null(ctxt, indent_arg)) {
        if ((r = ox_to_string(ctxt, indent_arg, indent)) == OX_ERR)
            goto end;
    }

    if ((r = ox_json_to_str(ctxt, v, indent, filter, map, rv)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    OX_VS_POP(ctxt, indent)
    return r;
}

/*Load this module.*/
OX_Result
ox_load (OX_Context *ctxt, OX_Value *s)
{
    ox_not_error(ox_script_set_desc(ctxt, s, &script_desc));
    return OX_OK;
}

/*?
 *? @package json JSON parser and generater.
 *? @lib JSON parser and generater.
 *?
 *? @callback JsonToStrFilterFn JSON to string filter function.
 *? @param owner The owner object of the item.
 *? @param key {String|Number} The key of the item.
 *? @param item The item value.
 *? @return {Bool} If the function returns false, the item will be ignored.
 *?
 *? @callback JsonToStrMaprFn JSON to string map function.
 *? @param owner The owner object of the item.
 *? @param key {String|Number} The key of the item.
 *? @param item The original item value.
 *? @return The new item value.
 *?
 *? @object{ JSON JSON
 *?
 *? @func from_file Load a JSON object from a file.
 *? @param filename {String} The JSON file's name.
 *? @return Return the JSON object.
 *? @throw {AccessError} Cannot open the file.
 *? @throw {SystemError} Read file failed.
 *? @throw {SyntaxError} The JSON syntax error.
 *?
 *? @func from_str Load a JSON object from a string.
 *? @param str {String} The JSON string.
 *? @return Return the JSON object.
 *? @throw {SyntaxError} The JSON syntax error.
 *?
 *? @func to_str Convert the value to a JSON string.
 *? @param v The value.
 *? @param indent {?String} Indent string.
 *? If indent is null, the generated JSON string is in single line.
 *? If indent is no null, the generated JSON string is in multi line,
 *? and this string is used as line start indentation.
 *? @param filer {?JsonToStrFilterFn} Filter function.
 *? If the value is an array or an object, json_to_str traverses all the items and properties.
 *? During the traversal of array or object, the filter function will be called.
 *? If the function returns false, the item or the property is not added to the JSON string.
 *? If the filter function is null, all the items or the properties will be added to the JSON string.
 *? @param map {?JsonToStrMapFn} Map function.
 *? During the traversal of array or object, the map function will be called to convert
 *? the item or the property to another value. If the map function is null, the items or
 *? the properties will maintain its original value.
 *? @return {String} The JSON string.
 *? @throw {ReferenceError} Ths value has circular reference in it.
 *?
 *? @object}
 */

/*Execute.*/
OX_Result
ox_exec (OX_Context *ctxt, OX_Value *f, OX_Value *s, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, v)

    /*JSON*/
    ox_not_error(ox_object_new(ctxt, v, NULL));
    ox_not_error(ox_script_set_value(ctxt, s, ID_JSON, v));

    /*JSON.from_file*/
    ox_not_error(ox_object_add_n_method_s(ctxt, v, "from_file", json_from_file));

    /*JSON.from_str*/
    ox_not_error(ox_object_add_n_method_s(ctxt, v, "from_str", json_from_str));

    /*JSON.to_str*/
    ox_not_error(ox_object_add_n_method_s(ctxt, v, "to_str", json_to_str));

    OX_VS_POP(ctxt, v)
    return OX_OK;
}
