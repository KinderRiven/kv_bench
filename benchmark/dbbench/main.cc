#include "db.h"
#include "db_bench.h"
#include "header.h"
#include "options.h"
#include "workload_generator.h"

struct workload_options {
public:
    int type;
    char name[256];

public:
    std::string result_path;
    int num_threads;
    size_t key_length;
    size_t value_length;

public:
    uint64_t dbsize;
    size_t workload_size;
    kv_benchmark::DB* db;
};

static void start_workload(struct workload_options* options)
{
    uint64_t _key_base = 1;
    kv_benchmark::DBBench* _benchmarks[32];
    size_t _range = (options->dbsize / options->value_length);
    uint32_t _seed = 1000;

    for (int i = 0; i < options->num_threads; i++) {
        _benchmarks[i] = new kv_benchmark::DBBench(options->type, _seed + i, options->key_length, options->value_length);
    }

    kv_benchmark::generator_parameter _gparam;
    _gparam.record_detail = 1;
    _gparam.key_length = options->key_length;
    _gparam.value_length = options->value_length;
    _gparam.num_threads = options->num_threads;
    _gparam.data_size = (options->workload_size / options->num_threads);
    _gparam.result_path = options->result_path;

    kv_benchmark::WorkloadGenerator* _run = new kv_benchmark::WorkloadGenerator(options->name, &_gparam, options->db, _benchmarks);
    _run->Run();
}

int main(int argc, char* argv[])
{
    // DB opation
    char _ssd_path[128] = "/home/hanshukai/mount/4800/db";
    char _pmem_path[128] = "/home/pmem0";

    // Workload Parameter
    int _num_threads = 8;
    size_t _key_length = 16;
    size_t _value_length = 1024;
    size_t _dbsize = 20UL * 1024 * 1024 * 1024;
    double _psize = 1;

    // workload generator
    for (int i = 0; i < argc; i++) {
        char junk;
        uint64_t n;
        double f;
        if (sscanf(argv[i], "--key_length=%llu%c", &n, &junk) == 1) {
            _key_length = n;
        } else if (sscanf(argv[i], "--value_length=%llu%c", &n, &junk) == 1) {
            _value_length = n;
        } else if (sscanf(argv[i], "--num_thread=%llu%c", &n, &junk) == 1) {
            _num_threads = n;
        } else if (sscanf(argv[i], "--dbsize=%llu%c", &n, &junk) == 1) { // GB
            _dbsize = n * (1024 * 1024 * 1024);
        } else if (sscanf(argv[i], "--psize=%lf%c", &f, &junk) == 1) { // GB
            _psize = f;
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
    _options.write_buffer_size = 8UL * 1024 * 1024 * 1024;
    _options.num_backend_thread = 2;
    kv_benchmark::DB::Open(_options, &_db);

    // CREATE RESULT SAVE PATH
    time_t _t = time(NULL);
    struct tm* _lt = localtime(&_t);
    char _result_path[128];
    sprintf(_result_path, "%04d%02d%02d_%02d%02d%02d", 1900 + _lt->tm_year, _lt->tm_mon, _lt->tm_mday, _lt->tm_hour, _lt->tm_min, _lt->tm_sec);
    mkdir(_result_path, 0777);

    struct workload_options _wopt;
    _wopt.key_length = _key_length;
    _wopt.value_length = _value_length;
    _wopt.num_threads = _num_threads;
    _wopt.dbsize = _dbsize;
    _wopt.db = _db;
    _wopt.result_path.assign(_result_path);

#if 0
    strcpy(_wopt.name, "WARMUP");
    _wopt.type = DBBENCH_PUT;
    _wopt.workload_size = _dbsize;
    _wopt.num_threads = 1;
    start_workload(&_wopt);
#endif

    strcpy(_wopt.name, "SINGLE_PUT");
    _wopt.type = DBBENCH_PUT;
    _wopt.workload_size = (size_t)(_psize * _dbsize);
    _wopt.num_threads = _num_threads;
    start_workload(&_wopt);

    strcpy(_wopt.name, "SINGLE_GET");
    _wopt.type = DBBENCH_GET;
    _wopt.num_threads = _num_threads;
    _wopt.workload_size = (size_t)(_psize * _dbsize);
    start_workload(&_wopt);
    return 0;
}
