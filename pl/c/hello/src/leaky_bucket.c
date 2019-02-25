#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "token_bucket.h"

int leaky_bucket_init(leaky_bucket_t *lb, uint64_t capacity, uint64_t rate)
{
    assert(capacity > 0);
    assert(rate > 0);

    lb->rate = rate;
    lb->capacity = capacity;
    lb->water = 0;
    lb->t0 = z_gettimeofday();
    lb->prev_t = 0;

    lb->inited = 1;

        printf("t0 %jd\n", lb->t0);

    return 0;
}

static uint64_t get_time_splice(leaky_bucket_t *lb, uint64_t now) {
    // printf("prev %jd now %jd\n", lb->t0, now);
    if (now == 0) {
        return 0;
    } else  {
        return (now - lb->t0) * lb->rate / 1000000;
    }
}

void leaky_bucket_destroy(leaky_bucket_t *lb)
{
    lb->inited = 0;
}

int leaky_bucket_take(leaky_bucket_t *lb, suseconds_t now, int *is_ready, uint64_t *delay)
{
    *is_ready = 0;
    (void) delay;

    assert(lb->inited);

    uint64_t ts1 = get_time_splice(lb, lb->prev_t);
    uint64_t ts2 = get_time_splice(lb, now);
    // printf("ts1 %jd ts2 %jd\n", ts1, ts2);
    if (ts1 < ts2) {
        if (lb->water > 0) {
            lb->prev_t = now;
            lb->water--;
            *is_ready = 1;
            goto out;
        }
    }

    if (lb->water + 1 <= lb->capacity) {
        lb->water++;
    }

#if 0
    printf("time %jd cap %ju rate %ju water %0.2f leak %0.2f ready %d\n",
            lb->t1, lb->capacity, lb->rate, lb->water, numberToLeak, *is_ready);
#endif

out:
    return 0;
}

void test_leaky_bucket() {
    int is_ready;

    uint64_t SEC2USEC = 1000000;

    leaky_bucket_t lb;
    leaky_bucket_init(&lb, 100000, 100);

    uint64_t t0, t1, t2;
    uint64_t success = 0, fail = 0;

    t0 = z_gettimeofday();
    t1 = t0;
    t2 = t0;

    srand(t0);

    while (1) {
        if (rand() % 1000 == 0) {
            usleep(100 * 1000);
        }

        t2 = z_gettimeofday();
        leaky_bucket_take(&lb, t2, &is_ready, NULL);
        if (!is_ready) {
            usleep(100);
            fail ++;
            continue;
        }

        success ++;

        if (t2 - t1 >= SEC2USEC) {
            printf("success %jd fail %jd rate %jd\n", success, fail, success * SEC2USEC / (t2 - t1));
            success = 0;
            fail = 0;
            t1 = t2;
        }
    }
}
