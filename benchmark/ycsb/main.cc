#include "db.h"
#include "header.h"
#include "options.h"
#include "workload_generator.h"
#include "ycsb.h"

struct workload_options {
public:
    int type;
    char name[256];

public:
    int num_threads;
    size_t key_length;
    size_t value_length;
    std::string result_path;

public:
    uint64_t dbsize;
    size_t workload_size;
    kv_benchmark::DB* db;
};

// --btype
// benchmark type
// warmup, ycsb-a, ycsb-b, ycsb-c, ycsb-d, ycsb-e, ycsb-f
static char g_benchmark_type[128] = "warmup";

// 0 is unifrom skew
// 1 is zipfian skew
static int g_zipf = 0;

// --ssd=
// data store path
static char g_ssd_path[128] = "/home/hanshukai/mount/4510/dbbench";

// --nvm=
// NVM file path
static char g_pmem_path[128] = "/home/pmem0/dbbench";

// --num_thread
// server thread count
static int g_num_threads = 1;

// --key_length
// key length
static size_t g_key_length = 16;

// --value_length
// value length
static size_t g_value_length = 1000;

// --dbsize (GB)
// warm up size (B)
static size_t g_dbsize = 10UL * 1024 * 1024 * 1024;

// --psize
// test size = g_psize * dbsize
static double g_psize = 0.2;

static void start_workload(struct workload_options* options)
{
    uint64_t _key_base = 1;
    kv_benchmark::YCSB* _benchmarks[32];
    size_t _range = (options->dbsize / options->value_length);

    for (int i = 0; i < options->num_threads; i++) {
        _benchmarks[i] = new kv_benchmark::YCSB(options->type, _key_base, _range, options->key_length, options->value_length);
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
    // workload generator
    for (int i = 0; i < argc; i++) {
        char junk;
        uint64_t n;
        double f;
        if (sscanf(argv[i], "--key_length=%llu%c", &n, &junk) == 1) {
            g_key_length = n;
        } else if (sscanf(argv[i], "--value_length=%llu%c", &n, &junk) == 1) {
            g_value_length = n;
        } else if (sscanf(argv[i], "--num_thread=%llu%c", &n, &junk) == 1) {
            g_num_threads = n;
        } else if (sscanf(argv[i], "--dbsize=%llu%c", &n, &junk) == 1) { // GB
            g_dbsize = n * (1024 * 1024 * 1024);
        } else if (sscanf(argv[i], "--psize=%lf%c", &f, &junk) == 1) { // GB
            g_psize = f;
        } else if (sscanf(argv[i], "--zipf=%llu%c", &n, &junk) == 1) { // GB
            g_zipf = n;
        } else if (strncmp(argv[i], "--nvm=", 6) == 0) {
            strcpy(g_pmem_path, argv[i] + 6);
        } else if (strncmp(argv[i], "--ssd=", 6) == 0) {
            strcpy(g_ssd_path, argv[i] + 6);
        } else if (strncmp(argv[i], "--btype=", 8) == 0) {
            strcpy(g_benchmark_type, argv[i] + 8);
        } else if (i > 0) {
            printf("+++ ERROR PARAMETER (%s)\n", argv[i]);
            exit(1);
        }
    }

    kv_benchmark::DB* _db = nullptr;
    kv_benchmark::Options _options;
    _options.nvm_path.assign(g_pmem_path);
    _options.db_path.assign(g_ssd_path);
    kv_benchmark::DB::Open(_options, &_db);

    // CREATE RESULT SAVE PATH
    time_t _t = time(NULL);
    struct tm* _lt = localtime(&_t);
    char _result_path[128];
    sprintf(_result_path, "%04d%02d%02d_%02d%02d%02d", 1900 + _lt->tm_year, _lt->tm_mon, _lt->tm_mday, _lt->tm_hour, _lt->tm_min, _lt->tm_sec);
    mkdir(_result_path, 0777);

    struct workload_options _wopt;
    _wopt.key_length = g_key_length;
    _wopt.value_length = g_value_length;
    _wopt.num_threads = g_num_threads;
    _wopt.dbsize = g_dbsize;
    _wopt.db = _db;
    _wopt.result_path.assign(_result_path);

    if (strcmp(g_benchmark_type, "warmup") == 0) {
        // 100% SEQ WRITE FOR WARM UP
        strcpy(_wopt.name, "WARMUP");
        _wopt.type = YCSB_SEQ_LOAD | g_zipf;
        _wopt.workload_size = g_dbsize;
        _wopt.num_threads = 1; // WE ONLY USE ONE THREAD TO WARM UP
        start_workload(&_wopt);
    }

    if (strcmp(g_benchmark_type, "ycsb-a") == 0) {
        // 50% UPDATE + 50% GET
        strcpy(_wopt.name, "YCSB_A");
        _wopt.type = YCSB_A | g_zipf;
        _wopt.workload_size = (size_t)(g_psize * g_dbsize);
        _wopt.num_threads = g_num_threads;
        start_workload(&_wopt);
    }

    if (strcmp(g_benchmark_type, "ycsb-b") == 0) {
        // 95% UPDATE + 5% GET
        strcpy(_wopt.name, "YCSB_B");
        _wopt.type = YCSB_B | g_zipf;
        _wopt.workload_size = (size_t)(g_psize * g_dbsize);
        _wopt.num_threads = g_num_threads;
        start_workload(&_wopt);
    }

    if (strcmp(g_benchmark_type, "ycsb-c") == 0) {
        // 100% GET
        strcpy(_wopt.name, "YCSB_C-0");
        _wopt.type = YCSB_C | g_zipf;
        _wopt.workload_size = (size_t)(g_psize * g_dbsize);
        _wopt.num_threads = g_num_threads;
        start_workload(&_wopt);
    }

    if (strcmp(g_benchmark_type, "ycsb-d") == 0) {
        // 100% GET
        strcpy(_wopt.name, "YCSB_C-1");
        _wopt.type = YCSB_D | g_zipf;
        _wopt.workload_size = (size_t)(g_psize * g_dbsize);
        _wopt.num_threads = g_num_threads;
        start_workload(&_wopt);
    }

    if (strcmp(g_benchmark_type, "ycsb-e") == 0) {
        // 100% GET
        strcpy(_wopt.name, "YCSB_C-1");
        _wopt.type = YCSB_E | g_zipf;
        _wopt.workload_size = (size_t)(g_psize * g_dbsize);
        _wopt.num_threads = g_num_threads;
        start_workload(&_wopt);
    }

    if (strcmp(g_benchmark_type, "ycsb-f") == 0) {
        // 100% GET
        strcpy(_wopt.name, "YCSB_C-1");
        _wopt.type = YCSB_F | g_zipf;
        _wopt.workload_size = (size_t)(g_psize * g_dbsize);
        _wopt.num_threads = g_num_threads;
        start_workload(&_wopt);
    }

    _db->Close();
    return 0;
}
