#include "benchmark.h"
#define DEBUG

Benchmark::Benchmark()
{
    ROCKSDB_BENCHMARK_NAMESPACE::Options opt
        = ROCKSDB_BENCHMARK_NAMESPACE::rocksdb_bench_options("ZippyDB");
    benchmark = new ROCKSDB_BENCHMARK_NAMESPACE::Benchmark(opt);
    Options options;
    pmemkv = new PmemKV(options);
    threads_info_ = new threads_info_t();
}

Benchmark::Benchmark(ROCKSDB_BENCHMARK_NAMESPACE::Options bench_options,
    Options kv_options)
{
    benchmark = new ROCKSDB_BENCHMARK_NAMESPACE::Benchmark(bench_options);
    // remember to initiate benchmark
    benchmark->InitiateBenchmark();
    pmemkv = new PmemKV(kv_options);
    threads_info_ = new threads_info_t();
}

Benchmark::~Benchmark()
{
    delete benchmark;
    delete pmemkv;
    delete threads_info_;
}

void Benchmark::Run()
{
    uint32_t thread_num = threads_info_->thread_num;
    pthread_t threads_[MAX_TEST_THREAD];

    for (int i = 0; i < thread_num; ++i) {
        (threads_info_->threads_params_)[i].param_benchmark = this->benchmark;
        (threads_info_->threads_params_)[i].param_pmemkv = this->pmemkv;
        pthread_create(threads_ + i, NULL, thread_task, (void*)(threads_info_->threads_params_ + i));
    }

    for (int i = 0; i < thread_num; ++i) {
        pthread_join(threads_[i], NULL);
    }
}

void* thread_task(void* thread_param)
{

    int tid = gettid();

    thread_param_t* param = static_cast<thread_param_t*>(thread_param);
    uint32_t done_opt = 0, opt_nums = param->opt_nums;
    int thread_id = param->thread_id;
    ROCKSDB_BENCHMARK_NAMESPACE::Benchmark* benchmark = param->param_benchmark;
    PmemKV* pmemkv = param->param_pmemkv;
    uint8_t key[MAX_KEY_LEN + 10];
    uint8_t val[MAX_VAL_LEN + 10];
    int match_opts = 0;
    while (done_opt < opt_nums) {
        ROCKSDB_BENCHMARK_NAMESPACE::SingleQuery query = benchmark->GetSingleQuery();

        memcpy(key, query.key.c_str(), query.key_size);
        memcpy(val, query.val.c_str(), query.val_size);

#ifdef DEBUG
        if (done_opt == 37266730) {
            printf("here is an error\n");
        }
#endif

        switch (query.type) {
        case ROCKSDB_BENCHMARK_NAMESPACE::Get:
            //printf("Time: %d Get\n",done_opt);
            pmemkv->Get(thread_id, (uint8_t*)key, query.key_size, (uint8_t*)val, query.val_size);
            break;
        case ROCKSDB_BENCHMARK_NAMESPACE::Put:
            //printf("Time: %d Put:%u\n",done_opt, query.val_size);
            pmemkv->Put(thread_id, (uint8_t*)key, query.key_size, (uint8_t*)val, query.val_size);
            break;
        default:
            break;
        }

        ++done_opt;
        if (done_opt % 100000 == 0) {
            printf("tid:%d  done:%d\n", tid, done_opt);
        }
    }
    printf("%d match pairs:%d\n", tid, match_opts);
}

void Benchmark::StatusReport()
{
    pmemkv->Print();
    benchmark->StatusReport();
}
