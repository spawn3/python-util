#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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


int normalize_path(const char *path, char *path2) {
        int i, len, begin, off;

        len = strlen(path);

        off = 0;
        begin = -1;
        for(i = 0; i < len; ++i) {
                if (path[i] == '/') {
                        if (begin == -1) {
                                continue;
                        }

                        path2[off++] = '/';
                        strncpy(path2 + off, path + begin, i - begin);
                        off += i - begin;
                        // stop a segment
                        begin = -1;
                } else {
                        if (begin == -1) {
                                // start a new segment
                                begin = i;
                        }
                }
        }

        if (begin != -1 && begin < i) {
                path2[off++] = '/';
                strncpy(path2 + off, path + begin, i - begin);
                off += i - begin;
        }

        path2[off] = '\0';
        printf("path %s\n", path2);
        return 0;
}


static int __make_sure_disk_forv3(char *disk)
{
        char newdisk[64];
        char split = '/';
        char *pos;

        pos = strchr(disk, split);
        if (pos) {
        } else {
                snprintf(newdisk, 64, "%s/0", disk);
                strcpy(disk, newdisk);
        }

        return 0;
}


int main(int argc, char **argv) {
        printf("argc %d\n", argc);
        for (int i=0; i < argc; ++i) {
                printf("argv[%d] %s\n", i, argv[i]);
        }

        char disk[64] = "v4";
        __make_sure_disk_forv3(disk);
        printf("disk = %s\n", disk);

        char path[256];
        normalize_path("//aa", path);
        normalize_path("//aa//bb//cc", path);
        normalize_path("//aa//bb//////cccc//", path);

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

        int *pi = malloc(sizeof(int));
        printf("pi %p\n", pi);

        test_basic();

        return 0;
}
