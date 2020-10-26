#include "db.h"

using namespace kv_benchmark;

int main(int argc, char** argv)
{
    DB* _db;
    Options _options;
    DB::Open(_options, &_db);
    return 0;
}