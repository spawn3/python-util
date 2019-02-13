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

int gcd(int m, int n) {
    int r = m % n;
    if (r == 0)
        return n;
    else
        return gcd(n, r);
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
    int key;
    struct __bintree_node *left, *right;
} bintree_node;

// typedef bintree_node *bintree_t;
typedef bintree_node *bintree_node_t;

bintree_node_t bintree_node_new(int key) {
    bintree_node_t p;

    p = (bintree_node *)malloc(sizeof(bintree_node));
    FatalError(p != NULL, "OOM");

    p->key = key;
    p->left = NULL;
    p->right = NULL;
    return p;
}

bintree_node *bintree_find(bintree_node *root, int x) {
    bintree_node *curr = root;

    while (curr) {
        if (x == curr->key)
            return curr;
        else if (x < curr->key)
            curr = curr->left;
        else
            curr = curr->right;
    }

    return NULL;
}

bintree_node *bintree_find_for_insert(bintree_node *root, int x) {
    bintree_node *prev = NULL;
    bintree_node *curr = root;

    while (curr) {
        if (x == curr->key)
            return NULL;
        else if (x < curr->key) {
            prev = curr;
            curr = curr->left;
        } else {
            prev = curr;
            curr = curr->right;
        }
    }

    return prev;
}

void bintree_insert(bintree_node **root, int key) {
    bintree_node *root2 = *root;
    if (root2 == NULL) {
        bintree_node *new_node = bintree_node_new(key);
        *root = new_node;
        return;
    }

    bintree_node *p = bintree_find_for_insert(root2, key);
    if (p != NULL) {
        bintree_node *new_node = bintree_node_new(key);
        if (key < p->key)
            p->left = new_node;
        else
            p->right = new_node;
    }

    /*
    bintree_node *curr = root;
    while (curr != NULL) {
        if (key <= curr->key) {
            if (curr->left == NULL) {
                curr->left = p;
                break;
            } else {
                curr = curr->left;
            }
        } else {
            if (curr->right == NULL) {
                curr->right = p;
                break;
            } else {
                curr = curr->right;
            }
        }
    }
    */
}

bintree_node *bintree_min(bintree_node *root) {
    bintree_node *curr = root;
    while (curr) {
        if (curr->left == NULL)
            break;
        curr = curr->left;
    }

    return curr;
}

bintree_node *bintree_max(bintree_node *root) {
    bintree_node *curr = root;
    while (curr) {
        if (curr->right == NULL)
            break;
        curr = curr->right;
    }

    return curr;
}

int bintree_depth(bintree_node *root) {
    if (root == NULL) {
        return 0;
    }

    int left = bintree_depth(root->left);
    int right = bintree_depth(root->right);

    // post order
    return MAX(left, right) + 1;
}

int bintree_depth_nr(bintree_node *root) {
    if (root == NULL)
        return 0;

    myqueue_t q;
    myqueue_init(&q);

    bintree_node *curr = root;
    myqueue_push(&q, curr);
    myqueue_push(&q, NULL);

    int level = 0;

    while (!myqueue_empty(&q)) {
        curr = myqueue_pop(&q);
        if (curr == NULL) {
            level ++;

            if (!myqueue_empty(&q)) {
                myqueue_push(&q, NULL);
            }

        } else {
            if (curr->left != NULL)
                myqueue_push(&q, curr->left);
            if (curr->right != NULL)
                myqueue_push(&q, curr->right);
        }
    }

    return level;
}

int bintree_levelsum_nr(bintree_node *root) {
    if (root == NULL)
        return 0;

    myqueue_t q;
    myqueue_init(&q);

    bintree_node *curr = root;
    myqueue_push(&q, curr);
    myqueue_push(&q, NULL);

    int level = 0;
    int level_sum = 0;
    int max_sum = 0;
    int max_level = 0;

    while (!myqueue_empty(&q)) {
        curr = myqueue_pop(&q);
        if (curr == NULL) {
            if (level_sum > max_sum) {
                max_sum = level_sum;
                max_level = level;
            }

            level ++;
            level_sum = 0;

            if (!myqueue_empty(&q)) {
                myqueue_push(&q, NULL);
            }
        } else {
            level_sum += curr->key;

            if (curr->left != NULL)
                myqueue_push(&q, curr->left);
            if (curr->right != NULL)
                myqueue_push(&q, curr->right);
        }
    }

    printf("level %d sum %d\n", max_level, max_sum);

    return level;
}

void bintree_level_core(bintree_node *root, int level) {
    if (root == NULL)
        return;

    printf("node %d level %d\n", root->key, level);

    if (root->left)
        bintree_level_core(root->left, level + 1);

    if (root->right)
        bintree_level_core(root->right, level + 1);
}

// print node and its level/depth
void bintree_level(bintree_node *root) {
    bintree_level_core(root, 0);
}

void bintree_dfs(bintree_node *root) {
    if (root == NULL) {
        return;
    }
    printf("%d\n", root->key);
    bintree_dfs(root->left);
    bintree_dfs(root->right);
}

int bintree_sum(bintree_node *arr[], int count) {
        int sum = 0;
        for (int i=0; i < count; ++i) {
            sum += arr[i]->key;
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

    sum += root->key;

    if (root->left == NULL && root->right == NULL) {
        printf("node %d sum %d\n", root->key, sum);
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

// typedef bintree_node *bintree_t;

typedef void (*visit_fn)(bintree_node *node);

void visit_fn_print(bintree_node *node) {
    printf("node %p %d\n", node, node->key);
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
    array_print_nr(10);

    int arr[] = {20, 10, 30, 40, 1};
    int arr_size = ARRSIZE(arr);

    assert(array_is_asc(arr, arr_size) == 0);

    assert(sum1(arr, arr_size) == 101);
    assert(sum2(arr, arr_size) == 101);
    assert(sum3(arr, arr_size) == 101);

    assert(myarray_sum(arr, 0, arr_size) == 101);

    for (int i=0; i < 10; i++) {
        assert(fab1(i) == fab2(i));
    }

    quick_sort(arr, 0, 4);
    // merge_sort(arr, 0, 4);
    array_print(arr, arr_size);

    assert(bisearch1(arr, arr_size, 1) == 0);
    assert(bisearch1(arr, arr_size, 10) == 1);
    assert(bisearch1(arr, arr_size, 20) == 2);
    assert(bisearch1(arr, arr_size, 30) == 3);
    assert(bisearch1(arr, arr_size, 40) == 4);
    assert(bisearch1(arr, arr_size, 8) == -1);
    assert(bisearch1(arr, arr_size, 50) == -1);

    assert(bisearch2(arr, 0, arr_size - 1, 1) == 0);
    assert(bisearch2(arr, 0, arr_size - 1, 10) == 1);
    assert(bisearch2(arr, 0, arr_size - 1, 20) == 2);
    assert(bisearch2(arr, 0, arr_size - 1, 30) == 3);
    assert(bisearch2(arr, 0, arr_size - 1, 40) == 4);
    assert(bisearch2(arr, 0, arr_size - 1, 8) == -1);
    assert(bisearch2(arr, 0, arr_size - 1, 50) == -1);

    int arr2[] = {1, 10, 20, 30, 40};
    int arr_size2 = ARRSIZE(arr2);
    assert(array_is_asc(arr2, arr_size2) == 1);

    array_reverse(arr2, arr_size2);
    assert(arr2[0] == 40);
    assert(arr2[1] == 30);
    assert(arr2[2] == 20);
    assert(arr2[3] == 10);
    assert(arr2[4] == 1);

    array_reverse(arr2, arr_size2);
    assert(arr2[0] == 1);
    assert(arr2[1] == 10);
    assert(arr2[2] == 20);
    assert(arr2[3] == 30);
    assert(arr2[4] == 40);
}

void test_string() {
    char *src = "abcd";
    char dst[256];

    printf("src %s dst %s\n", src, my_strcpy(dst, src));

    assert(strlen("\0") == 0);

    printf("sizeof %ld\n", sizeof("\0"));
    assert(sizeof("\0") == 2);

    assert(string_palindrome("abcdcba") == 1);
    assert(string_palindrome("abcdba") == 0);

    // string_perm("abc");

    char s[256] = "abc";
    string_perm(s);
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

void test_bintree() {
    bintree_node *root = NULL;

    bintree_insert(&root, 5);
    bintree_insert(&root, 3);
    bintree_insert(&root, 4);
    bintree_insert(&root, 6);
    bintree_insert(&root, 2);

    bintree_node *p;
    p = bintree_find(root, 5); assert(p->key == 5);
    p = bintree_find(root, 3); assert(p->key == 3);
    p = bintree_find(root, 4); assert(p->key == 4);
    p = bintree_find(root, 6); assert(p->key == 6);
    p = bintree_find(root, 2); assert(p->key == 2);
    p = bintree_find(root, 1); assert(p == NULL);

    p = bintree_min(root); assert(p->key == 2);
    p = bintree_max(root); assert(p->key == 6);

    bintree_level(root);

    printf("depth %d\n", bintree_depth(root));
    printf("depth %d\n", bintree_depth_nr(root));
    assert(bintree_depth(root) == bintree_depth_nr(root));

    bintree_levelsum_nr(root);

    // bintree_dfs(root);
    bintree_dfs_stack(root);
    //

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

    printf("test array:\n");
    test_array();

    printf("test string:\n");
    test_string();

    printf("\ntest single list:\n");
    test_slist();

    printf("\ntest binary tree:\n");
    test_bintree();

    return 0;
}
