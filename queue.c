#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *li = malloc(sizeof(struct list_head));
    if (li) {
        INIT_LIST_HEAD(li);
        return li;
    } else {
        return NULL;
    }
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;

    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, l, list)
        q_release_element(entry);

    free(l);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *n = malloc(sizeof(element_t));
    if (!n)
        return false;

    n->value = malloc(strlen(s) + 1);
    if (!n->value) {
        free(n);
        return false;
    }

    memcpy(n->value, s, strlen(s));
    n->value[strlen(s)] = '\0';

    list_add(&n->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *n = malloc(sizeof(element_t));
    if (!n)
        return false;

    n->value = malloc(strlen(s) + 1);
    if (!n->value) {
        free(n);
        return false;
    }

    memcpy(n->value, s, strlen(s));
    n->value[strlen(s)] = '\0';

    list_add_tail(&n->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *n = list_first_entry(head, element_t, list);
    list_del(head->next);

    if (sp) {
        strncpy(sp, n->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return n;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *n = list_last_entry(head, element_t, list);
    list_del(head->prev);

    if (sp) {
        strncpy(sp, n->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return n;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;

    if (list_is_singular(head)) {
        element_t *n = q_remove_head(head, NULL, 0);
        q_release_element(n);
        return true;
    }

    struct list_head *oneStep = head->next;
    struct list_head *twoStep = head->next->next;

    while (twoStep != head && twoStep->next != head) {
        oneStep = oneStep->next;
        twoStep = twoStep->next->next;
    }

    element_t *n = list_entry(oneStep, element_t, list);
    list_del(oneStep);
    q_release_element(n);

    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head)
        return false;
    if (list_empty(head) || list_is_singular(head))
        return true;

    struct list_head *node;
    struct list_head *safe;
    bool dup = false;
    list_for_each_safe (node, safe, head) {
        while (safe != head &&
               strcmp(list_entry(node, element_t, list)->value,
                      list_entry(safe, element_t, list)->value) == 0) {
            dup = true;
            safe = safe->next;
            element_t *n = list_entry(safe->prev, element_t, list);
            list_del(safe->prev);
            q_release_element(n);
        }

        if (dup) {
            element_t *n = list_entry(node, element_t, list);
            list_del(node);
            q_release_element(n);
            dup = false;
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *first, *second;
    for (first = head->next, second = first->next;
         first != head && second != head;
         first = first->next, second = first->next) {
        list_del(first);
        first->prev = second;
        first->next = second->next;
        second->next->prev = first;
        second->next = first;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head)
        list_move(node, head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head) || list_is_singular(head) || k < 2)
        return;

    struct list_head reversed;
    struct list_head tmp;
    struct list_head *node;
    INIT_LIST_HEAD(&reversed);
    INIT_LIST_HEAD(&tmp);
    for (int a = q_size(head) / k; a > 0; a--) {
        node = head;
        for (int i = 0; i < k; i++) {
            node = node->next;
        }
        list_cut_position(&tmp, head, node);
        q_reverse(&tmp);
        list_splice_tail_init(&tmp, &reversed);
    }
    list_splice_tail_init(head, &reversed);
    list_splice_tail_init(&reversed, head);
}

void mergeTwoLists(struct list_head *L1,
                   struct list_head *L2,
                   struct list_head *tmp,
                   bool descend)
{
    while (!list_empty(L1) && !list_empty(L2)) {
        if (descend) {
            if (strcmp(list_entry(L1->next, element_t, list)->value,
                       list_entry(L2->next, element_t, list)->value) > 0) {
                list_move_tail(L1->next, tmp);
            } else {
                list_move_tail(L2->next, tmp);
            }
        } else {
            if (strcmp(list_entry(L1->next, element_t, list)->value,
                       list_entry(L2->next, element_t, list)->value) < 0) {
                list_move_tail(L1->next, tmp);
            } else {
                list_move_tail(L2->next, tmp);
            }
        }
    }
    list_empty(L1) ? list_splice_tail_init(L2, tmp)
                   : list_splice_tail_init(L1, tmp);
    list_splice_tail_init(tmp, L2);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    // get mid
    struct list_head *oneStep = head->next;
    struct list_head *twoStep = head->next->next;
    while (twoStep != head && twoStep->next != head) {
        oneStep = oneStep->next;
        twoStep = twoStep->next->next;
    }

    // cut
    struct list_head head_to;
    INIT_LIST_HEAD(&head_to);
    list_cut_position(&head_to, head, oneStep);

    // recursive
    q_sort(&head_to, descend);
    q_sort(head, descend);

    // mergeSort
    struct list_head tmp;
    INIT_LIST_HEAD(&tmp);
    mergeTwoLists(&head_to, head, &tmp, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head) || list_is_singular(head))
        return 0;

    struct list_head *std = head->prev;
    struct list_head *cur = std->prev;
    struct list_head *tmp = NULL;
    int len = 1;

    while (cur != head) {
        // cur > std
        if (strcmp(list_entry(cur, element_t, list)->value,
                   list_entry(std, element_t, list)->value) < 0) {
            std = cur;
            cur = cur->prev;
            len++;
            // cur < std
        } else {
            tmp = cur->prev;
            list_del(cur);
            q_release_element(list_entry(cur, element_t, list));
            cur = tmp;
            tmp = NULL;
        }
    }
    return len;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head) || list_is_singular(head))
        return 0;

    struct list_head *std = head->prev;
    struct list_head *cur = std->prev;
    struct list_head *tmp = NULL;
    int len = 1;

    while (cur != head) {
        // cur > std
        if (strcmp(list_entry(cur, element_t, list)->value,
                   list_entry(std, element_t, list)->value) > 0) {
            std = cur;
            cur = cur->prev;
            len++;
            // cur < std
        } else {
            tmp = cur->prev;
            list_del(cur);
            q_release_element(list_entry(cur, element_t, list));
            cur = tmp;
            tmp = NULL;
        }
    }
    return len;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
