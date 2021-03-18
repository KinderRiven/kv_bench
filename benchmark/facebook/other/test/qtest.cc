#include "rocksdb_benchtools.h"
#include "pmemkv.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#define MAX_KEY_LEN (128)
#define MAX_VAL_LEN (4096)
#define MAX_THREAD_TEST (32)
#define MULTITHREAD
#define gettid() syscall(SYS_gettid)


struct thread_param {
    uint32_t opt_nums;
    int thread_id;
    PmemKV *pmemkv;
    ROCKSDB_BENCHMARK_NAMESPACE::Benchmark *benchmark;
};


static void* thread_task(void *param) {

    int tid = gettid();
    thread_param *p = (thread_param *)param;
    ROCKSDB_BENCHMARK_NAMESPACE::Benchmark *test = p->benchmark;
    PmemKV *pmem = p->pmemkv;
    uint32_t max_times = p->opt_nums;
    int time = 0;
    uint8_t key[MAX_KEY_LEN + 10];
    uint8_t val[MAX_VAL_LEN + 10];
    int thread_id = p->thread_id;

    while(time < max_times){
       ROCKSDB_BENCHMARK_NAMESPACE::SingleQuery query = test->GetSingleQuery();
       memcpy(key, query.key.c_str(), query.key_size);
       memcpy(val, query.val.c_str(), query.val_size); 
       switch (query.type)
       {
       case ROCKSDB_BENCHMARK_NAMESPACE::Get:
           //printf("tid:%d time:%d Get\n", tid, time);
           pmem->Get(thread_id, (uint8_t*)key, query.key_size, (uint8_t*)val, query.val_size);
           break;
       case ROCKSDB_BENCHMARK_NAMESPACE::Put:
           //printf("tid:%d time:%d Put\n", tid, time);
           pmem->Put(thread_id, (uint8_t*)key, query.key_size, (uint8_t*)val, query.val_size);
           break;
       default:
           break;
       }
       ++time;
       if(time % 100000 == 0) printf("tid:%d already done : %d\n", tid, time);
    }
}

int main(int argc, char *argv[]) {
    ROCKSDB_BENCHMARK_NAMESPACE::Options bench_options = 
            ROCKSDB_BENCHMARK_NAMESPACE::rocksdb_predef_bench_options("ZippyDB");
    
    Options pmemkv_options;

    int num_thread = 1;
    uint64_t n;
    char junk;

    char ch_bench_type[32] = "MixGraph";
    char ch_predef_bench[32] = "";
    uint64_t pmem_size = 512 * 1024 * 1024;

    char ch_kvindex_type[32] = "FPTREE";
    char ch_kvstore_type[32] = "PLOG";
    char ch_dram_allocator_type[32] = "PALLOCATOR";
    char ch_pmem_allocator_type[32] = "JALLOCATOR";

    uint32_t opt_num = 1000000;

    for(int i = 0; i < argc; ++i) {
        if (sscanf(argv[i], "--num_server_thread=%llu%c", &n, &junk)) {
            num_thread = n;
            pmemkv_options.num_server_thread = n;
        } else if(sscanf(argv[i], "--opt_num=%llu%c", &n, &junk)) {
            opt_num = n;
        } else if(sscanf(argv[i], "--key_length=%llu%c", &n, &junk)) {
            bench_options.key_size = n;
        } else if(sscanf(argv[i], "--key_num=%llu%c", &n, &junk)) {
            bench_options.flags_num = n;
        } else if(sscanf(argv[i], "--rand_seed=%llu", &n)) {
            bench_options.rand_seed = n;
        } else if(strncmp(argv[i], "--predef_bench=", 15) == 0) {
            strcpy(ch_predef_bench, argv[i] + 15);
        } else if(strncmp(argv[i], "--bench_type=", 13) == 0) {
            strcpy(ch_predef_bench, argv[i] + 13);
        } else if(sscanf(argv[i], "--index_persist=%llu%c", &n, &junk)) {
            pmemkv_options.index_in_aep = (n == 1) ? true : false;
        } else if(sscanf(argv[i], "--store_persist=%llu%c", &n, &junk)) {
            pmemkv_options.store_in_aep = (n == 1) ? true : false;
        } else if(sscanf(argv[i], "--use_static_value_size=%llu%c", &n, &junk)) {
            bench_options.use_static_value_size = n;
        } else if(sscanf(argv[i], "--value_size=%llu%c", &n, &junk)) {
            bench_options.flags_avg_value_size = n;
        } else if(strncmp(argv[i], "--index=", 8) == 0) {
            strcpy(ch_kvindex_type, argv[i] + 8);
        } else if(strncmp(argv[i], "--store=", 8) == 0) {
            strcpy(ch_kvstore_type, argv[i] + 8);
        } else if(strncmp(argv[i], "--dram_allocator=", 17) == 0) {
            strcpy(ch_dram_allocator_type, argv[i] + 17);
        } else if(strncmp(argv[i], "--pmem_allocator=", 17) == 0) {
            strcpy(ch_pmem_allocator_type, argv[i] + 17);
        } else if(strncmp(argv[i], "--pmem_file_path=", 17) == 0) {
            strcpy(pmemkv_options.pmem_file_path, argv[i] + 17);
        } else if(i > 0) {
            printf("Error Parameter [%s]!\n", argv[i]);
        }
    }

    if (memcmp(ch_kvindex_type, "PFHT", 4) == 0) {
        pmemkv_options.index_type = INDEX_PFHT_HASH;
    } else if(memcmp(ch_kvindex_type, "FAST_FAIR", 9) == 0){
        pmemkv_options.index_type = INDEX_FAST_FAIR;
    } else if(memcmp(ch_kvindex_type, "FPTREE", 6) == 0) {
        pmemkv_options.index_type = INDEX_FPTREE;
    } else if(memcmp(ch_kvindex_type, "LEVEL_HASH", 10) == 0) {
        pmemkv_options.index_type = INDEX_LEVEL_HASH;
    } else if(memcmp(ch_kvindex_type, "CCEH_LSB_HASH", 13) == 0) {
        pmemkv_options.index_type = INDEX_CCEH_LSB_HASH;
    } else if (memcmp(ch_kvindex_type, "HIKV", 4) == 0) {
        pmemkv_options.index_type = INDEX_HIKV;
    } else if (memcmp(ch_kvindex_type, "SKIPLIST", 8) == 0) {
        pmemkv_options.index_type = INDEX_SKIPLIST;
    } else if (memcmp(ch_kvindex_type, "BDSKIPLIST", 10) == 0) {
        pmemkv_options.index_type = INDEX_BDSKIPLIST;
    } else if (memcmp(ch_kvindex_type, "WBTREE", 6) == 0) {
        pmemkv_options.index_type = INDEX_WBTREE;
    } else if (memcmp(ch_kvindex_type, "WORT", 4) == 0) {
        pmemkv_options.index_type = INDEX_WORT;
    } else if (memcmp(ch_kvindex_type, "NORMAL_HASH", 11) == 0) {
        pmemkv_options.index_type = INDEX_NORMAL_HASH;
    } else if (memcmp(ch_kvindex_type, "SHADOW_HASH", 11) == 0) {
        pmemkv_options.index_type = INDEX_SHADOW_HASH;
    }

    if (memcmp(ch_kvstore_type, "PLOG", 4) == 0) {
        pmemkv_options.store_type = STORE_PLOG;
    } else if (memcmp(ch_kvstore_type, "SEGMENT", 7) == 0) {
        pmemkv_options.store_type = STORE_SEGMENT;
    } else if (memcmp(ch_kvstore_type, "BIT", 3) == 0) {
        pmemkv_options.store_type = STORE_BIT;
    } else if (memcmp(ch_kvstore_type, "NULL", 4) == 0) {
        pmemkv_options.store_type = STORE_NULL;
    }

    if(strlen(ch_predef_bench) > 0) {
        if(memcmp(ch_predef_bench, "ZippyDB", 7) == 0) {
            bench_options = ROCKSDB_BENCHMARK_NAMESPACE::rocksdb_predef_bench_options("ZippyDB");
        } else if(memcmp(ch_predef_bench, "UDB", 3) == 0) {
            bench_options = ROCKSDB_BENCHMARK_NAMESPACE::rocksdb_predef_bench_options("UDB");
        } else if(memcmp(ch_predef_bench, "MixRandom", 9) == 0) {
            bench_options = ROCKSDB_BENCHMARK_NAMESPACE::rocksdb_predef_bench_options("MixRandom");
        } else if(memcmp(ch_predef_bench, "ReadRandom", 10) == 0) {
            bench_options = ROCKSDB_BENCHMARK_NAMESPACE::rocksdb_predef_bench_options("ReadRandom");
        } else if(memcmp(ch_predef_bench, "AllRandom", 9) == 0 ) {
            bench_options = ROCKSDB_BENCHMARK_NAMESPACE::rocksdb_predef_bench_options("AllRandom");
        } else if(memcmp(ch_predef_bench, "AllDist", 7) == 0) {
            bench_options = ROCKSDB_BENCHMARK_NAMESPACE::rocksdb_predef_bench_options("AllDist");
        } else if(memcmp(ch_predef_bench, "PrefixDist", 10) == 0) {
            bench_options = ROCKSDB_BENCHMARK_NAMESPACE::rocksdb_predef_bench_options("PrefixDist");
        } else {
            printf("Error PreDefine benchmarks\n");
            return 0;
        }
    } else {
        assert(strlen(ch_bench_type) > 0);

        if(memcmp(ch_bench_type, "MixGraph", 8) == 0) {
            bench_options.bench_name = std::string(ch_bench_type);
        } else if(memcmp(ch_bench_type, "ReadRandom", strlen("ReadRandom")) == 0) {
            bench_options.bench_name = std::string(ch_bench_type);
        } else if(memcmp(ch_bench_type, "MixRandom", strlen("MixRandom")) == 0) {
            bench_options.bench_name = std::string(ch_bench_type);
        } else {
            printf("Error Bench Type!\n");
            return 0;
        }
    }

    thread_param params[MAX_THREAD_TEST];
    pthread_t threads[MAX_THREAD_TEST];

    ROCKSDB_BENCHMARK_NAMESPACE::Benchmark *benchmark 
        = new ROCKSDB_BENCHMARK_NAMESPACE::Benchmark(bench_options);
    
    benchmark->InitiateBenchmark();
    PmemKV *pmemkv = new PmemKV(pmemkv_options);

    #ifdef MULTITHREAD
        for(int i = 0; i < num_thread; ++i) {
            params[i].benchmark = benchmark;
            params[i].pmemkv = pmemkv;
            params[i].thread_id = i;
            params[i].opt_nums = opt_num;

            pthread_create(threads+i, NULL, thread_task, (void*)(&params[i]));
        }

        for(int i = 0; i < num_thread; ++i) {
            pthread_join(threads[i], NULL);
        }
    #else 
        params[0].benchmark = benchmark;
        params[0].pmemkv = pmemkv;
        params[0].thread_id = 0;
        params[0].opt_nums = opt_num;

        thread_task((void *)(&(params[0])));
    #endif

    benchmark->StatusReport();
    pmemkv->Print();

    delete benchmark;
    delete pmemkv;
    
    return 0;
}