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
 * Path.
 */

#define OX_LOG_TAG "io"

#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include "std.h"

#ifndef S_IFLNK
#define S_IFLNK 0
#endif

/*Declaration index.*/
enum {
    ID_Path,
    ID_dirname,
    ID_basename,
    ID_fullpath,
    ID_normpath,
    ID_MAX
};

/*Public table.*/
static const char*
pub_tab[] = {
    "Path",
    "dirname",
    "basename",
    "fullpath",
    "normpath",
    NULL
};

/*Script description.*/
static const OX_ScriptDesc
script_desc = {
    NULL,
    pub_tab,
    ID_MAX
};

/** Phase of the path data.*/
typedef enum {
    OX_PATH_NEED_CHECK,
    OX_PATH_EXIST,
    OX_PATH_NOT_EXIST
} OX_PathPhase;

/** Path data.*/
typedef struct {
    OX_Value     path;  /**< Path string.*/
    OX_PathPhase phase; /**< Phase of the path.*/
    int          en;    /**< Error number.*/
    struct stat  st;    /**< State of the file.*/
#ifndef ARCH_WIN
    OX_Bool      is_lnk;/**< Is a symbol link or not.*/
#endif /*ARCH_WIN*/
} OX_Path;

/*Scan referenced objects in the path data.*/
static void
path_scan (OX_Context *ctxt, void *p)
{
    OX_Path *path = p;

    ox_gc_scan_value(ctxt, &path->path);
}

/*Free the path data.*/
static void
path_free (OX_Context *ctxt, void *p)
{
    OX_Path *path = p;

    OX_DEL(ctxt, path);
}

/*Operation functions of the path.*/
static const OX_PrivateOps
path_ops = {
    path_scan,
    path_free
};

/*Get the path data from the value.*/
static OX_Path*
path_data_get (OX_Context *ctxt, OX_Value *v)
{
    OX_Path *path = ox_object_get_priv(ctxt, v, &path_ops);

    if (!path)
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a path"));

    return path;
}

/*Get the path file's state.*/
static struct stat*
path_get_stat (OX_Context *ctxt, OX_Path *path)
{
    int r;

    if (path->phase == OX_PATH_NEED_CHECK) {
        const char *cstr;

        cstr = ox_string_get_char_star(ctxt, &path->path);

#ifndef ARCH_WIN
        if (path->is_lnk)
            OX_SCHED(ctxt, r = lstat(cstr, &path->st));
        else
#endif /*ARCH_WIN*/
            OX_SCHED(ctxt, r = stat(cstr, &path->st));
            
        if (r == -1) {
            path->en = errno;
            path->phase = OX_PATH_NOT_EXIST;
        } else {
            path->phase = OX_PATH_EXIST;
        }
    }

    if (path->phase == OX_PATH_EXIST)
        return &path->st;

    return NULL;
}

/*Throw error of the path.*/
static OX_Result
path_error (OX_Context *ctxt, OX_Path *path)
{
    if (path->phase == OX_PATH_NOT_EXIST)
        return std_system_error_v(ctxt, "stat", path->en, "\"%s\"", ox_string_get_char_star(ctxt, &path->path));

    return OX_ERR;
}

/*Get the path's state and check if it exist.*/
static struct stat*
path_get_stat_check (OX_Context *ctxt, OX_Path *path)
{
    struct stat *st = path_get_stat(ctxt, path);

    if (!st)
        path_error(ctxt, path);

    return st;
}

/*Path.$inf.$init*/
static OX_Result
Path_inf_init (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
#ifndef ARCH_WIN
    OX_Value *is_lnk = ox_argument(ctxt, args, argc, 1);
#endif /*ARCH_WIN*/
    OX_VS_PUSH(ctxt, s)
    OX_Path *path;
    OX_Result r;

    if ((r = ox_to_string(ctxt, arg, s)) == OX_ERR)
        goto end;

    if (!OX_NEW(ctxt, path)) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    ox_value_copy(ctxt, &path->path, s);
    path->phase = OX_PATH_NEED_CHECK;
    
#ifndef ARCH_WIN
    path->is_lnk = ox_to_bool(ctxt, is_lnk);
#endif /*ARCH_WIN*/

    if ((r = ox_object_set_priv(ctxt, thiz, &path_ops, path)) == OX_ERR) {
        path_free(ctxt, path);
        goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*Path.$inf.$to_str*/
static OX_Result
Path_inf_to_str (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Path *path;

    if (!(path = path_data_get(ctxt, thiz)))
        return OX_ERR;

    ox_value_copy(ctxt, rv, &path->path);
    return OX_OK;
}

/*Path.$inf.exist get*/
static OX_Result
Path_inf_exist_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Path *path;

    if (!(path = path_data_get(ctxt, thiz)))
        return OX_ERR;

    path_get_stat(ctxt, path);

    if ((path->phase == OX_PATH_NOT_EXIST) && (path->en != ENOENT))
        return std_system_error_v(ctxt, "stat", path->en, "\"%s\"", ox_string_get_char_star(ctxt, &path->path));

    ox_value_set_bool(ctxt, rv, (path->phase == OX_PATH_EXIST));
    return OX_OK;
}

/*Path.$inf.size get*/
static OX_Result
Path_inf_size_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Path *path;
    struct stat *st;

    if (!(path = path_data_get(ctxt, thiz)))
        return OX_ERR;

    if (!(st = path_get_stat_check(ctxt, path)))
        return OX_ERR;

    ox_value_set_number(ctxt, rv, st->st_size);
    return OX_OK;
}

/*Path.$inf.atime get*/
static OX_Result
Path_inf_atime_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Path *path;
    struct stat *st;

    if (!(path = path_data_get(ctxt, thiz)))
        return OX_ERR;

    if (!(st = path_get_stat_check(ctxt, path)))
        return OX_ERR;

    ox_value_set_number(ctxt, rv, st->st_atime * 1000ll);
    return OX_OK;
}

/*Path.$inf.ctime get*/
static OX_Result
Path_inf_ctime_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Path *path;
    struct stat *st;

    if (!(path = path_data_get(ctxt, thiz)))
        return OX_ERR;

    if (!(st = path_get_stat_check(ctxt, path)))
        return OX_ERR;

    ox_value_set_number(ctxt, rv, st->st_ctime * 1000ll);
    return OX_OK;
}

/*Path.$inf.mtime get*/
static OX_Result
Path_inf_mtime_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Path *path;
    struct stat *st;

    if (!(path = path_data_get(ctxt, thiz)))
        return OX_ERR;

    if (!(st = path_get_stat_check(ctxt, path)))
        return OX_ERR;

    ox_value_set_number(ctxt, rv, st->st_mtime * 1000ll);
    return OX_OK;
}

/*Path.$inf.format get*/
static OX_Result
Path_inf_format_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Path *path;
    struct stat *st;
    int fmt;

    if (!(path = path_data_get(ctxt, thiz)))
        return OX_ERR;

    if (!(st = path_get_stat_check(ctxt, path)))
        return OX_ERR;

    fmt = st->st_mode & S_IFMT;

    ox_value_set_number(ctxt, rv, fmt);
    return OX_OK;
}

/*Path.$inf.mode get*/
static OX_Result
Path_inf_mode_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Path *path;
    struct stat *st;
    int mode;

    if (!(path = path_data_get(ctxt, thiz)))
        return OX_ERR;

    if (!(st = path_get_stat_check(ctxt, path)))
        return OX_ERR;

    mode = st->st_mode & (S_IRWXU|S_IRWXG|S_IRWXO);

    ox_value_set_number(ctxt, rv, mode);
    return OX_OK;
}

/*dirname*/
static OX_Result
Path_dirname (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *s_arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, s)
    const char *pn;
    char buf[PATH_MAX];
    char *dn;
    OX_Result r;

    if ((r = ox_to_string(ctxt, s_arg, s)) == OX_ERR)
        goto end;

    pn = ox_string_get_char_star(ctxt, s);

    snprintf(buf, sizeof(buf), "%s", pn);

    dn = dirname(buf);

    r = ox_path_to_str(ctxt, dn, rv);
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*basename*/
static OX_Result
Path_basename (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *s_arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, s)
    const char *pn;
    char buf[PATH_MAX];
    char *bn;
    OX_Result r;

    if ((r = ox_to_string(ctxt, s_arg, s)) == OX_ERR)
        goto end;

    pn = ox_string_get_char_star(ctxt, s);

    snprintf(buf, sizeof(buf), "%s", pn);

    bn = basename(buf);

    r = ox_string_from_char_star(ctxt, rv, bn);
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*fullpath*/
static OX_Result
Path_fullpath (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *s_arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, s)
    const char *pn;
    char buf[PATH_MAX];
    char *fp;
    OX_Result r;

    if ((r = ox_to_string(ctxt, s_arg, s)) == OX_ERR)
        goto end;

    pn = ox_string_get_char_star(ctxt, s);

    OX_SCHED(ctxt, fp = realpath(pn, buf));
    if (!fp) {
        r = std_system_error_v(ctxt, "realpath", errno, "\"%s\"", pn);
        goto end;
    }

    r = ox_path_to_str(ctxt, fp, rv);
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*normpath*/
static OX_Result
Path_normpath (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *s_arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, s)
    const char *c, *b;
    OX_CharBuffer cb;
    OX_VECTOR_TYPE_DECL(size_t) ps;
    OX_Bool has_root = OX_FALSE;
    size_t path_begin = 0;
    OX_Result r;

    ox_char_buffer_init(&cb);
    ox_vector_init(&ps);

    if ((r = ox_to_string(ctxt, s_arg, s)) == OX_ERR)
        goto end;

    c = ox_string_get_char_star(ctxt, s);

#ifdef ARCH_WIN
    if (ox_char_is_alpha(c[0]) && (c[1] == ':')) {
        char disk = *c;

        c += 2;

        if ((r = ox_char_buffer_append_char(ctxt, &cb, disk)) == OX_ERR)
            goto end;

        if ((r = ox_char_buffer_append_char(ctxt, &cb, ':')) == OX_ERR)
            goto end;

        path_begin = cb.len;
    }

    if ((*c == '/') || (*c == '\\'))
#else
    if (*c == '/')
#endif /*ARCH_WIN*/
    {
        if ((r = ox_char_buffer_append_char(ctxt, &cb, '/')) == OX_ERR)
            goto end;

        has_root = OX_TRUE;
        path_begin = cb.len;
    }

    b = c;
    while (1) {
        OX_Bool sep = OX_FALSE;

        if (*c == 0) {
            sep = OX_TRUE;
        } else

#ifdef ARCH_WIN
        if ((*c == '/') || (*c == '\\'))
#else
        if (*c == '/')
#endif
            sep = OX_TRUE;

        if (sep) {
            if (c > b) {
                if ((c == b + 1) && (*b == '.')) {
                } else if ((c == b + 2) && (b[0] == '.') && (b[1] == '.')) {
                    if (ps.len) {
                        size_t pos;

                        ps.len --;
                        pos = ox_vector_item(&ps, ps.len);
                        cb.len = pos;
                    } else if (has_root) {
                    } else {
                        if (cb.len > path_begin) {
                            if ((r = ox_char_buffer_append_char(ctxt, &cb, '/')) == OX_ERR)
                                goto end;
                        }

                        if ((r = ox_char_buffer_append_chars(ctxt, &cb, "..", 2)) == OX_ERR)
                            goto end;
                    }
                } else {
                    if ((r = ox_vector_append(ctxt, &ps, cb.len)) == OX_ERR)
                        goto end;

                    if (cb.len > path_begin) {
                        if ((r = ox_char_buffer_append_char(ctxt, &cb, '/')) == OX_ERR)
                            goto end;
                    }

                    if ((r = ox_char_buffer_append_chars(ctxt, &cb, b, c - b)) == OX_ERR)
                        goto end;
                }
            }

            if (*c == 0)
                break;

            c ++;
            b = c;
        } else {
            c ++;
        }
    }

    if (cb.len == 0) {
        if ((r = ox_char_buffer_append_char(ctxt, &cb, '.')) == OX_ERR)
            goto end;
    }

    r = ox_char_buffer_get_string(ctxt, &cb, rv);
end:
    ox_char_buffer_deinit(ctxt, &cb);
    ox_vector_deinit(ctxt, &ps);
    OX_VS_POP(ctxt, s)
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
 *? @lib Path functions.
 *?
 *? @class{ Path Path.
 *?
 *? @enum{ format File format.
 *? @item FMT_REG Regular file.
 *? @item FMT_DIR Directory.
 *? @item FMT_FIFO Named fifo.
 *? @item FMT_CHR Character device.
 *? @item FMT_BLK Block device.
 *? @item FMT_LNK Symbolic link file.
 *? @enum}
 *?
 *? @enum{ mode The file's access mode.
 *? @item MODE_RUSR Owner has read permission.
 *? @item MODE_WUSR Owner has write permission.
 *? @item MODE_XUSR Owner has execute permission.
 *? @item MODE_RGRP Group has read permission.
 *? @item MODE_WGRP Group has write permission.
 *? @item MODE_XGRP Group has execute permission.
 *? @item MODE_ROTH Others have read permission.
 *? @item MODE_WOTH Others have write permission.
 *? @item MODE_XOTH Others have execute permission.
 *? @enum}
 *?
 *? @func $init Initialize the path object.
 *? @param pathname {String} The pathname.
 *? @param is_lnk {Bool} The pathname is a symbol link.
 *? If is_lnk is true, used "lstat" to get the file's state instead of "stat". 
 *?
 *? @func $to_str Convert the path object to string.
 *? @return {String} The pathname string.
 *?
 *? @roacc exist {Bool} If the file exist.
 *? @roacc atime {Number} Last file access time in milliseconds from January 1, 1900.
 *? @roacc mtime {Number} Last file modify time in milliseconds from January 1, 1900.
 *? @roacc ctime {Number} Last status change time in milliseconds from January 1, 1900.
 *? @roacc format {Path.format} The format of the file.
 *? @roacc mode {Path.mode} The file's access mode.
 *? @class}
 *?
 *? @func dirname Get the directory part of the file's pathname.
 *? @param path {String} The file's full pathname.
 *? @return {String} The directory part.
 *?
 *? @func basename Remove the directory part and get the file part of the file's pathname.
 *? @param path {String} The file's full pathname.
 *? @return {String} The file part.
 *?
 *? @func fullpath Get the full pathname of the file.
 *? @param path {String} The pathname.
 *? @return {String} The file's full pathname.
 *? @throw {SystemError} The file does not exist.
 *?
 *? @func normpath Normalize the pathname.
 *? @param path {String} The pathname.
 *? @return {String} The normalized pathname.
 */

/*Execute.*/
OX_Result
ox_exec (OX_Context *ctxt, OX_Value *nf, OX_Value *s, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH_5(ctxt, c, inf, fmts, modes, f)

    /*Path.*/
    ox_not_error(ox_named_class_new_s(ctxt, c, inf, NULL, "Path"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_Path, c));

    ox_not_error(ox_named_native_func_new_s(ctxt, f, Path_dirname, NULL, "dirname"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_dirname, f));
    ox_not_error(ox_named_native_func_new_s(ctxt, f, Path_basename, NULL, "basename"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_basename, f));
    ox_not_error(ox_named_native_func_new_s(ctxt, f, Path_fullpath, NULL, "fullpath"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_fullpath, f));
    ox_not_error(ox_named_native_func_new_s(ctxt, f, Path_normpath, NULL, "normpath"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_normpath, f));

    /*Path.format.*/
    ox_not_error(ox_enum_new(ctxt, fmts, OX_ENUM_ENUM));
    ox_not_error(ox_object_add_const_s(ctxt, c, "format", fmts));
    ox_not_error(ox_enum_add_item_s(ctxt, fmts, c, "FMT_REG", S_IFREG));
    ox_not_error(ox_enum_add_item_s(ctxt, fmts, c, "FMT_DIR", S_IFDIR));
    ox_not_error(ox_enum_add_item_s(ctxt, fmts, c, "FMT_FIFO", S_IFIFO));
    ox_not_error(ox_enum_add_item_s(ctxt, fmts, c, "FMT_CHR", S_IFCHR));
    ox_not_error(ox_enum_add_item_s(ctxt, fmts, c, "FMT_BLK", S_IFBLK));
    ox_not_error(ox_enum_add_item_s(ctxt, fmts, c, "FMT_LNK", S_IFLNK));

    /*Path.mode.*/
    ox_not_error(ox_enum_new(ctxt, modes, OX_ENUM_BITFIELD));
    ox_not_error(ox_object_add_const_s(ctxt, c, "mode", modes));
    ox_not_error(ox_enum_add_item_s(ctxt, modes, c, "MODE_RUSR", S_IRUSR));
    ox_not_error(ox_enum_add_item_s(ctxt, modes, c, "MODE_WUSR", S_IWUSR));
    ox_not_error(ox_enum_add_item_s(ctxt, modes, c, "MODE_XUSR", S_IXUSR));
    ox_not_error(ox_enum_add_item_s(ctxt, modes, c, "MODE_RGRP", S_IRGRP));
    ox_not_error(ox_enum_add_item_s(ctxt, modes, c, "MODE_WGRP", S_IWGRP));
    ox_not_error(ox_enum_add_item_s(ctxt, modes, c, "MODE_XGRP", S_IXGRP));
    ox_not_error(ox_enum_add_item_s(ctxt, modes, c, "MODE_ROTH", S_IROTH));
    ox_not_error(ox_enum_add_item_s(ctxt, modes, c, "MODE_WOTH", S_IWOTH));
    ox_not_error(ox_enum_add_item_s(ctxt, modes, c, "MODE_XOTH", S_IXOTH));

    /*Path.$inf*/
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$init", Path_inf_init));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$to_str", Path_inf_to_str));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "exist", Path_inf_exist_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "size", Path_inf_size_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "atime", Path_inf_atime_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "mtime", Path_inf_mtime_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "ctime", Path_inf_ctime_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "format", Path_inf_format_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "mode", Path_inf_mode_get, NULL));

    OX_VS_POP(ctxt, c)
    return OX_OK;
}
