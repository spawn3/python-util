#ifndef __YLIB_TOKEN_BUCKET_H
#define __YLIB_TOKEN_BUCKET_H

#include <sys/types.h>
#include <stdint.h>
#include <time.h>

#include "pthread.h"

// #include "ylock.h"


#define USECONDS_PER_SEC (1000000)

static inline uint64_t z_gettimeofday() {
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return tv.tv_sec * 1000000 + tv.tv_usec;
}

#define _max(x, y) ((x) >= (y) ? (x) : (y))


typedef enum {
        TOKEN_BUCKET_POLICY_A = 0,
        TOKEN_BUCKET_POLICY_B = 1,
} token_bucket_policy_t;

typedef struct {
        uint64_t t;
        uint64_t consume;
} tb_sample_t;

typedef struct {
        char name[256];
        int inited;

        uint64_t rate;
        uint64_t capacity;
        uint64_t water;

        uint64_t t0;
        uint64_t prev_t;
} leaky_bucket_t;

typedef struct {
        char name[256];
        int inited;

        int priv;

        double rate;
        double capacity;
        double tokens;

        uint64_t t1;   ///上次token更新时间
        uint64_t t2;   ///上次消耗token时间

        // int include_leaky;
        // leaky_bucket_t lb;

} token_bucket_t;

typedef struct {
        long avg;      // fill rate
        long burst_max;
        int burst_time;

        uint64_t rate_expire; //usec
        uint64_t level_expire;

        long capacity; /* max * time */
        long level;    // tokens
        long rate;     // current value
} level_bucket_t;

/* token bucket */
int token_bucket_init(token_bucket_t *bucket, const char *name, uint64_t capacity, uint64_t rate, int private, int leaky);
int token_bucket_set(token_bucket_t *bucket, const char *name, uint64_t capacity, uint64_t rate, int private, int leaky);
void token_bucket_destroy(token_bucket_t *bucket);

int token_bucket_consume(token_bucket_t *bucket, uint64_t n, int *is_ready, uint64_t *delay);
int token_bucket_consume_loop(token_bucket_t *tb, uint64_t n, int us, int retry);

int token_bucket_inc(token_bucket_t *bucket, uint64_t n);

// leaky bucket

int leaky_bucket_init(leaky_bucket_t *lb, uint64_t capacity, uint64_t rate);
void leaky_bucket_destroy(leaky_bucket_t *lb);

int leaky_bucket_set(leaky_bucket_t *lb, uint64_t capacity, uint64_t rate);
int leaky_bucket_take(leaky_bucket_t *lb, suseconds_t now, int *is_ready, uint64_t *delay);

#endif
