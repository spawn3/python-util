#include "stdio.h"
#include "assert.h"
#include <string.h>

#include "resume.h"

#define CHAR_BIT 8

#define BIT_SET(a, n) (a[n/CHAR_BIT] |= (1<<(n%CHAR_BIT)))
#define BIT_DEL(a, n) (a[n/CHAR_BIT] &= (~(1<<(n%CHAR_BIT))))
#define BIT_GET(a, n) (a[n/CHAR_BIT] & (1<<(n%CHAR_BIT)))
#define BIT_CLEAR(a, n) memset(a, 0x0, n/CHAR_BIT)

int bit_count_one(unsigned int n) {
    int count = 0;
    while (n) {
        count += (n & 1);
        n >>= 1;
    }

    return count;
}

void test_bit() {

    // if i == 2 ^ n, then i & (i - 1) == 0
    for (int i=1; i < 10; i++) {
        printf("i %d %d\n", i, (i & (i-1)));
        // assert((i & (i - 1)) == 0);
    }

    assert(bit_count_one(1) == 1);
    assert(bit_count_one(2) == 1);
    assert(bit_count_one(3) == 2);
    assert(bit_count_one(4) == 1);
    assert(bit_count_one(5) == 2);

    char bitmap[4];
    BIT_CLEAR(bitmap, 32);
    for (int i=0; i < 32; i++) {
        assert(BIT_GET(bitmap, i) == 0);

        BIT_SET(bitmap, i);
        printf("%d %d\n", i, BIT_GET(bitmap, i));
        assert(BIT_GET(bitmap, i) != 0);

        BIT_DEL(bitmap, i);
        assert(BIT_GET(bitmap, i) == 0);
    }
}
