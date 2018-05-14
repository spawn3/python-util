#include <cstdio>
#include <cassert>


void test_bit() {
    for (int i=0; i < 129; i++) {
        // 负数表示法：2的补码
        printf("~%d = %d\n", i, ~i);
        assert(~i + 1 == -i);
    }

    int x = 0x1;
    const char *p = (const char *)&x;
    // printf("%p %d\n", p, *p++);
    // printf("%p %d\n", p, *p++);
    // printf("%p %d\n", p, *p++);
    // printf("%p %d\n", p, *p++);

    printf("%p %d\n", p, *p); p++;
    printf("%p %d\n", p, *p); p++;
    printf("%p %d\n", p, *p); p++;
    printf("%p %d\n", p, *p); p++;
}

void test_array() {
    int size = 5;
    int a[size];

    for (int i=0; i < size; i++) {
        a[i] = i;
    }

    for (int i=0; i < size; i++) {
        printf("%d\n", a[i]);
    }
}


int main() {
    test_bit();
    test_array();

    return 0;
}
