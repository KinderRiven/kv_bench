#include "db.h"
#include "rocksdb.h"

using namespace kv_benchmark;

void DB::Open(Options& options, DB** dbptr)
{
    *dbptr = new RocksDB(options);
}