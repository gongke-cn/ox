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

#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include "std.h"

/*Declaration index.*/
enum {
    ID_unlink,
    ID_rmdir,
    ID_rename,
    ID_mkdir,
    ID_mkdir_p,
    ID_chmod,
    ID_link,
    ID_symlink,
    ID_readlink,
    ID_MAX
};

/*Public table.*/
static const char*
pub_tab[] = {
    "unlink",
    "rmdir",
    "rename",
    "mkdir",
    "mkdir_p",
    "chmod",
    "link",
    "symlink",
    "readlink",
    NULL
};

/*Script descrition.*/
static const OX_ScriptDesc
script_desc = {
    NULL,
    pub_tab,
    ID_MAX
};

/*unlink*/
static OX_Result
FS_unlink (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *s_arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, s)
    const char *pn;
    OX_Result r;

    if ((r = ox_to_string(ctxt, s_arg, s)) == OX_ERR)
        goto end;

    pn = ox_string_get_char_star(ctxt, s);

    OX_SCHED(ctxt, r = unlink(pn));

    if (r == -1) {
        r = std_system_error_v(ctxt, "unlink", errno, "\"%s\"", pn);
        goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*rmdir*/
static OX_Result
FS_rmdir (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *s_arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, s)
    const char *pn;
    OX_Result r;

    if ((r = ox_to_string(ctxt, s_arg, s)) == OX_ERR)
        goto end;

    pn = ox_string_get_char_star(ctxt, s);

    OX_SCHED(ctxt, r = rmdir(pn));

    if (r == -1) {
        r = std_system_error_v(ctxt, "rmdir", errno, "\"%s\"", pn);
        goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*rename*/
static OX_Result
FS_rename (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *old_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *new_arg = ox_argument(ctxt, args, argc, 1);
    OX_VS_PUSH_2(ctxt, old_s, new_s)
    const char *old_pn, *new_pn;
    OX_Result r;

    if ((r = ox_to_string(ctxt, old_arg, old_s)) == OX_ERR)
        goto end;

    if ((r = ox_to_string(ctxt, new_arg, new_s)) == OX_ERR)
        goto end;

    old_pn = ox_string_get_char_star(ctxt, old_s);
    new_pn = ox_string_get_char_star(ctxt, new_s);

    OX_SCHED(ctxt, r = rename(old_pn, new_pn));

    if (r == -1) {
        r = std_system_error_v(ctxt, "rename", errno, "\"%s\", \"%s\"", old_pn, new_pn);
        goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, old_s)
    return r;
}

/*mkdir*/
static OX_Result
FS_mkdir (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *s_arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, s)
    const char *pn;
#ifndef ARCH_WIN
    OX_Value *mode_arg = ox_argument(ctxt, args, argc, 1);
    int32_t mode;
#endif /*ARCH_WIN*/
    OX_Result r;

    if ((r = ox_to_string(ctxt, s_arg, s)) == OX_ERR)
        goto end;

    pn = ox_string_get_char_star(ctxt, s);

#ifndef ARCH_WIN
    if (!ox_value_is_null(ctxt, mode_arg)) {
        if ((r = ox_to_int32(ctxt, mode_arg, &mode)) == OX_ERR)
            return r;
    } else {
        mode = 0777;
    }
#endif /*ARCH_WIN*/

    OX_SCHED(ctxt, r = mkdir(pn
#ifndef ARCH_WIN
            , mode
#endif /*ARCH_WIN*/
            ));

    if (r == -1) {
        r = std_system_error_v(ctxt, "mkdir", errno, "\"%s\"", pn);
        goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, s);
    return r;
}

static int
mkdir_p (const char *path)
{
    struct stat st;
    int r;

    r = stat(path, &st);
    if (r == -1) {
        if (errno == ENOENT) {
            char buf[PATH_MAX];
            char *dpath;

            snprintf(buf, sizeof(buf), "%s", path);
            dpath = dirname(buf);

            if (dpath && strcmp(dpath, "/") && strcmp(dpath, path)) {
                if ((r = mkdir_p(dpath)) == -1)
                    return r;
            }

#ifndef ARCH_WIN
            r = mkdir(path, 0777);
#else
            r = mkdir(path);
#endif
        }
    }

    return r;
}

/*mkdir_p*/
static OX_Result
FS_mkdir_p (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *s_arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, s)
    const char *pn;
    OX_Result r;

    if ((r = ox_to_string(ctxt, s_arg, s)) == OX_ERR)
        goto end;

    pn = ox_string_get_char_star(ctxt, s);

    OX_SCHED(ctxt, r = mkdir_p(pn));

    if (r == -1) {
        r = std_system_error_v(ctxt, "mkdir", errno, "\"%s\"", pn);
        goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*chmod*/
static OX_Result
FS_chmod (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *s_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *mode_arg = ox_argument(ctxt, args, argc, 1);
    OX_VS_PUSH(ctxt, s)
    int32_t mode;
    const char *pn;
    OX_Result r;

    if ((r = ox_to_string(ctxt, s_arg, s)) == OX_ERR)
        goto end;

    pn = ox_string_get_char_star(ctxt, s);

    if (!ox_value_is_null(ctxt, mode_arg)) {
        if ((r = ox_to_int32(ctxt, mode_arg, &mode)) == OX_ERR)
            return r;
    } else {
        mode = 0777;
    }

    OX_SCHED(ctxt, r = chmod(pn, mode));

    if (r == -1) {
        r = std_system_error_v(ctxt, "chmod", errno, "\"%s\", %d", pn, mode);
        goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*link*/
static OX_Result
FS_link (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
#ifdef ARCH_WIN
    return OX_OK;
#else /* !defined ARCH_WIN */
    OX_Value *old_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *new_arg = ox_argument(ctxt, args, argc, 1);
    OX_VS_PUSH_2(ctxt, old_s, new_s)
    const char *old_pn, *new_pn;
    OX_Result r;

    if ((r = ox_to_string(ctxt, old_arg, old_s)) == OX_ERR)
        goto end;

    if ((r = ox_to_string(ctxt, new_arg, new_s)) == OX_ERR)
        goto end;

    old_pn = ox_string_get_char_star(ctxt, old_s);
    new_pn = ox_string_get_char_star(ctxt, new_s);

    OX_SCHED(ctxt, r = link(old_pn, new_pn));

    if (r == -1) {
        r = std_system_error_v(ctxt, "link", errno, "\"%s\", \"%s\"", old_pn, new_pn);
        goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, old_s)
    return r;
#endif /*ARCH_WIN*/
}

/*symlink*/
static OX_Result
FS_symlink (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
#ifdef ARCH_WIN
    return OX_OK;
#else /* !defined ARCH_WIN */
    OX_Value *old_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *new_arg = ox_argument(ctxt, args, argc, 1);
    OX_VS_PUSH_2(ctxt, old_s, new_s)
    const char *old_pn, *new_pn;
    OX_Result r;

    if ((r = ox_to_string(ctxt, old_arg, old_s)) == OX_ERR)
        goto end;

    if ((r = ox_to_string(ctxt, new_arg, new_s)) == OX_ERR)
        goto end;

    old_pn = ox_string_get_char_star(ctxt, old_s);
    new_pn = ox_string_get_char_star(ctxt, new_s);

    OX_SCHED(ctxt, r = symlink(old_pn, new_pn));

    if (r == -1) {
        r = std_system_error_v(ctxt, "symlink", errno, "\"%s\", \"%s\"", old_pn, new_pn);
        goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, old_s)
    return r;
#endif /*ARCH_WIN*/
}

/*readlink*/
static OX_Result
FS_readlink (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
#ifdef ARCH_WIN
    return OX_OK;
#else /* !defined ARCH_WIN */
    OX_Value *s_arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, s)
    const char *pn;
    char buf[PATH_MAX];
    OX_Result r;
    ssize_t rr;

    if ((r = ox_to_string(ctxt, s_arg, s)) == OX_ERR)
        goto end;

    pn = ox_string_get_char_star(ctxt, s);

    OX_SCHED(ctxt, rr = readlink(pn, buf, sizeof(buf)));

    if (rr == -1) {
        r = std_system_error_v(ctxt, "readlink", errno, "\"%s\"", pn);
        goto end;
    }

    r = ox_string_from_chars(ctxt, rv, buf, rr);
end:
    OX_VS_POP(ctxt, s)
    return r;
#endif /*ARCH_WIN*/
}

/*Load this module.*/
OX_Result
ox_load (OX_Context *ctxt, OX_Value *s)
{
    ox_not_error(ox_script_set_desc(ctxt, s, &script_desc));
    return OX_OK;
}

/*?
 *? @lib File system operation function.
 *?
 *? @func unlink Remove a file.
 *? @param filename {String} The file's name which to be removed
 *? @throw {SystemError} Cannot remove the file.
 *?
 *? @func rmdir Remove a directory.
 *? Before removing he directory, all the files in it should be removed.
 *? @param dir {String} The directory's name which to be removed
 *? @throw {SystemError} Cannot remove the directory.
 *?
 *? @func rename Rename a file.
 *? @param old {String} The old filename.
 *? @param new {String} The new filename.
 *? @throw {SystemError} Rename failed.
 *?
 *? @func mkdir Create a directory.
 *? @param dir {String} The new dirsctory's name.
 *? @throw {SystemError} Cannot create the directory.
 *?
 *? @func mkdir_p Create a directory and its parents if it is not exist.
 *? @param dir {String} The new dirsctory's name.
 *? @throw {SystemError} Cannot create the directory.
 *?
 *? @func chmod Change the file's mode bits.
 *? @param filename {String} The filename.
 *? @param mode {Number} The file's new mode bits.
 *? @throw {SystemError} Change mode failed.
 *?
 *? @func link Make a link to the file.
 *? @param old {String} The old filename.
 *? @param new {String} The new filename.
 *? @throw {SystemError} Cannot link the file.
 *?
 *? @func symlink Make a symbol link to the file.
 *? @param old {String} The old filename.
 *? @param new {String} The new filename.
 *? @throw {SystemError} Cannot make the symbol link.
 *?
 *? @func readlink Read the symbol link.
 *? @param sym {String} The symbol link's filename.
 *? @return {String} The origin filename.
 *? @throw {SystemError} Read symbol link failed.
 */

/*Execute.*/
OX_Result
ox_exec (OX_Context *ctxt, OX_Value *nf, OX_Value *s, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, f)

    ox_not_error(ox_named_native_func_new_s(ctxt, f, FS_unlink, NULL, "unlink"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_unlink, f));
    ox_not_error(ox_named_native_func_new_s(ctxt, f, FS_rmdir, NULL, "rmdir"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_rmdir, f));
    ox_not_error(ox_named_native_func_new_s(ctxt, f, FS_rename, NULL, "rename"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_rename, f));
    ox_not_error(ox_named_native_func_new_s(ctxt, f, FS_mkdir, NULL, "mkdir"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_mkdir, f));
    ox_not_error(ox_named_native_func_new_s(ctxt, f, FS_mkdir_p, NULL, "mkdir_p"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_mkdir_p, f));
    ox_not_error(ox_named_native_func_new_s(ctxt, f, FS_chmod, NULL, "chmod"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_chmod, f));
    ox_not_error(ox_named_native_func_new_s(ctxt, f, FS_link, NULL, "link"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_link, f));
    ox_not_error(ox_named_native_func_new_s(ctxt, f, FS_symlink, NULL, "symlink"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_symlink, f));
    ox_not_error(ox_named_native_func_new_s(ctxt, f, FS_readlink, NULL, "readlink"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_readlink, f));

    OX_VS_POP(ctxt, f)
    return OX_OK;
}
