#include "workload_generator.h"
#include "header.h"
#include "timer.h"

using namespace kv_benchmark;

static int g_numa[] = {
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29
};

struct thread_param_t {
public:
    kv_benchmark::DB* db;
    kv_benchmark::DBBench* benchmark;

public:
    int thread_id;
    int count;

public: // result
    uint64_t sum_latency;
    uint64_t result_count[16];
    uint64_t result_latency[16];
    uint64_t result_success[16];

public:
    std::vector<uint64_t> vec_latency[16];

public:
    thread_param_t()
        : sum_latency(0)
    {
        memset(result_count, 0, sizeof(result_count));
        memset(result_latency, 0, sizeof(result_latency));
        memset(result_success, 0, sizeof(result_success));
    }
};

static void result_output(const char* name, std::vector<uint64_t>& data)
{
    std::ofstream fout(name);
    if (fout.is_open()) {
        for (int i = 0; i < data.size(); i++) {
            fout << data[i] << std::endl;
        }
        fout.close();
    }
}

static void thread_task(thread_param_t* param)
{
    int _thread_id = param->thread_id;

    cpu_set_t _mask;
    CPU_ZERO(&_mask);
    CPU_SET(g_numa[_thread_id], &_mask);
    if (pthread_setaffinity_np(pthread_self(), sizeof(_mask), &_mask) < 0) {
        printf("threadpool, set thread affinity failed.\n");
    }

    DB* _db = param->db;
    DBBench* _benchmark = param->benchmark;

    assert((_benchmark != nullptr) && (_db != nullptr));
    _benchmark->initlizate();

    int _count = param->count;
    bool _result;

    char _key[128] = {0};
    char _value[65535] = {0};
    size_t _key_length;
    size_t _value_length;

    Timer _t1, _t2;
    uint64_t _latency = 0;

    _t1.Start();
    for (int i = 0; i < _count; i++) {
        int __type = _benchmark->get_kv_pair(_key, _key_length, _value, _value_length);
        _t2.Start();
        if (__type == DBBENCH_PUT) {
            // TODO
            // DB::PUT()
            _result = _db->Put(_key, _key_length, _value, _value_length);
            param->result_count[__type]++;
            if (_result) {
                param->result_success[__type]++;
            }
        } else if (__type == DBBENCH_GET) {
            // TODO
            // DB::GET()
            _result = _db->Get(_key, _key_length, _value, _value_length);
            param->result_count[__type]++;
            if (_result) {
                param->result_success[__type]++;
            }
        } else if (__type == DBBENCH_DELETE) {
            // TODO
            // DB::DELETE
        } else if (__type == DBBENCH_SCAN) {
            // TODO
            // DB::Scan
        }
        _t2.Stop();
        _latency = _t2.Get();
        param->result_latency[__type] += _latency;
        param->sum_latency += _latency;
        param->vec_latency[__type].push_back(_latency);
    }
    _t1.Stop();
    printf("*** THREAD%02d FINISHED [TIME:%.2f]\n", _thread_id, _t1.GetSeconds());
}

WorkloadGenerator::WorkloadGenerator(const char* name, struct generator_parameter* param, DB* db, DBBench* benchmarks[])
    : db_(db)
    , num_threads_(param->num_threads)
    , result_path_(param->result_path)
    , data_size_(param->data_size)
    , key_length_(param->key_length)
    , value_length_(param->value_length)
{
    strcpy(name_, name);

    for (int i = 0; i < num_threads_; i++) {
        benchmarks_[i] = benchmarks[i];
    }
}

static char _g_oname[DBBENCH_NUM_OPT_TYPE][32] = { "PUT", "GET", "DELETE", "SCAN" };

void WorkloadGenerator::Run()
{
    std::thread _threads[32];
    thread_param_t _params[32];
    uint64_t _count = data_size_ / (value_length_ * num_threads_);

    for (int i = 0; i < num_threads_; i++) {
        _params[i].thread_id = i;
        _params[i].benchmark = benchmarks_[i];
        _params[i].db = db_;
        _params[i].count = _count;
        _threads[i] = std::thread(thread_task, &_params[i]);
    }

    for (int i = 0; i < num_threads_; i++) {
        _threads[i].join();
    }

    char _result_file[128];
    sprintf(_result_file, "%s/%s.result", result_path_.c_str(), name_);
    std::ofstream _fout(_result_file);

    for (int i = 0; i < num_threads_; i++) {
        double __lat = 1.0 * _params[i].sum_latency / (1000UL * _params[i].count);
        _fout << ">>thread" << i << std::endl;
        _fout << "  [0] count:" << _params[i].count << "]" << std::endl;
        _fout << "  [1] lat:" << __lat << "us" << std::endl;
        _fout << "  [2] iops:" << 1000000.0 / __lat << std::endl;
        for (int j = 0; j < DBBENCH_NUM_OPT_TYPE; j++) {
            if (_params[i].vec_latency[j].size() > 0) {
                char __name[128];
                sprintf(__name, "%s/%s_%s.lat", result_path_.c_str(), name_, _g_oname[j]);
                result_output(__name, _params[i].vec_latency[j]);
                __lat = 1.0 * _params[i].result_latency[j] / (1000UL * _params[i].result_count[j]);
                std::string __str = _g_oname[j];
                _fout << "  [" << __str << "][lat:" << __lat << "][iops:" << 1000000.0 / __lat << "][count:" << _params[i].result_count[j] << "|" << 100.0 * _params[i].result_count[j] / _params[i].count << "%%][success:" << _params[i].result_success[j] << "|" << 100.0 * _params[i].result_success[j] / _params[i].result_count[j] << "%%]" << std::endl;
            }
        }
    }
    _fout.close();
}
