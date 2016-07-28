#include <stdio.h>

#include "config.h"

#define DBG_SUBSYS S_LIBINTERFACE

#include "configure.h"
#include "dbg.h"
#include "lichbd.h"
#include "utils.h"

// #include "hello.h"

int main() {
        int ret;
        printf("Hello, world\n");

        ret = lichbd_init("");
        if (ret) {
                GOTO(err_ret, ret);
        }

        ret = utils_cat("/test/v4clean.sh");
        if (ret) {
                GOTO(err_ret, ret);
        }

        return 0;
err_ret:
        return ret;
}
