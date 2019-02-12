#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "assert.h"
#include "math.h"

#define MAX_SIZE 100
#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define ARRSIZE(arr) (sizeof(arr)/ sizeof(arr[0]))

static inline void swap(int *x, int *y) {
    int tmp = *x;
    *x = *y;
    *y = tmp;
}

void array_print(int arr[], int n) {
    for (int i=0; i < n; i++) {
        printf("%4d %d\n", i, arr[i]);
    }
}

int bisearch1(int arr[], int n, int x) {
    int left = 0;
    int right = n - 1;
    int mid = 0;

    while (left <= right) {
        mid = left + (right - left) / 2;
        if (x == arr[mid])
            return mid;
        else if (x < arr[mid])
            right = mid - 1;
        else
            left = mid + 1;
    }

    return -1;
}

int quicksort_partition(int arr[], int low, int high) {
    int r = low + rand() % (high - low + 1);
    swap(&arr[low], &arr[r]);

    // printf("r %d\n", r);

    int left = low, right = high, pivot_value = arr[low];

    while (left < right) {
        while (arr[left] <= pivot_value)
            left++;
        while (arr[right] > pivot_value)
            right --;

        if (left < right) {
            swap(&arr[left], &arr[right]);
        }
    }

    arr[low] = arr[right];
    arr[right] = pivot_value;
    return right;
}

void quicksort(int arr[], int low, int high) {
    if (low < high) {
        int pivot = quicksort_partition(arr, low, high);
        quicksort(arr, low, pivot - 1);
        quicksort(arr, pivot + 1, high);
    }
}

typedef struct __single_list {
    int value;
    struct __single_list *next;
} single_list;

typedef void (*single_list_scan_fn)(single_list *node);

single_list *single_list_malloc(int value) {
    single_list *p = malloc(sizeof(single_list));
    if (p == NULL)
        assert(0);

    p->next = NULL;
    p->value = value;
    return p;
}

void single_list_insert(single_list **head, int position, int value) {
    single_list *p = *head, *q;
    single_list *new_node = single_list_malloc(value);
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

single_list *__single_list_reverse(single_list *node, single_list *prev, single_list *new_head) {
    if (node == NULL)
        return new_head;

    single_list *next = node->next;
    if (next == NULL)
        new_head = node;

    // printf("%p %p %p new %p %d\n", prev, node, next, new_head, node->value);

    node->next = prev;
    return __single_list_reverse(next, node, new_head);
}

single_list *single_list_reverse(single_list *node) {
    return __single_list_reverse(node, NULL, NULL);
}

void single_list_print(single_list *node) {
    printf("node %p %d\n", node, node->value);
}

void single_list_scan(single_list *head, single_list_scan_fn fn) {
    printf("single_list_scan:\n");
    single_list *p = head;
    while (p != NULL) {
        fn(p);
        p = p->next;
    }
}

int gcd(int m, int n) {
    int r = m % n;
    if (r == 0)
        return n;
    else
        return gcd(n, r);
}

int sum1(int arr[], int n) {
    int sum = 0;
    for (int i=0; i < n; i++)
        sum += arr[i];

    // printf("sum %d\n", sum);
    return sum;
}

int __sum2(int arr[], int idx, int len, int sum) {
    if (len == 0)
        return sum;

    return __sum2(arr, idx + 1, len - 1, sum + arr[idx]);
}

int sum2(int arr[], int n) {
    return __sum2(arr, 0, n, 0);
}

int __sum3(int arr[], int idx, int len, int sum) {
    if (idx == len)
        return sum;

    return __sum3(arr, idx + 1, len, sum + arr[idx]);
}

int sum3(int arr[], int n) {
    return __sum3(arr, 0, n, 0);
}


int fab1(int n) {
    if (n == 0 || n == 1)
        return n;
    return fab1(n - 2) + fab1(n - 1);
}

int fab2(int n) {
    if (n == 0 || n == 1)
        return n;

    int twoback = 0;
    int oneback = 1;
    int current;
    for (int i=2; i <= n; i++) {
        current = twoback + oneback;
        twoback = oneback;
        oneback = current;
    }

    return current;
}

typedef struct {
    void *arr[MAX_SIZE];
    int count;
} mystack_t;

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

typedef struct {
    void *arr[MAX_SIZE];
    int front, rear;
} myqueue_t;

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

typedef struct __bintree_node {
    int value;
    struct __bintree_node *left, *right;
} bintree_node;

bintree_node *bintree_node_new(int value) {
    bintree_node *p;

    p = (bintree_node *)malloc(sizeof(bintree_node));

    p->value = value;
    p->left = NULL;
    p->right = NULL;
    return p;
}

bintree_node *bintree_insert(bintree_node *root, int value) {
    if (root == NULL) {
        root = bintree_node_new(value);
        return root;
    }

    bintree_node *tmp = root;
    bintree_node *p = bintree_node_new(value);

    while (tmp != NULL) {
        if (value <= tmp->value) {
            if (tmp->left == NULL) {
                tmp->left = p;
                break;
            } else {
                tmp = tmp->left;
            }
        } else {
            if (tmp->right == NULL) {
                tmp->right = p;
                break;
            } else {
                tmp = tmp->right;
            }
        }
    }

    return root;
}

int bintree_depth(bintree_node *root) {
    if (root == NULL) {
        return 0;
    }

    int left = bintree_depth(root->left);
    int right = bintree_depth(root->right);
    return MAX(left, right) + 1;
}

void bintree_dfs(bintree_node *root) {
    if (root == NULL) {
        return;
    }
    printf("%d\n", root->value);
    bintree_dfs(root->left);
    bintree_dfs(root->right);
}

int bintree_sum(bintree_node *arr[], int count) {
        int sum = 0;
        for (int i=0; i < count; ++i) {
            sum += arr[i]->value;
        }
        return sum;
}

void bintree_dfs_stack_1(bintree_node *root, bintree_node *stack[], int count) {
    if (root == NULL)
        return;

    stack[count++] = root;

    if (root->left == NULL && root->right == NULL) {
        printf("sum %d\n", bintree_sum(stack, count));
    } else {
        bintree_dfs_stack_1(root->left, stack, count);
        bintree_dfs_stack_1(root->right, stack, count);
    }
}

void bintree_dfs_stack_2(bintree_node *root, int sum) {
    if (root == NULL)
        return;

    sum += root->value;

    if (root->left == NULL && root->right == NULL) {
        printf("sum %d\n", sum);
    } else {
        bintree_dfs_stack_2(root->left, sum);
        bintree_dfs_stack_2(root->right, sum);
    }
}

void bintree_dfs_stack(bintree_node *root) {
    // bintree_node *stack[100];
    // int count = 0;

    bintree_dfs_stack_2(root, 0);
}

typedef bintree_node *bintree_t;

typedef void (*visit_fn)(bintree_node *node);

void visit_fn_print(bintree_node *node) {
    printf("node %p %d\n", node, node->value);
}

void bst_preorder(bintree_node *root, visit_fn fn) {
    if (root) {
        fn(root);
        bst_preorder(root->left, fn);
        bst_preorder(root->right, fn);
    }
}

void bst_inorder(bintree_node *root, visit_fn fn) {
    if (root) {
        bst_inorder(root->left, fn);
        fn(root);
        bst_inorder(root->right, fn);
    }
}

void bst_postorder(bintree_node *root, visit_fn fn) {
    if (root) {
        bst_postorder(root->left, fn);
        bst_postorder(root->right, fn);
        fn(root);
    }
}

void bst_levelorder(bintree_node *root, visit_fn fn) {
    if (root == NULL)
        return;

    myqueue_t q;
    myqueue_init(&q);

    bintree_node *p;

    myqueue_push(&q, root);
    while (!myqueue_empty(&q)) {
        p = myqueue_pop(&q);
        fn(p);

        if (p->left)
            myqueue_push(&q, p->left);

        if (p->right)
            myqueue_push(&q, p->right);
    }
}

void test_array() {
    int arr[] = {20, 10, 30, 40, 1};

    int arr_size = ARRSIZE(arr);

    assert(sum1(arr, arr_size) == 101);
    assert(sum2(arr, arr_size) == 101);
    assert(sum3(arr, arr_size) == 101);

    for (int i=0; i < 10; i++) {
        assert(fab1(i) == fab2(i));
    }

    quicksort(arr, 0, 4);
    array_print(arr, arr_size);

    assert(bisearch1(arr, arr_size, 1) == 0);
    assert(bisearch1(arr, arr_size, 10) == 1);
    assert(bisearch1(arr, arr_size, 20) == 2);
    assert(bisearch1(arr, arr_size, 30) == 3);
    assert(bisearch1(arr, arr_size, 40) == 4);
    assert(bisearch1(arr, arr_size, 8) == -1);
    assert(bisearch1(arr, arr_size, 50) == -1);


}

void test_slist() {
    single_list *head = NULL;
    single_list_insert(&head, 1, 1);
    single_list_insert(&head, 100, 2);
    single_list_insert(&head, 100, 3);
    single_list_insert(&head, 100, 4);
    single_list_insert(&head, 100, 5);
    single_list_scan(head, single_list_print);

    head = single_list_reverse(head);
    single_list_scan(head, single_list_print);
}

void test_bintree() {
    bintree_node *root = NULL;

    root = bintree_insert(root, 5);
    root = bintree_insert(root, 3);
    root = bintree_insert(root, 4);
    root = bintree_insert(root, 6);
    root = bintree_insert(root, 2);

    // bintree_dfs(root);
    bintree_dfs_stack(root);
    //
    printf("depth %d\n", bintree_depth(root));

    printf("preorder:\n");
    bst_preorder(root, visit_fn_print);
    printf("inorder:\n");
    bst_inorder(root, visit_fn_print);
    printf("postorder:\n");
    bst_postorder(root, visit_fn_print);
    printf("levelorder:\n");
    bst_levelorder(root, visit_fn_print);
}

int main() {
    printf("hello, world!\n");

    test_array();
    test_slist();
    test_bintree();

    return 0;
}
