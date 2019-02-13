#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "assert.h"
#include "math.h"

#include "resume.h"

void stack_init(mystack_t *stack) {
    memset(stack->arr, 0x0, sizeof(void *) * MAX_SIZE);
    stack->count = 0;
}

void stack_destroy(mystack_t *stack) {
    stack_init(stack);
}

int stack_empty(mystack_t *stack) {
    return stack->count == 0;
}

void stack_push(mystack_t *stack, void *p) {
    assert(stack->count < MAX_SIZE);
    stack->arr[stack->count++] = p;
}

void *stack_pop(mystack_t *stack) {
    if (stack->count == 0)
        return NULL;
    void *p = stack->arr[stack->count - 1];
    stack->arr[stack->count - 1] = NULL;
    stack->count--;
    return p;
}

void myqueue_init(myqueue_t *q) {
    memset(q->arr, 0x0, sizeof(void *) * MAX_SIZE);
    q->front = -1;
    q->rear = 0;
}

int myqueue_empty(myqueue_t *q) {
    return q->front == -1;
}

void myqueue_push(myqueue_t *q, void *p) {
    q->arr[q->rear] = p;
    if (q->front == -1)
        q->front = q->rear;
    q->rear = (q->rear + 1 ) % MAX_SIZE;
}

void *myqueue_pop(myqueue_t *q) {
    assert(!myqueue_empty(q));

    void *p = NULL;

    if (q->front != -1) {
        p = q->arr[q->front];
        if ((q->front + 1) % MAX_SIZE == q->rear) {
            q->front = -1;
            q->rear = 0;
        } else {
            q->front = (q->front + 1) % MAX_SIZE;
        }
    }

    return p;
}
