#ifndef INCLUDE_DB_BENCH_H_
#define INCLUDE_DB_BENCH_H_

#include "random.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DBBENCH_NUM_THREAD (16)
#define DBBENCH_KEY_LENGTH (8)
#define DBBENCH_VALUE_LENGTH (4096)

#define DBBENCH_NUM_OPT_TYPE (4)
#define DBBENCH_PUT (0)
#define DBBENCH_GET (1)
#define DBBENCH_DELETE (2)
#define DBBENCH_SCAN (3)

namespace kv_benchmark {
// Only Support Single Thread
class DBBench {
public:
    // key range is [base_key, base_key + key_range]
    DBBench(int type, uint32_t seed, size_t key_length, size_t value_length)
        : type_(type)
        , key_length_(key_length)
        , value_length_(value_length)
    {
        random_ = new Random(seed);
    }

    ~DBBench()
    {
    }

public:
    bool initlizate()
    {
        return true;
    }

private:
    void generate_kv_pair(uint64_t uid, char* key, char* value)
    {
        *((uint64_t*)key) = uid;
        *((uint64_t*)value) = uid;
    }

    int random_get_put()
    {
        return type_;
    }

public:
    int get_kv_pair(char* key, size_t& key_length, char* value, size_t& value_length)
    {
        generate_kv_pair((uint64_t)random_->Next(), key, value);
        key_length = key_length_;
        value_length_ = value_length_;
        return random_get_put();
    }

public:
    Random* random_;

    int type_;

    size_t key_length_;

    size_t value_length_;
};
};

#endif