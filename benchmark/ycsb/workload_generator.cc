#include "workload_generator.h"
#include "header.h"
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
    YCSB* _benchmark = param->benchmark;
    assert((_benchmark != nullptr) && (_db != nullptr));
    _benchmark->initlizate();

    int _count = param->count;
    bool _result;

    char _key[128];
    char _value[65535];
    size_t _key_length;
    size_t _value_length;

    Timer _t1, _t2;
    uint64_t _latency = 0;

    _t1.Start();
    for (int i = 0; i < _count; i++) {
        int __type = _benchmark->get_kv_pair(_key, _key_length, _value, _value_length);
        _t2.Start();
        if (__type == YCSB_PUT) {
            // TODO
            // DB::PUT()
            _result = _db->Put(_key, _key_length, _value, _value_length);
            param->result_count[__type]++;
            if (_result) {
                param->result_success[__type]++;
            }
        } else if (__type == YCSB_UPDATE) {
            // TODO
            // DB::UPDATE()
            _result = _db->Put(_key, _key_length, _value, _value_length);
            param->result_count[__type]++;
            if (_result) {
                param->result_success[__type]++;
            }
        } else if (__type == YCSB_GET) {
            // TODO
            // DB::GET()
            _result = _db->Get(_key, _key_length, _value, _value_length);
            param->result_count[__type]++;
            if (_result) {
                param->result_success[__type]++;
            }
        } else if (__type == YCSB_DELETE) {
            // TODO
            // DB::DELETE
        } else if (__type == YCSB_RMW) {
            // TODO
            // DB::Read&Update
        } else if (__type == YCSB_SCAN) {
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

WorkloadGenerator::WorkloadGenerator(struct generator_parameter* param, DB* db, YCSB* benchmarks[])
    : db_(db)
    , num_threads_(param->num_threads)
    , result_path_(param->result_path)
    , data_size_(param->data_size)
    , key_length_(param->key_length)
    , value_length_(param->value_length)
{
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

    for (int i = 0; i < num_threads_; i++) {
        for (int j = 0; j < YCSB_NUM_OPT_TYPE; j++) {
            if (_params[i].vec_latency[j].size() > 0) {
                char __name[128];
                sprintf(__name, "%s/%s_%s", result_path_.c_str(), _g_wname[benchmarks_[i]->type_ >> 1], _g_oname[j]);
                result_output(__name, _params[i].vec_latency[j]);
            }
        }
    }
}
