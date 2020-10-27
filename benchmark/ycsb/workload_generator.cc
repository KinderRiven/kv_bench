#include "ycsb.h"
#include "timer.h"
#include "workload_generator.h"
#include "header.h"

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
    uint64_t result_count[16];
    uint64_t result_latency[16];
    uint64_t result_iops[16];
    uint64_t result_success[16];
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
    CPU_SET(g_numa[thread_id], &_mask);
    if (pthread_setaffinity_np(pthread_self(), sizeof(_mask), &_mask) < 0) {
        printf("threadpool, set thread affinity failed.\n");
    }

    DB* _db = param->db;
    YCSB* _benchmark = param->benchmark;

    assert((_benchmark != nullptr) && (_db != nullptr));
    _benchmark->initlizate();

    int _count = param->count;
    int _result;

    char _key[128];
    char _value[65535];
    size_t _key_length;
    size_t _value_length;

    Timer _t1, _t2;
    uint64_t _latency = 0;

    _t1.Start();
    for (int i = 0; i < _count; i++) {
        int test_type = _benchmark->get_kv_pair(_key, _key_length, _value, _value_length);
        _t2.Start();
        if (test_type == YCSB_PUT) {
            _db->Put(_key, _key_length, _value, _value_length);
        } else if (test_type == YCSB_UPDATE) {
            _db->Put(_key, _key_length, _value, _value_length);
        } else if (test_type == YCSB_GET) {
            _db->Get(_key, _key_length, _value, _value_length);
        } else if (test_type == YCSB_DELETE) {
        } else if (test_type == YCSB_RMW) {
        } else if (test_type == YCSB_SCAN) {
        }
        _t2.Stop();
        _latency = _t2.Get();
    }
    _t1.Stop();
}

WorkloadGenerator::WorkloadGenerator(struct generator_parameter* param, DB* db, YCSB* benchmarks[])
    : db_(db)
    , num_threads_(param->num_threads)
    , result_path_(param->result_path)
    , data_size_(param->data_size)
{
    for (int i = 0; i < num_threads_; i++) {
        benchmarks_[i] = benchmarks[i];
    }
}

void WorkloadGenerator::Run()
{
    std::thread _threads[32];
    thread_param_t _params[32];

    for (int i = 0; i < num_threads_; i++) {
        memset(&_params[i], 0, sizeof(thread_param_t));
        _params[i].thread_id = i;
        _params[i].benchmark = benchmarks_[i];
        _params[i].db = db_;
        _threads[i] = std::thread(thread_task, &_params[i]);
    }

    for (int i = 0; i < num_threads_; i++) {
        _threads[i].join();
    }

    /*
    uint64_t total_iops = 0;
    uint64_t avg_latency = 0;
    uint64_t get_succeed = 0;
    uint64_t put_succeed = 0;
    uint64_t scan_succeed = 0;
    uint64_t update_succeed = 0;
    size_t sum_bw = 0;

    for (int i = 0; i < num_threads_; i++) {
        uint64_t sum_opt = 0;
        uint64_t sum_latency_ns = 0;
        uint64_t avg_latency_ns;
        for (int j = 0; j < YCSB_TYPE_COUNT; j++) {
            sum_opt += thread_params[i].sum_count[j];
            sum_latency_ns += thread_params[i].sum_latency[j];
        }
        double total_time = 1.0 * thread_params[i].total_time / (1000 * 1000 * 1000);
        avg_latency_ns = 1.0 * thread_params[i].total_time / sum_opt;
        uint64_t iops_2 = 1000000000.0 / avg_latency_ns;
        double bw = thread_params[i].bandwidth / (1024 * 1024 * total_time);
        sum_bw += bw;
        LOG(INFO) << "|- [Total][Count:" << sum_opt << "][Time:" << total_time << "seconds][IOPS:" << iops_2 << "][Latency:" << avg_latency_ns << "ns]";
        total_iops += iops_2;
        avg_latency += avg_latency_ns;
        put_succeed += thread_params[i].put_succeed;
        update_succeed += thread_params[i].update_succeed;
        get_succeed += thread_params[i].get_succeed;
        scan_succeed += thread_params[i].scan_succeed;
    }

    LOG(INFO) << "|- [IOPS:" << total_iops << "][Latency:" << avg_latency / num_threads_ << "ns]";
    LOG(INFO) << "|- [BW:" << sum_bw << "MB/s]";
    mkdir("detail_latency", 0777);
    for (int i = 0; i < num_threads_; i++) {
        for (int j = 0; j < YCSB_TYPE_COUNT; j++) {
            if (vec_latency_result[i][j].size() > 0) {
                char name[128];
                snprintf(name, sizeof(name), "detail_latency/[thread%d]_[%s_in_%s]_[%zuB].result", i, operator_name[j], workload_name[benchmarks_[i]->GetType() >> 1], benchmarks_[i]->GetValueLength());
                result_output(name, vec_latency_result[i][j]);
                vec_latency_result[i][j].clear();
            }
        }
    }
    LOG(INFO) << "|- [OK_PUT:" << put_succeed << "][OK_UPDATE:" << update_succeed << "][OK_GET:" << get_succeed << "][OK_SCAN:" << scan_succeed << "]";
    LOG(INFO) << "|-------------------------------------------";
*/
}
