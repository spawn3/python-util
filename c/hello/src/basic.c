#include <stdio.h>

// 直接把变量声明为类型
typedef int (*func_1)(int);
typedef int *array_of_intp[10];
typedef int *intp;

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

    return 0;
}


int test_basic() {
    test_array();

    return 0;
}
