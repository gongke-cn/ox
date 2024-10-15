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
 * Hash table.
 */

#define OX_LOG_TAG "ox_hash"

#include "ox_internal.h"

/*Add entry list to the new buffer.*/
static void
hash_ent_list_expand (OX_Context *ctxt, OX_Hash *hash, OX_HashEntry *el, OX_HashEntry **lb, size_t ll)
{
    OX_HashEntry *e, *ne, **p;
    size_t hc;

    for (e = el; e; e = ne) {
        ne = e->next;

        hc = hash->ops->key(ctxt, e->key);

        p = &lb[hc % ll];

        e->next = *p;
        *p = e;
    }
}

/*Expand the hash table.*/
static OX_Result
hash_expand (OX_Context *ctxt, OX_Hash *hash)
{
    OX_HashEntry **lb;
    size_t ll = hash->e_num;

    if (!OX_NEW_N_0(ctxt, lb, ll))
        return ox_throw_no_mem_error(ctxt);

    if (hash->l_num == 1) {
        hash_ent_list_expand(ctxt, hash, hash->e.list, lb, ll);
    } else {
        size_t i;

        for (i = 0; i < hash->l_num; i ++) {
            hash_ent_list_expand(ctxt, hash, hash->e.lists[i], lb, ll);
        }

        OX_DEL_N(ctxt, hash->e.lists, hash->l_num);
    }

    hash->e.lists = lb;
    hash->l_num = ll;

    return OX_OK;
}

/*Calculate the size_t type key code.*/
static size_t
size_hash_key (OX_Context *ctxt, void *k)
{
    return OX_PTR2SIZE(k);
}

/*Check if 2 size_t type keys are equal.*/
static OX_Bool
size_hash_equal (OX_Context *ctxt, void *k1, void *k2)
{
    return k1 == k2;
}

/*Hash table operation functions with size_t type key.*/
static const OX_HashOps
size_hash_ops = {
    size_hash_key,
    size_hash_equal
};

/**
 * Initialize a hash table use size_t type key.
 * @param hash The hash table to be initialized.
 */
void
ox_size_hash_init (OX_Hash *hash)
{
    return ox_hash_init(hash, &size_hash_ops);
}

/*Calculate the 0 terminated character string type key code.*/
static size_t
char_star_hash_key (OX_Context *ctxt, void *k)
{
    size_t v;
    char *c;

    if (!k)
        return 0;

    v = 0x19781009;
    c = k;

    while (*c) {
        v <<= 5;
        v |= *c;

        c ++;
    }

    return v;
}

/*Check if 2 0 terminated character strings are equal.*/
static OX_Bool
char_star_hash_equal (OX_Context *ctxt, void *k1, void *k2)
{
    return strcmp(k1, k2) == 0;
}

/*Hash table operation functions with 0 terminated character string type key.*/
static const OX_HashOps
char_star_hash_ops = {
    char_star_hash_key,
    char_star_hash_equal
};

/**
 *  Initialize a hash table use 0 terminatd character string type key.
 * @param hash The hash table to be initialized.
 */
void
ox_char_star_hash_init (OX_Hash *hash)
{
    return ox_hash_init(hash, &char_star_hash_ops);
}

/*Calculate the string type key code.*/
static size_t
string_hash_key (OX_Context *ctxt, void *k)
{
    OX_String *s = k;
    char *c;
    size_t v, left;

    if (s->len == 0)
        return 0;

    v = 0x19781009;
    c = s->chars;
    left = s->len;

    while (left --) {
        v <<= 5;
        v |= *c;

        c ++;
    }

    return v;
}

/*Check if 2 strings are equal.*/
static OX_Bool
string_hash_equal (OX_Context *ctxt, void *k1, void *k2)
{
    OX_String *s1 = k1;
    OX_String *s2 = k2;
    const char *c1, *c2;
    size_t l1, l2;

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

/*Hash table operation functions with string type key.*/
static const OX_HashOps
string_hash_ops = {
    string_hash_key,
    string_hash_equal
};

/**
 *  Initialize a hash table use string type key.
 * @param hash The hash table to be initialized.
 */
void
ox_string_hash_init (OX_Hash *hash)
{
    return ox_hash_init(hash, &string_hash_ops);
}

/*Calculate the value type key code.*/
static size_t
value_hash_key (OX_Context *ctxt, void *k)
{
    OX_Value *v = k;

    switch (ox_value_get_type(ctxt, v)) {
    case OX_VALUE_NULL:
        return 0;
    case OX_VALUE_BOOL:
        return ox_value_get_bool(ctxt, v);
    case OX_VALUE_NUMBER:
        return ox_value_get_number(ctxt, v);
    default:
        if (ox_value_is_string(ctxt, v))
            return string_hash_key(ctxt, ox_value_get_gco(ctxt, v));
        if (ox_value_is_cvalue(ctxt, v)) {
            OX_Value *cty = ox_cvalue_get_ctype(ctxt, v);
            OX_CTypeKind kind = ox_ctype_get_kind(ctxt, cty);

            if (kind == OX_CTYPE_VOID)
                return 0;
            if (kind & OX_CTYPE_FL_NUM) {
                size_t n;
                OX_Result r;

                if ((r = ox_to_size(ctxt, v, &n)) == OX_ERR)
                    return r;

                return n;
            }

            return OX_PTR2SIZE(ox_cvalue_get_pointer(ctxt, v));
        }

        return OX_PTR2SIZE(ox_value_get_gco(ctxt, v));
    }
}

/*Check if 2 values are equal.*/
static OX_Bool
value_hash_equal (OX_Context *ctxt, void *k1, void *k2)
{
    OX_Value *v1 = k1;
    OX_Value *v2 = k2;

    return ox_equal(ctxt, v1, v2);
}

/*Hash table operation functions with value type key.*/
static const OX_HashOps
value_hash_ops = {
    value_hash_key,
    value_hash_equal
};

/**
 *  Initialize a hash table use value type key.
 * @param hash The hash table to be initialized.
 */
void
ox_value_hash_init (OX_Hash *hash)
{
    return ox_hash_init(hash, &value_hash_ops);
}

/**
 * Release the hash table.
 * @param ctxt The current running context.
 * @param hash The hash table to be released.
 */
void
ox_hash_deinit (OX_Context *ctxt, OX_Hash *hash)
{
    assert(ctxt && hash);

    if (hash->l_num > 1) {
        if (hash->e.lists)
            OX_DEL_N(ctxt, hash->e.lists, hash->l_num);
    }
}

/**
 * Lookup an entry in the hash table by its key.
 * @param ctxt The current running context.
 * @param hash The hash table.
 * @param k The key of entry.
 * @param[out] pe Return the position where store the entry's pointer.
 * @return The entry with the key.
 * @retval NULL Cannot find the entry in the hash table.
 */
OX_HashEntry*
ox_hash_lookup (OX_Context *ctxt, OX_Hash *hash, void *k, OX_HashEntry ***pe)
{
    OX_HashEntry **p, *e;

    assert(ctxt && hash);

    if (hash->l_num > 1) {
        size_t hc = hash->ops->key(ctxt, k);

        p = &hash->e.lists[hc % hash->l_num];
    } else {
        p = &hash->e.list;
    }

    while ((e = *p)) {
        if (hash->ops->equal(ctxt, e->key, k))
            break;

        p = &e->next;
    }

    if (pe)
        *pe = p;

    return e;
}

/**
 * Insert a new entry to the hash table.
 * @param ctxt The current running context.
 * @param hash The hash table.
 * @param k The key of entry.
 * @param e The new entry.
 * @param pe If it is not NULL, it is the position where store the new entry.
 * It is returned from output parameter "pe" of function "ox_hash_lookup".
 * @retval OX_OK On success.
 * @retval OX_ERR On error.
 */
OX_Result
ox_hash_insert (OX_Context *ctxt, OX_Hash *hash, void *k, OX_HashEntry *e, OX_HashEntry **pe)
{
    assert(ctxt && hash && e);

    if (!pe) {
        OX_HashEntry *oe;

        oe = ox_hash_lookup(ctxt, hash, k, &pe);

        assert(oe == NULL);
    }

    e->key = k;
    e->next = *pe;
    *pe = e;

    hash->e_num ++;

    if ((hash->e_num > 8) && (hash->e_num > hash->l_num * 3)) {
        OX_Result r;

        if ((r = hash_expand(ctxt, hash)) == OX_ERR)
            return r;
    }

    return OX_OK;
}

/**
 * Remove an entry from the hash table.
 * @param ctxt The current running context.
 * @param hash The hash table.
 * @param k The key of entry.
 * @param pe If it is not NULL, it is the position where store the entry.
 * It is returned from output parameter "pe" of function "ox_hash_lookup".
 * @return The removed entry.
 */
OX_HashEntry*
ox_hash_remove (OX_Context *ctxt, OX_Hash *hash, void *k, OX_HashEntry **pe)
{
    OX_HashEntry *e;

    assert(ctxt && hash);

    if (!pe) {
        e = ox_hash_lookup(ctxt, hash, k, &pe);

        assert(e);
    } else {
        e = *pe;
    }

    *pe = e->next;

    hash->e_num --;

    return e;
}
