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
 * Directory.
 */

#define OX_LOG_TAG "dir"

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include "std.h"

/*Declaration index.*/
enum {
    ID_Dir,
    ID_MAX
};

/*Public table.*/
static const char*
pub_tab[] = {
    "Dir",
    NULL
};

/*Script description.*/
static const OX_ScriptDesc
script_desc = {
    NULL,
    pub_tab,
    ID_MAX
};

/** Directory data.*/
typedef struct {
    DIR           *dir; /**< Directory data.*/
    struct dirent *ent; /**< The current entry.*/
} OX_Dir;

/*Scan referenced objects in the directory data.*/
static void
dir_scan (OX_Context *ctxt, void *p)
{

}

/*Free the directory data.*/
static void
dir_free (OX_Context *ctxt, void *p)
{
    OX_Dir *dir = p;

    if (dir->dir)
        closedir(dir->dir);

    OX_DEL(ctxt, dir);
}

/*Operation functions of the directory data.*/
static const OX_PrivateOps
dir_ops = {
    dir_scan,
    dir_free
};

/*Get the directory data from the value.*/
static OX_Dir*
dir_data_get (OX_Context *ctxt, OX_Value *v)
{
    OX_Dir *dir = ox_object_get_priv(ctxt, v, &dir_ops);

    if (!dir)
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a directory"));

    return dir;
}

/*Get the directory data from the value and check if it is closed.*/
static OX_Dir*
dir_data_get_nc (OX_Context *ctxt, OX_Value *v)
{
    OX_Dir *dir = ox_object_get_priv(ctxt, v, &dir_ops);

    if (dir) {
        if (!dir->dir) {
            ox_throw_reference_error(ctxt, OX_TEXT("directory is already closed"));
            dir = NULL;
        }
    }

    return dir;
}

/*Dir.$inf.$init*/
static OX_Result
Dir_inf_init (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, s)
    const char *path;
    OX_Dir *dir;
    DIR *d;
    OX_Result r;

    if ((r = ox_to_string(ctxt, arg, s)) == OX_ERR)
        goto end;

    if (!OX_NEW(ctxt, dir)) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    path = ox_string_get_char_star(ctxt, s);

    OX_SCHED(ctxt, d = opendir(path));

    dir->dir = d;

    if (!dir->dir) {
        dir_free(ctxt, dir);
        r = std_system_error_v(ctxt, "opendir", errno, "\"%s\"", path);
        goto end;
    }

    dir->ent = readdir(dir->dir);

    if ((r = ox_object_set_priv(ctxt, thiz, &dir_ops, dir)) == OX_ERR) {
        dir_free(ctxt, dir);
        goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*Dir.$inf.$close*/
static OX_Result
Dir_inf_close (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Dir *dir;

    if (!(dir = dir_data_get(ctxt, thiz)))
        return OX_ERR;

    if (dir->dir) {
        closedir(dir->dir);
        dir->dir = NULL;
    }

    return OX_OK;
}

/*Dir.$inf.end get*/
static OX_Result
Dir_inf_end_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Dir *dir;
    OX_Bool b;

    if (!(dir = dir_data_get_nc(ctxt, thiz)))
        return OX_ERR;

    b = !dir->ent;
    ox_value_set_bool(ctxt, rv, b);
    return OX_OK;
}

/*Dir.$inf.value get*/
static OX_Result
Dir_inf_value_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Dir *dir;
    OX_Result r;

    if (!(dir = dir_data_get_nc(ctxt, thiz)))
        return OX_ERR;

    if (dir->ent) {
        if ((r = ox_string_from_char_star(ctxt, rv, dir->ent->d_name)) == OX_ERR)
            return r;
    } else {
        ox_value_set_null(ctxt, rv);
    }

    return OX_OK;
}

/*Dir.$inf.next*/
static OX_Result
Dir_inf_next (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Dir *dir;

    if (!(dir = dir_data_get_nc(ctxt, thiz)))
        return OX_ERR;

    if (dir->ent) {
        struct dirent *ent;

        OX_SCHED(ctxt, ent = readdir(dir->dir));

        dir->ent = ent;
    }

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
 *? @lib Directory.
 *? Directory object is used to traverse the files in the directory.
 *?
 *? @class{ Dir Directory.
 *? Directory can be used to traverse the files in it.
 *? @inherit {Iterator}
 *?
 *? @func $init Initialize the directory object.
 *? @param dir {String} The directory's name.
 *? @throw {SystemError} Cannot open the directory.
 *?
 *? @func $close Close the directory.
 *?
 *? @roacc end {Bool} If the end of directory is reached.
 *?
 *? @roacc value {String} The current traversed filename.
 *?
 *? @func next Move to the next file in the directory.
 *?
 *? @class}
 */

/*Execute.*/
OX_Result
ox_exec (OX_Context *ctxt, OX_Value *f, OX_Value *s, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH_2(ctxt, c, inf)

    /*Dir.*/
    ox_not_error(ox_named_class_new_s(ctxt, c, inf, NULL, "Dir"));
    ox_not_error(ox_class_inherit(ctxt, c, OX_OBJECT(ctxt, Iterator)));
    ox_not_error(ox_script_set_value(ctxt, s, ID_Dir, c));

    /*Dir.$init*/
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$init", Dir_inf_init));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$close", Dir_inf_close));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "end", Dir_inf_end_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "value", Dir_inf_value_get, NULL));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "next", Dir_inf_next));

    OX_VS_POP(ctxt, c)
    return OX_OK;
}
