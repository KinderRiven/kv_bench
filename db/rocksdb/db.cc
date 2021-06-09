/*
 * @Date: 2021-03-18 11:55:19
 * @LastEditors: Han Shukai
 * @LastEditTime: 2021-06-09 15:06:13
 * @FilePath: /kv_bench/db/rocksdb/db.cc
 */
#include "db.h"
#include "rocksdb.h"

using namespace kv_benchmark;

void DB::Open(Options& options, DB** dbptr)
{
    *dbptr = new RocksDB(options);
}

void DB::Close(DB* dbptr)
{
    delete dbptr;
}