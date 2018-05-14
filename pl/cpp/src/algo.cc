#include <cstdio>
#include <cassert>

int swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;

    return 0;
}

int partition(int *a, int p, int r) {
    int x = a[r];
    int i = p - 1;
    for (int j=p; j <= r - 1; j++) {
        if (a[j] <= x) {
            i++;
            swap(&a[i], &a[j]);
        }
    }

    swap(&a[i+1], &a[r]);
    return i+1;
}

// [p, r]
int quick_sort(int *a, int p, int r) {
    if (p < r) {
        int q = partition(a, p, r);
        quick_sort(a, p, q-1);
        quick_sort(a, q+1, r);
    }

    return 0;
}

void test_qsort() {
    int a[] = { 8, 7, 6, 5, 1, 3, 2, 4 };

    quick_sort(a, 0, 7);

    for (int i=0; i < 8; i++) {
        printf("%d\n", a[i]);
    }
}


int main() {
    test_qsort();
    return 0;
}
