#include "stdio.h"
#include "stdlib.h"
#include "assert.h"

#include "resume.h"

slist_item_t *slist_item_malloc(void *key) {
    slist_item_t *p = malloc(sizeof(slist_item_t));
    assert(p != NULL);

    p->key = key;
    p->next = NULL;
    return p;
}

void queue_init(queue_t *q) {
    q->head = NULL;
    q->tail = NULL;
    q->count = 0;
}

void queue_release(queue_t *q) {
    while (!queue_empty(q)) {
        queue_pop(q);
    }
}

int queue_size(queue_t *q) {
    return q->count;
}

int queue_empty(queue_t *q) {
    return q->count == 0;
}

void *queue_top(queue_t *q) {
    if (q->head == NULL) {
        assert(0);
    }

    return q->head->key;
}

void queue_push(queue_t *q, void *key) {
    slist_item_t *new_item = slist_item_malloc(key);
    if (q->tail == NULL) {
        q->head = new_item;
        q->tail = new_item;
    } else {
        q->tail->next = new_item;
        q->tail = new_item;
    }

    q->count++;
}

void *queue_pop(queue_t *q) {
    if (q->head == NULL) {
        assert(0);
    }

    slist_item_t *item = q->head;
    q->head = q->head->next;
    if (q->tail == item) {
        q->tail = NULL;
    }

    q->count--;

    void *key = item->key;
    free(item);
    return key;
}

void test_queue() {
    void *e1 = (void *)0x1;
    queue_t q;
    queue_init(&q);
    queue_push(&q, e1);
    assert(queue_top(&q) == e1);
    assert(queue_pop(&q) == e1);
    ASSERT_EQUAL(queue_size(&q), 0);
    queue_release(&q);
}
