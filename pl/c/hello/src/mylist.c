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

slist_node_t *slist_malloc(int key) {
    slist_node_t *p = malloc(sizeof(slist_node_t));
    if (p == NULL)
        assert(0);

    p->key = key;
    p->next = NULL;
    return p;
}

// the previous of 0th if head
// the previous of tail NULL is tail
slist_node_t *slist_find_prev(slist_node_t *head, int k) {
    if (head == NULL)
        return NULL;

    slist_node_t *curr = head;
    for (int i=0; i < k; i++) {
        if (curr->next == NULL) {
            return curr;
        } else {
            curr = curr->next;
        }
    }
    return curr;
}

void slist_insert(slist_node_t **head, int position, int key) {
    assert(position >= 0);

    slist_node_t *new_node = slist_malloc(key);
    FatalError(new_node != NULL, "OOM");

    if (*head == NULL) {
        *head = new_node;
        printf("empty position %d key %d\n", position, key);
    } else {
        printf("position %d key %d\n", position, key);
        if (position == 0) {
            new_node->next = (*head);
            *head = new_node;
        } else {
            slist_node_t *prev = slist_find_prev(*head, position - 1);
            new_node->next = prev->next;
            prev->next = new_node;
        }
    }

    /*
    slist_node_t *p = *head, *q;
    slist_node_t *new_node = slist_malloc(key);
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
    */
}

slist_node_t *slist_reverse_impl1(slist_node_t *node, slist_node_t *prev, slist_node_t *new_head) {
    if (node == NULL)
        return new_head;

    slist_node_t *next = node->next;
    if (next == NULL)
        new_head = node;

    // printf("%p %p %p new %p %d\n", prev, node, next, new_head, node->key);

    node->next = prev;
    return slist_reverse_impl1(next, node, new_head);
}

slist_node_t *slist_reverse_impl2(slist_node_t *head) {
    slist_node_t *new_head = NULL;
    slist_node_t *prev = NULL;
    slist_node_t *curr = head;
    slist_node_t *next;

    while (curr != NULL) {
        next = curr->next;
        if (next == NULL) {
            new_head = curr;
        }

        curr->next = prev;
        prev = curr;
        curr = next;
    }

    return new_head;
}

slist_node_t *slist_reverse(slist_node_t *head) {
    if (head == NULL)
        return NULL;
    // return slist_reverse_impl1(head, NULL, NULL);
    return slist_reverse_impl2(head);
}

slist_node_t *slist_kth_r(slist_node_t *head, int k) {
    slist_node_t *fast = head;
    for (int i=0; i < k - 1; i++) {
        if (fast != NULL)
            fast = fast->next;
        else
            return NULL;
    }

    slist_node_t *slow = head;
    while (fast) {
        if (fast->next == NULL) {
            break;
        }
        fast = fast->next;
        slow = slow->next;
    }

    return slow;
}

slist_node_t *slist_kth(slist_node_t *head, int k) {
    if (head == NULL || k == 0)
        return NULL;

    if (k > 0) {
        slist_node_t *curr = head;
        for (int i=0; i < k - 1; i++) {
            if (curr != NULL)
                curr = curr->next;
        }
        return curr;
    } else {
        return slist_kth_r(head, -k);
    }
}

void slist_print(slist_node_t *node) {
    printf("\tnode %p %d\n", node, node->key);
}

void slist_scan(slist_node_t *head, single_list_scan_fn fn) {
    printf("slist_scan:\n");
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

slist_node_t *slist_merge(slist_node_t *head1, slist_node_t *head2) {
    if (head1 == NULL)
        return head2;
    if (head2 == NULL)
        return head1;

    slist_node_t *merged = NULL;
    slist_node_t *curr = NULL;

    if (head1->key <= head2->key) {
        curr = head1;
        head1 = head1->next;
    } else {
        curr = head2;
        head2 = head2->next;
    }

    merged = curr;
    curr->next = NULL;

    while (head1 && head2) {
        if (head1->key <= head2->key) {
            curr->next = head1;
            head1 = head1->next;
        } else {
            curr->next = head2;
            head2 = head2->next;
        }

        curr = curr->next;
        curr->next = NULL;
    }

    while (head1) {
        curr->next = head1;
        head1 = head1->next;

        curr = curr->next;
        curr->next = NULL;
    }

    while (head2) {
        curr->next = head2;
        head2 = head2->next;

        curr = curr->next;
        curr->next = NULL;
    }

    return merged;
}

void test_slist() {
    slist_node_t *head = NULL;
    slist_insert(&head, 0, 1);
    slist_insert(&head, 1, 2);
    slist_insert(&head, 2, 3);
    slist_insert(&head, 3, 4);
    slist_insert(&head, 4, 5);

    assert(slist_getsize(head) == 5);
    slist_scan(head, slist_print);

    slist_node_t *p;
    p = slist_kth(head, 1); ASSERT_EQUAL(p->key, 1);
    p = slist_kth(head, 2); ASSERT_EQUAL(p->key, 2);
    p = slist_kth(head, 3); ASSERT_EQUAL(p->key, 3);
    p = slist_kth(head, 4); ASSERT_EQUAL(p->key, 4);
    p = slist_kth(head, 5); ASSERT_EQUAL(p->key, 5);

    // test reverse
    head = slist_reverse(head);
    assert(slist_getsize(head) == 5);
    slist_scan(head, slist_print);

    p = slist_kth(head, -1); ASSERT_EQUAL(p->key, 1);
    p = slist_kth(head, -2); ASSERT_EQUAL(p->key, 2);
    p = slist_kth(head, -3); ASSERT_EQUAL(p->key, 3);
    p = slist_kth(head, -4); ASSERT_EQUAL(p->key, 4);
    p = slist_kth(head, -5); ASSERT_EQUAL(p->key, 5);

    //test merge
    slist_node_t *head1 = NULL;
    slist_insert(&head1, 1, 1);
    slist_insert(&head1, 2, 3);
    slist_insert(&head1, 3, 5);
    slist_insert(&head1, 4, 7);
    slist_insert(&head1, 5, 9);

    assert(slist_getsize(head1) == 5);
    slist_scan(head1, slist_print);

    slist_node_t *head2 = NULL;
    slist_insert(&head2, 1, 2);
    slist_insert(&head2, 2, 4);
    slist_insert(&head2, 3, 6);
    slist_insert(&head2, 4, 8);
    slist_insert(&head2, 5, 10);

    assert(slist_getsize(head2) == 5);
    slist_scan(head2, slist_print);

    slist_node_t *merged = slist_merge(head1, head2);
    assert(slist_getsize(merged) == 10);
    slist_scan(merged, slist_print);
}
