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
 * String.
 */

#define OX_LOG_TAG "ox_string"

#include "ox_internal.h"

/*Get the owned keys in the string.*/
static OX_Result
string_keys (OX_Context *ctxt, OX_Value *o, OX_Value *keys)
{
    ox_value_set_null(ctxt, keys);
    return OX_OK;
}

/*Lookup the owned property in the string.*/
static OX_Result
string_lookup (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    OX_VS_PUSH(ctxt, s)
    OX_Result r;

    if (ox_value_is_number(ctxt, p)) {
        size_t id, len;

        if ((r = ox_to_string(ctxt, o, s)) == OX_ERR)
            goto end;

        if ((r = ox_to_index(ctxt, p, &id)) == OX_ERR)
            goto end;

        len = ox_string_length(ctxt, s);

        if (id < len) {
            const char *c = ox_string_get_char_star(ctxt, s);

            return ox_string_from_chars(ctxt, v, c + id, 1);
        }
    }

    ox_value_set_null(ctxt, v);
    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*Get property value of a string.*/
static OX_Result
string_get (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    OX_VS_PUSH(ctxt, s)
    OX_Result r;

    if (ox_value_is_number(ctxt, p)) {
        size_t id, len;

        if ((r = ox_to_string(ctxt, o, s)) == OX_ERR)
            goto end;

        if ((r = ox_to_index(ctxt, p, &id)) == OX_ERR)
            goto end;

        len = ox_string_length(ctxt, s);

        if (id >= len) {
            r = OX_FALSE;
        } else {
            const char *c = ox_string_get_char_star(ctxt, s);

            r = ox_string_from_chars(ctxt, v, c + id, 1);
        }
    } else {
        r = ox_object_get_t(ctxt, OX_OBJECT(ctxt, String_inf), p, v, o);
    }
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*Set property value of a string.*/
static OX_Result
string_set (OX_Context *ctxt, OX_Value *o, OX_Value *p, OX_Value *v)
{
    return ox_throw_type_error(ctxt, OX_TEXT("cannot set the property of a string"));
}

/*Delete the owned property of the string.*/
static OX_Result
string_del (OX_Context *ctxt, OX_Value *o, OX_Value *p)
{
    OX_VS_PUSH(ctxt, s)
    OX_Result r;

    if (ox_value_is_number(ctxt, p)) {
        size_t id, len;

        if ((r = ox_to_string(ctxt, o, s)) == OX_ERR)
            goto end;

        if ((r = ox_to_index(ctxt, p, &id)) == OX_ERR)
            goto end;

        len = ox_string_length(ctxt, s);

        if (id < len) {
            r = ox_throw_access_error(ctxt, OX_TEXT("string cannot be modified"));
            goto end;
        }
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*Call the string.*/
static OX_Result
string_call (OX_Context *ctxt, OX_Value *o, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *r)
{
    ox_value_copy(ctxt, r, o);
    return OX_OK;
}

/*Free a string.*/
static void
string_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_String *s = (OX_String*)gco;

    OX_DEL_N(ctxt, s->chars, s->len + 1);
    OX_DEL(ctxt, s);
}

/*Free a string with constant characters buffer.*/
static void
const_string_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_String *s = (OX_String*)gco;

    OX_DEL(ctxt, s);
}

/*Remove the singleton entry.*/
static void
singleton_string_remove (OX_Context *ctxt, OX_String *s)
{
    OX_VM *vm = ox_vm_get(ctxt);
    OX_HashEntry *e;

    e = ox_hash_remove(ctxt, &vm->str_singleton_hash, s, NULL);

    assert(e);

    OX_DEL(ctxt, e);
}

/*Free a singleton string.*/
static void
singleton_string_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_String *s = (OX_String*)gco;

    singleton_string_remove(ctxt, s);

    OX_DEL_N(ctxt, s->chars, s->len + 1);
    OX_DEL(ctxt, s);
}

/*Free a singleton string with constant characters buffer.*/
static void
const_singleton_string_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_String *s = (OX_String*)gco;

    singleton_string_remove(ctxt, s);

    OX_DEL(ctxt, s);
}

/*String's operation functions.*/
static const OX_ObjectOps
string_ops = {
    {
        OX_GCO_STRING,
        NULL,
        string_free
    },
    string_keys,
    string_lookup,
    string_get,
    string_set,
    string_del,
    string_call
};

/*String's operation functions which has constant characters buffer.*/
static const OX_ObjectOps
const_string_ops = {
    {
        OX_GCO_STRING,
        NULL,
        const_string_free
    },
    string_keys,
    string_lookup,
    string_get,
    string_set,
    string_del,
    string_call
};

/*Singleton string's operation functions.*/
static const OX_ObjectOps
singleton_string_ops = {
    {
        OX_GCO_SINGLETON_STRING,
        NULL,
        singleton_string_free
    },
    string_keys,
    string_lookup,
    string_get,
    string_set,
    string_del,
    string_call
};

/*Singleton string's operation functions which has constant characters buffer.*/
static const OX_ObjectOps
const_singleton_string_ops = {
    {
        OX_GCO_SINGLETON_STRING,
        NULL,
        const_singleton_string_free
    },
    string_keys,
    string_lookup,
    string_get,
    string_set,
    string_del,
    string_call
};

#ifdef OX_SUPPORT_MMAP
/*Free a string with is mapped from a file.*/
static void
map_string_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_String *s = (OX_String*)gco;

    if (s->chars)
        munmap(s->chars, s->len + 1);

    OX_DEL(ctxt, s);
}

/*Free a singleton string which is mapped from a file.*/
static void
map_singleton_string_free (OX_Context *ctxt, OX_GcObject *gco)
{
    OX_String *s = (OX_String*)gco;

    singleton_string_remove(ctxt, s);

    if (s->chars)
        munmap(s->chars, s->len + 1);

    OX_DEL(ctxt, s);
}

/*String's operation functions which is mapped from a file.*/
static const OX_ObjectOps
map_string_ops = {
    {
        OX_GCO_STRING,
        NULL,
        map_string_free
    },
    string_keys,
    string_lookup,
    string_get,
    string_set,
    string_del,
    string_call
};

/*Singleton string's operation functions which is mapped from a file.*/
static const OX_ObjectOps
map_singleton_string_ops = {
    {
        OX_GCO_SINGLETON_STRING,
        NULL,
        map_singleton_string_free
    },
    string_keys,
    string_lookup,
    string_get,
    string_set,
    string_del,
    string_call
};
#endif /*OX_SUPPORT_MMAP*/

/*Allocate a new string.*/
static OX_String*
str_alloc (OX_Context *ctxt, OX_Value *v, size_t len)
{
    OX_String *s;

    if (!OX_NEW(ctxt, s)) {
        ox_throw_no_mem_error(ctxt);
        return NULL;
    }

    if (len) {
        s->gco.ops = (OX_GcObjectOps*)&string_ops;
        s->len = len;

        if (!OX_NEW_N(ctxt, s->chars, len + 1)) {
            OX_DEL(ctxt, s);
            ox_throw_no_mem_error(ctxt);
            return NULL;
        }

        s->chars[len] = 0;
    } else {
        s->gco.ops = (OX_GcObjectOps*)&const_string_ops;
        s->chars = "";
        s->len = 0;
    }

    ox_value_set_gco(ctxt, v, s);
    ox_gc_add(ctxt, s);

    return s;
}

/**
 * Allocate a new string.
 * @param ctxt The current running context.
 * @param[out] v Return the string value.
 * @param len Length of the string.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_String*
ox_string_alloc (OX_Context *ctxt, OX_Value *v, size_t len)
{
    return str_alloc(ctxt, v, len);
}

/**
 * Create a string from a constant 0 terminated characters buffer.
 * @param ctxt The current running context.
 * @param[out] v Return the result string.
 * @param cstr The 0 terminated characters buffer.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_string_from_const_char_star (OX_Context *ctxt, OX_Value *v, const char *cstr)
{
    OX_String *s;

    assert(ctxt && v);

    if (!OX_NEW(ctxt, s))
        return ox_throw_no_mem_error(ctxt);

    s->gco.ops = (OX_GcObjectOps*)&const_string_ops;

    if (cstr) {
        s->chars = (char*)cstr;
        s->len = strlen(cstr);
    } else {
        s->chars = "";
        s->len = 0;
    }

    ox_value_set_gco(ctxt, v, s);
    ox_gc_add(ctxt, s);

    return OX_OK;
}

/**
 * Create a string from a 0 terminated characters buffer.
 * @param ctxt The current running context.
 * @param[out] v Return the result string.
 * @param cstr The 0 terminated characters buffer.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_string_from_char_star (OX_Context *ctxt, OX_Value *v, const char *cstr)
{
    size_t len = cstr ? strlen(cstr) : 0;

    return ox_string_from_chars(ctxt, v, cstr, len);
}

/**
 * Create a string from characters.
 * @param ctxt The current running context.
 * @param[out] v Return the result string.
 * @param chars The characters buffer.
 * @param len Length of the string.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_string_from_chars (OX_Context *ctxt, OX_Value *v, const char *chars, size_t len)
{
    OX_String *s;

    assert(ctxt && v);

    if (!(s = str_alloc(ctxt, v, len)))
        return OX_ERR;

    if (len)
        memcpy(s->chars, chars, len);

    return OX_OK;
}

/**
 * Create a string from a file.
 * @param ctxt The current running context.
 * @param[out] v Return the result string.
 * @param fn The filename.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_string_from_file (OX_Context *ctxt, OX_Value *v, const char *fn)
{
    struct stat sb;
    size_t len;
    int fd = -1;
    OX_String *s;
    OX_Result r;
#ifdef OX_SUPPORT_MMAP
    void *ptr = NULL;
#endif /*OX_SUPPORT_MMAP*/

    if ((fd = open(fn, O_RDONLY)) == -1) {
        r = ox_throw_system_error(ctxt, OX_TEXT("\"%s\" \"%s\" failed: %s"),
                "open", fn, strerror(errno));
        goto end;
    }

    if ((r = fstat(fd, &sb)) == -1) {
        r = ox_throw_system_error(ctxt, OX_TEXT("\"%s\" \"%s\" failed: %s"),
                "fstat", fn, strerror(errno));
        goto end;
    }
    len = sb.st_size;

#ifdef OX_SUPPORT_MMAP
    if (!(ptr = mmap(NULL, len + 1, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0))) {
        r = ox_throw_system_error(ctxt, OX_TEXT("\"%s\" failed: %s"),
                "mmap", strerror(errno));
        goto end;
    }

    if (!OX_NEW(ctxt, s)) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    s->gco.ops = (OX_GcObjectOps*)&map_string_ops;
    s->len = len;
    s->chars = ptr;
    s->chars[len] = 0;

    ox_value_set_gco(ctxt, v, s);
    ox_gc_add(ctxt, s);
#else /*!defined OX_SUPPORT_MMAP*/
    if (!(s = str_alloc(ctxt, v, len))) {
        r = OX_ERR;
        goto end;
    }

    if (read(fd, s->chars, len) != len) {
        r = ox_throw_system_error(ctxt, OX_TEXT("\"%s\" failed: %s"),
                "read", strerror(errno));
        goto end;
    }
#endif /*OX_SUPPORT_MMAP*/

    r = OX_OK;
end:
#ifdef OX_SUPPORT_MMAP
    if (r == OX_ERR) {
        if (ptr)
            munmap(ptr, len + 1);
    }
#endif /*OX_SUPPORT_MMAP*/
    if (fd != -1)
        close(fd);
    return r;
}

/**
 * Check if 2 strings are equal.
 * @param ctxt The current running context.
 * @param sv1 String 1.
 * @param sv2 String 2.
 * @retval OX_TRUE sv1 == sv2.
 * @retval OX_FALSE sv1 != sv2.
 */
OX_Bool
ox_string_equal (OX_Context *ctxt, OX_Value *sv1, OX_Value *sv2)
{
    OX_String *s1;
    OX_String *s2;
    const char *c1, *c2;
    size_t l1, l2;

    assert(ox_value_is_string(ctxt, sv1));
    assert(ox_value_is_string(ctxt, sv2));

    s1 = ox_value_get_gco(ctxt, sv1);
    s2 = ox_value_get_gco(ctxt, sv2);

    if (s1 == s2)
        return OX_TRUE;

    l1 = s1->len;
    l2 = s2->len;

    if (l1 != l2)
        return OX_FALSE;

    c1 = s1->chars;
    c2 = s2->chars;

    if (c1 == c2)
        return OX_TRUE;

    while (l1) {
        if (*c1 != *c2)
            return OX_FALSE;

        c1 ++;
        c2 ++;
        l1 --;
    }

    return OX_TRUE;
}

/**
 * Concatenate 2 strings.
 * @param ctxt The current running context.
 * @param s1 String 1.
 * @param s2 String 2.
 * @param[out] sr Return the result string.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_string_concat (OX_Context *ctxt, OX_Value *s1, OX_Value *s2, OX_Value *sr)
{
    OX_String *s, *sp1, *sp2;
    size_t l1, l2;

    assert(ctxt && s1 && s2 && sr);
    assert(ox_value_is_string(ctxt, s1));
    assert(ox_value_is_string(ctxt, s2));

    l1 = ox_string_length(ctxt, s1);
    l2 = ox_string_length(ctxt, s2);

    if (l1 == 0) {
        ox_value_copy(ctxt, sr, s2);
        return OX_OK;
    }

    if (l2 == 0) {
        ox_value_copy(ctxt, sr, s1);
        return OX_OK;
    }

    if (!OX_NEW(ctxt, s))
        return ox_throw_no_mem_error(ctxt);

    s->gco.ops = (OX_GcObjectOps*)&string_ops;
    s->len = l1 + l2;

    if (!OX_NEW_N(ctxt, s->chars, s->len + 1)) {
        OX_DEL(ctxt, s);
        return ox_throw_no_mem_error(ctxt);
    }

    sp1 = ox_value_get_gco(ctxt, s1);
    sp2 = ox_value_get_gco(ctxt, s2);

    memcpy(s->chars, sp1->chars, l1);
    memcpy(s->chars + l1, sp2->chars, l2);
    s->chars[s->len] = 0;

    ox_value_set_gco(ctxt, sr, s);
    ox_gc_add(ctxt, s);

    return OX_OK;
}

/**
 * Compare 2 strings.
 * @param ctxt The current running context.
 * @param s1 String 1.
 * @param s2 String 2.
 * @retval 0 s1 == s2.
 * @retval <0 s1 < s2.
 * @retval >0 s1 > s2.
 */
int
ox_string_compare (OX_Context *ctxt, OX_Value *s1, OX_Value *s2)
{
    const char *c1, *c2;

    assert(ctxt && s1 && s2);
    assert(ox_value_is_string(ctxt, s1));
    assert(ox_value_is_string(ctxt, s2));

    c1 = ox_string_get_char_star(ctxt, s1);
    c2 = ox_string_get_char_star(ctxt, s2);

    return strcmp(c1, c2);
}

/**
 * Get the substring.
 * @param ctxt The current running context.
 * @param s The origin string.
 * @param start The start position of the substring.
 * @param len The length of the substring.
 * @param[out] sr Return the result substring.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_string_substr (OX_Context *ctxt, OX_Value *s, size_t start, size_t len, OX_Value *sr)
{
    const char *c;
    size_t total;

    assert(ctxt && s && sr);
    assert(ox_value_is_string(ctxt, s));

    total = ox_string_length(ctxt, s);

    if (start >= total) {
        ox_value_copy(ctxt, sr, OX_STRING(ctxt, empty));
        return OX_OK;
    }

    if (start + len > total)
        len = total - start;

    if ((start == 0) && (len == total)) {
        ox_value_copy(ctxt, sr, s);
        return OX_OK;
    }

    c = ox_string_get_char_star(ctxt, s);

    return ox_string_from_chars(ctxt, sr, c + start, len);
}

/**
 * Trim the prefixed and postfixed spaces in the string.
 * @param ctxt The current running context.
 * @param s The origin string.
 * @param trim Trim mode.
 * @param[out] sr Return the result substring.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_string_trim (OX_Context *ctxt, OX_Value *s, OX_StringTrim trim, OX_Value *sr)
{
    const char *c, *ec;
    size_t len;

    assert(ctxt && s && sr);
    assert(ox_value_is_string(ctxt, s));

    len = ox_string_length(ctxt, s);
    c = ox_string_get_char_star(ctxt, s);
    ec = c + len;

    if (trim & OX_STRING_TRIM_HEAD) {
        while (ox_char_is_space(*c))
            c ++;
    }

    if (trim & OX_STRING_TRIM_TAIL) {
        while ((ec > c) && ox_char_is_space(ec[-1]))
            ec --;
    }

    return ox_string_from_chars(ctxt, sr, c, ec - c);
}

/**
 * Match the string with a pattern.
 * @param ctxt The current running context.
 * @param s The string.
 * @param m A regular expression or a substring.
 * @param pos Match start position.
 * @param[out] mr Return the match result.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_string_match (OX_Context *ctxt, OX_Value *s, OX_Value *m, size_t pos, OX_Value *mr)
{
    OX_Result r;

    assert(ctxt && s && m && mr);
    assert(ox_value_is_string(ctxt, s));
    assert(ox_value_is_string(ctxt, m) || ox_value_is_re(ctxt, m));

    if (ox_value_is_re(ctxt, m)) {
        /*Match with regular expression.*/
        r = ox_re_match(ctxt, m, s, pos, 0, mr);
    } else {
        /*Match with a string.*/
        const char *c, *mc;
        size_t len, mlen;

        c = ox_string_get_char_star(ctxt, s);
        mc = ox_string_get_char_star(ctxt, m);
        len = ox_string_length(ctxt, s);
        mlen = ox_string_length(ctxt, m);

        for (; pos + mlen <= len; pos ++) {
            if (memcmp(c + pos, mc, mlen) == 0)
                break;
        }

        if (pos + mlen > len) {
            ox_value_set_null(ctxt, mr);
            r = OX_OK;
        } else {
            OX_Slice slice;

            slice.start = pos;
            slice.end = pos + mlen;

            r = ox_match_new(ctxt, mr, s, pos, pos + mlen, 1, &slice);
        }
    }

    return r;
}

/** Replace segment type.*/
typedef enum {
    OX_REPLACE_SEG_STRING, /**< String.*/
    OX_REPLACE_SEG_CHAR,   /**< Character.*/
    OX_REPLACE_SEG_MATCH,  /**< Matched substring.*/
    OX_REPLACE_SEG_BEFORE, /**< Substring before the matched data.*/
    OX_REPLACE_SEG_AFTER   /**< Substring after the matched data.*/
} OX_ReplaceSegType;

/** Replace segment converter.*/
typedef enum {
    OX_REPLACE_SEG_ORIG,  /**< Keep origin string.*/
    OX_REPLACE_SEG_UPPER, /**< To uppercase.*/
    OX_REPLACE_SEG_LOWER  /**< To lowercase.*/
} OX_ReplaceSegConv;

/** Replace pattern segment.*/
typedef struct {
    OX_List           ln;   /**< List node data.*/
    OX_ReplaceSegType type; /**< Segment type.*/
    OX_ReplaceSegConv conv; /**< Converter.*/
    union {
        int           c;    /**< Character.*/
        int           mid;  /**< Matched substring's index.*/
        struct {
            const char *c;  /**< Characters of the string.*/
            size_t      len;/**< Length of the string.*/
        } s;                /**< String data.*/
    } s;                    /**< Segment data.*/
} OX_ReplaceSegment;

/** Replace pattern.*/
typedef struct {
    OX_List seg_list; /**< Segments list.*/
} OX_ReplacePattern;

/** Add a segment to the replace pattern.*/
static OX_ReplaceSegment*
replace_add_seg (OX_Context *ctxt, OX_ReplacePattern *rp, OX_ReplaceSegType type)
{
    OX_ReplaceSegment *rs;

    if (!OX_NEW(ctxt, rs)) {
        ox_throw_no_mem_error(ctxt);
        return NULL;
    }

    rs->type = type;
    rs->conv = OX_REPLACE_SEG_ORIG;

    ox_list_append(&rp->seg_list, &rs->ln);
    return rs;
}

/*Parse the replace pattern string.*/
static OX_Result
replace_pattern_init (OX_Context *ctxt, OX_ReplacePattern *rp, OX_Value *s)
{
    const char *c, *b, *e;
    size_t len;
    OX_ReplaceSegment *rs;

    c = ox_string_get_char_star(ctxt, s);
    len = ox_string_length(ctxt, s);
    b = c;
    e = c + len;

    while (c < e) {
        if (*c == '$') {
            OX_ReplaceSegConv conv = OX_REPLACE_SEG_ORIG;

            if (c > b) {
                if (!(rs = replace_add_seg(ctxt, rp, OX_REPLACE_SEG_STRING)))
                    return OX_ERR;

                rs->s.s.c = b;
                rs->s.s.len = c - b;
            }

            c ++;

            if (*c == 'u') {
                conv = OX_REPLACE_SEG_UPPER;
                c ++;
            } else if (*c == 'l') {
                conv = OX_REPLACE_SEG_LOWER;
                c ++;
            }

            switch (*c) {
            case '`':
                if (!(rs = replace_add_seg(ctxt, rp, OX_REPLACE_SEG_BEFORE)))
                    return OX_ERR;

                c ++;
                break;
            case '\'':
                if (!(rs = replace_add_seg(ctxt, rp, OX_REPLACE_SEG_AFTER)))
                    return OX_ERR;

                rs->conv = conv;
                c ++;
                break;
            case '&':
                if (!(rs = replace_add_seg(ctxt, rp, OX_REPLACE_SEG_MATCH)))
                    return OX_ERR;

                rs->s.mid = 0;
                rs->conv = conv;
                c ++;
                break;
            case '$':
                if (!(rs = replace_add_seg(ctxt, rp, OX_REPLACE_SEG_CHAR)))
                    return OX_ERR;

                rs->s.c = '$';
                c ++;
                break;
            default:
                if (ox_char_is_digit(*c)) {
                    int v = *c - '0';

                    c ++;

                    if (ox_char_is_digit(*c)) {
                        v *= 10;
                        v += *c - '0';
                        c ++;
                    }

                    if (!(rs = replace_add_seg(ctxt, rp, OX_REPLACE_SEG_MATCH)))
                        return OX_ERR;

                    rs->s.mid = v;
                    rs->conv = conv;
                } else {
                    if (!(rs = replace_add_seg(ctxt, rp, OX_REPLACE_SEG_CHAR)))
                        return OX_ERR;

                    rs->s.c = '$';
                }
                break;
            }

            b = c;
        } else {
            c ++;
        }
    }

    if (e > b) {
        if (!(rs = replace_add_seg(ctxt, rp, OX_REPLACE_SEG_STRING)))
            return OX_ERR;

        rs->s.s.c = b;
        rs->s.s.len = e - b;
    }

    return OX_OK;
}

/*Replace with the pattern string.*/
static OX_Result
replace_pattern_replace (OX_Context *ctxt, OX_Value *s, OX_ReplacePattern *rp,
        OX_Match *m, OX_CharBuffer *cb)
{
    const char *c, *newc;
    size_t len, clen;
    OX_ReplaceSegment *rs;
    OX_Result r;

    c = ox_string_get_char_star(ctxt, s);
    len = ox_string_length(ctxt, s);

    ox_list_foreach_c(&rp->seg_list, rs, OX_ReplaceSegment, ln) {
        OX_ReplaceSegConv conv = OX_REPLACE_SEG_ORIG;

        switch (rs->type) {
        case OX_REPLACE_SEG_CHAR:
            if ((r = ox_char_buffer_append_char(ctxt, cb, rs->s.c)) == OX_ERR)
                return r;
            continue;
        case OX_REPLACE_SEG_STRING:
            newc = rs->s.s.c;
            clen = rs->s.s.len;
            break;
        case OX_REPLACE_SEG_MATCH:
            if (rs->s.mid < m->group_num) {
                newc = c + m->slices[rs->s.mid].start;
                clen = m->slices[rs->s.mid].end - m->slices[rs->s.mid].start;
            } else {
                newc = NULL;
                clen = 0;
            }
            conv = rs->conv;
            break;
        case OX_REPLACE_SEG_BEFORE:
            newc = c;
            clen = m->start;
            conv = rs->conv;
            break;
        case OX_REPLACE_SEG_AFTER:
            newc = c + m->end;
            clen = len - m->end;
            conv = rs->conv;
            break;
        default:
            assert(0);
        }

        if (conv == OX_REPLACE_SEG_ORIG) {
            if ((r = ox_char_buffer_append_chars(ctxt, cb, newc, clen)) == OX_ERR)
                return r;
        } else if (clen) {
            const char *src;
            char *dst;
            size_t left;

            if ((r = ox_vector_expand_capacity(ctxt, cb, cb->len + clen)) == OX_ERR)
                return r;

            src = newc;
            dst = cb->items + cb->len;
            left = clen;

            while (left --) {
                if (conv == OX_REPLACE_SEG_UPPER) {
                    *dst = ox_char_to_upper(*src);
                } else {
                    *dst = ox_char_to_lower(*src);
                }

                src ++;
                dst ++;
            }

            cb->len += clen;
        }
    }

    return OX_OK;
}

/*Release the replace pattern.*/
static void
replace_pattern_deinit (OX_Context *ctxt, OX_ReplacePattern *rp)
{
    OX_ReplaceSegment *rs, *nrs;

    ox_list_foreach_safe_c(&rp->seg_list, rs, nrs, OX_ReplaceSegment, ln) {
        OX_DEL(ctxt, rs);
    }
}

/**
 * Replace the substrings in the string.
 * @param ctxt The current running context.
 * @param s The string.
 * @param m A regular expression or a substring.
 * @param rep Replace pattern string of replace function.
 * @param pos Replace start position.
 * @param once Only replace once.
 * @param[out] rs Return the result string.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_string_replace (OX_Context *ctxt, OX_Value *s, OX_Value *m, OX_Value *rep,
        size_t pos, OX_Bool once, OX_Value *rs)
{
    OX_CharBuffer cb;
    const char *c;
    size_t len, begin = 0;
    OX_Result r;
    OX_Match *mp;
    OX_Bool is_func;
    OX_ReplacePattern rp;
    OX_VS_PUSH_3(ctxt, mr, fr, sr)

    assert(ctxt && s && m && rep && rs);
    assert(ox_value_is_string(ctxt, s));
    assert(ox_value_is_string(ctxt, m) || ox_value_is_re(ctxt, m));

    ox_char_buffer_init(&cb);
    ox_list_init(&rp.seg_list);

    len = ox_string_length(ctxt, s);
    c = ox_string_get_char_star(ctxt, s);
    is_func = !ox_value_is_string(ctxt, rep);
    if (!is_func) {
        if ((r = replace_pattern_init(ctxt, &rp, rep)) == OX_ERR)
            goto end;
    }

    while (pos <= len) {
        if ((r = ox_string_match(ctxt, s, m, pos, mr)) == OX_ERR)
            goto end;

        if (ox_value_is_null(ctxt, mr)) {
            break;
        } else {
            mp = ox_match_get_data(ctxt, mr);

            if (mp->start != begin) {
                if ((r = ox_char_buffer_append_chars(ctxt, &cb, c + begin, mp->start - begin)) == OX_ERR)
                    goto end;
            }

            if (is_func) {
                if ((r = ox_call(ctxt, rep, ox_value_null(ctxt), mr, 1, fr)) == OX_ERR)
                    goto end;

                if ((r = ox_to_string(ctxt, fr, sr)) == OX_ERR)
                    goto end;

                if ((r = ox_char_buffer_append_string(ctxt, &cb, sr)) == OX_ERR)
                    goto end;
            } else {
                if ((r = replace_pattern_replace(ctxt, s, &rp, mp, &cb)) == OX_ERR)
                    goto end;
            }

            if (mp->end == pos)
                pos ++;
            else
                pos = mp->end;

            begin = pos;

            if (once)
                break;
        }
    }

    if (pos < len) {
        if ((r = ox_char_buffer_append_chars(ctxt, &cb, c + pos, len - pos)) == OX_ERR)
            goto end;
    }

    r = ox_char_buffer_get_string(ctxt, &cb, rs);
end:
    if (!is_func)
        replace_pattern_deinit(ctxt, &rp);
    ox_char_buffer_deinit(ctxt, &cb);
    OX_VS_POP(ctxt, mr)
    return r;
}

/**
 * Convert the string to singleton.
 * @param ctxt The current running context.
 * @param s The string.
 * @param[out] ss Return the singleton string.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_string_singleton_inner (OX_Context *ctxt, OX_String *s, OX_String **ss)
{
    OX_VM *vm = ox_vm_get(ctxt);
    const OX_ObjectOps *ops;
    OX_HashEntry *e, **pe;
    OX_Result r;

    e = ox_hash_lookup(ctxt, &vm->str_singleton_hash, s, &pe);
    if (e) {
        *ss = (OX_String*)e->key;
        return OX_OK;
    }

    if (!OX_NEW(ctxt, e))
        return ox_throw_no_mem_error(ctxt);

    if ((r = ox_hash_insert(ctxt, &vm->str_singleton_hash, s, e, pe)) == OX_ERR)
        return r;

    ops = (OX_ObjectOps*)s->gco.ops;
    if (ops == &string_ops)
        s->gco.ops = (OX_GcObjectOps*)&singleton_string_ops;
    else if (ops == &const_string_ops)
        s->gco.ops = (OX_GcObjectOps*)&const_singleton_string_ops;
#ifdef OX_SUPPORT_MMAP
    else
        s->gco.ops = (OX_GcObjectOps*)&map_singleton_string_ops;
#endif /*OX_SUPPORT_MMAP*/

    *ss = s;

    return OX_OK;
}

/**
 * Initialize the singleton string hash table.
 * @param ctxt The running context.
 */
void
ox_string_singleton_init (OX_Context *ctxt)
{
    OX_VM *vm = ox_vm_get(ctxt);

    ox_string_hash_init(&vm->str_singleton_hash);
}

/**
 * Release the singleton string hash table.
 * @param ctxt The running context.
 */
void
ox_string_singleton_deinit (OX_Context *ctxt)
{
    OX_VM *vm = ox_vm_get(ctxt);
    size_t i;
    OX_HashEntry *e, *ne;

    ox_hash_foreach_safe(&vm->str_singleton_hash, i, e, ne) {
        OX_DEL(ctxt, e);
    }

    ox_hash_deinit(ctxt, &vm->str_singleton_hash);
}

/*Check if the value is a string.*/
static OX_Result
check_string (OX_Context *ctxt, OX_Value *v, OX_Value *s)
{
    return ox_to_string(ctxt, v, s);
}

/*String.from_char*/
static OX_Result
String_from_char (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *c_arg = ox_argument(ctxt, args, argc, 0);
    uint8_t c = 0;
    OX_Result r;

    if ((r = ox_to_uint8(ctxt, c_arg, &c)) == OX_ERR)
        return r;

    return ox_string_from_chars(ctxt, rv, (char*)&c, 1);
}

/*String.from_uchar*/
static OX_Result
String_from_uchar (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *uc_arg = ox_argument(ctxt, args, argc, 0);
    uint32_t uc;
    char buf[8];
    int len;
    OX_Result r;

    if ((r = ox_to_uint32(ctxt, uc_arg, &uc)) == OX_ERR)
        return r;

    len = ox_uc_to_utf8(uc, buf);
    if (len == -1)
        return ox_throw_range_error(ctxt, OX_TEXT("illegal unicode character"));

    return ox_string_from_chars(ctxt, rv, buf, len);
}

/*String.from_file*/
static OX_Result
String_from_file (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *fn_arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, fn)
    const char *fn_cstr;
    OX_Result r;

    if ((r = ox_to_string(ctxt, fn_arg, fn)) == OX_ERR)
        goto end;

    fn_cstr = ox_string_get_char_star(ctxt, fn);

    r = ox_string_from_file(ctxt, rv, fn_cstr);
end:
    OX_VS_POP(ctxt, fn)
    return r;
}

/*String.is*/
static OX_Result
String_is (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *v = ox_argument(ctxt, args, argc, 0);
    OX_Bool b;

    b = ox_value_is_string(ctxt, v);
    ox_value_set_bool(ctxt, rv, b);
    return OX_OK;
}

/*String.$inf.length get*/
static OX_Result
String_inf_length_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, s)
    size_t len;
    OX_Result r;

    if ((r = check_string(ctxt, thiz, s)) == OX_ERR)
        goto end;

    len = ox_string_length(ctxt, s);
    ox_value_set_number(ctxt, rv, len);

    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/** String iterator type.*/
typedef enum {
    OX_STRING_ITER_STR,  /**< Substring.*/
    OX_STRING_ITER_CHAR, /**< Character.*/
    OX_STRING_ITER_UCHAR /**< Unicode character.*/
} OX_StringIterType;

/** String iterator.*/
typedef struct {
    OX_StringIterType type; /**< Iterator's type.*/
    OX_Value          s;    /**< The string.*/
    size_t            pos;  /**< The current position.*/
} OX_StringIter;

/*Scan referenced objects in the string iterator.*/
static void
string_iter_scan (OX_Context *ctxt, void *ptr)
{
    OX_StringIter *si = ptr;

    ox_gc_scan_value(ctxt, &si->s);
}

/*Free the string iterator.*/
static void
string_iter_free (OX_Context *ctxt, void *ptr)
{
    OX_StringIter *si = ptr;

    OX_DEL(ctxt, si);
}

/*String iterator's operation functions.*/
static const OX_PrivateOps
string_iter_ops = {
    string_iter_scan,
    string_iter_free
};

/*Get the string iterator data.*/
static OX_StringIter*
string_iter_get (OX_Context *ctxt, OX_Value *o)
{
    OX_StringIter *si;

    if (!(si = ox_object_get_priv(ctxt, o, &string_iter_ops)))
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a string iterator"));

    return si;
}

/*Create a new string iterator.*/
static OX_Result
string_iter_new (OX_Context *ctxt, OX_Value *iter, OX_Value *v, OX_StringIterType type)
{
    OX_VS_PUSH(ctxt, s)
    OX_StringIter *si;
    OX_Result r;

    if ((r = check_string(ctxt, v, s)) == OX_ERR)
        goto end;

    if ((r = ox_object_new(ctxt, iter, OX_OBJECT(ctxt, StringIterator_inf))) == OX_ERR)
        goto end;

    if (!OX_NEW(ctxt, si)) {
        r = ox_throw_no_mem_error(ctxt);
        goto end;
    }

    ox_value_copy(ctxt, &si->s, s);
    si->type = type;
    si->pos = 0;

    if ((r = ox_object_set_priv(ctxt, iter, &string_iter_ops, si)) == OX_ERR) {
        OX_DEL(ctxt, si);
        goto end;
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*String.$inf.$iter*/
static OX_Result
String_inf_iter (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    return string_iter_new(ctxt, rv, thiz, OX_STRING_ITER_STR);
}

/*String.$inf.chars*/
static OX_Result
String_inf_chars (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    return string_iter_new(ctxt, rv, thiz, OX_STRING_ITER_CHAR);
}

/*String.$inf.uchars*/
static OX_Result
String_inf_uchars (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    return string_iter_new(ctxt, rv, thiz, OX_STRING_ITER_UCHAR);
}

/*String.$inf.$to_num*/
static OX_Result
String_inf_to_num (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *s = ox_value_stack_push_n(ctxt, 3);
    OX_Value *inputv = ox_values_item(ctxt, s, 1);
    OX_Input *input;
    OX_Token tok;
    OX_Lex lex;
    OX_Number n = NAN;
    OX_Bool neg = OX_FALSE;
    OX_Result r;

    if ((r = check_string(ctxt, thiz, s)) == OX_ERR)
        goto end;

    if ((r = ox_string_input_new(ctxt, inputv, s)) == OX_ERR)
        goto end;

    input = ox_value_get_gco(ctxt, inputv);
    ox_lex_init(ctxt, &lex, input);

    tok.v = ox_values_item(ctxt, s, 2);

    ox_lex_token(ctxt, &lex, &tok, 0);
    if (tok.type == '-') {
        neg = OX_TRUE;
        ox_lex_token(ctxt, &lex, &tok, 0);
    }

    if (tok.type != OX_TOKEN_NUMBER)
        goto number_end;
    n = ox_value_get_number(ctxt, tok.v);

    ox_lex_token(ctxt, &lex, &tok, 0);
    if (tok.type != OX_TOKEN_END) {
        n = NAN;
        goto number_end;
    }

    if (neg)
        n = -n;
number_end:
    ox_value_set_number(ctxt, rv, n);

    ox_lex_deinit(ctxt, &lex);
    ox_input_close(ctxt, inputv);
end:
    ox_value_stack_pop(ctxt, s);
    return r;
}

/*String.$inf.trim*/
static OX_Result
String_inf_trim (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, s)
    OX_Result r;

    if ((r = ox_to_string(ctxt, thiz, s)) == OX_ERR)
        goto end;

    if ((r = ox_string_trim(ctxt, s, OX_STRING_TRIM_BOTH, rv)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*String.$inf.trim_h*/
static OX_Result
String_inf_trim_h (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, s)
    OX_Result r;

    if ((r = ox_to_string(ctxt, thiz, s)) == OX_ERR)
        goto end;

    if ((r = ox_string_trim(ctxt, s, OX_STRING_TRIM_HEAD, rv)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*String.$inf.trim_t*/
static OX_Result
String_inf_trim_t (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, s)
    OX_Result r;

    if ((r = ox_to_string(ctxt, thiz, s)) == OX_ERR)
        goto end;

    if ((r = ox_string_trim(ctxt, s, OX_STRING_TRIM_TAIL, rv)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*String.$inf.pad_h*/
static OX_Result
String_inf_pad_h (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *width = ox_argument(ctxt, args, argc, 0);
    OX_Value *pad = ox_argument(ctxt, args, argc, 1);
    size_t width_i = 0, len, left;
    OX_VS_PUSH_2(ctxt, s, pad_s)
    const char *pad_cstr = " ";
    size_t pad_len = 1;
    OX_CharBuffer cb;
    OX_Result r;

    ox_char_buffer_init(&cb);

    if ((r = ox_to_string(ctxt, thiz, s)) == OX_ERR)
        goto end;

    if ((r = ox_to_index(ctxt, width, &width_i)) == OX_ERR)
        goto end;

    len = ox_string_length(ctxt, s);
    if (len >= width_i) {
        ox_value_copy(ctxt, rv, s);
        r = OX_OK;
        goto end;
    }

    if (!ox_value_is_null(ctxt, pad)) {
        if ((r = ox_to_string(ctxt, pad, pad_s)) == OX_ERR)
            goto end;

        if (ox_string_length(ctxt, pad_s) == 0) {
            r = ox_throw_range_error(ctxt, OX_TEXT("pad string cannot be \"\""));
            goto end;
        }

        pad_cstr = ox_string_get_char_star(ctxt, pad_s);
        pad_len = ox_string_length(ctxt, pad_s);
    }

    left = width_i - len;
    while (left) {
        size_t n = OX_MIN(left, pad_len);

        if ((r = ox_char_buffer_append_chars(ctxt, &cb, pad_cstr, n)) == OX_ERR)
            goto end;

        left -= n;
    }

    if ((r = ox_char_buffer_append_string(ctxt, &cb, s)) == OX_ERR)
        goto end;

    r = ox_char_buffer_get_string(ctxt, &cb, rv);
end:
    ox_char_buffer_deinit(ctxt, &cb);
    OX_VS_POP(ctxt, s)
    return r;
}

/*String.$inf.pad_t*/
static OX_Result
String_inf_pad_t (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *width = ox_argument(ctxt, args, argc, 0);
    OX_Value *pad = ox_argument(ctxt, args, argc, 1);
    size_t width_i = 0, len, left;
    OX_VS_PUSH_2(ctxt, s, pad_s)
    const char *pad_cstr = " ";
    size_t pad_len = 1;
    OX_CharBuffer cb;
    OX_Result r;

    ox_char_buffer_init(&cb);

    if ((r = ox_to_string(ctxt, thiz, s)) == OX_ERR)
        goto end;

    if ((r = ox_to_index(ctxt, width, &width_i)) == OX_ERR)
        goto end;

    len = ox_string_length(ctxt, s);
    if (len >= width_i) {
        ox_value_copy(ctxt, rv, s);
        r = OX_OK;
        goto end;
    }

    if (!ox_value_is_null(ctxt, pad)) {
        if ((r = ox_to_string(ctxt, pad, pad_s)) == OX_ERR)
            goto end;

        if (ox_string_length(ctxt, pad_s) == 0) {
            r = ox_throw_range_error(ctxt, OX_TEXT("pad string cannot be \"\""));
            goto end;
        }

        pad_cstr = ox_string_get_char_star(ctxt, pad_s);
        pad_len = ox_string_length(ctxt, pad_s);
    }

    if ((r = ox_char_buffer_append_string(ctxt, &cb, s)) == OX_ERR)
        goto end;

    left = width_i - len;
    while (left) {
        size_t n = OX_MIN(left, pad_len);

        if ((r = ox_char_buffer_append_chars(ctxt, &cb, pad_cstr, n)) == OX_ERR)
            goto end;

        left -= n;
    }

    r = ox_char_buffer_get_string(ctxt, &cb, rv);
end:
    ox_char_buffer_deinit(ctxt, &cb);
    OX_VS_POP(ctxt, s)
    return r;
}

/*String.$inf.match*/
static OX_Result
String_inf_match (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *m_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *pos_arg = ox_argument(ctxt, args, argc, 1);
    size_t pos = 0;
    OX_VS_PUSH_2(ctxt, s, ms)
    OX_Result r;

    if ((r = ox_to_string(ctxt, thiz, s)) == OX_ERR)
        goto end;

    if (!ox_value_is_null(ctxt, pos_arg)) {
        ssize_t p;

        if ((r = ox_to_ssize(ctxt, pos_arg, &p)) == OX_ERR)
            goto end;

        if (p < 0) {
            p = ox_string_length(ctxt, s) + p;

            if (p < 0)
                p = 0;
        }

        pos = p;
    }

    if (!ox_value_is_re(ctxt, m_arg)) {
        if ((r = ox_to_string(ctxt, m_arg, ms)) == OX_ERR)
            goto end;
    } else {
        ox_value_copy(ctxt, ms, m_arg);
        /*ox_re_disassemble(ctxt, ms, stderr);*/
    }

    r = ox_string_match(ctxt, s, ms, pos, rv);
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*String.$inf.replace*/
static OX_Result
String_inf_replace (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *m_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *r_arg = ox_argument(ctxt, args, argc, 1);
    OX_Value *p_arg = ox_argument(ctxt, args, argc, 2);
    OX_Value *o_arg = ox_argument(ctxt, args, argc, 3);
    OX_VS_PUSH_2(ctxt, s, m)
    size_t pos = 0;
    OX_Bool once = OX_FALSE;
    OX_Result r;

    if ((r = ox_to_string(ctxt, thiz, s)) == OX_ERR)
        goto end;

    if (ox_value_is_re(ctxt, m_arg)) {
        ox_value_copy(ctxt, m, m_arg);
    } else {
        if ((r = ox_to_string(ctxt, m_arg, m)) == OX_ERR)
            goto end;
    }

    if (!ox_value_is_null(ctxt, p_arg)) {
        ssize_t p;

        if ((r = ox_to_ssize(ctxt, p_arg, &p)) == OX_ERR)
            goto end;

        if (p < 0) {
            p = ox_string_length(ctxt, s) + p;
            if (p < 0)
                p = 0;
        }

        pos = p;
    }

    once = ox_to_bool(ctxt, o_arg);

    r = ox_string_replace(ctxt, s, m, r_arg, pos, once, rv);
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/** String match iterator.*/
typedef struct {
    OX_Value s;   /**< String.*/
    OX_Value pat; /**< Match pattern.*/
    OX_Value mr;  /**< Match result.*/
    size_t   pos; /**< Current position.*/
} OX_MatchIter;

/*Scan referenced objects in the match iterator.*/
static void
match_iter_scan (OX_Context *ctxt, void *p)
{
    OX_MatchIter *mi = p;

    ox_gc_scan_value(ctxt, &mi->s);
    ox_gc_scan_value(ctxt, &mi->pat);
    ox_gc_scan_value(ctxt, &mi->mr);
}

/*Free the match iterator.*/
static void
match_iter_free (OX_Context *ctxt, void *p)
{
    OX_MatchIter *mi = p;

    OX_DEL(ctxt, mi);
}

/*Operation functions of match iterator.*/
static const OX_PrivateOps
match_iter_ops = {
    match_iter_scan,
    match_iter_free
};

/*Create a new match iterator.*/
static OX_MatchIter*
match_iter_new (OX_Context *ctxt, OX_Value *v, OX_Value *s_arg, OX_Value *pat_arg, size_t pos, OX_Value *inf)
{
    OX_VS_PUSH_2(ctxt, s, pat)
    OX_MatchIter *mi = NULL;
    OX_Result r;

    if ((r = ox_to_string(ctxt, s_arg, s)) == OX_ERR)
        goto end;

    if (ox_value_is_re(ctxt, pat_arg)) {
        ox_value_copy(ctxt, pat, pat_arg);
    } else {
        if ((r = ox_to_string(ctxt, pat_arg, pat)) == OX_ERR)
            goto end;
    }

    if ((r = ox_object_new(ctxt, v, inf)) == OX_ERR)
        goto end;

    if (!OX_NEW(ctxt, mi)) {
        ox_throw_no_mem_error(ctxt);
        goto end;
    }

    ox_value_copy(ctxt, &mi->s, s);
    ox_value_copy(ctxt, &mi->pat, pat);
    ox_value_set_null(ctxt, &mi->mr);
    mi->pos = pos;

    if ((r = ox_object_set_priv(ctxt, v, &match_iter_ops, mi)) == OX_ERR) {
        match_iter_free(ctxt, mi);
        mi = NULL;
        goto end;
    }

    if ((r = ox_string_match(ctxt, s, pat, pos, &mi->mr)) == OX_ERR)
        mi = NULL;
end:
    OX_VS_POP(ctxt, s)
    return mi;
}

/*Get the match iterator from the value.*/
static OX_MatchIter*
match_iter_get (OX_Context *ctxt, OX_Value *v)
{
    OX_MatchIter *mi = ox_object_get_priv(ctxt, v, &match_iter_ops);

    if (!mi)
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a match iterator"));

    return mi;
}

/*String.$inf.match_iter*/
static OX_Result
String_inf_match_iter (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *pat = ox_argument(ctxt, args, argc, 0);
    OX_Value *pos_arg = ox_argument(ctxt, args, argc, 1);
    OX_VS_PUSH(ctxt, s)
    size_t pos = 0;
    OX_Result r;

    if ((r = ox_to_string(ctxt, thiz, s)) == OX_ERR)
        goto end;

    if (!ox_value_is_null(ctxt, pos_arg)) {
        ssize_t p;

        if ((r = ox_to_ssize(ctxt, pos_arg, &p)) == OX_ERR)
            goto end;

        if (p < 0) {
            p = ox_string_length(ctxt, s) + p;
            if (p < 0)
                p = 0;
        }

        pos = p;
    }

    if (!match_iter_new(ctxt, rv, s, pat, pos, OX_OBJECT(ctxt, MatchIterator_inf)))
        return OX_ERR;

    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*String.$inf.split*/
static OX_Result
String_inf_split (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *pat = ox_argument(ctxt, args, argc, 0);

    if (!(match_iter_new(ctxt, rv, thiz, pat, 0, OX_OBJECT(ctxt, SplitIterator_inf))))
        return OX_ERR;

    return OX_OK;
}

/*String.$inf.slice*/
static OX_Result
String_inf_slice (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *start_arg = ox_argument(ctxt, args, argc, 0);
    OX_Value *end_arg = ox_argument(ctxt, args, argc, 1);
    size_t start = 0, end = 0, len;
    OX_VS_PUSH(ctxt, s)
    OX_Result r;

    if ((r = ox_to_string(ctxt, thiz, s)) == OX_ERR)
        goto end;

    len = ox_string_length(ctxt, s);

    if (!ox_value_is_null(ctxt, start_arg)) {
        ssize_t p;

        if ((r = ox_to_ssize(ctxt, start_arg, &p)) == OX_ERR)
            goto end;

        if (p < 0) {
            p = len + p;
            if (p < 0)
                p = 0;
        }

        start = p;
    }

    if (!ox_value_is_null(ctxt, end_arg)) {
        ssize_t p;

        if ((r = ox_to_ssize(ctxt, end_arg, &p)) == OX_ERR)
            goto end;

        if (p < 0) {
            p = len + p;
            if (p < 0)
                p = 0;
        }
        
        end = p;
    } else {
        end = len;
    }

    if (start >= len)
        start = len;
    if (end >= len)
        end = len;
    if (end < start)
        end = start;

    r = ox_string_substr(ctxt, s, start, end - start, rv);
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*String.$inf.char_at*/
static OX_Result
String_inf_char_at (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *pos_arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, s)
    size_t pos = 0;
    const char *c;
    size_t len;
    OX_Result r;

    if ((r = ox_to_string(ctxt, thiz, s)) == OX_ERR)
        goto end;

    len = ox_string_length(ctxt, s);

    if (!ox_value_is_null(ctxt, pos_arg)) {
        ssize_t p;

        if ((r = ox_to_ssize(ctxt, pos_arg, &p)) == OX_ERR)
            goto end;

        if (p < 0) {
            p = len + p;
            if (p < 0) {
                ox_value_set_null(ctxt, rv);
                r = OX_OK;
                goto end;
            }
        }

        pos = p;
    }

    if (pos >= len) {
        ox_value_set_null(ctxt, rv);
    } else {
        c = ox_string_get_char_star(ctxt, s);
        ox_value_set_number(ctxt, rv, c[pos]);
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*String.$inf.to_lower*/
static OX_Result
String_inf_to_lower (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, s)
    size_t len;
    OX_String *sp;
    OX_Result r;

    if ((r = ox_to_string(ctxt, thiz, s)) == OX_ERR)
        goto end;

    len = ox_string_length(ctxt, s);
    if (!(sp = str_alloc(ctxt, rv, len))) {
        r = OX_ERR;
        goto end;
    }

    if (len) {
        const char *src = ox_string_get_char_star(ctxt, s);
        char *dst = sp->chars;
        size_t left = len;

        while (left --) {
            *dst ++ = ox_char_to_lower(*src ++);
        }
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*String.$inf.to_upper*/
static OX_Result
String_inf_to_upper (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH(ctxt, s)
    size_t len;
    OX_String *sp;
    OX_Result r;

    if ((r = ox_to_string(ctxt, thiz, s)) == OX_ERR)
        goto end;

    len = ox_string_length(ctxt, s);
    if (!(sp = str_alloc(ctxt, rv, len))) {
        r = OX_ERR;
        goto end;
    }

    if (len) {
        const char *src = ox_string_get_char_star(ctxt, s);
        char *dst = sp->chars;
        size_t left = len;

        while (left --) {
            *dst ++ = ox_char_to_upper(*src ++);
        }
    }

    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*String.$inf.lookup_char*/
static OX_Result
String_inf_lookup_char (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *carg = ox_argument(ctxt, args, argc, 0);
    OX_Value *parg = ox_argument(ctxt, args, argc, 1);
    int32_t c;
    OX_VS_PUSH(ctxt, s)
    size_t len, pos = 0;
    ssize_t off = -1;
    const char *pc;
    OX_Result r;

    if ((r = ox_to_string(ctxt, thiz, s)) == OX_ERR)
        goto end;

    if ((r = ox_to_int32(ctxt, carg, &c)) == OX_ERR)
        goto end;

    len = ox_string_length(ctxt, s);

    if (argc > 1) {
        ssize_t p;

        if ((r = ox_to_ssize(ctxt, parg, &p)) == OX_ERR)
            goto end;

        if (p < 0) {
            p = len + p;
            if (p < 0)
                p = 0;
        }

        pos = p;
    }

    pc = ox_string_get_char_star(ctxt, s);

    while (pos < len) {
        if (pc[pos] == c) {
            off = pos;
            break;
        }

        pos ++;
    }

    ox_value_set_number(ctxt, rv, off);
    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*String.$inf.lookup_char_r*/
static OX_Result
String_inf_lookup_char_r (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *carg = ox_argument(ctxt, args, argc, 0);
    OX_Value *parg = ox_argument(ctxt, args, argc, 1);
    int32_t c;
    OX_VS_PUSH(ctxt, s)
    ssize_t pos, len, off = -1;
    const char *pc;
    OX_Result r;

    if ((r = ox_to_string(ctxt, thiz, s)) == OX_ERR)
        goto end;

    if ((r = ox_to_int32(ctxt, carg, &c)) == OX_ERR)
        goto end;

    len = ox_string_length(ctxt, s);

    if (argc > 1) {
        ssize_t p;

        if ((r = ox_to_ssize(ctxt, parg, &p)) == OX_ERR)
            goto end;

        if (p < 0) {
            p = len + p;
            if (p < 0)
                p = -1;
        }

        pos = p;
    } else {
        pos = len - 1;
    }

    pos = OX_MIN(pos, len - 1);

    pc = ox_string_get_char_star(ctxt, s);

    while (pos >= 0) {
        if (pc[pos] == c) {
            off = pos;
            break;
        }

        pos --;
    }

    ox_value_set_number(ctxt, rv, off);
    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*String.$inf.compare*/
static OX_Result
String_inf_compare (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *sarg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH_2(ctxt, s1, s2)
    int n;
    OX_Result r;

    if ((r = ox_to_string(ctxt, thiz, s1)) == OX_ERR)
        goto end;

    if ((r = ox_to_string(ctxt, sarg, s2)) == OX_ERR)
        goto end;

    n = ox_string_compare(ctxt, s1, s2);
    ox_value_set_number(ctxt, rv, n);
    r = OX_OK;
end:
    OX_VS_POP(ctxt, s1);
    return r;
}

/*String.$inf.to_int8_ptr*/
static OX_Result
String_inf_to_int8_ptr (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH_2(ctxt, s, ty)
    OX_Value *ity;
    size_t len;
    OX_CValueInfo cvi;
    OX_Result r;

    if ((r = ox_to_string(ctxt, thiz, s)) == OX_ERR)
        goto end;

    len = ox_string_length(ctxt, s);

    ity = ox_ctype_get(ctxt, OX_CTYPE_I8);
    if ((r = ox_ctype_pointer(ctxt, ty, ity, len)) == OX_ERR)
        goto end;

    cvi.v.p = (void*)ox_string_get_char_star(ctxt, s);
    cvi.base = s;

    r = ox_cvalue_new(ctxt, rv, ty, &cvi);
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*String.$inf.to_uint8_ptr*/
static OX_Result
String_inf_to_uint8_ptr (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_VS_PUSH_2(ctxt, s, ty)
    OX_Value *ity;
    size_t len;
    OX_CValueInfo cvi;
    OX_Result r;

    if ((r = ox_to_string(ctxt, thiz, s)) == OX_ERR)
        goto end;

    len = ox_string_length(ctxt, s);

    ity = ox_ctype_get(ctxt, OX_CTYPE_U8);
    if ((r = ox_ctype_pointer(ctxt, ty, ity, len)) == OX_ERR)
        goto end;

    cvi.v.p = (void*)ox_string_get_char_star(ctxt, s);
    cvi.base = s;

    r = ox_cvalue_new(ctxt, rv, ty, &cvi);
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*StringIterator.$inf.next*/
static OX_Result
StringIterator_inf_next (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_StringIter *si;
    size_t len;

    if (!(si = string_iter_get(ctxt, thiz)))
        return OX_ERR;

    len = ox_string_length(ctxt, &si->s);

    if (si->type == OX_STRING_ITER_UCHAR) {
        const char *c = ox_string_get_char_star(ctxt, &si->s);
        size_t left = len - si->pos;
        int uc;

        uc = ox_uc_from_utf8(c + si->pos, &left);
        if (uc == -1) {
            ox_throw_range_error(ctxt, OX_TEXT("illegal unicode character"));
            return OX_ERR;
        }

        si->pos = len - left;
    } else {
        if (si->pos < len)
            si->pos ++;
    }

    return OX_OK;
}

/*StringIterator.$inf.end get*/
static OX_Result
StringIterator_inf_end_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_StringIter *si;
    size_t len;
    OX_Bool b;

    if (!(si = string_iter_get(ctxt, thiz)))
        return OX_ERR;

    len = ox_string_length(ctxt, &si->s);

    b = si->pos >= len;
    ox_value_set_bool(ctxt, rv, b);

    return OX_OK;
}

/*StringIterator.$inf.value get*/
static OX_Result
StringIterator_inf_value_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_StringIter *si;
    size_t len;
    OX_Result r = OX_OK;

    if (!(si = string_iter_get(ctxt, thiz)))
        return OX_ERR;

    len = ox_string_length(ctxt, &si->s);

    if (si->pos < len) {
        const char *c = ox_string_get_char_star(ctxt, &si->s);

        switch (si->type) {
        case OX_STRING_ITER_STR:
            r = ox_string_from_chars(ctxt, rv, c + si->pos, 1);
            break;
        case OX_STRING_ITER_CHAR:
            ox_value_set_number(ctxt, rv, c[si->pos]);
            break;
        case OX_STRING_ITER_UCHAR: {
            const char *c = ox_string_get_char_star(ctxt, &si->s);
            size_t left = len - si->pos;
            int uc;

            uc = ox_uc_from_utf8(c + si->pos, &left);
            if (uc == -1) {
                ox_throw_range_error(ctxt, OX_TEXT("illegal unicode character"));
                return OX_ERR;
            }

            ox_value_set_number(ctxt, rv, uc);
            break;
        }
        }
    } else {
        ox_value_set_null(ctxt, rv);
    }

    return r;
}

/*MatchIterator.$inf.end get*/
static OX_Result
MatchIterator_inf_end_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_MatchIter *mi;
    OX_Bool b;

    if (!(mi = match_iter_get(ctxt, thiz)))
        return OX_ERR;

    b = ox_value_is_null(ctxt, &mi->mr);
    ox_value_set_bool(ctxt, rv, b);
    return OX_OK;
}

/*MatchIterator.$inf.value get*/
static OX_Result
MatchIterator_inf_value_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_MatchIter *mi;

    if (!(mi = match_iter_get(ctxt, thiz)))
        return OX_ERR;

    ox_value_copy(ctxt, rv, &mi->mr);
    return OX_OK;
}

/*MatchIterator.$inf.next*/
static OX_Result
MatchIterator_inf_next (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_MatchIter *mi;
    OX_Match *m;
    OX_Result r;

    if (!(mi = match_iter_get(ctxt, thiz)))
        return OX_ERR;

    if (!ox_value_is_null(ctxt, &mi->mr)) {
        m = ox_match_get_data(ctxt, &mi->mr);

        if (m->end == mi->pos)
            mi->pos ++;
        else
            mi->pos = m->end;

        if ((r = ox_string_match(ctxt, &mi->s, &mi->pat, mi->pos, &mi->mr)) == OX_ERR)
            return r;
    }

    return OX_OK;
}

/*SplitIterator.$inf.end get*/
static OX_Result
SplitIterator_inf_end_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_MatchIter *mi;
    OX_Bool b;

    if (!(mi = match_iter_get(ctxt, thiz)))
        return OX_ERR;

    b = ox_value_is_null(ctxt, &mi->mr) && (mi->pos > ox_string_length(ctxt, &mi->s));
    ox_value_set_bool(ctxt, rv, b);
    return OX_OK;
}

/*SplitIterator.$inf.value get*/
static OX_Result
SplitIterator_inf_value_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_MatchIter *mi;
    OX_Match *m;
    OX_Result r;

    if (!(mi = match_iter_get(ctxt, thiz)))
        return OX_ERR;

    if (!ox_value_is_null(ctxt, &mi->mr)) {
        m = ox_match_get_data(ctxt, &mi->mr);
        r = ox_string_substr(ctxt, &mi->s, mi->pos, m->start - mi->pos, rv);
    } else {
        size_t len = ox_string_length(ctxt, &mi->s);

        if (mi->pos <= len) {
            r = ox_string_substr(ctxt, &mi->s, mi->pos, len - mi->pos, rv);
        } else {
            ox_value_set_null(ctxt, rv);
            r = OX_OK;
        }
    }

    return r;
}

/*SplitIterator.$inf.next*/
static OX_Result
SplitIterator_inf_next (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_MatchIter *mi;
    OX_Match *m;
    OX_Result r;

    if (!(mi = match_iter_get(ctxt, thiz)))
        return OX_ERR;

    if (!ox_value_is_null(ctxt, &mi->mr)) {
        m = ox_match_get_data(ctxt, &mi->mr);

        if (m->end == mi->pos)
            mi->pos ++;
        else
            mi->pos = m->end;

        if ((r = ox_string_match(ctxt, &mi->s, &mi->pat, mi->pos, &mi->mr)) == OX_ERR)
            return r;
    } else {
        size_t len = ox_string_length(ctxt, &mi->s);

        if (mi->pos <= len)
            mi->pos = len + 1;
    }

    return OX_OK;
}

/** String builder.*/
typedef struct {
    OX_CharBuffer cb; /**< Character buffer.*/
} OX_StringBuilder;

/*Free the stirng builder.*/
static void
sb_free (OX_Context *ctxt, void *p)
{
    OX_StringBuilder *sb = p;

    ox_char_buffer_deinit(ctxt, &sb->cb);
    OX_DEL(ctxt, sb);
}

/*Operation functions of string builder.*/
static const OX_PrivateOps
sb_ops = {
    NULL,
    sb_free
};

/*Get the string builder from the value.*/
static OX_StringBuilder*
sb_get (OX_Context *ctxt, OX_Value *v)
{
    OX_StringBuilder *sb = ox_object_get_priv(ctxt, v, &sb_ops);

    if (!sb)
        ox_throw_type_error(ctxt, OX_TEXT("the value is not a string builder"));

    return sb;
}

/*StringBuilder.$inf.$init*/
static OX_Result
StringBuilder_inf_init (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_StringBuilder *sb;
    OX_Result r;

    if (!OX_NEW(ctxt, sb))
        return ox_throw_no_mem_error(ctxt);

    ox_char_buffer_init(&sb->cb);

    if ((r = ox_object_set_priv(ctxt, thiz, &sb_ops, sb)) == OX_ERR) {
        sb_free(ctxt, sb);
        return r;
    }

    return OX_OK;
}

/*StringBuilder.$inf.append*/
static OX_Result
StringBuilder_inf_append (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_VS_PUSH(ctxt, s)
    OX_StringBuilder *sb;
    OX_Result r;

    if (!(sb = sb_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_to_string(ctxt, arg, s)) == OX_ERR)
        goto end;

    if ((r = ox_char_buffer_append_string(ctxt, &sb->cb, s)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    OX_VS_POP(ctxt, s)
    return r;
}

/*StringBuilder.$inf.append_char*/
static OX_Result
StringBuilder_inf_append_char (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    int32_t c;
    OX_StringBuilder *sb;
    OX_Result r;

    if (!(sb = sb_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_to_int32(ctxt, arg, &c)) == OX_ERR)
        goto end;

    if ((r = ox_char_buffer_append_char(ctxt, &sb->cb, c)) == OX_ERR)
        goto end;

    r = OX_OK;
end:
    return r;
}

/*StringBuilder.$inf.$to_str*/
static OX_Result
StringBuilder_inf_to_str (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_StringBuilder *sb;
    OX_Result r;

    if (!(sb = sb_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    r = ox_char_buffer_get_string(ctxt, &sb->cb, rv);
end:
    return r;
}

/*StringBuilder.$inf.length get*/
static OX_Result
StringBuilder_inf_length_get (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_StringBuilder *sb;
    OX_Result r;

    if (!(sb = sb_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    ox_value_set_number(ctxt, rv, sb->cb.len);
    r = OX_OK;
end:
    return r;
}

/*StringBuilder.$inf.length set*/
static OX_Result
StringBuilder_inf_length_set (OX_Context *ctxt, OX_Value *f, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);
    OX_StringBuilder *sb;
    size_t len = 0;
    OX_Result r;

    if (!(sb = sb_get(ctxt, thiz))) {
        r = OX_ERR;
        goto end;
    }

    if ((r = ox_to_index(ctxt, arg, &len)) == OX_ERR)
        goto end;

    if (len < sb->cb.len)
        sb->cb.len = len;

    r = OX_OK;
end:
    return r;
}

/*?
 *? @lib {String} String class.
 *?
 *? @callback ReplaceFunc String replace callback function.
 *? @param match {Match} The match result,
 *? @return {String} The replace string.
 *?
 *? @class{ String String.
 *? OX string is encoded in UTF-8.
 *?
 *? @sfunc from_char Create a string from a character.
 *? @param c {Number} The character.
 *? @return {String} The result string.
 *?
 *? @sfunc from_uchar Create a string from an unicode character.
 *? @param uc {Number} The unicode character.
 *? @return {String} The result string.
 *? @throw {RangeError} The argument is not an valid unicode character.
 *?
 *? @sfunc from_file Load the string from a text file.
 *? The text file must be encoded in UTF-8.
 *? @param filename {String} The filename of the input file.
 *? @return {String} The result string.
 *? @throw {SystemError} File operate failed.
 *?
 *? @roacc length {Number} The characters length of the string.
 *?
 *? @func $iter Create an iterator to traverse all the characters in the string.
 *? The value of the iterator is a 1 character length string contains each character of the string.
 *? @return {Iterator[String]} The iterator used to traverse the characters.
 *?
 *? @func chars Create an iterator to traverse all the characters in the string.
 *? The value of the iterator is the character code value.
 *? @return {Iterator[Number]} The iterator used to traverse the characters.
 *?
 *? @func uchars Create an iterator to traverse all the unicode characters in the string.
 *? The value of the iterator is the unicode character code value.
 *? @return {Iterator[Number]} The iterator used to traverse the unicode characters.
 *?
 *? @func $to_num Convert the string to number.
 *? The number string can has leading and trailing space characters.
 *? The number string syntax is same as number literal syntax of OX.
 *?
 *? @func trim Trim the leading and trailing space characters of the string.
 *? @return {String} The result string.
 *?
 *? @func trim_h Trim the leading space characters at the head of the string.
 *? @return {String} The result string.
 *?
 *? @func trim_t Trim the trailing space characters at the tail of the string.
 *? @return {String} The result string.
 *?
 *? @func pad_h Pad pattern string at the head of the string.
 *? @param width {Number} The reault string's length.
 *? If width <= the length of the string, directly return the current string.
 *? Then the pattern string will be added repeatly.
 *? @param pat {String} ="" The pattern string.
 *? @return {String} The result string.
 *?
 *? @func pad_t Pad pattern string at the tail of the string.
 *? @param width {Number} The reault string's length.
 *? If width <= the length of the string, directly return the current string.
 *? Then the pattern string will be added repeatly.
 *? @param pat {String} ="" The pattern string.
 *? @return {String} The result string.
 *?
 *? @func match Match the string with a pattern.
 *? @param pat {String|Re} The match pattern.
 *? @ul{
 *? @li If pat is a regular expression, match the regular expression in the string.
 *? @li If pat is a string, search for this substring.
 *? @ul}
 *? @param pos {Number} =0 Start position to match the pattern.
 *? If pos < 0, the start position is the string's length + pos.
 *? @return {?Match} The match rsult.
 *? If cannot match any substring, return null.
 *?
 *? @func replace Replace the substrings.
 *? @param pat {String|Re} The substring match pattern.
 *? @param rep {String|ReplaceFunc} Replace description string.
 *? @ul{
 *? @li If rep is a function, it will convert the match result to the reaplce string.
 *? @li If rep is a string, it is the replace string.\
 *? Character '$' is used for special replace command.\
 *? 'u' or 'l' flag can be used after '$'.\
 *? 'u' means convert the string to uppercase.\
 *? 'l' means convert the string to lowercase.
 *? @ul{
 *? @li "$$" means replace with character '$'.
 *? @li "$&" means replace with the matched substring.
 *? @li "$N" (N is 0~9) means replace with sub-slice in the matched substring.
 *? @li "$`" means replace with the substring before the matched substring.
 *? @li "$'" means replace with the substring after the matched substring.
 *? @ul}
 *? @ul}
 *? @param pos {Number} =0 Start position to the match the substrings.
 *? If pos < 0, the start position is the string's length + pos.
 *? @param once {Bool} =false Only replace one substring or not.
 *? @return The result string.
 *?
 *? @func match_iter Create an iterator to terverse the matched result of the string.
 *? @param pat {String|Re} The match pattern.
 *? @ul{
 *? @li If pat is a regular expression, match the regular expression in the string.
 *? @li If pat is a string, search for this substring.
 *? @ul}
 *? @param pos {Number} =0 Start position to match the pattern.
 *? If pos < 0, the start position is the string's length + pos.
 *? @return {Iterator[Match]} The iterator used to traverse the match results.
 *?
 *? @func split Split the string into slices.
 *? @param sep {String|Re} The separator pattern.
 *? @return {Iterator[String]} The iterator used to traverse the separated slices.
 *?
 *? @func slice Get the slice of the string.
 *? @param begin {Number} The begin position of the slice.
 *? If begin < 0, the begin position is the string's length + begin.
 *? @param end {Number} =this.length The end position of the slice.
 *? If end < 0, the end position is the string's length + end.
 *? The end position is the last character's position of the slice plus 1.
 *? @return {String} The slice string.
 *?
 *? @func char_at Get the character at the position.
 *? @param pos {Number} The position of the character.
 *? If pos < 0, the position is the string's length + pos.
 *? @return {?Number} The character's code number.
 *? If the position is < 0 or >= length of the string, return null.
 *?
 *? @func to_upper Convert the string to uppercase.
 *? @return {String} The uppercase string.
 *?
 *? @func to_lower Convert the string to lowercase.
 *? @return {String} The lowercase string.
 *?
 *? @func lookup_char Lookup a character in the string.
 *? @param c {Number} The character's code number.
 *? @param pos {Number} =0 Position to start searching.
 *? If pos < 0, the start position is the string's length + pos.
 *? @return {Number} The character's position.
 *? If cannot find the character, return -1.
 *?
 *? @func lookup_char_r Lookup a character in the string revresely.
 *? @param c {Number} The character's code number.
 *? @param pos {Number} =0 Position to start searching.
 *? If pos < 0, the start position is the string's length + pos.
 *? @return {Number} The character's position.
 *? If cannot find the character, return -1.
 *?
 *? @func compare Compare the string to another one.
 *? @param s The string to be compared to.
 *? @return {Number} The number compare result.
 *? @ul{
 *? @li 0 This string equal to s.
 *? @li <0 This string less than s.
 *? @li >0 This string greater than s.
 *? @ul}
 *?
 *? @func to_int8_ptr Get the Int8 pointer of the string.
 *? @return {C:Int8*} The Int8 pointer.
 *?
 *? @func to_uint8_ptr Get the UInt8 pointer of the string.
 *? @return {C:UInt8*} The Int8 pointer.
 *?
 *? @class{ Builder The string builder.
 *?
 *? @func $init Initialize the string builder.
 *?
 *? @func append_char Append a character to the string builder.
 *? @param c {Number} The character to be appended.
 *?
 *? @func append Append a string to the string builder.
 *? @param s {String} The substring to be appended.
 *?
 *? @func $to_str Create a string from the string builder.
 *? @return {String} The result string.
 *?
 *? @acc length {Number} The characters length in the string builder.
 *?
 *? @class}
 *?
 *? @class}
 */

/**
 * Initialize the string class.
 * @param ctxt The current running context.
 */
void
ox_string_class_init (OX_Context *ctxt)
{
    OX_VS_PUSH_3(ctxt, iter, b, binf)

    /*String*/
    ox_not_error(ox_named_class_new_s(ctxt, OX_OBJECT(ctxt, String), OX_OBJECT(ctxt, String_inf), NULL, "String"));
    ox_not_error(ox_object_add_const_s(ctxt, OX_OBJECT(ctxt, Global), "String", OX_OBJECT(ctxt, String)));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String), "from_char",
            String_from_char));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String), "from_uchar",
            String_from_uchar));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String), "from_file",
            String_from_file));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String), "is",
            String_is));

    /*String.Builder*/
    ox_not_error(ox_named_class_new_s(ctxt, b, binf, OX_OBJECT(ctxt, String), "Builder"));
    ox_not_error(ox_object_add_const_s(ctxt, OX_OBJECT(ctxt, String), "Builder", b));

    /*String.Builder.$inf*/
    ox_not_error(ox_object_add_n_method_s(ctxt, binf, "$init",
            StringBuilder_inf_init));
    ox_not_error(ox_object_add_n_method_s(ctxt, binf, "append",
            StringBuilder_inf_append));
    ox_not_error(ox_object_add_n_method_s(ctxt, binf, "append_char",
            StringBuilder_inf_append_char));
    ox_not_error(ox_object_add_n_method_s(ctxt, binf, "$to_str",
            StringBuilder_inf_to_str));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, binf, "length",
            StringBuilder_inf_length_get, StringBuilder_inf_length_set));

    /*String_inf*/
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, String_inf), "length",
            String_inf_length_get, NULL));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String_inf), "$iter",
            String_inf_iter));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String_inf), "chars",
            String_inf_chars));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String_inf), "uchars",
            String_inf_uchars));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String_inf), "$to_num",
            String_inf_to_num));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String_inf), "trim",
            String_inf_trim));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String_inf), "trim_h",
            String_inf_trim_h));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String_inf), "trim_t",
            String_inf_trim_t));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String_inf), "pad_h",
            String_inf_pad_h));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String_inf), "pad_t",
            String_inf_pad_t));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String_inf), "match",
            String_inf_match));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String_inf), "replace",
            String_inf_replace));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String_inf), "match_iter",
            String_inf_match_iter));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String_inf), "split",
            String_inf_split));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String_inf), "slice",
            String_inf_slice));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String_inf), "char_at",
            String_inf_char_at));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String_inf), "to_lower",
            String_inf_to_lower));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String_inf), "to_upper",
            String_inf_to_upper));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String_inf), "lookup_char",
            String_inf_lookup_char));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String_inf), "lookup_char_r",
            String_inf_lookup_char_r));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String_inf), "compare",
            String_inf_compare));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String_inf), "to_int8_ptr",
            String_inf_to_int8_ptr));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, String_inf), "to_uint8_ptr",
            String_inf_to_uint8_ptr));

    /*StringIterator*/
    ox_not_error(ox_named_class_new_s(ctxt, iter, OX_OBJECT(ctxt, StringIterator_inf),
            OX_OBJECT(ctxt, String), "Iterator"));
    ox_not_error(ox_class_inherit(ctxt, iter, OX_OBJECT(ctxt, Iterator)));
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, StringIterator_inf), "next",
            StringIterator_inf_next));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, StringIterator_inf), "end",
            StringIterator_inf_end_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, StringIterator_inf), "value",
            StringIterator_inf_value_get, NULL));

    /*MatchIterator.*/
    ox_not_error(ox_named_class_new_s(ctxt, iter, OX_OBJECT(ctxt, MatchIterator_inf),
            OX_OBJECT(ctxt, String), "MatchIterator"));
    ox_not_error(ox_class_inherit(ctxt, iter, OX_OBJECT(ctxt, Iterator)));
    /*MatchIterator_inf.*/
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, MatchIterator_inf), "next",
            MatchIterator_inf_next));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, MatchIterator_inf), "end",
            MatchIterator_inf_end_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, MatchIterator_inf), "value",
            MatchIterator_inf_value_get, NULL));

    /*SplitIterator.*/
    ox_not_error(ox_named_class_new_s(ctxt, iter, OX_OBJECT(ctxt, SplitIterator_inf),
            OX_OBJECT(ctxt, String), "SplitIterator"));
    ox_not_error(ox_class_inherit(ctxt, iter, OX_OBJECT(ctxt, Iterator)));
    /*SplitIterator_inf.*/
    ox_not_error(ox_object_add_n_method_s(ctxt, OX_OBJECT(ctxt, SplitIterator_inf), "next",
            SplitIterator_inf_next));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, SplitIterator_inf), "end",
            SplitIterator_inf_end_get, NULL));
    ox_not_error(ox_object_add_n_accessor_s(ctxt, OX_OBJECT(ctxt, SplitIterator_inf), "value",
            SplitIterator_inf_value_get, NULL));

    OX_VS_POP(ctxt, iter)
}

/**
 * Call the string class.
 * @param ctxt The current running context.
 * @param o The string class object.
 * @param thiz This argument.
 * @param args Arguments.
 * @param argc Arguments' count.
 * @param[out] rv Return value.
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_string_class_call (OX_Context *ctxt, OX_Value *o, OX_Value *thiz, OX_Value *args, size_t argc, OX_Value *rv)
{
    OX_Value *arg = ox_argument(ctxt, args, argc, 0);

    return ox_to_string(ctxt, arg, rv);
}
