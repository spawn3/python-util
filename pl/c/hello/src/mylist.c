#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "assert.h"
#include "math.h"

#include "resume.h"


typedef struct __single_node {
    int key;
    struct __single_node *next;
} slist_node_t;

typedef void (*single_list_scan_fn)(slist_node_t *node);

slist_node_t *single_list_malloc(int key) {
    slist_node_t *p = malloc(sizeof(slist_node_t));
    if (p == NULL)
        assert(0);

    p->next = NULL;
    p->key = key;
    return p;
}

void single_list_insert(slist_node_t **head, int position, int key) {
    slist_node_t *p = *head, *q;
    slist_node_t *new_node = single_list_malloc(key);
    if (position == 1) {
        new_node->next = p;
        *head = new_node;
    } else {
        int k = 0;
        while (p != NULL && k < position) {
            k++;
            q = p;
            p = p->next;
        }

        new_node->next = p;
        q->next = new_node;
    }
}

slist_node_t *__single_list_reverse(slist_node_t *node, slist_node_t *prev, slist_node_t *new_head) {
    if (node == NULL)
        return new_head;

    slist_node_t *next = node->next;
    if (next == NULL)
        new_head = node;

    // printf("%p %p %p new %p %d\n", prev, node, next, new_head, node->key);

    node->next = prev;
    return __single_list_reverse(next, node, new_head);
}

slist_node_t *single_list_reverse(slist_node_t *node) {
    return __single_list_reverse(node, NULL, NULL);
}

void single_list_print(slist_node_t *node) {
    printf("node %p %d\n", node, node->key);
}

void single_list_scan(slist_node_t *head, single_list_scan_fn fn) {
    printf("single_list_scan:\n");
    slist_node_t *p = head;
    while (p != NULL) {
        fn(p);
        p = p->next;
    }
}

int slist_getsize(slist_node_t *head) {
    int count = 0;
    slist_node_t *curr = head;
    while (curr) {
        count ++;
        curr = curr->next;
    }
    return count;
}

void test_slist() {
    slist_node_t *head = NULL;
    single_list_insert(&head, 1, 1);
    single_list_insert(&head, 100, 2);
    single_list_insert(&head, 100, 3);
    single_list_insert(&head, 100, 4);
    single_list_insert(&head, 100, 5);

    assert(slist_getsize(head) == 5);
    single_list_scan(head, single_list_print);

    head = single_list_reverse(head);
    assert(slist_getsize(head) == 5);
    single_list_scan(head, single_list_print);
}
