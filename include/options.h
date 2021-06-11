/*
 * @Author: your name
 * @Date: 2021-03-18 11:55:19
 * @LastEditTime: 2021-06-11 13:46:40
 * @LastEditors: Han Shukai
 * @Description: In User Settings Edit
 * @FilePath: /kv_bench/include/options.h
 */
#ifndef INCLUDE_OPTIONS_H_
#define INCLUDE_OPTIONS_H_

namespace kv_benchmark {
class Options {
public:
    Options()
        : db_path("dbdir")
        , nvm_path("/home/pmem0")
    {
        write_buffer_size = 64 * 1024 * 1024;
        num_backend_thread = 1;
    };

    ~Options() {};

public:
    std::string db_path;

    std::string nvm_path;

    size_t write_buffer_size;

    int num_backend_thread;
};
}

#endif