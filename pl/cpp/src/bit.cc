#include <cstdio>
#include <cassert>

#define IS_POWER_OF_2(x) (!((x) & ((x) - 1)))

static inline int round_up_to_pow_of_2(int val) {
    int x = 1;
    while (x < val) {
        x <<= 1;
    }

    return x;
}

static inline int round_down_to_pow_of_2(int val) {
    int x = 1;
    while (x <= val) {
        x <<= 1;
    }

    x >>= 1;
    return x;
}

// n & (n-1) 消去最后的1


void test_bit() {
    for (int i=0; i < 129; i++) {
        // 负数表示法：2的补码
        printf("~%d = %d\n", i, ~i);
        assert(~i + 1 == -i);
    }

    int x = 0x01020304;
    const char *p = (const char *)&x;
    // printf("%p %d\n", p, *p++);
    // printf("%p %d\n", p, *p++);
    // printf("%p %d\n", p, *p++);
    // printf("%p %d\n", p, *p++);

    printf("%p %d\n", p, *p); p++;
    printf("%p %d\n", p, *p); p++;
    printf("%p %d\n", p, *p); p++;
    printf("%p %d\n", p, *p); p++;

    for (int i=0; i < 7; i++) {
        if (IS_POWER_OF_2(i)) {
            printf("%d is power of 2!\n", i);
        }
    }

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
