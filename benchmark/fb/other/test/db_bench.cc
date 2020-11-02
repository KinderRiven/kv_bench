#include "benchmark.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    Options pmemkv_options;
    ROCKSDB_BENCHMARK_NAMESPACE::Options bench_options
            = ROCKSDB_BENCHMARK_NAMESPACE::rocksdb_bench_options("ZippyDB");

// parse command line parameters
    int num_server_thread = 1;
    int num_backbend_thread = 1;

    char ch_kvindex_type[32] = "FPTREE";
    char ch_kvstore_type[32] = "SEGMENT";
    char ch_dram_allocator_type[32] = "PALLOCATOR";
    char ch_pmem_allocator_type[32] = "JALLOCATOR";

    char ch_bench_names[32] = "ZippyDB";
    uint32_t opt_nums = 1000000;

    for(int i = 0; i < argc; i++) {
        double d;
        uint64_t n;
        char junk;
        // parse pmemkv parameters
        if (sscanf(argv[i], "--pmem_size=%llu%c", &n, &junk) == 1) {
            pmemkv_options.pmem_file_size = n * 1024 * 1024;    //MB
        } else if (sscanf(argv[i], "--num_server_thread=%llu%c", &n, &junk) == 1) {
            num_server_thread = n;
            pmemkv_options.num_server_thread = n;
        } else if (sscanf(argv[i], "--num_backend_thread=%llu%c", &n, &junk) == 1) {
            num_backbend_thread = n;
            pmemkv_options.num_backend_thread = n;
        } else if (sscanf(argv[i], "--index_persist=%llu%c", &n, &junk) == 1) {
            pmemkv_options.index_in_aep = (n == 1) ? true : false;
        } else if (sscanf(argv[i], "--store_persist=%llu%c", &n, &junk) == 1) {
            pmemkv_options.store_in_aep = (n == 1) ? true : false;
        } else if (strncmp(argv[i], "--index=", 8) == 0) {
            strcpy(ch_kvindex_type, argv[i] + 8);
        } else if (strncmp(argv[i], "--store=", 8) == 0) {
            strcpy(ch_kvstore_type, argv[i] + 8);
        } else if (strncmp(argv[i], "--dram_allocator=", 17) == 0) {
            strcpy(ch_dram_allocator_type, argv[i] + 17);
        } else if (strncmp(argv[i], "--pmem_allocator=", 17) == 0) {
            strcpy(ch_pmem_allocator_type, argv[i] + 17);
        } else if (strncmp(argv[i], "--pmem_file_path=", 17) == 0) {
            strcpy(pmemkv_options.pmem_file_path, argv[i] + 17);
        } // parse benchmark parameters
        else if (strncmp(argv[i], "--benchmark=", 12) == 0) {
            strcpy(ch_bench_names, argv[i] + 12);
            bench_options.bench_name = std::string(ch_bench_names);
        } else if (sscanf(argv[i], "--num_opt=%llu%c", &n, &junk)) {
            opt_nums = n;
        } else if (sscanf(argv[i], "--opt_num=%llu%c", &n, &junk)) {
            opt_nums = n;
        } else if (sscanf(argv[i], "--key_length=%llu%c", &n, &junk)) {
            bench_options.key_size = n;
        } else if (sscanf(argv[i], "--value_length=%llu%c", &n, &junk)) {
            bench_options.flags_value_size_max = n;
        } else if (sscanf(argv[i], "--range_num=%llu%c%", &n, &junk)) {
            bench_options.keyrange_num = n;
        } else if (sscanf(argv[i], "--key_num=%llu%c", &n, &junk)) {
            bench_options.flags_num = n;
        } else if (i > 0) {
            printf("Error Parameter [%s]!\n", argv[i]);
            return 0;
        }
    }

    if (memcmp(ch_kvindex_type, "PFHT", 4) == 0) {
        pmemkv_options.index_type = INDEX_PFHT_HASH;
    } else if (memcmp(ch_kvindex_type, "FAST_FAIR", 9) == 0) {
        pmemkv_options.index_type = INDEX_FAST_FAIR;
    } else if (memcmp(ch_kvindex_type, "FPTREE", 7) == 0) {
        pmemkv_options.index_type = INDEX_FPTREE;
    } else if (memcmp(ch_kvindex_type, "LEVEL_HASH", 10) == 0) {
        pmemkv_options.index_type = INDEX_LEVEL_HASH;
    } else if (memcmp(ch_kvindex_type, "CCEH_LSB_HASH", 13) == 0) {
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

    Benchmark *bench = new Benchmark(bench_options, pmemkv_options);
    (bench->threads_info_)->thread_num = num_server_thread;
    
    for(int i = 0; i < num_server_thread; ++i) { 
        (bench->threads_info_->threads_params_)[i].opt_nums = opt_nums;
        (bench->threads_info_->threads_params_)[i].thread_id = i;
    }
    bench->Run();
    bench->StatusReport();
    delete bench;
    return 0;
}