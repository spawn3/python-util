#include "stdio.h"
#include "stdlib.h"
#include "assert.h"

#include "resume.h"

void array_print(int arr[], int n) {
    for (int i=0; i < n; i++) {
        printf("%4d %d\n", i, arr[i]);
    }
}

int array_is_asc_impl(int arr[], int start, int len) {
    if (len < 2)
        return 1;

    if (arr[start] >= arr[start+1])
        return 0;

    return array_is_asc_impl(arr, start + 1, len - 1);
}

int array_is_asc(int arr[], int n) {
    return array_is_asc_impl(arr, 0, n);
}

void array_print_nr(int n) {
    if (n < 1)
        return;
    else if (n == 1) {
        printf("%d\n", n);
    } else {
        array_print_nr(n-1);
        printf("%d\n", n);
    }
}

void array_reverse_impl(int arr[], int left, int right) {
    if (left < right) {
        swap(&arr[left], &arr[right]);
        array_reverse_impl(arr, left + 1, right - 1);
    }
}

void array_reverse(int arr[], int n) {
    /*
    for (int i=0; i < n/2; i++) {
        swap(&arr[i], &arr[n-1-i]);
    }

    int i=0, j = n-1;
    while (i < j) {
        swap(&arr[i++], &arr[j--]);
    }
    */

    array_reverse_impl(arr, 0, n - 1);
}


int myarray_sum_impl1(int arr[], int start, int len) {
    if (len == 0)
        return 0;
    return arr[start] + myarray_sum_impl1(arr, start + 1, len - 1);
}


int myarray_sum(int arr[], int start, int len) {
    return myarray_sum_impl1(arr, start, len);
}

int sum1(int arr[], int n) {
    int sum = 0;
    for (int i=0; i < n; i++)
        sum += arr[i];

    // printf("sum %d\n", sum);
    return sum;
}

// from left to right
// use [begin, end)
// NOT use accumulator
int __sum2_impl2(int arr[], int begin, int end) {
    if (begin == end)
        return 0;

    return arr[begin] + __sum2_impl2(arr, begin + 1, end);
}

// from left to right
// use accumulator
int __sum2_impl1(int arr[], int idx, int end, int sum) {
    if (idx == end)
        return sum;

    return __sum2_impl1(arr, idx + 1, end, arr[idx] + sum);
}

int sum2(int arr[], int n) {
    // return __sum2_impl1(arr, 0, n, 0);
    return __sum2_impl2(arr, 0, n);
}

// from right to left
int __sum3(int arr[], int len, int sum) {
    if (len == 0)
        return sum;

    return __sum3(arr, len - 1, sum + arr[len - 1]);
}

int sum3(int arr[], int n) {
    return __sum3(arr, n, 0);
}

void test_array() {
    array_print_nr(10);

    int arr[] = {20, 10, 30, 40, 1};
    int arr_size = ARRSIZE(arr);

    assert(array_is_asc(arr, arr_size) == 0);

    assert(sum1(arr, arr_size) == 101);
    assert(sum2(arr, arr_size) == 101);
    assert(sum3(arr, arr_size) == 101);

    assert(myarray_sum(arr, 0, arr_size) == 101);

    for (int i=0; i < 10; i++) {
        assert(fab1(i) == fab2(i));
    }

    quick_sort(arr, 0, 4);
    // merge_sort(arr, 0, 4);
    array_print(arr, arr_size);

    assert(bisearch1(arr, arr_size, 1) == 0);
    assert(bisearch1(arr, arr_size, 10) == 1);
    assert(bisearch1(arr, arr_size, 20) == 2);
    assert(bisearch1(arr, arr_size, 30) == 3);
    assert(bisearch1(arr, arr_size, 40) == 4);
    assert(bisearch1(arr, arr_size, 8) == -1);
    assert(bisearch1(arr, arr_size, 50) == -1);

    assert(bisearch2(arr, 0, arr_size - 1, 1) == 0);
    assert(bisearch2(arr, 0, arr_size - 1, 10) == 1);
    assert(bisearch2(arr, 0, arr_size - 1, 20) == 2);
    assert(bisearch2(arr, 0, arr_size - 1, 30) == 3);
    assert(bisearch2(arr, 0, arr_size - 1, 40) == 4);
    assert(bisearch2(arr, 0, arr_size - 1, 8) == -1);
    assert(bisearch2(arr, 0, arr_size - 1, 50) == -1);

    int arr2[] = {1, 10, 20, 30, 40};
    int arr_size2 = ARRSIZE(arr2);
    assert(array_is_asc(arr2, arr_size2) == 1);

    array_reverse(arr2, arr_size2);
    assert(arr2[0] == 40);
    assert(arr2[1] == 30);
    assert(arr2[2] == 20);
    assert(arr2[3] == 10);
    assert(arr2[4] == 1);

    array_reverse(arr2, arr_size2);
    assert(arr2[0] == 1);
    assert(arr2[1] == 10);
    assert(arr2[2] == 20);
    assert(arr2[3] == 30);
    assert(arr2[4] == 40);
}

