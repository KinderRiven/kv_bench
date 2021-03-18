#ifndef BENCHMARK_H_
#define BENCHMARK_H_

#include "rocksdb_benchtools.h"
#include "pmemkv.h"
#include <pthread.h>

#define MAX_TEST_THREAD (32)
#define MAX_KEY_LEN     (16)
#define MAX_VAL_LEN     (2048)

struct thread_param_t {
    uint32_t opt_nums;
    int thread_id;
    ROCKSDB_BENCHMARK_NAMESPACE::Benchmark *param_benchmark;
    PmemKV *param_pmemkv;
};
struct threads_info_t {
    uint32_t thread_num;
    thread_param_t threads_params_[MAX_TEST_THREAD];
};

class Benchmark {
public:
    Benchmark();
    Benchmark(ROCKSDB_BENCHMARK_NAMESPACE::Options, Options);
    ~Benchmark();
    void Run();
    void StatusReport();
    threads_info_t *threads_info_;
private:
    ROCKSDB_BENCHMARK_NAMESPACE::Benchmark *benchmark;
    PmemKV *pmemkv;
};

void *thread_task(void *);

#endif