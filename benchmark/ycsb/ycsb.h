#ifndef INCLUDE_YCSB_BENCHMARK_H_
#define INCLUDE_YCSB_BENCHMARK_H_

#include "random.h"
#include "workload_ycsb.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define YCSB_NUM_THREAD (16)
#define YCSB_KEY_LENGTH (8)
#define YCSB_VALUE_LENGTH (4096)

#define YCSB_NUM_OPT_TYPE (6)
#define YCSB_PUT (0)
#define YCSB_UPDATE (1)
#define YCSB_GET (2)
#define YCSB_DELETE (3)
#define YCSB_SCAN (4)
#define YCSB_RMW (5)

#define YCSB_ZIPFAN (1)
// YCSB-A: 50% updates, 50% reads
#define YCSB_A (0 << 1)
// YCSB-B: 5% updates, 95% reads
#define YCSB_B (1 << 1)
// YCSB-C: 100% reads
#define YCSB_C (2 << 1)
// YCSB-E: 5% updates, 95% scans (50 items)
#define YCSB_D (3 << 1)
// YCSB-D: 5% updates, 95% lastest-reads
#define YCSB_E (4 << 1)
// YCSB-F: 50% read-modify-write, 50% reads
#define YCSB_F (5 << 1)
// YCSB-Load-A: 100% writes
#define YCSB_RANDOM_LOAD (6 << 1)
// SEQ load
#define YCSB_SEQ_LOAD (7 << 1)
// 100% write
#define YCSB_ONLY_WRITE (8 << 1)

namespace kv_benchmark {
// Only Support Single Thread
class YCSB {
public:
    // key range is [base_key, base_key + key_range]
    YCSB(int type, uint64_t base_key, size_t key_range, size_t key_length, size_t value_length)
        : type_(type & (~1))
        , zipfan_(type & 1)
        , base_key_(base_key)
        , key_range_(key_range)
        , key_length_(key_length)
        , value_length_(value_length)
        , sequential_key_id_(0)
    {
        printf(">>[YCSB] CREATE A NEW YCSB BENCHMARK!\n");
        init_zipf_generator(1, key_range_);
        printf("  [TYPE/ZIPFAN:%d/%d][KV_LENGTH:%zu/%zu][RANGE:(%llu,%llu)]\n", type_, zipfan_, key_length_, value_length_, base_key_, base_key_ + key_range_);
    }

    ~YCSB()
    {
    }

public:
    bool initlizate()
    {
        init_seed();
        return true;
    }

private:
    void generate_kv_pair(uint64_t uid, char* key, char* value)
    {
        uid += base_key_;
        *((uint64_t*)key) = uid;
        *((uint64_t*)value) = uid;
    }

    int random_get_put(int test)
    {
        long random = uniform_next() % 100;

        switch (test) {
        case YCSB_SEQ_LOAD:
            return YCSB_PUT;
        case YCSB_RANDOM_LOAD:
            return YCSB_PUT;
        case YCSB_ONLY_WRITE:
            return YCSB_UPDATE;
        case YCSB_A:
            return (random >= 50) ? YCSB_UPDATE : YCSB_GET;
        case YCSB_B:
            return (random >= 95) ? YCSB_UPDATE : YCSB_GET;
        case YCSB_C:
            return YCSB_GET;
        case YCSB_D:
            return (random >= 95) ? YCSB_UPDATE : YCSB_GET;
        case YCSB_E:
            return (random >= 95) ? YCSB_PUT : YCSB_SCAN;
        case YCSB_F:
            return (random >= 50) ? YCSB_RMW : YCSB_GET;
        default:
            return -1;
        }
    }

public:
    int get_kv_pair(char* key, size_t& key_length, char* value, size_t& value_length)
    {
        int _opt_type;

        if (type_ == YCSB_SEQ_LOAD) {
            generate_kv_pair(++sequential_key_id_, key, value);
        } else if (zipfan_) {
            generate_kv_pair(zipf_next(), key, value);
        } else {
            generate_kv_pair(uniform_next(), key, value);
        }

        _opt_type = random_get_put(type_);
        key_length = key_length_;
        value_length = value_length_;
        return _opt_type;
    }

public:
    uint64_t base_key_;

    uint64_t key_range_;

    size_t key_length_;

    size_t value_length_;

    int type_;

    int zipfan_;

    uint64_t sequential_key_id_;
};
};

#endif