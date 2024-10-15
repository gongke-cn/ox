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
 * System functions.
 */

#define OX_LOG_TAG "system"

#include <stdlib.h>
#include <unistd.h>
#include "std.h"

/*Declaration index.*/
enum {
    ID_system,
    ID_getenv,
    ID_setenv,
    ID_msleep,
    ID_getcwd,
    ID_chdir,
    ID_gettid,
    ID_MAX
};

/*Public table.*/
static const char*
pub_tab[] = {
    "system",
    "getenv",
    "setenv",
    "msleep",
    "getcwd",
    "chdir",
    "gettid",
    NULL
};

/*Script descrition.*/
static const OX_ScriptDesc
script_desc = {
    NULL,
    pub_tab,
    ID_MAX
};

/*system.*/
static OX_Result
system_func (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, s)
    const char *cmd;
    OX_Result r;

    if ((r = ox_to_string(ctxt, arg, s)) == OX_ERR)
        goto end;

    cmd = ox_string_get_char_star(ctxt, s);
    r = system(cmd);
    if (r == -1) {
        r = std_system_error_v(ctxt, "system", errno, "\"%s\"", cmd);
        goto end;
    }

    ox_value_set_number(ctxt, rv, r);
    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*getenv.*/
static OX_Result
getenv_func (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *vn_arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, vn_s)
    const char *vn;
    char *val;
    OX_Result r;

    if ((r = ox_to_string(ctxt, vn_arg, vn_s)) == OX_ERR)
        goto end;

    vn = ox_string_get_char_star(ctxt, vn_s);

    val = getenv(vn);
    if (val) {
        r = ox_string_from_char_star(ctxt, rv, val);
    } else {
        ox_value_set_null(ctxt, rv);
        r = OX_OK;
    }
end:
    OX_VS_POP(ctxt, vn_s)
    return r;
}

/*setenv.*/
static OX_Result
setenv_func (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *vn_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *vv_arg = ox_argument(ctxt, args, argc, 1);
    OX_VS_PUSH_2(ctxt, vn_s, vv_s)
    const char *vn, *vv;
    OX_Result r;

    if ((r = ox_to_string(ctxt, vn_arg, vn_s)) == OX_ERR)
        goto end;

    vn = ox_string_get_char_star(ctxt, vn_s);

    if (!ox_value_is_null(ctxt, vv_arg)) {
        if ((r = ox_to_string(ctxt, vv_arg, vv_s)) == OX_ERR)
            goto end;

        vv = ox_string_get_char_star(ctxt, vv_s);
    } else {
        vv = NULL;
    }

#ifdef ARCH_WIN
    {
        size_t nlen = ox_string_length(ctxt, vn_s);
        size_t vlen = vv ? ox_string_length(ctxt, vv_s) : 0;
        char buf[nlen + vlen + 2];

        snprintf(buf, sizeof(buf), "%s=%s",
                vn,
                vv ? vv : "'");

        r = putenv(buf);
        if (r == OX_ERR)
            std_system_error(ctxt, "putenv");
    }
#else /* !defined ARCH_WIN */
    if (vv) {
        r = setenv(vn, vv, OX_TRUE);
        if (r == OX_ERR)
            std_system_error(ctxt, "setenv");
    } else {
        r = unsetenv(vn);
        if (r == OX_ERR)
            std_system_error(ctxt, "unsetenv");
    }
#endif /*ARCH_WIN*/
end:
    OX_VS_POP(ctxt, vn_s)
    return r;
}

/*msleep.*/
static OX_Result
msleep_func (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_Number n;
    OX_Result r;

    if ((r = ox_to_number(ctxt, arg, &n)) == OX_ERR)
        return r;

    if (n > 0) {
        ox_unlock(ctxt);
        usleep(n * 1000);
        ox_lock(ctxt);
    }

    return OX_OK;
}

/*getcwd.*/
static OX_Result
getcwd_func (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    char buf[PATH_MAX];
    char *s;

    if (!(s = getcwd(buf, sizeof(buf)))) {
        return std_system_error(ctxt, "getcwd");
    }

    return ox_path_to_str(ctxt, s, rv);
}

/*chdir.*/
static OX_Result
chdir_func (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, s)
    const char *cstr;
    OX_Result r;

    if ((r = ox_to_string(ctxt, arg, s)) == OX_ERR)
        goto end;

    cstr = ox_string_get_char_star(ctxt, s);

    if ((r = chdir(cstr)) == -1) {
        r = std_system_error(ctxt, "chdir");
        goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*gettid.*/
static OX_Result
gettid_func (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    int id = ox_thread_id();

    ox_value_set_number(ctxt, rv, id);

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
 *? @lib System related functions.
 *?
 *? @func system Execute a shell command.
 *? Create a new process and run the shell command.
 *? And returns after the command has been completed.
 *? @param cmd {String} The command line string.
 *? @return {Number} The return value of the shell.
 *? @throw {SystemError} Run the shell failed.
 *?
 *? @func getenv Get the value of an environment variable.
 *? @param name {String} The name of the environment variable.
 *? @return {?String} The value of the environment variable.
 *? If the variable is not defined, return null.
 *?
 *? @func setenv Set the value of an environment variable.
 *? If the variable is not defined, add a new variable with the value.
 *? @param name {String} The name of the environment variable.
 *? @param val {String} The new value of the environment variable.
 *? @throw {SystemError} Set the environment variable failed.
 *?
 *? @func msleep Suspend current thread's execution for millisecond intervals.
 *? @param ms {Number} Milliseconds.
 *?
 *? @func chdir Change current working directory.
 *? @param dir {String} The new working directory's name.
 *? @throw {SystemError} Change working directory failed.
 *?
 *? @func gettid Get the current thread's ID.
 *? @return {Number} The current thread's ID.
 *?
 */

/*Execute.*/
OX_Result
ox_exec (OX_Context *ctxt, OX_Value *f, OX_Value *s, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, v)

    /*system.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, system_func, NULL, "system"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_system, v));

    /*getenv.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, getenv_func, NULL, "getenv"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_getenv, v));

    /*setenv.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, setenv_func, NULL, "setenv"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_setenv, v));

    /*mdelay.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, msleep_func, NULL, "msleep"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_msleep, v));

    /*getcwd.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, getcwd_func, NULL, "getcwd"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_getcwd, v));

    /*chdir.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, chdir_func, NULL, "chdir"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_chdir, v));

    /*gettid.*/
    ox_not_error(ox_named_native_func_new_s(ctxt, v, gettid_func, NULL, "gettid"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_gettid, v));

    OX_VS_POP(ctxt, v)
    return OX_OK;
}
