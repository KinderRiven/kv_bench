#include "db.h"
#include "pebblesdb.h"

using namespace kv_benchmark;

void DB::Open(Options& options, DB** dbptr)
{
    *dbptr = new PebblesDB(options);
}

void DB::Close(DB* dbptr)
{
    delete dbptr;
}