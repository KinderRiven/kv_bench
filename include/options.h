/*
 * @Author: your name
 * @Date: 2021-03-18 11:55:19
 * @LastEditTime: 2021-03-26 10:46:35
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: /kv_bench/include/options.h
 */
#ifndef INCLUDE_OPTIONS_H_
#define INCLUDE_OPTIONS_H_

namespace kv_benchmark {
class Options {
public:
    Options() : db_path("dbdir"), nvm_path("/home/pmem0") {};

    ~Options(){};

public:
    std::string db_path;

    std::string nvm_path;

    size_t write_buffer_size;

    int num_backend_thread;
};
}

#endif