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
 * Log
 */

#define OX_LOG_TAG "log"

#include "ox_internal.h"

/*Declaration index.*/
enum {
    ID_Log,
    ID_STR_level,
    ID_STR_tag,
    ID_MAX
};

/*Public table.*/
static const char*
pub_tab[] = {
    "Log",
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

/*Log.level getter.*/
static OX_Result
Log_level_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_LogLevel l;

    l = ox_log_get_level(ctxt);

    ox_value_set_number(ctxt, rv, l);
    return OX_OK;
}

/*Log.level setter.*/
static OX_Result
Log_level_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    int32_t i;
    OX_Result r;

    if ((r = ox_to_int32(ctxt, arg, &i)) == OX_ERR)
        return r;

    if ((i > OX_LOG_LEVEL_NONE) || (i < OX_LOG_LEVEL_ALL))
        return ox_throw_range_error(ctxt, OX_TEXT("level value overflow"));

    ox_log_set_level(ctxt, i);

    return OX_OK;
}

/*Log.fields getter.*/
static OX_Result
Log_fields_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_LogField fv;

    fv = ox_log_get_fields(ctxt);

    ox_value_set_number(ctxt, rv, fv);
    return OX_OK;
}

/*Log.fields setter.*/
static OX_Result
Log_fields_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    int32_t i;
    OX_Result r;

    if ((r = ox_to_int32(ctxt, arg, &i)) == OX_ERR)
        return r;

    ox_log_set_fields(ctxt, i);

    return OX_OK;
}

/*Log.$inf.$init.*/
static OX_Result
Log_inf_init (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *tag_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *level_arg = ox_argument(ctxt, args, argc, 1);
    OX_VS_PUSH_2(ctxt, tag, level);
    int32_t lv = OX_LOG_LEVEL_ALL;
    OX_Result r;

    if (ox_value_is_null(ctxt, tag_arg)) {
        if ((r = ox_string_from_const_char_star(ctxt, tag, "log")) == OX_ERR)
            goto end;
    } else if ((r = ox_to_string(ctxt, tag_arg, tag)) == OX_ERR) {
        goto end;
    }

    if (!ox_value_is_null(ctxt, level_arg)) {
        if ((r = ox_to_int32(ctxt, level_arg, &lv)) == OX_ERR)
            goto end;
    }

    ox_value_set_number(ctxt, level, lv);

    if ((r = ox_set(ctxt, thiz, ox_script_get_value(ctxt, f, ID_STR_tag), tag)) == OX_ERR)
        goto end;

    if ((r = ox_set(ctxt, thiz, ox_script_get_value(ctxt, f, ID_STR_level), level)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    OX_VS_POP(ctxt, tag);
    return r;
}

/*Output log message.*/
static OX_Result
Log_inf_output (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_LogLevel level)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH_6(ctxt, msg, tag, level_p, tag_str, func_name, file_name);
    int32_t level_i, line = 0;
    const char *tag_cstr = NULL, *msg_cstr, *file = NULL, *func = NULL;
    OX_LogLevel log_level = ox_log_get_level(ctxt);
    OX_LogField log_field = ox_log_get_fields(ctxt);
    OX_Result r;

    /*Level check.*/
    if (level < log_level) {
        r = OX_OK;
        goto end;
    }

    if ((r = ox_get_throw(ctxt, thiz, ox_script_get_value(ctxt, f, ID_STR_level), level_p)) == OX_ERR)
        goto end;

    if ((r = ox_to_int32(ctxt, level_p, &level_i)) == OX_ERR)
        goto end;

    if (level < level_i) {
        r = OX_OK;
        goto end;
    }

    /*Get log message.*/
    if ((r = ox_to_string(ctxt, arg, msg)) == OX_ERR)
        goto end;

    msg_cstr = ox_string_get_char_star(ctxt, msg);

    /*Get tag.*/
    if (log_field & OX_LOG_FIELD_TAG) {
        if ((r = ox_get_throw(ctxt, thiz, ox_script_get_value(ctxt, f, ID_STR_tag), tag)) == OX_ERR)
            goto end;

        if ((r = ox_to_string(ctxt, tag, tag_str)) == OX_ERR)
            goto end;

        tag_cstr = ox_string_get_char_star(ctxt, tag_str);
    }

    /*Get filename, function name and line number.*/
    if (log_field & (OX_LOG_FIELD_FILE|OX_LOG_FIELD_FUNC|OX_LOG_FIELD_LINE)) {
        OX_Frame *frame = ctxt->frames;

        if (frame->bot) {
            frame = frame->bot;

            if (log_field & OX_LOG_FIELD_FUNC) {
                if ((r = ox_get_full_name(ctxt, &frame->func, func_name)) == OX_ERR)
                    goto end;

                func = ox_string_get_char_star(ctxt, func_name);
            }

            if (ox_value_get_gco_type(ctxt, &frame->func) == OX_GCO_FUNCTION) {
                if (log_field & OX_LOG_FIELD_FILE) {
                    OX_Function *fn = ox_value_get_gco(ctxt, &frame->func);
                    OX_BcScript *bs = fn->sfunc->script;

                    if ((r = ox_string_from_char_star(ctxt, file_name, bs->input->name)) == OX_ERR)
                        return r;

                    file = ox_string_get_char_star(ctxt, file_name);
                }

                if (log_field & OX_LOG_FIELD_LINE) {
                    line = ox_function_lookup_line(ctxt, &frame->func, frame->ip);
                }
            }
        }
    }

    ox_log(ctxt, level, tag_cstr, file, func, line, "%s", msg_cstr);
    r = OX_OK;
end:
    OX_VS_POP(ctxt, msg);
    return r;
}

/*Log.$inf.debug.*/
static OX_Result
Log_inf_debug (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    return Log_inf_output(ctxt, f, thiz, args, argc, OX_LOG_LEVEL_DEBUG);
}

/*Log.$inf.info.*/
static OX_Result
Log_inf_info (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    return Log_inf_output(ctxt, f, thiz, args, argc, OX_LOG_LEVEL_INFO);
}

/*Log.$inf.warning.*/
static OX_Result
Log_inf_warning (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    return Log_inf_output(ctxt, f, thiz, args, argc, OX_LOG_LEVEL_WARNING);
}

/*Log.$inf.error.*/
static OX_Result
Log_inf_error (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    return Log_inf_output(ctxt, f, thiz, args, argc, OX_LOG_LEVEL_ERROR);
}

/*Log.$inf.fatal.*/
static OX_Result
Log_inf_fatal (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    return Log_inf_output(ctxt, f, thiz, args, argc, OX_LOG_LEVEL_FATAL);
}

/*?
 *? @lib Log functions.
 *?
 *? @class{ Log Log.
 *?
 *? @const ALL {Number} Output all log message.
 *? @const NONE {Number} Do not output any log message.
 *? @const DEBUG {Number} Ouput log message which level greater than or equal to debug.
 *? @const INFO {Number} Ouput log message which level greater than or equal to normal information.
 *? @const WARNING {Number} Ouput log message which level greater than or equal to warning.
 *? @const ERROR {Number} Ouput log message which level greater than or equal to error.
 *? @const FATAL {Number} Ouput log message which level greater than or equal to fatal error.
 *?
 *? @const ALL_FIELDS {Number} Output all fields of the log message.
 *? @const LEVEL {Number} Output log level field.
 *? @const DATE {Number} Output date field.
 *? @const TIME {Number} Output time field.
 *? @const MSEC {Number} Output microseconds field.
 *? @const THREAD {Number} Output thread id field.
 *? @const TAG {Number} Output log tag field.
 *? @const FILE {Number} Output filename field.
 *? @const FUNC {Number} Output function name field.
 *? @const LINE {Number} Output line number field.
 *?
 *? @sacc level {Number} The log which level grater than or equal to it should be outputted.
 *? @sacc fields {Number} The enabled log fields.
 *?
 *? @func $init Initialize a new log object.
 *? @param tag {?String} The log tag.
 *? @param level {?Number} The log level which greater than or equal to it should be outputted.
 *?
 *? @func debug Output debug log message.
 *? @param msg {String} The log message.
 *?
 *? @func info Output normal information log message.
 *? @param msg {String} The log message.
 *?
 *? @func warning Output warning log message.
 *? @param msg {String} The log message.
 *?
 *? @func error Output error log message.
 *? @param msg {String} The log message.
 *?
 *? @func fatal Output fatal error log message.
 *? @param msg {String} The log message.
 *?
 *? @class}
 */

/*Execute.*/
OX_Result
ox_exec (OX_Context *ctxt, OX_Value *f, OX_Value *s, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH_3(ctxt, c, inf, v)

    /*Log.*/
    ox_not_error(ox_named_class_new_s(ctxt, c, inf, NULL, "Log"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_Log, c));

    /*Strings.*/
    ox_not_error(ox_string_from_const_char_star(ctxt, v, "level"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_STR_level, v));
    ox_not_error(ox_string_from_const_char_star(ctxt, v, "tag"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_STR_tag, v));

    /*Log levels.*/
    ox_value_set_number(ctxt, v, OX_LOG_LEVEL_ALL);
    ox_not_error(ox_object_add_const_s(ctxt, c, "ALL", v));
    ox_value_set_number(ctxt, v, OX_LOG_LEVEL_NONE);
    ox_not_error(ox_object_add_const_s(ctxt, c, "NONE", v));
    ox_value_set_number(ctxt, v, OX_LOG_LEVEL_DEBUG);
    ox_not_error(ox_object_add_const_s(ctxt, c, "DEBUG", v));
    ox_value_set_number(ctxt, v, OX_LOG_LEVEL_INFO);
    ox_not_error(ox_object_add_const_s(ctxt, c, "INFO", v));
    ox_value_set_number(ctxt, v, OX_LOG_LEVEL_WARNING);
    ox_not_error(ox_object_add_const_s(ctxt, c, "WARNING", v));
    ox_value_set_number(ctxt, v, OX_LOG_LEVEL_ERROR);
    ox_not_error(ox_object_add_const_s(ctxt, c, "ERROR", v));
    ox_value_set_number(ctxt, v, OX_LOG_LEVEL_FATAL);
    ox_not_error(ox_object_add_const_s(ctxt, c, "FATAL", v));

    /*Log fields.*/
    ox_value_set_number(ctxt, v, OX_LOG_FIELD_ALL);
    ox_not_error(ox_object_add_const_s(ctxt, c, "ALL_FIELDS", v));
    ox_value_set_number(ctxt, v, OX_LOG_FIELD_LEVEL);
    ox_not_error(ox_object_add_const_s(ctxt, c, "LEVEL", v));
    ox_value_set_number(ctxt, v, OX_LOG_FIELD_DATE);
    ox_not_error(ox_object_add_const_s(ctxt, c, "DATE", v));
    ox_value_set_number(ctxt, v, OX_LOG_FIELD_TIME);
    ox_not_error(ox_object_add_const_s(ctxt, c, "TIME", v));
    ox_value_set_number(ctxt, v, OX_LOG_FIELD_MSEC);
    ox_not_error(ox_object_add_const_s(ctxt, c, "MSEC", v));
    ox_value_set_number(ctxt, v, OX_LOG_FIELD_TAG);
    ox_not_error(ox_object_add_const_s(ctxt, c, "TAG", v));
    ox_value_set_number(ctxt, v, OX_LOG_FIELD_FILE);
    ox_not_error(ox_object_add_const_s(ctxt, c, "FILE", v));
    ox_value_set_number(ctxt, v, OX_LOG_FIELD_FUNC);
    ox_not_error(ox_object_add_const_s(ctxt, c, "FUNC", v));
    ox_value_set_number(ctxt, v, OX_LOG_FIELD_LINE);
    ox_not_error(ox_object_add_const_s(ctxt, c, "LINE", v));
    ox_value_set_number(ctxt, v, OX_LOG_FIELD_THREAD);
    ox_not_error(ox_object_add_const_s(ctxt, c, "THREAD", v));

    /*Log.level*/
    ox_not_error(ox_object_add_n_accessor_s(ctxt, c, "level", Log_level_get, Log_level_set));

    /*Log.fields*/
    ox_not_error(ox_object_add_n_accessor_s(ctxt, c, "fields", Log_fields_get, Log_fields_set));

    /*Log.$inf.$init*/
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$init", Log_inf_init));

    /*Log.$inf.debug*/
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "debug", Log_inf_debug));

    /*Log.$inf.info*/
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "info", Log_inf_info));

    /*Log.$inf.warning*/
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "warning", Log_inf_warning));

    /*Log.$inf.error*/
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "error", Log_inf_error));

    /*Log.$inf.fatal*/
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "fatal", Log_inf_fatal));

    OX_VS_POP(ctxt, c)
    return OX_OK;
}