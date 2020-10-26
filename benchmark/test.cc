#include "db.h"
#include "header.h"
#include "options.h"

using namespace kv_benchmark;

int main(int argc, char** argv)
{
    DB* _db;
    Options _options;
    DB::Open(_options, &_db);

    char _key[128];
    char _value[256];
    size_t _key_length = 16;
    size_t _value_length = 128;

    for (int i = 0; i < 100000; i++) {
        sprintf(_key, "%llu", i);
        sprintf(_value, "%llu", i);
        _db->Put(_key, _key_length, _value, _value_length);
    }
    DB::Close(_db);
    return 0;
}