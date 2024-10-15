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
 * File input.
 */

#define OX_LOG_TAG "ox_file_input"

#include "ox_internal.h"

/*Read character buffer size.*/
#define OX_C_BUF_LEN     1024
/*Unget character buffer size.*/
#define OX_UNGET_BUF_LEN 16

/*Unicode character's encoding.*/
#if __BYTE_ORDER == __LITTLE_ENDIAN
    #define OX_UCS "UCS-4LE"
#else
    #define OX_UCS "UCS-4BE"
#endif

/** File input.*/
typedef struct {
    OX_Input  input;     /**< Base input data.*/
    FILE     *fp;        /**< File.*/
    iconv_t   cd;        /**< iconv device.*/
    char     *c_buf;     /**< Read characters buffer.*/
    size_t    c_num;     /**< Read characters' number.*/
    size_t    c_off;     /**< The first character's offset.*/
    int      *unget_buf; /**< Unget characters stack.*/
    size_t    unget_num; /**< Unget characters' number.*/
} OX_FileInput;

/*Scan referenced objects in the file input.*/
static void
file_input_scan (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_FileInput *fi = (OX_FileInput*)gco;

    ox_input_scan(ctxt, &fi->input);
}

/*Close the file input.*/
static void
file_input_close (OX_Context *ctxt, OX_Input *input)
{
    OX_FileInput *fi = (OX_FileInput*)input;

    if (fi->fp)
        fclose(fi->fp);

    if (fi->cd != (iconv_t)-1)
        iconv_close(fi->cd);
    if (fi->c_buf)
        OX_DEL_N(ctxt, fi->c_buf, OX_C_BUF_LEN);
    if (fi->unget_buf)
        OX_DEL_N(ctxt, fi->unget_buf, OX_UNGET_BUF_LEN);
}

/*Free the file input.*/
static void
file_input_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_FileInput *fi = (OX_FileInput*)gco;

    if (!(fi->input.status & OX_INPUT_ST_CLOSED))
        file_input_close(ctxt, &fi->input);

    ox_input_deinit(ctxt, &fi->input);

    OX_DEL(ctxt, fi);
}

/*Get a character from the file input.*/
static int
file_input_get_char (OX_Context *ctxt, OX_Input *input)
{
    OX_FileInput *fi = (OX_FileInput*)input;
    OX_Bool error = OX_FALSE;
    char *in_b, *out_b;
    uint32_t uc;
    size_t in_left, in_orig_left, out_left, rn, cn;
    int c;

    /*Get character from unget character buffer.*/
    if (fi->unget_num) {
        fi->unget_num --;
        return fi->unget_buf[fi->unget_num];
    }

    /*Convert unicode character.*/
    while (1) {
        if (fi->fp && (fi->c_num < 16)) {
            ssize_t n;

            /*Read file.*/
            if (fi->c_num && fi->c_off)
                memmove(fi->c_buf, fi->c_buf + fi->c_off, fi->c_num);

            fi->c_off = 0;

            n = fread(fi->c_buf + fi->c_num, 1, OX_C_BUF_LEN - fi->c_num, fi->fp);
            if (n == 0) {
                if (fi->c_num == 0)
                    return OX_INPUT_END;
            } else {
                fi->c_num += n;
            }
        }

        /*Convert the encoding.*/
        in_b = fi->c_buf + fi->c_off;
        in_left = fi->c_num;
        in_orig_left = in_left;
        out_b = (char*)&uc;
        out_left = sizeof(uint32_t);

        rn = iconv(fi->cd, &in_b, &in_left, &out_b, &out_left);
        
        cn = in_orig_left - in_left;
        fi->c_off += cn;
        fi->c_num -= cn;

        if (out_left == 0)
            break;

        /*Illegal character.*/
        assert((rn == (size_t)-1) && (errno == EILSEQ));

        if (!error) {
            OX_Location loc;
            int old_status;

            error = OX_TRUE;
            fi->input.status |= OX_INPUT_ST_ERR;

            ox_input_get_loc(input, &loc.first_line, &loc.first_column);
            ox_input_get_loc(input, &loc.last_line, &loc.last_column);

            old_status = fi->input.status;
            fi->input.status |= OX_INPUT_ST_NOT_SHOW;
            ox_error(ctxt, input, &loc, OX_TEXT("illegal character"));
            fi->input.status = old_status;
        }

        fi->c_off ++;
        fi->c_num --;
    }

    c = uc;

    return c;
}

/*Push back a character to the file input.*/
static void
file_input_unget_char (OX_Context *ctxt, OX_Input *input, int c)
{
    OX_FileInput *fi = (OX_FileInput*)input;

    assert(fi->unget_num < OX_UNGET_BUF_LEN);

    fi->unget_buf[fi->unget_num ++] = c;
}

/*Get the file input's current read position..*/
static long
file_input_tell (OX_Context *ctxt, OX_Input *input)
{
    OX_FileInput *fi = (OX_FileInput*)input;

    return ftell(fi->fp) - fi->c_num;
}

/*Reopen the file input.*/
static OX_Result
file_input_reopen (OX_Context *ctxt, OX_Input *input, OX_Value *v, long off)
{
    OX_FileInput *fi = (OX_FileInput*)input;
    OX_FileInput *nfi;
    OX_Result r;

    if ((r = ox_file_input_new(ctxt, v, fi->input.name)) == OX_ERR)
        return r;

    nfi = ox_value_get_gco(ctxt, v);

    if (fseek(nfi->fp, off, SEEK_SET) == -1) {
        ox_input_close(ctxt, v);
        return ox_throw_system_error(ctxt, OX_TEXT("\"%s\" failed: %s"),
                "fseek", strerror(errno));
    }

    return OX_OK;
}

/*File input's operation functions.*/
static const OX_InputOps
file_input_ops = {
    {
        OX_GCO_FILE_INPUT,
        file_input_scan,
        file_input_free
    },
    file_input_get_char,
    file_input_unget_char,
    file_input_tell,
    file_input_close,
    file_input_reopen
};

/**
 * Create a new file input.
 * @param ctxt The current running context.
 * @param[out] inputv Return the new input value.
 * @param filename The filename.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_file_input_new (OX_Context *ctxt, OX_Value *inputv, const char *filename)
{
    OX_VM *vm;
    OX_FileInput *fi;
    OX_Result r;
    char *enc;

    assert(ctxt && inputv && filename);

    if (!OX_NEW(ctxt, fi))
        return ox_throw_no_mem_error(ctxt);

    ox_input_init(ctxt, &fi->input);

    fi->fp = NULL;
    fi->cd = (iconv_t)-1;
    fi->c_buf = NULL;
    fi->c_num = 0;
    fi->c_off = 0;
    fi->unget_buf = NULL;
    fi->unget_num = 0;

    fi->input.gco.ops = (OX_GcObjectOps*)&file_input_ops;

    if (!(fi->fp = fopen(filename, "rb"))) {
        r = ox_throw_access_error(ctxt, OX_TEXT("cannot open file \"%s\""), filename);
        goto end;
    }

    vm = ox_vm_get(ctxt);
    enc = vm->file_enc ? vm->file_enc : "UTF-8";

    fi->cd = iconv_open(OX_UCS, enc);
    if (fi->cd == (iconv_t)-1) {
        r = ox_throw_system_error(ctxt, OX_TEXT("\"%s\" %s -> %s failed"),
                "iconv_open", enc, OX_UCS);
        goto end;
    }

    if (!(fi->input.name = ox_strdup(ctxt, filename))) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    if (!OX_NEW_N(ctxt, fi->c_buf, OX_C_BUF_LEN)) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    if (!OX_NEW_N(ctxt, fi->unget_buf, OX_UNGET_BUF_LEN)) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    ox_value_set_gco(ctxt, inputv, fi);
    ox_gc_add(ctxt, fi);

    r = OX_OK;
end:
    if (r == OX_ERR) {
        file_input_close(ctxt, &fi->input);
        OX_DEL(ctxt, fi);
    }

    return r;
}
