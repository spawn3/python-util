#include "stdio.h"
#include "assert.h"

#include "resume.h"

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
}
