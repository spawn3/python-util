#include "stdio.h"
#include "stdlib.h"
#include "assert.h"
#include "string.h"

#include "resume.h"

char *my_strcpy(char *dst, const char *src) {
    char *old_dst = dst;

    if (src != NULL && dst != NULL) {
        while (*src != '\0') {
            *dst++ = *src++;
        }
        *dst = '\0';
    }

    return old_dst;
}

void *my_memcpy(void *dst, const void *src, int n) {
    if (dst == NULL || src == NULL) {
        return dst;
    }

    if (dst == src)
        return dst;

    void *old_dst = dst;
    for (int i=0; i < n; i++) {
        *(char *)dst++ = *(char *)src++;
    }

    return old_dst;
}

int string_palindrome_impl(const char *s, int left, int right) {
    while (left < right) {
        if (s[left++] != s[right--])
            return 0;
    }
    return 1;
}

int string_palindrome(const char *s) {
    int n = strlen(s);
    if (n <= 1)
        return 1;

    return string_palindrome_impl(s, 0, n-1);
}

void string_perm_impl(char *s, int start, int end) {
    if (start == end) {
        printf("%s\n", s);
    } else {
        char tmp;
        for (int i=start; i < end; i++) {
            if (i != start) {
                tmp = s[start];
                s[start] = s[i];
                s[i] = tmp;
            }

            string_perm_impl(s, start + 1, end);

            if (i != start) {
                tmp = s[start];
                s[start] = s[i];
                s[i] = tmp;
            }
        }
    }

}

void string_perm(char *s) {
    int n = strlen(s);
    if (n == 0)
        return;
    string_perm_impl(s, 0, n);
}
