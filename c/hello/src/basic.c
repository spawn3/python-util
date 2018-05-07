#include <stdio.h>
#include <assert.h>

// 直接把变量声明为类型
typedef int (*func_1)(int);
typedef int *array_of_intp[10];
typedef int *intp;

/**
 * ABA problem
 */
int cas(int *p, int expected, int desired) {
    if (*p == expected) {
        *p = desired;
        return 1;
    }

    return 0;
}

int str_cmp(const char *s1, const char *s2) {
    int i;

    for (i=0; ; i++) {
        if (s1[i] < s2[i])
            return -1;
        else if (s1[i] > s2[i])
            return 1;
        else {
            if (s1[i] == '\0')
                return 0;
        }
    }

    return 0;
}

char *str_cpy(char *dest, const char *src) {
    int i;

    for (i = 0; src[i]; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';

    return dest;
}

int array_find(int *arr, int n, int k) {
    int low = 0, high = n - 1, mid;

    if (n < 1)
        return -1;

    while (low <= high) {
        mid = (low + high) / 2;
        if (arr[mid] == k) {
            return mid;
        } else if (arr[mid] < k) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    return -1;
}

int test_array() {
    int *a[] = {NULL, NULL};
    // 从变量标识符开始
    // end with the basic type
    // go right when you can, go left when you must
    // 优先级[],()大于*
    int *(*b)[] = &a;

    (void) a;

    printf("sizeof(int *) = %ld\n", sizeof(int *));
    printf("sizeof(func_1) = %ld\n", sizeof(func_1));
    printf("sizeof(a) = %ld\n", sizeof(a));
    printf("a %p &a[0] %p &a[1] %p\n", a, &a[0], &a[1]);
    printf("b %p\n", b);

    int i = 2;
    intp pi = &i;
    printf("i = %d\n", *pi);

    // array
    int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    printf("idx of 0: %d\n", array_find(arr, 10, 0));
    printf("idx of 1: %d\n", array_find(arr, 10, 1));
    printf("idx of 5: %d\n", array_find(arr, 10, 5));
    printf("idx of 10: %d\n", array_find(arr, 10, 10));
    printf("idx of 11: %d\n", array_find(arr, 10, 11));

    // string
    printf("== TEST string ==\n");
    printf("null char %d\n", '\0');

    assert(str_cmp("abc", "abc") == 0);
    assert(str_cmp("abc", "ab") > 0);
    assert(str_cmp("abc", "abcd") < 0);

    char dest[256];
    printf("str_cpy %s\n", str_cpy(dest, "abc"));
    printf("str_cpy %s\n", str_cpy(dest, "abcd"));
    printf("str_cpy %s\n", str_cpy(dest, "ab"));

    return 0;
}


int test_basic() {
    test_array();

    return 0;
}
