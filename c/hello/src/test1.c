#include <stdio.h>
#include <assert.h>


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

    assert(sizeof(char) == 1);
    assert(sizeof(short) == 2);
    assert(sizeof(int) == 4);
    assert(sizeof(long) == 8);

    assert(sizeof(chkid_t) == 16);
}
