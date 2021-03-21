/*
 * @Author: your name
 * @Date: 2021-03-18 11:55:19
 * @LastEditTime: 2021-03-20 20:16:25
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: /kv_bench/benchmark/test.cc
 */
#include "db.h"
#include "header.h"
#include "options.h"

using namespace kv_benchmark;

int main(int argc, char** argv)
{
    DB* _db;
    Options _options;
    DB::Open(_options, &_db);

    bool _result;
    char _key[128] = { 0 };
    char _value[4096] = { 0 };
    size_t _key_length = 16;
    size_t _value_length = 1024;
    uint64_t _cnt = 0;

    for (int i = 0; i < 1000000; i++) {
        *(uint64_t*)_key = (uint64_t)i;
        *(uint64_t*)_value = (uint64_t)i;
        _db->Put(_key, _key_length, _value, _value_length);
    }
    for (int i = 0; i < 1000000; i++) {
        *(uint64_t*)_key = (uint64_t)i;
        *(uint64_t*)_value = 0;
        _result = _db->Get(_key, _key_length, _value, _value_length);
        if (_result) {
            uint64_t __v = *((uint64_t*)_value);
            assert(__v == i);
            _cnt++;
        }
    }
    printf("SUCCESS:%llu/%.2f%%\n", _cnt, 100.0 * _cnt / 1000000);
    DB::Close(_db);
    return 0;
}