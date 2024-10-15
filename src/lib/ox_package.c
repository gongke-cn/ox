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
 * Package manager.
 */

#define OX_LOG_TAG "ox_package"

#include "ox_internal.h"

/*Lookup the package.*/
static OX_Result
package_lookup (OX_Context *ctxt, const char *dir, OX_Value *name, OX_Value *pkg)
{
    struct stat sb;
    OX_VM *vm = ox_vm_get(ctxt);
    OX_VS_PUSH_2(ctxt, input, pathv)
    const char *ncstr;
    OX_Bool openned = OX_FALSE;
    char *pn;
    char path[PATH_MAX];
    OX_Result r;

    pn = (char*)ox_string_get_char_star(ctxt, name);
    snprintf(path, sizeof(path), "%s/%s", dir, pn);

    if ((stat(path, &sb) == -1) || !S_ISDIR(sb.st_mode)) {
        r = OX_FALSE;
        goto end;
    }

    /*Create package path string.*/
    if ((r = ox_string_from_char_star(ctxt, pathv, path)) == OX_ERR)
        goto end;

    /*Load the "package.ox" file.*/
    snprintf(path, sizeof(path), "%s/%s/package.ox", dir, pn);
    if ((r = ox_file_input_new(ctxt, input, path)) == OX_ERR)
        goto end;
    openned = OX_TRUE;

    if ((r = ox_json_parse(ctxt, input, OX_FALSE, pkg)) == OX_ERR)
        goto end;

    if (!ox_value_is_object(ctxt, pkg)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("package information must be an object"));
        goto end;
    }

    /*Store name and path of the packages.*/
    if ((r = ox_set(ctxt, pkg, OX_STRING(ctxt, _name), name)) == OX_ERR)
        goto end;

    if ((r = ox_set(ctxt, pkg, OX_STRING(ctxt, path), pathv)) == OX_ERR)
        goto end;

    if ((r = ox_set(ctxt, &vm->packages, name, pkg)) == OX_ERR)
        goto end;

    /*Bind text domain.*/
    snprintf(path, sizeof(path), "%s/%s/locale", dir, pn);
    ncstr = ox_string_get_char_star(ctxt, name);
    bindtextdomain(ncstr, path);

    r = OX_OK;
end:
    if (openned)
        ox_input_close(ctxt, input);
    OX_VS_POP(ctxt, input)
    return r;
}

/**
 * Initialize the package manager data.
 * @param ctxt The current running context.
 */
void
ox_package_init (OX_Context *ctxt)
{
    OX_VM *vm = ox_vm_get(ctxt);

    ox_list_init(&vm->pkg_dirs);

    ox_not_error(ox_object_new(ctxt, &vm->packages, NULL));
}

/**
 * Release the package manager data.
 * @param ctxt The current running context.
 */
void
ox_package_deinit (OX_Context *ctxt)
{
    OX_VM *vm = ox_vm_get(ctxt);
    OX_PackageDir *pd, *npd;

    ox_list_foreach_safe_c(&vm->pkg_dirs, pd, npd, OX_PackageDir, ln) {
        ox_strfree(ctxt, pd->dir);
        OX_DEL(ctxt, pd);
    }
}

/**
 * Add a package lookup directory.
 * @param ctxt The current running context.
 * @param dir The directory to be added.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_package_add_dir (OX_Context *ctxt, const char *dir)
{
    OX_VM *vm;
    OX_PackageDir *pd;

    assert(ctxt && dir);

    vm = ox_vm_get(ctxt);

    if (!OX_NEW(ctxt, pd))
        return ox_throw_no_mem_error(ctxt);

    if (!(pd->dir = ox_strdup(ctxt, dir))) {
        OX_DEL(ctxt, pd);
        return ox_throw_no_mem_error(ctxt);
    }

    OX_LOG_D(ctxt, "add package lookup directory \"%s\"", dir);

    ox_list_append(&vm->pkg_dirs, &pd->ln);
    return OX_OK;
}

/**
 * Get the package lookup directories.
 * @param ctxt The current running context.
 * @param[out] dirs Return the directories array.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_package_get_dirs (OX_Context *ctxt, OX_Value *dirs)
{
    OX_Result r;
    OX_VM *vm;
    OX_PackageDir *pd;
    OX_VS_PUSH(ctxt, dir)

    assert(ctxt && dirs);

    vm = ox_vm_get(ctxt);

    if ((r = ox_array_new(ctxt, dirs, 0)) == OX_ERR)
        goto end;

    ox_list_foreach_c(&vm->pkg_dirs, pd, OX_PackageDir, ln) {
        if ((r = ox_string_from_char_star(ctxt, dir, pd->dir)) == OX_ERR)
            goto end;

        if ((r = ox_array_append(ctxt, dirs, dir)) == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, dir)
    return r;
}

/**
 * Lookup the package.
 * @param ctxt The current running context.
 * @param name The name of the package.
 * @param[out] pkg Return the package information.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_package_lookup (OX_Context *ctxt, OX_Value *name, OX_Value *pkg)
{
    OX_VM *vm;
    OX_VS_PUSH(ctxt, pn)
    OX_PackageDir *pd;
    OX_Result r;

    assert(ctxt && name && pkg);
    assert(ox_value_is_string(ctxt, name));

    vm = ox_vm_get(ctxt);

    if ((r = ox_string_trim(ctxt, name, OX_STRING_TRIM_BOTH, pn)) == OX_ERR)
        goto end;

    if ((r = ox_get(ctxt, &vm->packages, pn, pkg)) == OX_ERR)
        goto end;

    if (ox_value_is_null(ctxt, pkg)) {
        ox_list_foreach_c(&vm->pkg_dirs, pd, OX_PackageDir, ln) {
            r = package_lookup(ctxt, pd->dir, name, pkg);
            if (r != OX_FALSE)
                goto end;
        }

        r = ox_throw_reference_error(ctxt,
                OX_TEXT("cannot find package \"%s\""),
                ox_string_get_char_star(ctxt, name));
    }
end:
    OX_VS_POP(ctxt, pn)
    return r;
}

/**
 * Get the packge script.
 * @param ctxt The current running context.
 * @param name The name of the package.
 * @param[out] script Return the script of the package.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_package_script (OX_Context *ctxt, OX_Value *name, OX_Value *script)
{
    OX_VS_PUSH_4(ctxt, pkg, dir, libs, lib)
    OX_CharBuffer cb;
    OX_Result r;
    size_t tlen;

    assert(ctxt && name && script);

    ox_char_buffer_init(&cb);

    if ((r = ox_package_lookup(ctxt, name, pkg)) == OX_ERR)
        goto end;

    if ((r = ox_get(ctxt, pkg, OX_STRING(ctxt, script), script)) == OX_ERR)
        goto end;

    if (ox_value_is_null(ctxt, script)) {
        OX_Script *s;

        ox_not_error(ox_get(ctxt, pkg, OX_STRING(ctxt, path), dir));
        ox_not_error(ox_get(ctxt, pkg, OX_STRING(ctxt, libraries), libs));

        if (!(s = ox_script_new(ctxt, script, dir))) {
            r = OX_ERR;
            goto end;
        }

        if ((r = ox_script_set_text_domain(ctxt, script, name, NULL)) == OX_ERR)
            goto end;

        if (ox_value_is_array(ctxt, libs)) {
            size_t i, len;

            len = ox_array_length(ctxt, libs);

            if (len) {
                if ((r = ox_script_alloc_refs(ctxt, s, len, len)) == OX_ERR)
                    goto end;

                if ((r = ox_char_buffer_append_string(ctxt, &cb, name)) == OX_ERR)
                    goto end;
                if ((r = ox_char_buffer_append_char(ctxt, &cb, '/')) == OX_ERR)
                    goto end;
                tlen = cb.len;

                for (i = 0; i < len; i ++) {
                    OX_ScriptRef *ref = &s->refs[i];
                    OX_ScriptRefItem *item = &s->ref_items[i];

                    if ((r = ox_array_get_item(ctxt, libs, i, lib)) == OX_ERR)
                        goto end;

                    cb.len = tlen;
                    if ((r = ox_char_buffer_append_string(ctxt, &cb, lib)) == OX_ERR)
                        goto end;

                    if ((r = ox_char_buffer_get_string(ctxt, &cb, &ref->filename)) == OX_ERR)
                        goto end;

                    ref->item_start = i;
                    ref->item_num = 1;

                    ox_value_set_bool(ctxt, &item->name, OX_TRUE);
                    ox_value_copy(ctxt, &item->orig, OX_STRING(ctxt, star));
                }
            }
        }

        ox_not_error(ox_set(ctxt, pkg, OX_STRING(ctxt, script), script));
    }

    r = OX_OK;
end:
    ox_char_buffer_deinit(ctxt, &cb);
    OX_VS_POP(ctxt, pkg)
    return r;
}

/*Check if 2 script names are equal.*/
static OX_Bool
script_name_equal (OX_Context *ctxt, OX_Value *n1, OX_Value *n2)
{
    const char *c1 = ox_string_get_char_star(ctxt, n1);
    const char *c2 = ox_string_get_char_star(ctxt, n2);
    size_t l1 = ox_string_length(ctxt, n1);
    size_t l2 = ox_string_length(ctxt, n2);
    typedef enum {
        SUFFIX_UNKNOWN,
        SUFFIX_OX,
        SUFFIX_OXN
    } Suffix;
    Suffix s1 = SUFFIX_UNKNOWN, s2 = SUFFIX_UNKNOWN;

    if ((l1 >= 4) && !strcasecmp(c1, ".oxn")) {
        s1 = SUFFIX_OXN;
        l1 -= 4;
    } else if ((l1 >= 3) && !strcasecmp(c1, ".ox")) {
        s1 = SUFFIX_OX;
        l1 -= 3;
    }

    if ((l2 >= 4) && !strcasecmp(c2, ".oxn")) {
        s2 = SUFFIX_OXN;
        l2 -= 4;
    } else if ((l2 >= 3) && !strcasecmp(c2, ".ox")) {
        s2 = SUFFIX_OX;
        l2 -= 3;
    }

    if ((s1 != SUFFIX_UNKNOWN) && (s2 != SUFFIX_UNKNOWN))
        return strcasecmp(c1, c2) == 0;

    if (l1 != l2)
        return OX_FALSE;

    return strncasecmp(c1, c2, l1) == 0;
}

/**
 * Get the library from the package.
 * @param ctxt The current running context.
 * @param pkg The package information.
 * @param lib The library name.
 * @param path The library's pathname.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_package_get_lib (OX_Context *ctxt, OX_Value *pkg, OX_Value *lib, OX_Value *path)
{
    OX_VS_PUSH_6(ctxt, libs, ln, item, li, dir, name)
    OX_Bool found = OX_FALSE;
    OX_CharBuffer cb;
    OX_Result r;

    assert(ctxt && pkg && lib && path);
    assert(ox_value_is_object(ctxt, pkg));

    ox_char_buffer_init(&cb);

    if ((r = ox_get(ctxt, pkg, OX_STRING(ctxt, _name), name)) == OX_ERR)
        goto end;

    if (!lib || ox_value_is_null(ctxt, lib)) {
        ln = name;
    } else {
        if ((r = ox_string_trim(ctxt, lib, OX_STRING_TRIM_BOTH, ln)) == OX_ERR)
            goto end;
    }

    if ((r = ox_get(ctxt, pkg, OX_STRING(ctxt, libraries), libs)) == OX_ERR)
        goto end;

    if (ox_value_is_array(ctxt, libs)) {
        size_t i, len;

        len = ox_array_length(ctxt, libs);
        for (i = 0; i < len; i ++) {
            if ((r = ox_array_get_item(ctxt, libs, i, item)) == OX_ERR)
                goto end;

            if ((r = ox_string_trim(ctxt, item, OX_STRING_TRIM_BOTH, li)) == OX_ERR)
                goto end;

            if (script_name_equal(ctxt, ln, li)) {
                found = OX_TRUE;
                break;
            }
        }
    }

    if (!found) {
        return ox_throw_reference_error(ctxt,
                OX_TEXT("cannot find library \"%s\" in package \"%s\""),
                ox_string_get_char_star(ctxt, ln),
                ox_string_get_char_star(ctxt, name));
    } else {
        if ((r = ox_get(ctxt, pkg, OX_STRING(ctxt, path), dir)) == OX_ERR)
            goto end;

        if ((r = ox_char_buffer_append_string(ctxt, &cb, dir)) == OX_ERR)
            goto end;

        if ((r = ox_char_buffer_append_char(ctxt, &cb, '/')) == OX_ERR)
            goto end;

        if ((r = ox_char_buffer_append_string(ctxt, &cb, li)) == OX_ERR)
            goto end;

        if ((r = ox_char_buffer_get_string(ctxt, &cb, path)) == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    ox_char_buffer_deinit(ctxt, &cb);
    OX_VS_POP(ctxt, libs)
    return r;
}

/**
 * Get the executable program from the package.
 * @param ctxt The current running context.
 * @param pkg The package information.
 * @param exe The executable program's name.
 * @param path The library's pathname.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_package_get_exe (OX_Context *ctxt, OX_Value *pkg, OX_Value *exe, OX_Value *path)
{
    OX_VS_PUSH_6(ctxt, exes, item, ei, en, dir, name)
    OX_Bool found = OX_FALSE;
    OX_CharBuffer cb;
    OX_Result r;

    assert(ctxt && pkg && exe && path);
    assert(ox_value_is_object(ctxt, pkg));
    assert(ox_value_is_string(ctxt, exe));

    ox_char_buffer_init(&cb);

    if ((r = ox_get(ctxt, pkg, OX_STRING(ctxt, _name), name)) == OX_ERR)
        goto end;

    if (!exe || ox_value_is_null(ctxt, exe)) {
        en = name;
    } else {
        if ((r = ox_string_trim(ctxt, exe, OX_STRING_TRIM_BOTH, en)) == OX_ERR)
            goto end;
    }

    if ((r = ox_get(ctxt, pkg, OX_STRING(ctxt, executables), exes)) == OX_ERR)
        goto end;

    if (ox_value_is_array(ctxt, exes)) {
        size_t i, len;

        len = ox_array_length(ctxt, exes);
        for (i = 0; i < len; i ++) {
            if ((r = ox_array_get_item(ctxt, exes, i, item)) == OX_ERR)
                goto end;

            if ((r = ox_string_trim(ctxt, item, OX_STRING_TRIM_BOTH, ei)) == OX_ERR)
                goto end;

            if (script_name_equal(ctxt, en, ei)) {
                found = OX_TRUE;
                break;
            }
        }
    }

    if (!found) {
        return ox_throw_reference_error(ctxt,
                OX_TEXT("cannot find executable program \"%s\" in package \"%s\""),
                ox_string_get_char_star(ctxt, en),
                ox_string_get_char_star(ctxt, name));
    } else {
        if ((r = ox_get(ctxt, pkg, OX_STRING(ctxt, path), dir)) == OX_ERR)
            goto end;

        if ((r = ox_char_buffer_append_string(ctxt, &cb, dir)) == OX_ERR)
            goto end;

        if ((r = ox_char_buffer_append_char(ctxt, &cb, '/')) == OX_ERR)
            goto end;

        if ((r = ox_char_buffer_append_string(ctxt, &cb, ei)) == OX_ERR)
            goto end;

        if ((r = ox_char_buffer_get_string(ctxt, &cb, path)) == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    ox_char_buffer_deinit(ctxt, &cb);
    OX_VS_POP(ctxt, exes)
    return r;
}

/**
 * Scan the referenced objects in the packages.
 * @param ctxt The current running context.
 */
void
ox_gc_scan_package (OX_Context *ctxt)
{
    OX_VM *vm = ox_vm_get(ctxt);

    ox_gc_scan_value(ctxt, &vm->packages);
}
