#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "assert.h"
#include "math.h"

#include "resume.h"

#define BISEARCH_EXACT 0
#define BISEARCH_FIRST 1
#define BISEARCH_LAST  2

int bisearch1(int arr[], int n, int x) {
    int left = 0;
    int right = n - 1;
    int mid = 0;

    while (left <= right) {
        mid = left + (right - left) / 2;
        if (x == arr[mid])
            return mid;
        else if (x < arr[mid])
            right = mid - 1;
        else
            left = mid + 1;
    }

    return -1;
}

int bisearch2(int arr[], int left, int right, int x) {
    if (left <= right) {
        int mid = left + (right - left) / 2;
        if (x == arr[mid])
            return mid;
        else if (x < arr[mid])
            return bisearch2(arr, left, mid - 1, x);
        else
            return bisearch2(arr, mid + 1, right, x);
    }

    return -1;
}

int bisearch3(int arr[], int left, int right, int x, int flag) {
    if (arr == NULL)
        return -1;

    int mid = -1;
    while (left <= right) {
        mid = left + (right - left) / 2;

        if (x == arr[mid]) {
            if (flag == BISEARCH_EXACT) {
                return mid;
            } else if (flag == BISEARCH_FIRST) { // first
                // TODO
                if (mid == left || arr[mid - 1] != x)
                    return mid;
                else
                    right = mid - 1;
            } else { // last
                if (mid == right || arr[mid + 1] != x)
                    return mid;
                else
                    left = mid + 1;
            }
        } else if (x < arr[mid]) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }

    return -1;
}

int quicksort_partition(int arr[], int low, int high) {
    int r = low + rand() % (high - low + 1);
    swap(&arr[low], &arr[r]);

    // printf("r %d\n", r);

    int left = low, right = high, pivot_value = arr[low];

    while (left < right) {
        while (arr[left] <= pivot_value)
            left++;
        while (arr[right] > pivot_value)
            right --;

        if (left < right) {
            swap(&arr[left], &arr[right]);
        }
    }

    // arr[low] = arr[right];
    // arr[right] = pivot_value;
    swap(&arr[low], &arr[right]);
    return right;
}

void quick_sort(int arr[], int low, int high) {
    if (low < high) {
        int pivot = quicksort_partition(arr, low, high);
        quick_sort(arr, low, pivot - 1);
        quick_sort(arr, pivot + 1, high);
    }
}

void merge_core(int arr[], int tmp[], int left, int mid, int right) {
    int size = right - left + 1;
    int right_idx = mid + 1;
    int tmp_idx = left;

    while (left <= mid && right_idx <= right) {
        if (arr[left] <= arr[right_idx]) {
            tmp[tmp_idx++] = arr[left++];
        } else {
            tmp[tmp_idx++] = arr[right_idx++];
        }
    }

    while (left <= mid) {
        tmp[tmp_idx++] = arr[left++];
    }

    while (right_idx <= right) {
        tmp[tmp_idx++] = arr[right_idx++];
    }

    for (int i=0; i < size; i++) {
        arr[right] = tmp[right];
        right--;
    }
}

void merge_sort_core(int arr[], int tmp[], int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        merge_sort_core(arr, tmp, left, mid);
        merge_sort_core(arr, tmp, mid + 1, right);
        merge_core(arr, tmp, left, mid, right);
    }
}

void merge_sort(int arr[], int left, int right) {
    if (left >= 0 && left < right) {
        int *tmp = malloc(sizeof(int) * (right - left + 1));
        FatalError(tmp != NULL, "OOM");

        merge_sort_core(arr, tmp, left, right);

        free(tmp);
    }
}

int gcd(int m, int n) {
    int r = m % n;
    if (r == 0)
        return n;
    else
        return gcd(n, r);
}

int fib1(int n) {
    if (n == 0 || n == 1)
        return n;
    return fib1(n - 2) + fib1(n - 1);
}

int fib2(int n) {
    if (n == 0 || n == 1)
        return n;

    int twoback = 0;
    int oneback = 1;
    int current;
    for (int i=2; i <= n; i++) {
        current = twoback + oneback;
        twoback = oneback;
        oneback = current;
    }

    return current;
}

int fib3_impl(int n, int a, int b) {
    if (n == 0)
        return a + b;

    return fib3_impl(n - 1, b, a + b);
}

int fib3(int n) {
    if (n == 1)
        return 0;
    else if (n == 2)
        return 1;

    return fib3_impl(n - 2, 0, 1);
}

int __sum1_impl(int arr[], int n, int sum) {
    if (n == 0)
        return sum;
    return __sum1_impl(arr, n - 1, arr[n-1] + sum);
}

int __sum1(int arr[], int n) {
    return __sum1_impl(arr, n, 0);
}

void test_algo() {
    int arr[] = {1, 3, 5, 7, 7, 7, 9};
    int arr_size = ARRSIZE(arr);

    ASSERT_EQUAL(bisearch3(arr, 0, arr_size - 1, 1, BISEARCH_FIRST), 0);
    ASSERT_EQUAL(bisearch3(arr, 0, arr_size - 1, 1, BISEARCH_LAST), 0);

    ASSERT_EQUAL(bisearch3(arr, 0, arr_size - 1, 5, BISEARCH_FIRST), 2);
    ASSERT_EQUAL(bisearch3(arr, 0, arr_size - 1, 5, BISEARCH_LAST), 2);

    ASSERT_EQUAL(bisearch3(arr, 0, arr_size - 1, 7, BISEARCH_FIRST), 3);
    ASSERT_EQUAL(bisearch3(arr, 0, arr_size - 1, 7, BISEARCH_LAST), 5);

    ASSERT_EQUAL(bisearch3(arr, 0, arr_size - 1, 10, BISEARCH_FIRST), -1);
    ASSERT_EQUAL(bisearch3(arr, 0, arr_size - 1, 10, BISEARCH_LAST), -1);

    for (int i=3; i < 10; i++) {
        ASSERT_EQUAL(fib1(i), fib2(i));
        ASSERT_EQUAL(fib1(i), fib3(i));
    }

    ASSERT_EQUAL(__sum1(arr, arr_size), sum1(arr, arr_size));
}
