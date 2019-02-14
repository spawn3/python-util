#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "assert.h"
#include "math.h"

#include "resume.h"

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

    queue_t q;
    queue_init(&q);

    bintree_node *curr = root;
    queue_push(&q, curr);
    queue_push(&q, NULL);

    int level = 0;

    while (!queue_empty(&q)) {
        curr = queue_pop(&q);
        if (curr == NULL) {
            level ++;

            if (!queue_empty(&q)) {
                queue_push(&q, NULL);
            }

        } else {
            if (curr->left != NULL)
                queue_push(&q, curr->left);
            if (curr->right != NULL)
                queue_push(&q, curr->right);
        }
    }

    queue_release(&q);
    return level;
}

int bintree_levelsum_nr(bintree_node *root) {
    if (root == NULL)
        return 0;

    queue_t q;
    queue_init(&q);

    bintree_node *curr = root;
    queue_push(&q, curr);
    queue_push(&q, NULL);

    int level = 0;
    int level_sum = 0;
    int max_sum = 0;
    int max_level = 0;

    while (!queue_empty(&q)) {
        curr = queue_pop(&q);
        if (curr == NULL) {
            if (level_sum > max_sum) {
                max_sum = level_sum;
                max_level = level;
            }

            level ++;
            level_sum = 0;

            if (!queue_empty(&q)) {
                queue_push(&q, NULL);
            }
        } else {
            level_sum += curr->key;

            if (curr->left != NULL)
                queue_push(&q, curr->left);
            if (curr->right != NULL)
                queue_push(&q, curr->right);
        }
    }

    queue_release(&q);
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

// typedef bintree_node *bintree_t;

typedef void (*visit_fn)(bintree_node *node);

void visit_fn_print(bintree_node *node) {
    printf("\tnode %p %d\n", node, node->key);
}

void bst_preorder(bintree_node *root, visit_fn fn) {
    if (root) {
        fn(root);
        bst_preorder(root->left, fn);
        bst_preorder(root->right, fn);
    }
}

void bintree_dfs_stack_2(bintree_node *root, bintree_node *path[], int count, int sum) {
    if (root == NULL)
        return;

    path[count++] = root;
    sum += root->key;

    if (root->left == NULL && root->right == NULL) {
        printf("sum %d node %d\n", sum, root->key);
        for (int i=0; i < count; i++) {
            printf("\tsum %d node %d\n", sum, path[i]->key);
        }
    } else {
        bintree_dfs_stack_2(root->left, path, count, sum);
        bintree_dfs_stack_2(root->right, path, count, sum);
    }
}

void bintree_dfs_stack(bintree_node *root) {
    // bintree_node *stack[100];
    // int count = 0;
    bintree_node *path[MAX_SIZE];
    int count = 0;

    bintree_dfs_stack_2(root, path, count, 0);
}

// path and path sum
void bintree_path_impl(bintree_node *root, int expected_sum, int current_sum) {
    if (root == NULL)
        return;

    current_sum += root->key;

    int leaf = (root->left == NULL && root->right == NULL);
    if (leaf && current_sum == expected_sum) {
        printf("find node %d\n", root->key);
    }

    if (root->left)
        bintree_path_impl(root->left, expected_sum, current_sum);
    if (root->right)
        bintree_path_impl(root->right, expected_sum, current_sum);
}

void bintree_path(bintree_node *root, int expected_sum) {
    bintree_path_impl(root, expected_sum, 0);
}

// preorder, postorder and inorder have different results
void bintree_mirror(bintree_node *root) {
    if (root == NULL)
        return;

    if (root->left)
        bintree_mirror(root->left);

    if (root->right)
        bintree_mirror(root->right);

    bintree_node *tmp = root->left;
    root->left = root->right;
    root->right = tmp;
}

int bintree_all_ancestors(bintree_node *root, int key) {
    if (root == NULL)
        return 0;

    // int cond1 = (root->key == key);
    int cond2 = (root->left && root->left->key == key);
    int cond3 = (root->right && root->right->key == key);
    if (cond2 || cond3 || bintree_all_ancestors(root->left, key) || bintree_all_ancestors(root->right, key)) {
        printf("\tnode %p %d\n", root, root->key);
        return 1;
    }

    return 0;
}

bintree_node *bintree_least_common_ancestor(bintree_node *root, int key1, int key2) {
    if (root == NULL)
        return NULL;

    // printf("key %d\n", root->key);

    if (root->key == key1 || root->key == key2)
        return root;

    bintree_node *left = bintree_least_common_ancestor(root->left, key1, key2);
    bintree_node *right = bintree_least_common_ancestor(root->right, key1, key2);
    if (left && right)
        return root;

    return left ? left : right;
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

    queue_t q;
    queue_init(&q);

    bintree_node *p;

    queue_push(&q, root);
    while (!queue_empty(&q)) {
        p = queue_pop(&q);
        fn(p);

        if (p->left)
            queue_push(&q, p->left);

        if (p->right)
            queue_push(&q, p->right);
    }

    queue_release(&q);
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

    printf("preorder:\n");
    bst_preorder(root, visit_fn_print);
    printf("inorder:\n");
    bst_inorder(root, visit_fn_print);
    printf("postorder:\n");
    bst_postorder(root, visit_fn_print);
    printf("levelorder:\n");
    bst_levelorder(root, visit_fn_print);

    printf("111\n");

    // bintree_dfs(root);
    bintree_dfs_stack(root);
    bintree_path(root, 11);

    printf("222\n");

    bintree_all_ancestors(root, 4);

    printf("222 - 000\n");

    bintree_node *lca  = bintree_least_common_ancestor(root, 4, 6);
    assert(lca->key == 5);

    printf("333\n");

    lca  = bintree_least_common_ancestor(root, 2, 4);
    ASSERT_EQUAL(lca->key, 3);

    lca  = bintree_least_common_ancestor(root, 2, 3);
    assert(lca->key == 3);

    printf("444\n");

    bintree_mirror(root);
    printf("preorder:\n");
    bst_preorder(root, visit_fn_print);

    bintree_mirror(root);
    printf("preorder:\n");
    bst_preorder(root, visit_fn_print);
}
