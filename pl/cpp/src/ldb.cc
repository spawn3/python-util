#include <cassert>
#include "leveldb/db.h"

void test_ldb() {
    leveldb::DB *db;
    leveldb::Options options;
    options.create_if_missing = true;

    leveldb::Status status = leveldb::DB::Open(options, "/tmp/testdb", &db);
    assert(status.ok());

}

int main() {
    return 0;
}
