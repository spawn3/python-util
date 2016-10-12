#include <stdio.h>

#include "hello.h"


int sum1(int *arr, int count, int i) {
	if (i == count - 1) {
		return arr[i];
	} else {
		return arr[i] + sum1(arr, count, i+1);
	}
}


int sum(int *arr, int count) {
	return sum1(arr, count, 0);
}


int f1(int a, int b) {
	return a + b;
}

int f2(int a, int b) {
	return a - b;
}


int main(int argc, char **argv) {
	printf("argc %d\n", argc);
	for (int i=0; i < argc; ++i) {
		printf("argv[%d] %s\n", i, argv[i]);
	}
	printf("Hello, world\n");
	test_sizeof();

	int array[] = {1,2,3,4,5};
	printf("sum = %d\n", sum(array, 5));

	int (*fp)(int, int);
	int (*fp_array[])(int, int) = {f1, f2};

	for(int i=0; i < 2; ++i) {
		fp = fp_array[i];
		printf("1 x 2 = %d\n", fp(1, 2));
	}	

	return 0;
}
