#pragma once

#ifndef OS_WIN
#include <unistd.h>
#endif

#include <algorithm>
#include <atomic>
#include <cinttypes>
#include <condition_variable>
#include <cstddef>
#include <fcntl.h>
#include <fstream>
#include <inttypes.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/time.h>
#include <sys/types.h>
#include <thread>
#include <unordered_map>
#include <vector>

#include "distributions.h"
#include "random.h"
#include "rocksdb_benchmark_namespace.h"
#include "slice.h"

int64_t ConvertKeyToInt(std::string s, std::size_t key_size = 8);

namespace ROCKSDB_BENCHMARK_NAMESPACE {
enum QueryType {
    Get,
    Put,
    SeekForPrev,
    Delete,
    SingleDelete,
    Merge,
};

inline uint64_t NowTimeOfMicros()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return static_cast<uint64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
}

class Options {
public:
    std::string bench_name;
    int64_t flags_num;
    double read_random_exp_range;
    int64_t keys_per_prefix;
    int key_size;
    uint64_t rand_seed;
    int flags_ops_between_duration_checks;
    int64_t entries_per_batch, flags_multiread_stride;
    unsigned int flags_value_size_min, flags_value_size_max;
    int64_t flags_mix_max_scan_len, flags_mix_max_value_size;
    int64_t flags_avg_value_size;
    double flags_compression_ratio;
    double flags_mix_get_ratio, flags_mix_put_ratio, flags_mix_seek_ratio;
    //parameters for two term exponetial distributions
    double flags_keyrange_dist_a, flags_keyrange_dist_b, flags_keyrange_dist_c, flags_keyrange_dist_d;
    double flags_key_dist_a, flags_key_dist_b;
    // pareto distributions
    double flags_value_theta, flags_value_k, flags_value_sigma;
    double flags_iter_theta, flags_iter_k, flags_iter_sigma;
    int64_t keyrange_num, monitor_range_num;
    int32_t use_static_value_size;

    Options(std::string bench_name,
        int64_t flags_num, double read_random_exp_range,
        int64_t keys_per_prefix, int key_size, uint64_t rand_seed,
        int flags_ops_between_duration_checks,
        int64_t entries_per_batch, int64_t flags_multiread_stride,
        unsigned int flags_value_size_min, unsigned int flags_value_size_max,
        int64_t flags_mix_max_scan_len, int64_t flags_mix_max_value_size,
        int64_t flags_avg_value_size,
        double flags_compression_ratio,
        double flags_mix_get_ratio, double flags_mix_put_ratio, double flags_mix_seek_ratio,
        double flags_keyrange_dist_a, double flags_keyrange_dist_b, double flags_keyrange_dist_c, double flags_keyrange_dist_d,
        double flags_key_dist_a, double flags_key_dist_b,
        double flags_value_theta, double flags_value_k, double flags_value_sigma,
        double flags_iter_theta, double flags_iter_k, double flags_iter_sigma,
        int64_t keyrange_num, int64_t monitor_range_num,
        int32_t use_static_value_size)
        : bench_name(bench_name)
        , flags_num(flags_num)
        , read_random_exp_range(read_random_exp_range)
        , keys_per_prefix(keys_per_prefix)
        , key_size(key_size)
        , rand_seed(rand_seed)
        , flags_ops_between_duration_checks(flags_ops_between_duration_checks)
        , entries_per_batch(entries_per_batch)
        , flags_multiread_stride(flags_multiread_stride)
        , flags_value_size_min(flags_value_size_min)
        , flags_value_size_max(flags_value_size_max)
        , flags_mix_max_scan_len(flags_mix_max_scan_len)
        , flags_mix_max_value_size(flags_mix_max_value_size)
        , flags_avg_value_size(flags_avg_value_size)
        , flags_compression_ratio(flags_compression_ratio)
        , flags_mix_get_ratio(flags_mix_get_ratio)
        , flags_mix_put_ratio(flags_mix_put_ratio)
        , flags_mix_seek_ratio(flags_mix_seek_ratio)
        , flags_keyrange_dist_a(flags_keyrange_dist_a)
        , flags_keyrange_dist_b(flags_keyrange_dist_b)
        , flags_keyrange_dist_c(flags_keyrange_dist_c)
        , flags_keyrange_dist_d(flags_keyrange_dist_d)
        , flags_key_dist_a(flags_key_dist_a)
        , flags_key_dist_b(flags_key_dist_b)
        , flags_value_theta(flags_value_theta)
        , flags_value_k(flags_value_k)
        , flags_value_sigma(flags_value_sigma)
        , flags_iter_theta(flags_iter_theta)
        , flags_iter_k(flags_iter_k)
        , flags_iter_sigma(flags_iter_sigma)
        , keyrange_num(keyrange_num)
        , monitor_range_num(monitor_range_num)
        , use_static_value_size(use_static_value_size)
    {
    }
};

class Duration {
public:
    Duration(uint64_t max_seconds, int64_t max_ops, int64_t ops_per_stage = 0)
    {
        max_seconds_ = max_seconds;
        max_ops_ = max_ops;
        ops_per_stage_ = (ops_per_stage > 0) ? ops_per_stage : max_ops;
        ops_ = 0;
        start_at_ = NowTimeOfMicros();
    }

    int64_t GetStage() { return std::min(ops_, max_ops_ - 1) / ops_per_stage_; }

    bool Done(int64_t increment, int flags_ops_between_duration_checks)
    {
        if (increment <= 0)
            increment = 1; // avoid Done(0) and infinite loops
        ops_ += increment;

        if (max_seconds_) {
            // Recheck every appx 1000 ops (exact iff increment is factor of 1000)
            auto granularity = flags_ops_between_duration_checks;
            if ((ops_ / granularity) != ((ops_ - increment) / granularity)) {
                uint64_t now = NowTimeOfMicros();
                return ((now - start_at_) / 1000000) >= max_seconds_;
            } else {
                return false;
            }
        } else {
            return ops_ > max_ops_;
        }
    }

private:
    uint64_t max_seconds_;
    int64_t max_ops_;
    int64_t ops_per_stage_;
    int64_t ops_;
    uint64_t start_at_;
};

class PerformanceCounter {

    // a range bound equals interval [low, high)
    struct RangeCounter {
        int64_t low, high;
        int64_t access_times;
        int64_t read_times, write_times;
        RangeCounter(int64_t low, int64_t high, int64_t ori_access,
            int64_t ori_read, int64_t ori_write)
            : low(low)
            , high(high)
            , access_times(ori_access)
            , read_times(ori_read)
            , write_times(ori_write)
        {
        }
    };

private:
    int64_t total_keys;
    int64_t range_num;
    int64_t range_size;
    int64_t read_times, write_times, iter_times;
    int64_t total_times;
    int64_t write_bytes, read_bytes;
    const int64_t default_range_size = 100000;
    uint64_t start_time, end_time;
    std::vector<RangeCounter> range_list;
    std::mutex* mtx;

public:
    // constructor
    PerformanceCounter(int64_t total_keys, int64_t p_range_num)
        : total_keys(total_keys)
        , range_num(p_range_num)
        , read_times(0)
        , write_times(0)
        , iter_times(0)
        , write_bytes(0)
        , read_bytes(0)
        , total_times(0)
    {
        if (range_num > 0) {
            range_size = total_keys / range_num;
        } else {
            range_size = default_range_size;
            range_num = (total_keys - 1) / range_size + 1;
        }
        end_time = start_time = NowTimeOfMicros();
        mtx = new std::mutex();
    }

    ~PerformanceCounter() { delete mtx; }

    Status InitiateCounter()
    {
        range_list.clear();
        for (size_t i = 0; i < range_num - 1; ++i) {
            range_list.push_back(RangeCounter(i * range_size, (i + 1) * range_size, 0, 0, 0));
        }
        range_list.push_back(RangeCounter((range_num - 1) * range_size, total_keys + 1, 0, 0, 0));

        return Status::OK();
    }

    Status AddOpRecord(QueryType type, int64_t size, std::string key)
    {

        int64_t key_ = ConvertKeyToInt(key);
        size_t idx = key_ / range_size;
        mtx->lock();
        switch (type) {
        case Get:
            ++read_times;
            read_bytes += size;
            range_list[idx].read_times += 1;
            break;
        case Put:
            ++write_times;
            write_bytes += size;
            range_list[idx].write_times += 1;
            break;
        case SeekForPrev:
            ++iter_times;
            read_bytes += size;
        default:
            break;
        }
        ++total_times;
        range_list[idx].access_times += 1;
        // record timepoint
        end_time = NowTimeOfMicros();
        //printf("total times: %d\n", total_times);
        mtx->unlock();
        return Status::OK();
    }

    // StatusReport member function are showed as:
    //

    void PrintTable()
    {
        fprintf(stdout, "------------------------------------Range Counters--------------------------------------------------------------\n");
        printf("\t\t\tREAD\t\t\tWRITE\t\t\tTOTAL\t\t\tREAD RATIO\t\tWRITE RATIO\t\tITER RATIO\n");
        for (size_t i = 0; i < range_list.size(); ++i) {
            // print range bound first:
            //printf("[%" PRId64  ",%" PRId64 ")\t", range_list[i].low, range_list[i].high);
            printf("range%d\t\t", i);
            double range_read_ratio = (double)range_list[i].read_times / (read_times > 0 ? read_times : 1);
            double range_write_ratio = (double)range_list[i].write_times / (write_times > 0 ? write_times : 1);
            double range_total_ratio = (double)range_list[i].access_times / (total_times > 0 ? total_times : 1);
            double range_inter_read_ratio = (double)range_list[i].read_times / (range_list[i].access_times > 0 ? range_list[i].access_times : 1);
            double range_inter_write_ratio = (double)range_list[i].write_times / (range_list[i].access_times > 0 ? range_list[i].access_times : 1);
            printf("%8" PRId64 " (%.3lf%%)\t"
                   "%8" PRId64 " (%.3lf%%)\t"
                   "%8" PRId64 " (%.3lf%%)\t\t%.3lf%%\t\t\t%.3lf%%\n",
                range_list[i].read_times, range_read_ratio * 100,
                range_list[i].write_times, range_write_ratio * 100,
                range_list[i].access_times, range_total_ratio * 100,
                range_inter_read_ratio * 100,
                range_inter_write_ratio * 100);
        }
        double total_read_ratio = (double)read_times / (total_times > 0 ? total_times : 1);
        double total_write_ratio = (double)write_times / (total_times > 0 ? total_times : 1);
        double total_iter_ratio = (double)iter_times / (total_times > 0 ? total_times : 1);

        printf("Total\t\t%8" PRId64 " (%.3lf%%)\t%8" PRId64 " (%.3lf%%)\t%8" PRId64 " (%.3lf%%)\t\t%.3lf%%\t\t\t%.3lf%%\t\t\t%.3lf%%\n",
            read_times, (double)read_times / std::max(read_times, (int64_t)1) * 100,
            write_times, (double)write_times / std::max(write_times, (int64_t)1) * 100,
            total_times, (double)total_times / std::max(total_times, (int64_t)1) * 100,
            total_read_ratio * 100, total_write_ratio * 100,
            total_iter_ratio * 100);
        fprintf(stdout, "----------------------------------------------------------------------------------------------------------------\n");
    }

    void StatusReport()
    {
        fprintf(stdout, "Total Queries: %" PRId64 "\n", total_times);
        fprintf(stdout, "ReadSize: %.3lfMB\n", (double)read_bytes / (1024 * 1024));
        fprintf(stdout, "WriteSize: %.3lfMB\n", (double)write_bytes / (1024 * 1024));
        fprintf(stdout, "TimePassed: %.4lfs\n", (double)(end_time - start_time) / 1000000);
        PrintTable();
    }
};

struct SingleQuery {
    bool valid;
    QueryType type;
    std::string key, val;
    size_t key_size, val_size;
    size_t scan_len;

public:
    SingleQuery();
    SingleQuery(const SingleQuery& rhs);
    ~SingleQuery();
};

class Benchmark {
private:
    /* Decide which benchmark */
    std::string bench_name;
    bool initiated;
    /* general used parameters */
    int64_t flags_num;
    double read_random_exp_range_;
    Random64 rand;
    std::vector<std::string> keys_;
    int64_t keys_per_prefix;
    int key_size_;
    int prefix_size_;
    int64_t keys_per_prefix_;
    const int64_t default_value_max = 1 * 1024 * 1024;
    int flags_ops_between_duration_checks;
    int64_t entries_per_batch;
    int64_t flags_multiread_stride;
    // Note that value_max is not the same as flags_value_size_min or flags_value_size_max
    // the later two parameters are sued for generating distribution
    int64_t value_max = default_value_max;
    unsigned int flags_value_size_min, flags_value_size_max;
    //static unsigned int value_size = 100;
    constexpr bool static kLittleEndian = true;
    /* MixGraph parameters */
    int64_t flags_mix_max_scan_len, flags_mix_max_value_size;
    int64_t flags_avg_value_size;
    double flags_mix_get_ratio, flags_mix_put_ratio, flags_mix_seek_ratio;
    //Parameters for Two Terms Exponetial distributions
    double flags_keyrange_dist_a = 0.0, flags_keyrange_dist_b = 0.0;
    double flags_keyrange_dist_c = 0.0, flags_keyrange_dist_d = 0.0;
    double flags_key_dist_a, flags_key_dist_b;
    bool use_prefix_modeling = false;
    bool use_random_modeling = false;
    // Parameters for Pareto distributions
    double flags_value_theta, flags_value_k, flags_value_sigma;
    double flags_iter_theta, flags_iter_k, flags_iter_sigma;
    int64_t keyrange_num, monitor_range_num;
    int32_t use_static_value_size;
    /* query generators */
    GenerateTwoTermExpKeys gen_exp;
    RandomGenerator gen;
    QueryDecider query;
    PerformanceCounter counter;

public:
    Benchmark(Options options);

    Status InitiateBenchmark();

    SingleQuery GetSingleQuery();

    void GenerateKeyFromInt(uint64_t v, int64_t num_keys, Slice* key);

    Slice AllocateKey(std::unique_ptr<const char[]>* key_guard);

    void StatusReport();

private:
    int64_t GetRandomKey(Random64* rand);

    // Concrete Benchmarks:
    SingleQuery MixGraph();

    SingleQuery ReadRandom();

    SingleQuery MixRandom();
};

Options rocksdb_predef_bench_options(std::string bench_type);
Benchmark rocksdb_bench_tool(std::string bench_type);
}
