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
 * Double linked list.
 */

#ifndef _OX_LIST_H_
#define _OX_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

/** Declare and initialize a list.*/
#define OX_LIST_DECL(l) OX_List l = {&l, &l}

/**
 * Check if the list is empty.
 * @param l The list.
 * @retval OX_TRUE The list is empty.
 * @retval OX_FALSE The list is not empty.
 */
static inline int
ox_list_empty (OX_List *l)
{
    return l->next == l;
}

/**
 * Initialize a double linked list.
 * @param l The list to be intialized.
 */
static inline void
ox_list_init (OX_List *l)
{
    l->prev = l;
    l->next = l;
}

/**
 * Remove the node from the list.
 * @param n The list node to be removed.
 */
static inline void
ox_list_remove (OX_List *n)
{
    n->prev->next = n->next;
    n->next->prev = n->prev;
}

/**
 * Add a node to the tail of the list.
 * @param l The list.
 * @param n The node to be added.
 */
static inline void
ox_list_append (OX_List *l, OX_List *n)
{
    n->prev = l->prev;
    n->next = l;
    l->prev->next = n;
    l->prev = n;
}

/**
 * Add a node to the head of the list.
 * @param l The list.
 * @param n The node to be added.
 */
static inline void
ox_list_prepend (OX_List *l, OX_List *n)
{
    n->prev = l;
    n->next = l->next;
    l->next->prev = n;
    l->next = n;
}

/**
 * Get the head node of the list.
 * @param l The list.
 * @return The head node of the list.
 * @retval NULL The list is empty.
 */
static inline OX_List*
ox_list_head (OX_List *l)
{
    if (ox_list_empty(l))
        return NULL;

    return l->next;
}

/**
 * Get the tail node of the list.
 * @param l The list.
 * @return The tail node of the list.
 * @retval NULL The list is empty.
 */
static inline OX_List*
ox_list_tail (OX_List *l)
{
    if (ox_list_empty(l))
        return NULL;

    return l->prev;
}

/**
 * Get the next node of the list.
 * @param l The list.
 * @param n The current node.
 * @return The next node.
 * @retval NULL n is the last node of the list.
 */
static inline OX_List*
ox_list_next (OX_List *l, OX_List *n)
{
    return (n->next == l) ? NULL : n->next;
}

/**
 * Get the previous node of the list.
 * @param l The list.
 * @param n The current node.
 * @return The previous node.
 * @retval NULL n is the first node of the list.
 */
static inline OX_List*
ox_list_prev (OX_List *l, OX_List *n)
{
    return (n->prev == l) ? NULL : n->prev;
}

/**
 * Join 2 lists.
 * @param l1 List 1.
 * @param l2 list 2.
 */
static inline void
ox_list_join (OX_List *l1, OX_List *l2)
{
    if (!ox_list_empty(l2)) {
        OX_List *h1, *h2, *t1, *t2;

        h1 = l1;
        t1 = l1->prev;
        h2 = l2->next;
        t2 = l2->prev;

        h1->prev = t2;
        h2->prev = t1;
        t1->next = h2;
        t2->next = h1;
    }
}

/**
 * Get the head node's container of the list.
 * @param l The list.
 * @param s The container structure type.
 * @param m The list node member.
 * @return The head container pointer.
 * @retval NULL The list is empty.
 */
#define ox_list_head_c(l, s, m)\
    (ox_list_empty(l) ? NULL : OX_CONTAINER_OF((l)->next, s, m))

/**
 * Get the tail node's container of the list.
 * @param l The list.
 * @param s The container structure type.
 * @param m The list node member.
 * @return The tail container pointer.
 * @retval NULL The list is empty.
 */
#define ox_list_tail_c(l, s, m)\
    (ox_list_empty(l) ? NULL : OX_CONTAINER_OF((l)->prev, s, m))

/**
 * Get the next node's container.
 * @param l The list.
 * @param c The current node's container.
 * @param s The container structure type.
 * @param m The list node member.
 * @return The next node's container.
 * @retval NULL c is the last node's container.
 */
#define ox_list_next_c(l, c, s, m)\
    (((c)->m.next == (l)) ? NULL : OX_CONTAINER_OF((c)->m.next, s, m))

/**
 * Get the previous node's container.
 * @param l The list.
 * @param c The current node's container.
 * @param s The container structure type.
 * @param m The list node member.
 * @return The previous node's container.
 * @retval NULL c is the first node's container.
 */
#define ox_list_prev_c(l, c, s, m)\
    (((c)->m.prev == (l)) ? NULL : OX_CONTAINER_OF((c)->m.prev, s, m))

/**
 * Traverse the nodes in the list.
 * @param l The list.
 * @param n The variable store each node.
 */
#define ox_list_foreach(l, n)\
    for ((n) = (l)->next; (n) != (l); (n) = (n)->next)

/**
 * Traverse the nodes in the list reversely.
 * @param l The list.
 * @param n The variable store each node.
 */
#define ox_list_foreach_r(l, n)\
    for ((n) = (l)->prev; (n) != (l); (n) = (n)->prev)

/**
 * Traverse the nodes in the list safely.
 * Safe means the node can be removed.
 * @param l The list.
 * @param n The variable store each node.
 * @param t The temporary variable store the next node.
 */
#define ox_list_foreach_safe(l, n, t)\
    for ((n) = (l)->next; (n) != (l) ? ((t) = (n)->next) : 0; (n) = (t))

/**
 * Traverse the nodes in the list reversely and safely.
 * Safe means the node can be removed.
 * @param l The list.
 * @param n The variable store each node.
 * @param t The temporary variable store the previous node.
 */
#define ox_list_foreach_safe_r(l, n, t)\
    for ((n) = (l)->prev; (n) != (l) ? ((t) = (n)->prev) : 0; (n) = (t))

/**
 * Traverse each node's container pointer of the list.
 * @param l The list.
 * @param c The varaible store the container pointer.
 * @param s The container structure's type.
 * @param m The list node member of the structure s.
 */
#define ox_list_foreach_c(l, c, s, m)\
    for ((c) = OX_CONTAINER_OF((l)->next, s, m);\
            (c) != OX_CONTAINER_OF(l, s, m);\
            (c) = OX_CONTAINER_OF((c)->m.next, s, m))

/**
 * Traverse each node's container pointer of the list reversely.
 * @param l The list.
 * @param c The varaible store the container pointer.
 * @param s The container structure's type.
 * @param m The list node member of the structure s.
 */
#define ox_list_foreach_c_r(l, c, s, m)\
    for ((c) = OX_CONTAINER_OF((l)->prev, s, m);\
            (c) != OX_CONTAINER_OF(l, s, m);\
            (c) = OX_CONTAINER_OF((c)->m.prev, s, m))

/**
 * Traverse each node's container pointer of the list safely.
 * @param l The list.
 * @param c The varaible store the container pointer.
 * @param t The temporary variable store the next container pointer.
 * @param s The container structure's type.
 * @param m The list node member of the structure s.
 */
#define ox_list_foreach_safe_c(l, c, t, s, m)\
    for ((c) = OX_CONTAINER_OF((l)->next, s, m);\
            (c) != OX_CONTAINER_OF(l, s, m) ? ((t) = OX_CONTAINER_OF((c)->m.next, s, m)) : 0;\
            (c) = (t))

/**
 * Traverse each node's container pointer of the list reversely and safely.
 * @param l The list.
 * @param c The varaible store the container pointer.
 * @param t The temporary variable store the next container pointer.
 * @param s The container structure's type.
 * @param m The list node member of the structure s.
 */
#define ox_list_foreach_safe_c_r(l, c, t, s, m)\
    for ((c) = OX_CONTAINER_OF((l)->prev, s, m);\
            (c) != OX_CONTAINER_OF(l, s, m) ? ((t) = OX_CONTAINER_OF((c)->m.prev, s, m)) : 0;\
            (c) = (t))

#ifdef __cplusplus
}
#endif

#endif
