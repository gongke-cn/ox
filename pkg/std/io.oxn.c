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
 * IO library.
 */

#define OX_LOG_TAG "io"

#include "std.h"

/*Declaration index.*/
enum {
    ID_File,
    ID_stdin,
    ID_stdout,
    ID_stderr,
    ID_FILE,
    ID_FileIterator_inf,
    ID_File_inf,
    ID_MAX
};

/*Public table.*/
static const char*
pub_tab[] = {
    "File",
    "stdin",
    "stdout",
    "stderr",
    "FILE",
    NULL
};

static const OX_ScriptDesc
script_desc = {
    NULL,
    pub_tab,
    ID_MAX
};

/** File iterator.*/
typedef struct {
    OX_Value file; /**< The file object.*/
    OX_Value fn;   /**< The function.*/
    OX_Value v;    /**< The current value.*/
} OX_FileIter;

/*Scan reference objects in the file iterator.*/
static void
file_iter_scan (OX_Context *ctxt, void *p)
{
    OX_FileIter *fi = p;

    ox_gc_scan_value(ctxt, &fi->file);
    ox_gc_scan_value(ctxt, &fi->fn);
    ox_gc_scan_value(ctxt, &fi->v);
}

/*Free the file iterator.*/
static void
file_iter_free (OX_Context *ctxt, void *p)
{
    OX_FileIter *fi = p;

    OX_DEL(ctxt, fi);
}

/*Operation functions of file iterator.*/
static const OX_PrivateOps
file_iter_ops = {
    file_iter_scan,
    file_iter_free
};

/*Get the file iterator data from the value.*/
static OX_FileIter*
file_iter_get (OX_Context *ctxt, OX_Value *v)
{
    OX_FileIter *fi = ox_object_get_priv(ctxt, v, &file_iter_ops);

    if (!fi)
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a file iterator"));

    return fi;
}

/*FileIterator.$inf.end get*/
static OX_Result
FileIterator_inf_end_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_FileIter *fi;
    OX_Bool b;

    if (!(fi = file_iter_get(ctxt, thiz)))
        return OX_ERR;

    b = ox_value_is_null(ctxt, &fi->v);
    ox_value_set_bool(ctxt, rv, b);
    return OX_OK;
}

/*FileIterator.$inf.value get*/
static OX_Result
FileIterator_inf_value_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_FileIter *fi;

    if (!(fi = file_iter_get(ctxt, thiz)))
        return OX_ERR;

    ox_value_copy(ctxt, rv, &fi->v);
    return OX_OK;
}

/*FileIterator.$inf.next*/
static OX_Result
FileIterator_inf_next (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_FileIter *fi;
    OX_Result r;

    if (!(fi = file_iter_get(ctxt, thiz)))
        return OX_ERR;

    if (!ox_value_is_null(ctxt, &fi->v)) {
        r = ox_call(ctxt, &fi->fn, ox_value_null(ctxt), &fi->file, 1, &fi->v);
        if (r == OX_ERR)
            return r;
    }

    return OX_OK;
}

/*FileIterator.$inf.$close*/
static OX_Result
FileIterator_inf_close (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_FileIter *fi;

    if (!(fi = file_iter_get(ctxt, thiz)))
        return OX_ERR;

    return ox_close(ctxt, &fi->file);
}

/*Set the file script.*/
static OX_Result
file_script_set (OX_Context *ctxt, OX_Value *s)
{
    OX_Result r;
    OX_VS_PUSH_2(ctxt, c, inf)

    if ((r = ox_named_class_new_s(ctxt, c, inf, NULL, "FileIterator")) == OX_ERR)
        goto end;

    if ((r = ox_script_set_value(ctxt, s, ID_FileIterator_inf, inf)) == OX_ERR)
        goto end;

    if ((r = ox_class_inherit(ctxt, c, OX_OBJECT(ctxt, Iterator))) == OX_ERR)
        goto end;

    if ((r = ox_object_add_n_accessor_s(ctxt, inf, "end", FileIterator_inf_end_get, NULL)) == OX_ERR)
        goto end;

    if ((r = ox_object_add_n_accessor_s(ctxt, inf, "value", FileIterator_inf_value_get, NULL)) == OX_ERR)
        goto end;

    if ((r = ox_object_add_n_method_s(ctxt, inf, "next", FileIterator_inf_next)) == OX_ERR)
        goto end;

    if ((r = ox_object_add_n_method_s(ctxt, inf, "$close", FileIterator_inf_close)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    OX_VS_POP(ctxt, c)
    return r;
}

/*Free the file.*/
static void
file_free (OX_Context *ctxt, void *p)
{
    FILE *fp = p;

    if (fp) {
        if ((fp != stdin) && (fp != stdout) && (fp != stderr))
            fclose(fp);
    }
}

/*File's operation functions.*/
static OX_PrivateOps
file_ops = {
    NULL,
    file_free
};

/*File.from_c*/
static OX_Result
File_from_c (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    void *p;
    OX_Value *fty;
    OX_Result r;

    fty = ox_script_get_value(ctxt, f, ID_FILE);

    if (!ox_value_is_cptr(ctxt, arg, fty)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a \"FILE*\""));
        goto end;
    }

    p = ox_cvalue_get_pointer(ctxt, arg);
    if (!p) {
        ox_value_set_null(ctxt, rv);
    } else {
        OX_Value *inf = ox_script_get_value(ctxt, f, ID_File_inf);

        if ((r = ox_object_new(ctxt, rv, inf)) == OX_ERR)
            goto end;

        if ((r = ox_object_set_priv(ctxt, rv, &file_ops, p)) == OX_ERR)
            goto end;
    }

    r = OX_OK;
end:
    return r;
}

/*File.load_text*/
static OX_Result
File_load_text (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *name_arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, name_str)
    const char *name;
    OX_Result r;

    if ((r = ox_to_string(ctxt, name_arg, name_str)) == OX_ERR)
        goto end;
    name = ox_string_get_char_star(ctxt, name_str);

    r = ox_string_from_file(ctxt, rv, name);
end:
    OX_VS_POP(ctxt, name_str)
    return r;
}

/*File.load_data*/
static OX_Result
File_load_data (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *name_arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH_2(ctxt, name_str, pty)
    OX_Value *ity;
    struct stat sb;
    const char *name;
    uint8_t *buf = NULL;
    size_t len = 0;
    FILE *fp = NULL;
    OX_CValueInfo cvi;
    OX_Result r;

    if ((r = ox_to_string(ctxt, name_arg, name_str)) == OX_ERR)
        goto end;
    name = ox_string_get_char_star(ctxt, name_str);

    if ((r = stat(name, &sb)) == -1) {
        r = std_system_error(ctxt, "stat");
        goto end;
    }
    len = sb.st_size;

    if (!(fp = fopen(name, "rb"))) {
        r = std_system_error_v(ctxt, "fopen", errno, OX_TEXT("cannot open file \"%s\""), name);
        goto end;
    }

    if (!OX_NEW_N(ctxt, buf, len)) {
        r = OX_ERR;
        goto end;
    }

    if (fread(buf, 1, len, fp) != len) {
        r = std_system_error(ctxt, "fread");
        goto end;
    }

    ity = ox_ctype_get(ctxt, OX_CTYPE_U8);
    if ((r = ox_ctype_pointer(ctxt, pty, ity, len)) == OX_ERR)
        goto end;

    cvi.v.p = buf;
    cvi.base = NULL;
    cvi.own = OX_CPTR_NON_OWNER;

    if ((r = ox_cvalue_new(ctxt, rv, pty, &cvi)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    if (r == OX_ERR) {
        if (buf)
            OX_DEL_N(ctxt, buf, len);
    }
    if (fp)
        fclose(fp);
    OX_VS_POP(ctxt, name_str)
    return r;
}

/*File.store_text*/
static OX_Result
File_store_text (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *name_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *txt_arg = ox_argument(ctxt, args, argc, 1);
    OX_VS_PUSH_2(ctxt, name_str, txt_str)
    const char *name, *txt;
    size_t len;
    FILE *fp = NULL;
    OX_Result r;

    if ((r = ox_to_string(ctxt, name_arg, name_str)) == OX_ERR)
        goto end;
    name = ox_string_get_char_star(ctxt, name_str);

    if ((r = ox_to_string(ctxt, txt_arg, txt_str)) == OX_ERR)
        goto end;
    txt = ox_string_get_char_star(ctxt, txt_str);
    len = ox_string_length(ctxt, txt_str);

    if (!(fp = fopen(name, "wb"))) {
        r = std_system_error_v(ctxt, "fopen", errno, OX_TEXT("cannot open file \"%s\""), name);
        goto end;
    }

    if (fwrite(txt, 1, len, fp) != len) {
        r = std_system_error(ctxt, "fwrite");
        goto end;
    }

    r = OX_OK;
end:
    if (fp)
        fclose(fp);
    OX_VS_POP(ctxt, name_str)
    return r;
}

/*File.store_data*/
static OX_Result
File_store_data (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *name_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *data_arg = ox_argument(ctxt, args, argc, 1);
    OX_VS_PUSH(ctxt, name_str)
    OX_Value *ty;
    const char *name;
    uint8_t *p;
    size_t len;
    FILE *fp = NULL;
    OX_Result r;

    if ((r = ox_to_string(ctxt, name_arg, name_str)) == OX_ERR)
        goto end;
    name = ox_string_get_char_star(ctxt, name_str);

    if (!ox_value_is_cvalue(ctxt, data_arg)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a C value"));
        goto end;
    }

    ty = ox_cvalue_get_ctype(ctxt, data_arg);
    len = ox_ctype_get_size(ctxt, ty);
    p = ox_cvalue_get_pointer(ctxt, data_arg);

    if (!(fp = fopen(name, "wb"))) {
        r = std_system_error_v(ctxt, "fopen", errno, OX_TEXT("cannot open file \"%s\""), name);
        goto end;
    }

    if (fwrite(p, 1, len, fp) != len) {
        r = std_system_error(ctxt, "fwrite");
        goto end;
    }

    r = OX_OK;
end:
    if (fp)
        fclose(fp);
    OX_VS_POP(ctxt, name_str)
    return r;
}

/*File.$inf.$init*/
static OX_Result
File_inf_init (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *name = ox_argument(ctxt, args, argc, 0);
    OX_Value *mode = ox_argument(ctxt, args, argc, 1);
    OX_VS_PUSH_2(ctxt, nstr, mstr)
    const char *ncstr, *mcstr;
    FILE *fp;
    OX_Result r;

    if ((r = ox_to_string(ctxt, name, nstr)) == OX_ERR)
        goto end;
    ncstr = ox_string_get_char_star(ctxt, nstr);

    if (!ox_value_is_null(ctxt, mode)) {
        if ((r = ox_to_string(ctxt, mode, mstr)) == OX_ERR)
            goto end;
        mcstr = ox_string_get_char_star(ctxt, mstr);
    } else {
        mcstr = "rb";
    }

    OX_SCHED(ctxt, fp = fopen(ncstr, mcstr));

    if (!fp) {
        r = std_system_error_v(ctxt, "fopen", errno, "\"%s\", \"%s\"", ncstr, mcstr);
        goto end;
    }

    if ((r = ox_object_set_priv(ctxt, thiz, &file_ops, fp)) == OX_ERR) {
        fclose(fp);
        goto end;
    }
end:
    OX_VS_POP(ctxt, nstr)
    return r;
}

/*Get file from the value.*/
static OX_Result
file_get (OX_Context *ctxt, OX_Value *v, FILE **pf)
{
    if (ox_object_get_priv_ops(ctxt, v) != &file_ops) {
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a file"));
        return OX_ERR;
    }

    *pf = ox_object_get_priv(ctxt, v, &file_ops);

    return OX_OK;
}

/*Get the openned file from the value.*/
static FILE*
openned_file_get (OX_Context *ctxt, OX_Value *v)
{
    FILE *fp;
    OX_Result r;

    if ((r = file_get(ctxt, v, &fp)) == OX_ERR)
        return NULL;

    if (!fp) {
        ox_throw_reference_error(ctxt, OX_TEXT("the file is closed"));
        return NULL;
    }

    return fp;
}

/*File.$inf.$close*/
static OX_Result
File_inf_close (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    FILE *fp;
    OX_Result r;

    if ((r = file_get(ctxt, thiz, &fp)) == OX_ERR)
        return r;

    if (fp)
        ox_object_set_priv(ctxt, thiz, &file_ops, NULL);
    return OX_OK;
}

/*File.$inf.puts*/
static OX_Result
File_inf_puts (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, str)
    const char *cstr;
    FILE *fp;
    OX_Result r;
    int n;

    if (!(fp = openned_file_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_to_string(ctxt, arg, str)) == OX_ERR)
        goto end;

    cstr = ox_string_get_char_star(ctxt, str);
    OX_SCHED(ctxt, n = fputs(cstr, fp));
    if (n == EOF) {
        r = std_system_error(ctxt, "fputs");
        goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, str)
    return r;
}

/*File.$inf.gets*/
static OX_Result
File_inf_gets (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    FILE *fp;
    OX_CharBuffer cb;
    char *rs;
    OX_Result r;

    ox_char_buffer_init(&cb);

    if (!(fp = openned_file_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_vector_expand_capacity(ctxt, &cb, 256)) == OX_ERR)
        goto end;

    while (1) {
        size_t len;

        OX_SCHED(ctxt, rs = fgets(cb.items + cb.len, cb.cap - cb.len, fp));
        if (!rs)
            break;

        len = strlen(cb.items + cb.len);
        cb.len += len;

        if (cb.items[cb.len - 1] == '\n')
            break;

        if ((r = ox_vector_expand_capacity(ctxt, &cb, cb.cap + 1)) == OX_ERR)
            goto end;
    }

    if (cb.len) {
        if ((r = ox_char_buffer_get_string(ctxt, &cb, rv)) == OX_ERR)
            goto end;
    } else {
        ox_value_set_null(ctxt, rv);
    }

    r = OX_OK;
end:
    ox_char_buffer_deinit(ctxt, &cb);
    return r;
}

/*File.$inf.putc*/
static OX_Result
File_inf_putc (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    FILE *fp;
    int32_t c;
    OX_Result r;

    if (!(fp = openned_file_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_to_int32(ctxt, arg, &c)) == OX_ERR)
        goto end;

    OX_SCHED(ctxt, r = fputc(c, fp));

    if (r == EOF) {
        r = ox_throw_system_error(ctxt, OX_TEXT("\"%s\" error"), "fputc");
        goto end;
    }

    r = OX_OK;
end:
    return r;
}

/*File.$inf.getc*/
static OX_Result
File_inf_getc (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    FILE *fp;
    int32_t c;
    OX_Result r;

    if (!(fp = openned_file_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_to_int32(ctxt, arg, &c)) == OX_ERR)
        goto end;

    OX_SCHED(ctxt, r = fgetc(fp));
    if (r == EOF) {
        if (ferror(fp)) {
            r = ox_throw_system_error(ctxt, OX_TEXT("\"%s\" error"), "fgetc");
            goto end;
        }

        ox_value_set_null(ctxt, rv);
    } else {
        ox_value_set_number(ctxt, rv, r);
    }

    r = OX_OK;
end:
    return r;
}

/*File.$inf.read*/
static OX_Result
File_inf_read (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *buf = ox_argument(ctxt, args, argc, 0);
    OX_Value *start = ox_argument(ctxt, args, argc, 1);
    OX_Value *len = ox_argument(ctxt, args, argc, 2);
    OX_Value *cty;
    size_t start_idx = 0, len_idx = 0, read_num;
    OX_CArrayInfo ai;
    uint8_t *p;
    FILE *fp;
    OX_Result r;

    if (!(fp = openned_file_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if (!ox_value_is_cvalue(ctxt, buf)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a C value"));
        goto end;
    }

    cty = ox_cvalue_get_ctype(ctxt, buf);

    if ((r = ox_ctype_get_array_info(ctxt, cty, &ai)) == OX_ERR)
        goto end;

    if (ai.isize == -1) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the C array's item size is unknown"));
        goto end;
    }

    if (!(p = ox_cvalue_get_pointer(ctxt, buf))) {
        ox_value_set_number(ctxt, rv, 0);
        r = OX_OK;
        goto end;
    }

    if (!ox_value_is_null(ctxt, start)) {
        if ((r = ox_to_index(ctxt, start, &start_idx)) == OX_ERR)
            goto end;

        if ((ai.len != -1) && (start_idx >= ai.len)) {
            ox_value_set_number(ctxt, rv, 0);
            r = OX_OK;
            goto end;
        }
    }

    if (!ox_value_is_null(ctxt, len)) {
        if ((r = ox_to_index(ctxt, len, &len_idx)) == OX_ERR)
            goto end;
    } else {
        if (ai.len == -1) {
            r = ox_throw_type_error(ctxt, OX_TEXT("the C array's length is unknown"));
            goto end;
        }

        len_idx = ai.len - start_idx;
    }

    OX_SCHED(ctxt, read_num = fread(p + ai.isize * start_idx, ai.isize, len_idx, fp));
    ox_value_set_number(ctxt, rv, read_num / ai.isize);

    r = OX_OK;
end:
    return r;
}

/*File.$inf.write*/
static OX_Result
File_inf_write (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *buf = ox_argument(ctxt, args, argc, 0);
    OX_Value *start = ox_argument(ctxt, args, argc, 1);
    OX_Value *len = ox_argument(ctxt, args, argc, 2);
    OX_Value *cty;
    size_t start_idx = 0, len_idx = 0, read_num;
    OX_CArrayInfo ai;
    uint8_t *p;
    FILE *fp;
    OX_Result r;

    if (!(fp = openned_file_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if (!ox_value_is_cvalue(ctxt, buf)) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the value is not a C value"));
        goto end;
    }

    cty = ox_cvalue_get_ctype(ctxt, buf);

    if ((r = ox_ctype_get_array_info(ctxt, cty, &ai)) == OX_ERR)
        goto end;

    if (ai.isize == -1) {
        r = ox_throw_type_error(ctxt, OX_TEXT("the C array's item size is unknown"));
        goto end;
    }

    if (!(p = ox_cvalue_get_pointer(ctxt, buf))) {
        ox_value_set_number(ctxt, rv, 0);
        r = OX_OK;
        goto end;
    }

    if (!ox_value_is_null(ctxt, start)) {
        if ((r = ox_to_index(ctxt, start, &start_idx)) == OX_ERR)
            goto end;

        if ((ai.len != -1) && (start_idx >= ai.len)) {
            ox_value_set_number(ctxt, rv, 0);
            r = OX_OK;
            goto end;
        }
    }

    if (!ox_value_is_null(ctxt, len)) {
        if ((r = ox_to_index(ctxt, len, &len_idx)) == OX_ERR)
            goto end;
    } else {
        if (ai.len == -1) {
            r = ox_throw_type_error(ctxt, OX_TEXT("the C array's length is unknown"));
            goto end;
        }

        len_idx = ai.len - start_idx;
    }

    OX_SCHED(ctxt, read_num = fwrite(p + ai.isize * start_idx, ai.isize, len_idx, fp));
    ox_value_set_number(ctxt, rv, read_num / ai.isize);

    r = OX_OK;
end:
    return r;
}

/*File.$inf.flush*/
static OX_Result
File_inf_flush (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    FILE *fp;
    int n;

    if (!(fp = openned_file_get(ctxt, thiz)))
        return OX_ERR;

    OX_SCHED(ctxt, n = fflush(fp));
    if (n == EOF) {
        return std_system_error(ctxt, "fflush");
    }

    return OX_OK;
}

/*File.$inf.seek*/
static OX_Result
File_inf_seek (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *offv = ox_argument(ctxt, args, argc, 0);
    OX_Value *posv = ox_argument(ctxt, args, argc, 1);
    int64_t off;
    int32_t pos;
    FILE *fp;
    int r;

    if (!(fp = openned_file_get(ctxt, thiz)))
        return OX_ERR;

    if ((r = ox_to_int64(ctxt, offv, &off)) == OX_ERR)
        return r;

    if (ox_value_is_null(ctxt, posv)) {
        pos = SEEK_SET;
    } else if ((r = ox_to_int32(ctxt, posv, &pos)) == OX_ERR) {
        return r;
    }

    OX_SCHED(ctxt, r = fseek(fp, off, pos));
    if (r == -1) {
        return std_system_error(ctxt, "fseek");
    }

    return OX_OK;
}

/*File.$inf.eof get*/
static OX_Result
File_inf_eof_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    FILE *fp;
    int n;

    if (!(fp = openned_file_get(ctxt, thiz)))
        return OX_ERR;

    n = feof(fp);
    ox_value_set_bool(ctxt, rv, n);

    return OX_OK;
}

/*File.$inf.error get*/
static OX_Result
File_inf_error_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    FILE *fp;
    int n;

    if (!(fp = openned_file_get(ctxt, thiz)))
        return OX_ERR;

    n = ferror(fp);
    ox_value_set_bool(ctxt, rv, n);

    return OX_OK;
}

/*File.$inf.is_tty get*/
static OX_Result
File_inf_is_tty_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    FILE *fp;
    int b;

    if (!(fp = openned_file_get(ctxt, thiz)))
        return OX_ERR;

    b = isatty(fileno(fp));
    ox_value_set_bool(ctxt, rv, b);

    return OX_OK;
}

/*File.$inf.$iter*/
static OX_Result
File_inf_iter (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *fn = ox_argument(ctxt, args, argc, 0);
    OX_Value *inf = ox_script_get_value(ctxt, f, ID_FileIterator_inf);
    OX_FileIter *fi;
    OX_Result r;

    if ((r = ox_object_new(ctxt, rv, inf)) == OX_ERR)
        return r;

    if (!OX_NEW(ctxt, fi))
        return ox_throw_no_mem_error(ctxt);

    ox_value_copy(ctxt, &fi->file, thiz);
    ox_value_copy(ctxt, &fi->fn, fn);

    if ((r = ox_call(ctxt, fn, ox_value_null(ctxt), thiz, 1, &fi->v)) == OX_ERR) {
        file_iter_free(ctxt, fi);
        return r;
    }

    if ((r = ox_object_set_priv(ctxt, rv, &file_iter_ops, fi)) == OX_ERR) {
        file_iter_free(ctxt, fi);
        return r;
    }

    return OX_OK;
}

/*File.$inf.to_c*/
static OX_Result
File_inf_to_c (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    FILE *fp;
    OX_Result r;

    if ((r = file_get(ctxt, thiz, &fp)) == OX_ERR)
        return r;

    if (!fp) {
        ox_value_set_null(ctxt, rv);
        r = OX_OK;
    } else {
        OX_CValueInfo cvi;
        OX_Value *fty;
        OX_VS_PUSH(ctxt, pty)

        cvi.v.p = fp;
        cvi.base = thiz;
        cvi.own = OX_CPTR_NON_OWNER;

        fty = ox_script_get_value(ctxt, f, ID_FILE);
        r = ox_ctype_pointer(ctxt, pty, fty, -1);
        if (r == OX_OK)
            r = ox_cvalue_new(ctxt, rv, pty, &cvi);

        OX_VS_POP(ctxt, pty)
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
 *? @lib Input and output functions.
 *?
 *? @class{ File Input and output stream.
 *?
 *? @const SEEK_SET {Number} Seek from the start of the file.
 *? @const SEEK_CUR {Number} Seek from current position.
 *? @const SEEK_END {Number} Seek from the end of the file.
 *?
 *? @sfunc from_c Create a file from "FILE*".
 *? @param ptr {C:FILE*} C Pointer its type is "FILE*".
 *? @return {File} The new file object
 *?
 *? @sfunc load_text Load a text file as string.
 *? The text file is a text file with UTF-8 character encoding.
 *? @param filename {String} The filename.
 *? @return {String} The string with the text data in the file.
 *? @throw {SystemError} Cannot open the file.
 *?
 *? @sfunc load_data Load a binary file.
 *? @param filename {String} The filename.
 *? @return {C:UInt8*} Return the buffer with the file data.
 *? @throw {SystemError} Cannot open the file.
 *?
 *? @sfunc store_text Store the string to the file.
 *? @param filename {String} The filename.
 *? @param text {String} The string to be stored.
 *? @throw {SystemError} Write the file failed.
 *?
 *? @sfunc store_data Store the data to the file.
 *? @param filename {String} The filename.
 *? @param buf {C:void*} The data buffer to be stored.
 *? @throw {SystemError} Write the file failed.
 *?
 *? @func $init Initialize a file object.
 *? @param filename {String} The filename.
 *? @param mode {String} Operation mode description string.
 *? The mode string's syntax is same as "mode" parameter of C function "fopen()".
 *? @ul{
 *? @li "r" means open for reading.
 *? @li "r+" means open for reading and writing,
 *? @li "w" means open for writing.
 *? @li "w+" means open for reading and writing, and file will be created if it does not exist.
 *? @li "a" means open for writing, file will be created if it does not exist, and stream is pointed at end of the file.
 *? @li "a+" means open for reading and writing, file will be created if it does not exist, and stream is pointed at end of the file.
 *? @ul}
 *? @throw {SystemError} Cannot open the file.
 *?
 *? @func $close Close the file.
 *?
 *? @func puts Write a string to the file.
 *? @param str {String} The string to be written.
 *? @throw {SystemError} Write failed.
 *?
 *? @func gets Read a string from the file.
 *? Read from the file until get a newline or reach the end of the file.
 *? @return {?String} Return the string read from the file.
 *? If no character read and reach the end of the file, return null.
 *? @throw {SystemError} Read failed.
 *?
 *? @func putc Write a character to the file.
 *? @param c {Number} The character.
 *? @throw {SystemError} Write failed.
 *?
 *? @func getc Read a character from the file.
 *? @return {?Number} The character read.
 *? If reach the end of the file, return null.
 *?
 *? @func read Read data from the file.
 *? @param buf {C:void*} The data buffer to store the read data.
 *? @param start {Number} =0 Starting position of the buffer to store the data.
 *? @param len {Number} =buf.length-start The number of items to be read.
 *? @return {Number} The real number of read items.
 *? @throw {SystemError} Read failed.
 *?
 *? @func write Write data to the file.
 *? @param buf {C:void*} The data buffer store the write data.
 *? @param start {Number} =0 Starting position of the buffer store the write data.
 *? @param len {Number} =buf.length-start The number of items to be written.
 *? @return {Number} The real number of written items.
 *? @throw {SystemError} Read failed.
 *?
 *? @func flush Flush the file's internal buffers.
 *?
 *? @func seek Move the file pointer.
 *? @param off {Number} The seek offset.
 *? @param whence {?Number} =File.SEEK_SET The seek relative position.
 *?
 *? @roacc eof {Bool} If the file is end.
 *? @roacc error {Bool} If the file has error.
 *? @roacc is_tty {Bool} If the file is a terminal.
 *?
 *? @func $iter Create a file reader iterator.
 *? @param fn {Function} File reader function.
 *? @return {Iterator} The file reader iterator.
 *? The iterator will call the function "fn" repeatedly with the file used as its parameter.
 *? Each time the iterator returns the function's return value, until the functions return null.
 *?
 *? @func to_c Get the "FILE*" of the file object.
 *? @return {C:FILE*} The "FILE*" C pointer of this object.
 *?
 *? @class}
 *?
 *? @const stdin {File} Standard input file.
 *? @const stdout {File} Standard output file.
 *? @const stderr {File} Standard error file.
 */

/*Execute.*/
OX_Result
ox_exec (OX_Context *ctxt, OX_Value *f, OX_Value *s, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH_4(ctxt, c, inf, o, v)

    ox_not_error(file_script_set(ctxt, s));

    /*File.*/
    ox_not_error(ox_named_class_new_s(ctxt, c, inf, NULL, "File"));
    ox_not_error(ox_script_set_value(ctxt, s, ID_File, c));
    ox_not_error(ox_script_set_value(ctxt, s, ID_File_inf, inf));
    ox_not_error(ox_object_add_n_method_s(ctxt, c, "from_c", File_from_c));
    ox_not_error(ox_object_add_n_method_s(ctxt, c, "load_text", File_load_text));
    ox_not_error(ox_object_add_n_method_s(ctxt, c, "load_data", File_load_data));
    ox_not_error(ox_object_add_n_method_s(ctxt, c, "store_text", File_store_text));
    ox_not_error(ox_object_add_n_method_s(ctxt, c, "store_data", File_store_data));

    ox_value_set_number(ctxt, v, SEEK_SET);
    ox_not_error(ox_object_add_const_s(ctxt, c, "SEEK_SET", v));
    ox_value_set_number(ctxt, v, SEEK_CUR);
    ox_not_error(ox_object_add_const_s(ctxt, c, "SEEK_CUR", v));
    ox_value_set_number(ctxt, v, SEEK_END);
    ox_not_error(ox_object_add_const_s(ctxt, c, "SEEK_END", v));

    /*File.$inf*/
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$init", File_inf_init));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$close", File_inf_close));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "puts", File_inf_puts));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "gets", File_inf_gets));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "putc", File_inf_putc));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "getc", File_inf_getc));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "read", File_inf_read));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "write", File_inf_write));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "flush", File_inf_flush));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "eof", File_inf_eof_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "error", File_inf_error_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, inf, "is_tty", File_inf_is_tty_get, NULL));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "$iter", File_inf_iter));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "to_c", File_inf_to_c));
    ox_not_error(ox_object_add_n_method_s(ctxt, inf, "seek", File_inf_seek));

    /*stdin*/
    ox_not_error(ox_object_new(ctxt, o, inf));
    ox_not_error(ox_object_set_priv(ctxt, o, &file_ops, stdin));
    ox_not_error(ox_script_set_value(ctxt, s, ID_stdin, o));

    /*stdout*/
    ox_not_error(ox_object_new(ctxt, o, inf));
    ox_not_error(ox_object_set_priv(ctxt, o, &file_ops, stdout));
    ox_not_error(ox_script_set_value(ctxt, s, ID_stdout, o));

    /*stderr*/
    ox_not_error(ox_object_new(ctxt, o, inf));
    ox_not_error(ox_object_set_priv(ctxt, o, &file_ops, stderr));
    ox_not_error(ox_script_set_value(ctxt, s, ID_stderr, o));

    /*Ctype "FILE"*/
    ox_not_error(ox_ctype_struct(ctxt, o, NULL, -1));
    ox_not_error(ox_script_set_value(ctxt, s, ID_FILE, o));

    OX_VS_POP(ctxt, c)
    return OX_OK;
}
