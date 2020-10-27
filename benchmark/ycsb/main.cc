#include "db.h"
#include "header.h"
#include "options.h"
#include "workload_generator.h"
#include "ycsb.h"

int main(int argc, char* argv[])
{
    kv_benchmark::DB* _db;
    kv_benchmark::Options _options;
    kv_benchmark::YCSB* _benchmarks[32];

    // DB opation
    char _ssd_path[128] = "/home/hanshukai/dir1";
    char _pmem_path[128] = "/home/pmem0";

    // Workload Parameter
    int _num_thread = 1;
    size_t _key_length = 16;
    size_t _value_length = 1000;

    // workload generator
    for (int i = 0; i < argc; i++) {
        char junk;
        uint64_t n;
        if (strncmp(argv[i], "--nvm=", 6) == 0) {
            strcpy(_pmem_path, argv[i] + 6);
        } else if (strncmp(argv[i], "--ssd=", 6) == 0) {
            strcpy(_ssd_path, argv[i] + 6);
        } else {
            return 0;
        }
    }

    for (int i = 0; i < _num_thread; i++) {
        _benchmarks[i] = new kv_benchmark::YCSB(YCSB_SEQ_LOAD, 1, 1000000, 16, 1024);
    }

    kv_benchmark::DB::Open(_options, &_db);
    kv_benchmark::generator_parameter _gparam;
    _gparam.key_length = _key_length;
    _gparam.value_length = _value_length;
    _gparam.num_threads = _num_thread;
    kv_benchmark::WorkloadGenerator *_warmup = new kv_benchmark::WorkloadGenerator(&_gparam, _db, _benchmarks);
    _warmup->Run();
    return 0;
}
