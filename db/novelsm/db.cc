#include "db.h"
#include "novelsm.h"

using namespace kv_benchmark;

void DB::Open(Options& options, DB** dbptr)
{
    *dbptr = new NoveLSM(options);
}

void DB::Close(DB* dbptr)
{
    delete dbptr;
}