#include <stdio.h>
#include <assert.h>

#define ASSERT_SIZEOF(type, len) do {                 \
    printf("sizeof(" #type ") = %lu\n", sizeof(type)); \
    assert(sizeof(type) == len);                      \
} while(0)


typedef struct __chkid_t {
    char c;
    int id;
    int idx;
    int type;
} chkid_t;


void test_sizeof() {
    // printf("sizeof(chkid_t) = %lu\n", sizeof(chkid_t));
    // printf("sizeof(char) = %lu\n", sizeof(char));
    // printf("sizeof(short) = %lu\n", sizeof(short));
    // printf("sizeof(int) = %lu\n", sizeof(int));
    // printf("sizeof(long) = %lu\n", sizeof(long));

    ASSERT_SIZEOF(char, 1);
    ASSERT_SIZEOF(short, 2);
    ASSERT_SIZEOF(int, 4);
    ASSERT_SIZEOF(long, 8);

    ASSERT_SIZEOF(chkid_t, 16);
}
