#include "db.h"
#include "leveldb.h"

using namespace kv_benchmark;

void DB::Open(Options& options, DB** dbptr)
{
    *dbptr = new LevelDB(options);
}

void DB::Close(DB* dbptr)
{
    delete dbptr;
}