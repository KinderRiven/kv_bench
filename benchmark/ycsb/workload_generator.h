#ifndef INCLUDE_WORKLOAD_GENERATOR_H_
#define INCLUDE_WORKLOAD_GENERATOR_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "db.h"
#include "ycsb.h"

namespace kv_benchmark {

struct generator_parameter {
public:
    int num_threads;

    int record_detail;

    size_t key_length;

    size_t value_length;

    uint64_t data_size;

    std::string result_path;
};

class WorkloadGenerator {
public:
    WorkloadGenerator(struct generator_parameter *param, DB *db, YCSB *benchmark[]);

public:
    void Run();

    void Print();

private:
    int num_threads_;

    int record_detail_;

    size_t data_size_;

    size_t key_length_;

    size_t value_length_;

    std::string result_path_;

    DB* db_;

    YCSB* benchmarks_[32];
};
};

#endif