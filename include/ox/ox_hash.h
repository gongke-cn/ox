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

#ifndef _OX_HASH_H_
#define _OX_HASH_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get the entries number in the hash table.
 * @param hash The hash table.
 * @return The number of the entries.
 */
static inline size_t
ox_hash_size (OX_Hash *hash)
{
    return hash->e_num;
}

/**
 * Initialize the hash table.
 * @param hash The hash table to be initialized.
 * @param ops The operation functions.
 */
static inline void
ox_hash_init (OX_Hash *hash, const OX_HashOps *ops)
{
    hash->ops = ops;
    hash->e.list = NULL;
    hash->e_num = 0;
    hash->l_num = 1;
}

/**
 * Initialize a hash table use size_t type key.
 * @param hash The hash table to be initialized.
 */
extern void
ox_size_hash_init (OX_Hash *hash);

/**
 *  Initialize a hash table use 0 terminatd character string type key.
 * @param hash The hash table to be initialized.
 */
extern void
ox_char_star_hash_init (OX_Hash *hash);

/**
 *  Initialize a hash table use string type key.
 * @param hash The hash table to be initialized.
 */
extern void
ox_string_hash_init (OX_Hash *hash);

/**
 *  Initialize a hash table use value type key.
 * @param hash The hash table to be initialized.
 */
extern void
ox_value_hash_init (OX_Hash *hash);

/**
 * Release the hash table.
 * @param ctxt The current running context.
 * @param hash The hash table to be released.
 */
extern void
ox_hash_deinit (OX_Context *ctxt, OX_Hash *hash);

/**
 * Lookup an entry in the hash table by its key.
 * @param ctxt The current running context.
 * @param hash The hash table.
 * @param k The key of entry.
 * @param[out] pe Return the position where store the entry's pointer.
 * @return The entry with the key.
 * @retval NULL Cannot find the entry in the hash table.
 */
extern OX_HashEntry*
ox_hash_lookup (OX_Context *ctxt, OX_Hash *hash, void *k, OX_HashEntry ***pe);

/**
 * Lookup an entry in the hash table by its key with pointer offset.
 * It is used by "ox_hash_lookup_c".
 * @param ctxt The current running context.
 * @param hash The hash table.
 * @param k The key of entry.
 * @param[out] pe Return the position where store the entry's pointer.
 * @param off The pointer offset.
 * @return The entry with the key.
 * @retval NULL Cannot find the entry in the hash table.
 */
static inline void*
ox_hash_lookup_offset (OX_Context *ctxt, OX_Hash *hash, void *k, OX_HashEntry ***pe, size_t off)
{
    OX_HashEntry *e = ox_hash_lookup(ctxt, hash, k, pe);

    if (!e)
        return NULL;

    return OX_SIZE2PTR(OX_PTR2SIZE(e) - off);
}

/**
 * Lookup an entry's container pointer in the hash table by its key.
 * @param ctxt The current running context.
 * @param hash The hash table.
 * @param k The key of entry.
 * @param[out] pe Return the position where store the entry's pointer.
 * @param s The container structure type.
 * @param m The hash table entry member in the structure s.
 * @return The container pointer.
 * @retval NULL Cannot find the entry in the hash table.
 */
#define ox_hash_lookup_c(c, h, k, p, s, m)\
    ox_hash_lookup_offset(c, h, k, p, OX_OFFSET_OF(s, m))

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
extern OX_Result
ox_hash_insert (OX_Context *ctxt, OX_Hash *hash, void *k, OX_HashEntry *e, OX_HashEntry **pe);

/**
 * Remove an entry from the hash table.
 * @param ctxt The current running context.
 * @param hash The hash table.
 * @param k The key of entry.
 * @param pe If it is not NULL, it is the position where store the entry.
 * It is returned from output parameter "pe" of function "ox_hash_lookup".
 * @return The removed entry.
 */
extern OX_HashEntry*
ox_hash_remove (OX_Context *ctxt, OX_Hash *hash, void *k, OX_HashEntry **pe);

/**
 * Traverse the entries in the hash table.
 * @param h The hash table.
 * @param i The variable store the entry list index.
 * @param n The variable store each entry.
 */
#define ox_hash_foreach(h, i, n)\
    for ((i) = 0; (i) < (h)->l_num; (i)++)\
        for ((n) = (h)->l_num == 1 ? (h)->e.list : (h)->e.lists[i];\
                (n);\
                (n) = (n)->next)

/**
 * Traverse the entries in the hash table safely.
 * @param h The hash table.
 * @param i The variable store the entry list index.
 * @param n The variable store each entry.
 * @param t The temporary variable store the next entry.
 */
#define ox_hash_foreach_safe(h, i, n, t)\
    for ((i) = 0; (i) < (h)->l_num; (i)++)\
        for ((n) = (h)->l_num == 1 ? (h)->e.list : (h)->e.lists[i];\
                (n) ? ((t) = (n)->next, 1) : 0;\
                (n) = (t))

/**
 * Get the hash entry's container pointer.
 * @param s The container structure's type.
 * @param m The hash entry member of the structure type.
 */
#define ox_hash_entry_c(e, s, m)\
    ((e) ? OX_CONTAINER_OF(e, s, m) : NULL)

/**
 * Traverse the entries' container pointers in the hash table.
 * @param h The hash table.
 * @param i The variable store the entry list index.
 * @param c The variable store each entry's container pointer.
 * @param s The container structure's type.
 * @param m The hash entry member of the structure type.
 */
#define ox_hash_foreach_c(h, i, c, s, m)\
    for ((i) = 0; (i) < (h)->l_num; (i)++)\
        for ((c) = (h)->l_num == 1 ? ox_hash_entry_c((h)->e.list, s, m) : ox_hash_entry_c((h)->e.lists[i], s, m);\
                (c);\
                (c) = ox_hash_entry_c((c)->m.next, s, m))

/**
 * Traverse the entries' container pointers in the hash table safely.
 * @param h The hash table.
 * @param i The variable store the entry list index.
 * @param c The variable store each entry's container pointer.
 * @param t The temporary variable store the next entry's container pointer.
 * @param s The container structure's type.
 * @param m The hash entry member of the structure type.
 */
#define ox_hash_foreach_safe_c(h, i, c, t, s, m)\
    for ((i) = 0; (i) < (h)->l_num; (i)++)\
        for ((c) = (h)->l_num == 1 ? ox_hash_entry_c((h)->e.list, s, m) : ox_hash_entry_c((h)->e.lists[i], s, m);\
                (c) ? ((t) = ox_hash_entry_c((c)->m.next, s, m), 1) : 0;\
                (c) = (t))

#ifdef __cplusplus
}
#endif

#endif
