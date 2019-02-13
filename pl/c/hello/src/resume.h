#ifndef __RESUME_H
#define __RESUME_H

#define MAX(x, y) ((x) > (y) ? (x) : (y))

static inline void swap(int *x, int *y) {
    /*
    int tmp = *x;
    *x = *y;
    *y = tmp;

    *x = *x + *y;
    *y = *x - *y;
    *x = *x - *y;
    */

    *x = *x ^ *y;
    *y = *x ^ *y;
    *x = *x ^ *y;
}

// array
#define MAX_SIZE 100
#define ARRSIZE(arr) (sizeof(arr)/ sizeof(arr[0]))

static inline void FatalError(int condition, const char *msg) {
    if (!condition) {
        printf("%s\n", msg);
        assert(0);
    }
}

void array_print(int arr[], int n);
void array_print_nr(int n);
int array_is_asc(int arr[], int n);
void array_reverse(int arr[], int n);

int myarray_sum(int arr[], int start, int len);
int sum1(int arr[], int n);
int sum2(int arr[], int n);
int sum3(int arr[], int n);

char *my_strcpy(char *dst, const char *src);
void *my_memcpy(void *dst, const void *src, int n);
int string_palindrome(const char *s);
void string_perm(char *s);

int bisearch1(int arr[], int n, int x);
int bisearch2(int arr[], int left, int right, int x);
void quick_sort(int arr[], int low, int high);
void merge_sort(int arr[], int left, int right);

#endif
