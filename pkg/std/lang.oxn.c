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
 * OX language functions.
 */

#define OX_LOG_TAG "lang"

#include <stdlib.h>
#include "std.h"

/*Declaration index.*/
enum {
    ID_OX,
    ID_StackEntry_inf,
    ID_MAX
};

/*Public table.*/
static const char*
pub_tab[] = {
    "OX",
    NULL
};

/*Script description.*/
static const OX_ScriptDesc
script_desc = {
    NULL,
    pub_tab,
    ID_MAX
};

/** Stack entry.*/
typedef struct {
    OX_Frame *frame; /**< The frame.*/
    int       ip;    /**< The instruction pointer.*/
} StackEntry;

/*Scan referenced objects in the stack entry.*/
static void
stack_entry_scan (OX_Context *ctxt, void *p)
{
    StackEntry *se = p;

    ox_gc_mark(ctxt, se->frame);
}

/*Free the stack entry.*/
static void
stack_entry_free (OX_Context *ctxt, void *p)
{
    StackEntry *se = p;

    OX_DEL(ctxt, se);
}

/*Stack entry's operation functions.*/
static const OX_PrivateOps
stack_entry_ops = {
    stack_entry_scan,
    stack_entry_free
};

/*Create a new stack entry object.*/
static OX_Result
stack_entry_new (OX_Context *ctxt, OX_Value *v, OX_Frame *f, OX_Value *inf)
{
    StackEntry *se;
    OX_Result r;

    if ((r = ox_object_new(ctxt, v, inf)) == OX_ERR)
        return r;

    if (!OX_NEW(ctxt, se))
        return ox_throw_no_mem_error(ctxt);

    se->frame = f;
    se->ip = f->ip;

    if ((r = ox_object_set_priv(ctxt, v, &stack_entry_ops, se)) == OX_ERR) {
        stack_entry_free(ctxt, se);
        return r;
    }

    return OX_OK;
}

/*Get the stack entry data from the value.*/
static StackEntry*
stack_entry_get (OX_Context *ctxt, OX_Value *v)
{
    StackEntry *se = ox_object_get_priv(ctxt, v, &stack_entry_ops);

    if (!se)
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a stack entry"));

    return se;
}

/*StackEntry.$inf.function get.*/
static OX_Result
StackEntry_inf_function_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    StackEntry *se;

    if (!(se = stack_entry_get(ctxt, thiz)))
        return OX_ERR;

    ox_value_copy(ctxt, rv, &se->frame->func);
    return OX_OK;
}

/*StackEntry.$inf.filename get.*/
static OX_Result
StackEntry_inf_filename_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    StackEntry *se;
    OX_Result r;

    if (!(se = stack_entry_get(ctxt, thiz)))
        return OX_ERR;

    if (ox_value_get_gco_type(ctxt, &se->frame->func) == OX_GCO_FUNCTION) {
        OX_Function *func = ox_value_get_gco(ctxt, &se->frame->func);
        OX_BcScript *bs = func->sfunc->script;

        if ((r = ox_string_from_char_star(ctxt, rv, bs->input->name)) == OX_ERR)
            return r;
    } else {
        OX_NativeFunc *nf = ox_value_get_gco(ctxt, &se->frame->func);
        OX_Script *ns = nf->script;

        if (ns->he.key) {
            if ((r = ox_string_from_char_star(ctxt, rv, ns->he.key)) == OX_ERR)
                return r;
        } else {
            ox_value_set_null(ctxt, rv);
        }
    }

    return OX_OK;
}

/*StackEntry.$inf.line get.*/
static OX_Result
StackEntry_inf_line_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    StackEntry *se;
    int line;

    if (!(se = stack_entry_get(ctxt, thiz)))
        return OX_ERR;

    if (ox_value_get_gco_type(ctxt, &se->frame->func) == OX_GCO_FUNCTION) {
        line = ox_function_lookup_line(ctxt, &se->frame->func, se->ip);
    } else {
        line = 0;
    }
    
    ox_value_set_number(ctxt, rv, line);
    return OX_OK;
}

/*Lookup the name in the frame.*/
static OX_Result
stack_lookup (OX_Context *ctxt, OX_Frame *f, OX_Value *n, OX_Frame **pf, OX_ScriptDecl **psd)
{
    OX_ScriptDecl *sd;
    OX_Frame *cf = f;
    OX_Function *func;
    OX_String *k;
    OX_Result r;

    if ((r = ox_string_singleton(ctxt, n)) == OX_ERR)
        return r;

    func = ox_value_get_gco(ctxt, &cf->func);
    k = ox_value_get_gco(ctxt, n);
    sd = ox_hash_lookup_c(ctxt, &func->sfunc->decl_hash, k, NULL, OX_ScriptDecl, he);
    if (!sd) {
        OX_Frame **sf, **ef;

        sf = func->frames;
        ef = sf + func->sfunc->frame_num;

        while (sf < ef) {
            cf = *sf;
            func = ox_value_get_gco(ctxt, &cf->func);
            sd = ox_hash_lookup_c(ctxt, &func->sfunc->decl_hash, k, NULL, OX_ScriptDecl, he);
            if (sd)
                break;

            sf ++;
        }
    }

    *pf = cf;
    *psd = sd;
    return OX_OK;
}

/*StackEntry.$inf.get.*/
static OX_Result
StackEntry_inf_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *n_arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, n)
    StackEntry *se;
    OX_Result r;

    if (!(se = stack_entry_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_to_string(ctxt, n_arg, n)) == OX_ERR)
        goto end;

    if (ox_value_get_gco_type(ctxt, &se->frame->func) == OX_GCO_FUNCTION) {
        OX_Frame *tf;
        OX_ScriptDecl *sd;

        if ((r = stack_lookup(ctxt, se->frame, n, &tf, &sd)) == OX_ERR)
            goto end;

        if (r)
            ox_value_copy(ctxt, rv, &tf->v[sd->id]);
        else
            ox_value_set_null(ctxt, rv);
    } else {
        ox_value_set_null(ctxt, rv);
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, n)
    return r;
}

/*StackEntry.$inf.set.*/
static OX_Result
StackEntry_inf_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *n_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *v = ox_argument(ctxt, args, argc, 1);
    OX_VS_PUSH(ctxt, n)
    StackEntry *se;
    OX_Result r;

    if (!(se = stack_entry_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_to_string(ctxt, n_arg, n)) == OX_ERR)
        goto end;

    if (ox_value_get_gco_type(ctxt, &se->frame->func) == OX_GCO_FUNCTION) {
        OX_Frame *tf;
        OX_ScriptDecl *sd;

        if ((r = stack_lookup(ctxt, se->frame, n, &tf, &sd)) == OX_ERR)
            goto end;

        if (!r) {
            r = ox_throw_reference_error(ctxt, OX_TEXT("cannot find \"%s\""),
                    ox_string_get_char_star(ctxt, n));
            goto end;
        }

        if ((sd->type == OX_DECL_CONST) || (sd->type == OX_DECL_REF)) {
            r = ox_throw_access_error(ctxt, OX_TEXT("cannot set the constant \"%s\""),
                    ox_string_get_char_star(ctxt, n));
            goto end;
        }

        ox_value_copy(ctxt, &tf->v[sd->id], v);
    } else {
        r = ox_throw_reference_error(ctxt, OX_TEXT("cannot set variable in native function"));
        goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, n)
    return r;
}

/*Allocate and set the language data.*/
static OX_Result
lang_set (OX_Context *ctxt, OX_Value *s)
{
    OX_VS_PUSH(ctxt, inf)
    OX_Result r;

    if ((r = ox_interface_new(ctxt, inf)) == OX_ERR)
        goto end;

    if ((r = ox_script_set_value(ctxt, s, ID_StackEntry_inf, inf)) == OX_ERR)
        goto end;

    ox_object_add_n_accessor_s(ctxt, inf, "function", StackEntry_inf_function_get, NULL);
    ox_object_add_n_accessor_s(ctxt, inf, "filename", StackEntry_inf_filename_get, NULL);
    ox_object_add_n_accessor_s(ctxt, inf, "line", StackEntry_inf_line_get, NULL);
    ox_object_add_n_method_s(ctxt, inf, "get", StackEntry_inf_get);
    ox_object_add_n_method_s(ctxt, inf, "set", StackEntry_inf_set);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, inf)
    return r;
}

/*OX.eval.*/
static OX_Result
eval_func (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH_4(ctxt, s, input, ast, script)
    OX_Bool input_ok = OX_FALSE;
    OX_Result r;

    if ((r = ox_to_string(ctxt, arg, s)) == OX_ERR)
        goto end;

    if ((r = ox_string_input_new(ctxt, input, s)) == OX_ERR)
        goto end;
    input_ok = OX_TRUE;

    if ((r = ox_parse(ctxt, input, ast, 0)) == OX_ERR)
        goto end;

    if ((r = ox_compile(ctxt, input, ast, script, OX_COMPILE_FL_CURR|OX_COMPILE_FL_EXPR)) == OX_ERR)
        goto end;

    ox_decompile(ctxt, script, stderr);

    if ((r = ox_call(ctxt, script, ox_value_null(ctxt), NULL, 0, rv)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    if (input_ok)
        ox_input_close(ctxt, input);
    OX_VS_POP(ctxt, s)
    return r;
}

/*OX.script.*/
static OX_Result
script_func (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH_2(ctxt, s, base)
    OX_Result r;

    if ((r = ox_to_string(ctxt, arg, s)) == OX_ERR)
        goto end;

    if ((r = ox_script_current(ctxt, base)) == OX_ERR)
        goto end;

    r = ox_script_load(ctxt, rv, base, s);
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*OX.file.*/
static OX_Result
file_func (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH_3(ctxt, s, input, ast)
    char path[PATH_MAX];
    const char *cstr, *rp;
    OX_Script *sp;
    OX_Result r;

    if ((r = ox_to_string(ctxt, arg, s)) == OX_ERR)
        goto end;

    cstr = ox_string_get_char_star(ctxt, s);

    if ((r = ox_file_input_new(ctxt, input, cstr)) == OX_ERR)
        goto end;

    if ((r = ox_parse(ctxt, input, ast, 0)) == OX_ERR)
        goto end;

    if ((r = ox_compile(ctxt, input, ast, rv, OX_COMPILE_FL_EXPR)) == OX_ERR)
        goto end;

    /*Store the pathname.*/
    if (!(rp = realpath(cstr, path))) {
        r = std_system_error(ctxt, "realpath");
        goto end;
    }

    sp = ox_value_get_gco(ctxt, rv);
    if (!(sp->he.key = ox_strdup(ctxt, rp))) {
        r = OX_ERR;
        goto end;
    }

    r = OX_OK;
end:
    if (!ox_value_is_null(ctxt, input))
        ox_input_close(ctxt, input);
    OX_VS_POP(ctxt, s)
    return r;
}

/*OX.stack.*/
static OX_Result
stack_func (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *size_arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, item)
    size_t size = SIZE_MAX;
    size_t i = 0;
    OX_Value *inf = ox_script_get_value(ctxt, f, ID_StackEntry_inf);
    OX_Frame *frame;
    OX_Result r;

    if (!ox_value_is_null(ctxt, size_arg)) {
        if ((r = ox_to_index(ctxt, size_arg, &size)) == OX_ERR)
            goto end;
    }

    if ((r = ox_array_new(ctxt, rv, 0)) == OX_ERR)
        goto end;

    frame = ox_frame_get(ctxt);
    while (frame && (i < size)) {
        if ((r = stack_entry_new(ctxt, item, frame, inf)) == OX_ERR)
            goto end;
        if ((r = ox_array_set_item(ctxt, rv, i, item)) == OX_ERR)
            goto end;

        frame = frame->bot;
        i ++;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, item)
    return r;
}

/*OX.ast_from_file.*/
static OX_Result
ast_from_file_func (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *fn_arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH_2(ctxt, fn_str, input)
    const char *fn;
    OX_Result r;

    if ((r = ox_to_string(ctxt, fn_arg, fn_str)) == OX_ERR)
        goto end;

    fn = ox_string_get_char_star(ctxt, fn_str);

    if ((r = ox_file_input_new(ctxt, input, fn)) == OX_ERR)
        goto end;

    if ((r = ox_parse(ctxt, input, rv, OX_PARSE_FL_RETURN|OX_PARSE_FL_DOC)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    if (!ox_value_is_null(ctxt, input))
        ox_input_close(ctxt, input);
    OX_VS_POP(ctxt, fn_str)
    return r;
}

/*OX.ast_from_str.*/
static OX_Result
ast_from_str_func (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH_2(ctxt, str, input)
    OX_Result r;

    if ((r = ox_to_string(ctxt, arg, str)) == OX_ERR)
        goto end;

    if ((r = ox_string_input_new(ctxt, input, str)) == OX_ERR)
        goto end;

    if ((r = ox_parse(ctxt, input, rv, OX_PARSE_FL_RETURN|OX_PARSE_FL_DOC)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    if (!ox_value_is_null(ctxt, input))
        ox_input_close(ctxt, input);
    OX_VS_POP(ctxt, str)
    return r;
}

/*OX.install_dir getter.*/
static OX_Result
OX_install_dir_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    const char *dir = ox_get_install_dir(ctxt);
    OX_Result r;

    if (dir) {
        r = ox_string_from_const_char_star(ctxt, rv, dir);
    } else {
        ox_value_set_null(ctxt, rv);
        r = OX_OK;
    }

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
 *? @package std Standard libraries.
 */

/*?
 *? @lib OX language related functions.
 *?
 *? @object{ OX OX language object.
 *?
 *? @func eval Parse and run the OX script source code.
 *? @param src {String} The source string of the script.
 *? @return The return value of the script.
 *? @throw {SyntaxError} The source code has syntax error.
 *?
 *? @func script Load the script file.
 *? @param name {String} The name of the script.
 *? @ul{
 *? @li If name is starting with "/", it is the absolute pathname of the script file.
 *? @li If name is starting with "./" or "../", it is the relative pathname of the script file.
 *? @li If the name format is similar to "PACKAGE/SCRIPT", it descript a script library managed by package manager,
 *? @li "PACKAGE" is the package name, "SCRIPT" is the exported library name of the script.
 *? @li If the name format is similar to "PACKAGE", it descript a script library managed by package manager, the exported library name is same as "PACKAGE".
 *? @ul}
 *? @return {Object} The script object.
 *? @throw {ReferenceError} Cannot find the script.
 *? @throw {SyntaxError} The script has syntax error.
 *?
 *? @func file Load a file as OX script.
 *? @param filename {String} The filename of the script.
 *? @return {Object} The script object.
 *? @throw {AccessError} Cannot open the file.
 *? @throw {SystemError} Read the script file failed.
 *? @throw {SyntaxError} The script has syntax error.
 *?
 *? @func stack Get the OX stack entries' information.
 *? @param depth {?Number} The depth of the stack to be got.
 *? If depth is null, means to get the full stack.
 *? @return {[StackEntry]} The stack entries' information array.
 *?
 *? @func ast_from_file Parse the script file to abstract syntax tree.
 *? @param filename {String} The script file's name.
 *? @return The abstract syntax tree object.
 *? @throw {AccessError} Cannot open the file.
 *? @throw {SystemError} Read the script file failed.
 *? @throw {SyntaxError} The script has syntax error.
 *?
 *? @func ast_from_str Parse the source string to abstract syntax tree.
 *? @param src {String} The script source string.
 *? @return The abstract syntax tree object.
 *? @throw {SyntaxError} The script has syntax error.
 *?
 *? @const package_dirs {[String]} The package lookup directories array,
 *? @const install_dir {String} The OX installation directory.
 *? @const lib_dir {String} The OX libraries directory.
 *? @const target {String} The target architecture of the OX program,
 *? @const arch {String} The architecture name.
 *? @const vendor {String} The machine vendor name.
 *? @const os {String} The operating system name.
 *? @const abi {String} The ABI name.
 *? @const libs {String} The linker flags of the OX program.
 *? @const version {String} The version number of OX.
 *?
 *? @object}
 */

/*Execute.*/
OX_Result
ox_exec (OX_Context *ctxt, OX_Value *f, OX_Value *s, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH_2(ctxt, o, v)
    const char *b, *e;

    ox_not_error(lang_set(ctxt, s));

    /*Object OX.*/
    ox_not_error(ox_object_new(ctxt, o, NULL));
    ox_not_error(ox_script_set_value(ctxt, s, ID_OX, o));

    /*OX.eval.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, eval_func, o, "eval"));
    ox_not_error(ox_object_add_const_s(ctxt, o, "eval", v));

    /*OX.script.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, script_func, o, "script"));
    ox_not_error(ox_object_add_const_s(ctxt, o, "script", v));

    /*OX.file.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, file_func, o, "file"));
    ox_not_error(ox_object_add_const_s(ctxt, o, "file", v));

    /*OX.stack.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, stack_func, o, "stack"));
    ox_not_error(ox_object_add_const_s(ctxt, o, "stack", v));

    /*OX.ast_from_file.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, ast_from_file_func, o, "ast_from_file"));
    ox_not_error(ox_object_add_const_s(ctxt, o, "ast_from_file", v));

    /*OX.ast_from_str.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, ast_from_str_func, o, "ast_from_str"));
    ox_not_error(ox_object_add_const_s(ctxt, o, "ast_from_str", v));

    /*OX.package_dirs.*/
    ox_not_error(ox_package_get_dirs(ctxt, v));
    ox_not_error(ox_object_add_const_s(ctxt, o, "package_dirs", v));

    /*OX.install_dir.*/
    ox_not_error(ox_object_add_n_accessor_s(ctxt, o, "install_dir", OX_install_dir_get, NULL));

    /*OX.lib_dir.*/
    ox_not_error(ox_string_from_const_char_star(ctxt, v, ox_get_lib_dir(ctxt)));
    ox_not_error(ox_object_add_const_s(ctxt, o, "lib_dir", v));

    /*OX.target.*/
    ox_not_error(ox_string_from_const_char_star(ctxt, v, TARGET));
    ox_not_error(ox_object_add_const_s(ctxt, o, "target", v));

    /*OX.arch*/
    b = TARGET;
    e = strchr(b, '-');
    ox_not_error(ox_string_from_chars(ctxt, v, b, e - b));
    ox_not_error(ox_object_add_const_s(ctxt, o, "arch", v));

    /*OX.vendor*/
    b = e + 1;
    e = strchr(b, '-');
    ox_not_error(ox_string_from_chars(ctxt, v, b, e - b));
    ox_not_error(ox_object_add_const_s(ctxt, o, "vendor", v));

    /*OX.os*/
    b = e + 1;
    e = strchr(b, '-');
    ox_not_error(ox_string_from_chars(ctxt, v, b, e - b));
    ox_not_error(ox_object_add_const_s(ctxt, o, "os", v));

    /*OX.abi*/
    b = e + 1;
    ox_not_error(ox_string_from_char_star(ctxt, v, b));
    ox_not_error(ox_object_add_const_s(ctxt, o, "abi", v));

    /*OX.libs.*/
    ox_not_error(ox_string_from_const_char_star(ctxt, v, ox_get_ld_flags()));
    ox_not_error(ox_object_add_const_s(ctxt, o, "libs", v));

    /*OX.version.*/
    ox_not_error(ox_string_from_const_char_star(ctxt, v, ox_get_version()));
    ox_not_error(ox_object_add_const_s(ctxt, o, "version", v));

    OX_VS_POP(ctxt, o)
    return OX_OK;
}
