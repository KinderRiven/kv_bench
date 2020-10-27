#include "db.h"
#include "header.h"
#include "options.h"
#include "workload_generator.h"
#include "ycsb.h"

int main(int argc, char* argv[])
{
    // DB opation
    char _ssd_path[128] = "/home/hanshukai/dir1";
    char _pmem_path[128] = "/home/pmem0";

    // Workload Parameter
    int _num_thread = 1;
    size_t _key_length = 16;
    size_t _value_length = 1000;
    size_t _data_size = 1024 * 1024 * 1024;

    // workload generator
    for (int i = 0; i < argc; i++) {
        char junk;
        uint64_t n;
        if (sscanf(argv[i], "--key_length=%llu%c", &n, &junk) == 1) {
            _key_length = n;
        } else if (sscanf(argv[i], "--value_length=%llu%c", &n, &junk) == 1) {
            _value_length = n;
        } else if (sscanf(argv[i], "--num_thread=%llu%c", &n, &junk) == 1) {
            _num_thread = n;
        } else if (sscanf(argv[i], "--size=%llu%c", &n, &junk) == 1) { // GB
            _data_size = n * (1024 * 1024 * 1024);
        } else if (strncmp(argv[i], "--nvm=", 6) == 0) {
            strcpy(_pmem_path, argv[i] + 6);
        } else if (strncmp(argv[i], "--ssd=", 6) == 0) {
            strcpy(_ssd_path, argv[i] + 6);
        } else if (i > 0) {
            printf("+++ ERROR PARAMETER (%s)\n", argv[i]);
            exit(1);
        }
    }

    kv_benchmark::DB* _db = nullptr;
    kv_benchmark::Options _options;
    _options.nvm_path.assign(_pmem_path);
    _options.db_path.assign(_ssd_path);
    kv_benchmark::DB::Open(_options, &_db);

    uint64_t _key_base = 1;
    kv_benchmark::YCSB* _benchmarks[32];
    size_t _range = _data_size / (_num_thread * _value_length);
    for (int i = 0; i < _num_thread; i++) {
        _benchmarks[i] = new kv_benchmark::YCSB(YCSB_SEQ_LOAD, _key_base, _range, _key_length, _value_length);
        _key_base += _range;
    }

    kv_benchmark::generator_parameter _gparam;
    _gparam.key_length = _key_length;
    _gparam.value_length = _value_length;
    _gparam.num_threads = _num_thread;
    _gparam.data_size = _data_size;

    kv_benchmark::WorkloadGenerator* _warmup = new kv_benchmark::WorkloadGenerator(&_gparam, _db, _benchmarks);
    _warmup->Run();
    return 0;
}
