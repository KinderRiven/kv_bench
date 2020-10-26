#include "db.h"
#include "matrixkv.h"

using namespace kv_benchmark;

void DB::Open(Options& options, DB** dbptr)
{
    *dbptr = new MatrixKV(options);
}

void DB::Close(DB* dbptr)
{
    delete dbptr;
}