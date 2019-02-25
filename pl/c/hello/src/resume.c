#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "assert.h"
#include "math.h"

#include "resume.h"

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

    printf("\ntest algo:\n");
    test_algo();

    printf("\ntest queue:\n");
    test_queue();

    printf("\ntest bit:\n");
    test_bit();

    printf("\ntest graph:\n");
    test_graph();

    // test_token_bucket();
    test_leaky_bucket();

    return 0;
}
