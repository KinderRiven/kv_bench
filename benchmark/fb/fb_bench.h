#ifndef INCLUDE_FB_BENCH_H_
#define INCLUDE_FB_BENCH_H_

#include "random.h"
#include "rocksdb_benchtools.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FBBENCH_NUM_THREAD (16)
#define FBBENCH_KEY_LENGTH (8)
#define FBBENCH_VALUE_LENGTH (4096)

// enum QueryType {
//    Get,
//    Put,
//    SeekForPrev,
//    Delete,
//    SingleDelete,
//    Merge,
// };

#define FBBENCH_NUM_OPT_TYPE (6)
#define FBBENCH_GET (ROCKSDB_BENCHMARK_NAMESPACE::Get)
#define FBBENCH_PUT (ROCKSDB_BENCHMARK_NAMESPACE::Put)
#define FBBENCH_SEEK (ROCKSDB_BENCHMARK_NAMESPACE::SeekForPrev)
#define FBBENCH_DELETE (ROCKSDB_BENCHMARK_NAMESPACE::Delete)
#define FBBENCH_SINGLE_DELETE (ROCKSDB_BENCHMARK_NAMESPACE::SingleDelete)
#define FBBENCH_MERGE (ROCKSDB_BENCHMARK_NAMESPACE::Merge)

namespace kv_benchmark {
// Only Support Single Thread
class FBBench {
public:
    // key range is [base_key, base_key + key_range]
    FBBench(const char* type, uint32_t seed, size_t key_length, size_t value_length)
        : key_length_(key_length)
        , value_length_(value_length)
    {
        ROCKSDB_BENCHMARK_NAMESPACE::Options _options = ROCKSDB_BENCHMARK_NAMESPACE::rocksdb_predef_bench_options(type);
        benchmark_ = new ROCKSDB_BENCHMARK_NAMESPACE::Benchmark(_options);
        benchmark_->InitiateBenchmark();
    }

    ~FBBench()
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
        // Nothing to do.
    }

    int random_get_put()
    {
        // Nothing to do.
        return -1;
    }

public:
    int get_kv_pair(char* key, size_t& key_length, char* value, size_t& value_length)
    {
        ROCKSDB_BENCHMARK_NAMESPACE::SingleQuery _sq = benchmark_->GetSingleQuery();
        key_length = _sq.key_size;
        value_length = _sq.val_size;
        memcpy(key, _sq.key.c_str(), key_length);
        memcpy(value, _sq.val.c_str(), value_length);
        return _sq.type;
    }

public:
    ROCKSDB_BENCHMARK_NAMESPACE::Benchmark* benchmark_;

    size_t key_length_;

    size_t value_length_;
};
};

#endif