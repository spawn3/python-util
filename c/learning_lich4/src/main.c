#include <stdio.h>
#include <sys/param.h>

#include "config.h"

#define DBG_SUBSYS S_LIBINTERFACE

#include "configure.h"
#include "dbg.h"
#include "lichbd.h"
#include "utils.h"

#include "buffer.h"

static inline void chunk_bound(uint64_t fsize, uint32_t idx, uint64_t *offset, int *size) {
        *offset = idx * LICH_CHUNK_SPLIT;
        *size = MIN(MAX(fsize - *offset, 0), LICH_CHUNK_SPLIT);
}

void test_page_bound() {
        uint64_t offset;
        int size;

        chunk_bound(5, 0, &offset, &size);

        printf("offset = %lu, size = %d\n", offset, size);
}

void test_buffer() {
        buffer_t buf;

        mbuffer_init(&buf, 0);

        mbuffer_appendmem(&buf, "abc", 3);

        printf("first: %s\n", (char *)mbuffer_head(&buf));

        mbuffer_free(&buf);
}

void test_lichbd() {
        int ret;
        ret = lichbd_init("");
        if (ret) {
                GOTO(err_ret, ret);
        }

        ret = utils_cat("/test/v4clean.sh");
        if (ret) {
                GOTO(err_ret, ret);
        }
err_ret:
        (void)0;
}


int main() {
        int ret;

        printf("Hello, world\n");
        test_page_bound();

        // mem_cache_init();
        // mem_hugepage_init();
        ret = env_init_simple("test");
        if (ret) {
                goto err_ret;
        }

        printf("-------------------------------------\n");
        test_buffer();

        return 0;
err_ret:
        return ret;
}
