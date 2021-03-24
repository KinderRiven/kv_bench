#include "workload_generator.h"
// #include "concurrentqueue.h"
#include "header.h"
#include "tbb/concurrent_queue.h"
#include "timer.h"
#include "ycsb.h"

using namespace kv_benchmark;

static int g_numa[] = {
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29
};

struct thread_param_t {
public:
    kv_benchmark::DB* db;
    kv_benchmark::YCSB* benchmark;

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

#define PRINT_THREAD_LATENCY
#define PRINT_TOTAL_LATENCY

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

static tbb::concurrent_queue<uint64_t> g_queue[16];

static void result_output2(const char* name, tbb::concurrent_queue<uint64_t>* queue)
{
    std::ofstream fout(name);
    if (fout.is_open()) {
        uint64_t _lat;
        while (!queue->empty()) {
            queue->try_pop(_lat);
            fout << _lat << std::endl;
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
    YCSB* _benchmark = param->benchmark;

    assert((_benchmark != nullptr) && (_db != nullptr));
    _benchmark->initlizate();

    int _count = param->count;
    bool _result;

    char _key[128] = { 0 };
    char _value[65535] = { 0 };
    size_t _key_length;
    size_t _value_length;

    Timer _t1, _t2;
    uint64_t _latency = 0;

    _t1.Start();
    for (int i = 0; i < _count; i++) {
        int __type = _benchmark->get_kv_pair(_key, _key_length, _value, _value_length);
        _t2.Start();
        if (__type == YCSB_PUT) {
            // DB::PUT()
            _result = _db->Put(_key, _key_length, _value, _value_length);
            param->result_count[__type]++;
            if (_result) {
                param->result_success[__type]++;
            }
        } else if (__type == YCSB_UPDATE) {
            // DB::UPDATE()
            _result = _db->Put(_key, _key_length, _value, _value_length);
            param->result_count[__type]++;
            if (_result) {
                param->result_success[__type]++;
            }
        } else if (__type == YCSB_GET) {
            // DB::GET()
            _result = _db->Get(_key, _key_length, _value, _value_length);
            param->result_count[__type]++;
            if (_result) {
                param->result_success[__type]++;
            }
        } else if (__type == YCSB_DELETE) {
            // DB::DELETE
            _result = _db->Delete(_key, _key_length);
            param->result_count[__type]++;
            if (_result) {
                param->result_success[__type]++;
            }
        } else if (__type == YCSB_RMW) {
            // DB::Read&Update
            _result = _db->Get(_key, _key_length, _value, _value_length); // read
            if (_result) {
                _result = _db->Put(_key, _key_length, _value, _value_length); // modify
            }
            param->result_count[__type]++;
            if (_result) {
                param->result_success[__type]++;
            }
        } else if (__type == YCSB_SCAN) {
            // DB::Scan
        }
        _t2.Stop();
        _latency = _t2.Get();
        g_queue[__type].push(_latency); // push to global queue
        param->result_latency[__type] += _latency;
        param->sum_latency += _latency;
        param->vec_latency[__type].push_back(_latency);
    }
    _t1.Stop();
    printf("*** THREAD%02d FINISHED [TIME:%.2f]\n", _thread_id, _t1.GetSeconds());
}

WorkloadGenerator::WorkloadGenerator(const char* name, struct generator_parameter* param, DB* db, YCSB* benchmarks[])
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

static char _g_oname[YCSB_NUM_OPT_TYPE][32] = { "PUT", "UPDATE", "GET", "DELETE", "SCAN", "RMW" };
static char _g_wname[YCSB_NUM_WORKLOAD_TYPE][32] = { "A", "B", "C", "D", "E", "F", "RANDOM", "SEQ" };

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

    // PRINT LATENCY FOR EACH THREAD
    for (int i = 0; i < num_threads_; i++) {
        double __lat = 1.0 * _params[i].sum_latency / (1000UL * _params[i].count);
        // print to file
        _fout << ">>thread" << i << std::endl;
        _fout << "  [0] count:" << _params[i].count << "]" << std::endl;
        _fout << "  [1] lat:" << __lat << "us" << std::endl;
        _fout << "  [2] iops:" << 1000000.0 / __lat << std::endl;
        // loop
        for (int j = 0; j < YCSB_NUM_OPT_TYPE; j++) {
            if (_params[i].vec_latency[j].size() > 0) {
#ifdef PRINT_THREAD_LATENCY
                char __name[128];
                // {thread_id}_{benchmark_name}_{operator_name}
                sprintf(__name, "%s/thread%d_%s_%s.lat", result_path_.c_str(), i, name_, _g_oname[j]);
                result_output(__name, _params[i].vec_latency[j]);
#endif
                __lat = 1.0 * _params[i].result_latency[j] / (1000UL * _params[i].result_count[j]);
                std::string __str = _g_oname[j];
                // print to file
                _fout << "  [" << __str << "][lat:" << __lat << "][iops:" << 1000000.0 / __lat << "][count:" << _params[i].result_count[j] << "|" << 100.0 * _params[i].result_count[j] / _params[i].count << "%%][success:" << _params[i].result_success[j] << "|" << 100.0 * _params[i].result_success[j] / _params[i].result_count[j] << "%%]" << std::endl;
            }
        }
    }
#ifdef PRINT_TOTAL_LATENCY
    // PRINT TOTAL THREAD LATENCY
    for (int i = 0; i < YCSB_NUM_OPT_TYPE; i++) {
        if (g_queue[i].unsafe_size() > 0) {
            char _result_file2[128];
            sprintf(_result_file2, "%s/total_%s_%s.lat", result_path_.c_str(), name_, _g_oname[i]);
            result_output2(_result_file2, &g_queue[i]);
        }
    }
#endif
    _fout.close();
    return;
}
