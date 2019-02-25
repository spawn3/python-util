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

/**
 * @todo use lock-free/CAS
 *
 */

int token_bucket_init(token_bucket_t *bucket, const char *name,
        uint64_t capacity, uint64_t rate, int private, int leaky)
{
    strcpy(bucket->name, name);
    bucket->capacity = capacity;
    bucket->tokens = capacity;
    bucket->rate = rate;

    bucket->priv = private;
    bucket->inited = 1;

    // DWARN("name %s private %d leaky %d capacity %0.1f rate %0.1f burst %ju\n",
    //       name, bucket->priv, bucket->include_leaky, bucket->capacity, bucket->rate, bucket->burst_max);

    return 0;
}

void token_bucket_destroy(token_bucket_t *bucket)
{
    if (bucket->inited) {
        bucket->inited = 0;
    }
}

int token_bucket_set(token_bucket_t *bucket, const char *name, uint64_t capacity, uint64_t rate, int private, int leaky)
{
    if (!bucket->inited) {
        token_bucket_init(bucket, name, capacity, rate, private, leaky);
    } else {
        bucket->capacity = capacity;
        bucket->rate = rate;

        //printf("private %d capacity %0.1f rate %0.1f\n",
        //      bucket->priv, bucket->capacity, bucket->rate);
    }

    return 0;
}

static inline int _token_bucket_update_tokens(token_bucket_t *bucket, uint64_t now)
{
    double delta = 0.0;

    now = z_gettimeofday();
    if (bucket->tokens < bucket->capacity) {
        /*　如果系统时间修改， delta　可能时负数*/
        delta = 1.0 * bucket->rate * (now - bucket->t1) / USECONDS_PER_SEC;
        bucket->tokens += fabs(delta);
    }

    // tokens不能大于capactiy
    if (bucket->tokens > bucket->capacity) {
        bucket->tokens = bucket->capacity;
    }

    // TODO float point?
    assert(bucket->tokens >= 0 && bucket->tokens <= bucket->capacity + 1e-6);
#if 0
    printf("timestamp %ju, capacity %0.2f, rate %0.2f, tokens %0.6f, delta %0.6f\n",
            now, bucket->capacity, bucket->rate, bucket->tokens, delta);
#endif

    bucket->t1 = now;

    return 0;
}

int token_bucket_consume(token_bucket_t *bucket, uint64_t n, int *is_ready, uint64_t *delay)
{
    *is_ready = 0;

    uint64_t now = z_gettimeofday();

    // Limit outflow speed
#if 0
    double delta = 1.0 * bucket->burst_max * (now - bucket->t2) / USECONDS_PER_SEC;
    if ((delta < n)) {
        printf("token bucket overflow, wait burst_max:%f\n", bucket->burst_max);
        goto out;
    }
#endif

    _token_bucket_update_tokens(bucket, now);

    if (n <= (int)bucket->tokens) {
        // TODO 增加条件: 控制最新采样周期的消费量

        bucket->tokens -= n;

        // 消耗成功， 更新上次消耗token时间
        bucket->t2 = now;

        *is_ready = 1;
    } else {
        uint64_t _delay = (uint64_t)(USECONDS_PER_SEC * (n - bucket->tokens) / bucket->rate);
        // printf("n %jd tokens %.2f rate %.2f delay %ju\n", n, bucket->tokens, bucket->rate, _delay);

        if (delay) {
            *delay = _delay;
        }
    }

    return 0;
}

int token_bucket_consume_loop(token_bucket_t *tb, uint64_t n, int us, int retry_max)
{
    int ret = 0, is_ready = 0, retry = 0;

    while (1) {
#if 0
        if (retry_max > 0 && retry > retry_max) {
            ret = EAGAIN;
            // printf("max %d retry %d ret %d\n", retry_max, retry, ret);
            break;
        }
#endif

        retry++;

        token_bucket_consume(tb, n, &is_ready, NULL);
        if ((is_ready)) {
            break;
        } else {
            usleep(us);
            continue;
        }
    }

    return ret;
}

int token_bucket_inc(token_bucket_t *bucket, uint64_t n)
{
    bucket->tokens += n;

    if (bucket->tokens > bucket->capacity) {
        bucket->tokens = bucket->capacity;
    }

    return 0;
}

void test_token_bucket() {
    int ret;
    uint64_t success = 0, fail = 0;
    token_bucket_t tb;

    token_bucket_init(&tb, "test", 10, 100, 1, 0);

    uint64_t t0, t1, t2;

    t0 = z_gettimeofday();
    t1 = t0;
    t2 = t0;

    srand(t0);

    uint64_t SEC2USEC = 1000000;

    while (1) {
        if (rand() % 100 == 0) {
            usleep(100 * 1000);
        }

        ret = token_bucket_consume_loop(&tb, 1, 100, 10);
        if (ret == 0) {
            success++;
        } else {
            fail++;
        }

        t2 = z_gettimeofday();
        if (t2 - t1 >= SEC2USEC) {
            printf("success %jd fail %jd rate %jd\n", success, fail, success * SEC2USEC / (t2 - t1));
            success = 0;
            fail = 0;
            t1 = t2;
        }
    }

    token_bucket_destroy(&tb);
}
