/*
 * @Date: 2021-04-17 11:58:39
 * @LastEditors: Han Shukai
 * @LastEditTime: 2021-06-09 11:03:32
 * @FilePath: /SplitKV/benchmark/go-ycsb/rocksdb_main.cc
 */

#include <algorithm>
#include <assert.h>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <pthread.h>
#include <queue>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <thread>
#include <time.h>
#include <unistd.h>
#include <vector>

#include "db.h"
#include "table.h"
#include "timer.h"

using namespace rocksdb;

const static uint32_t kNumThread = 8;
const static uint32_t kKeySize = 20;
const static uint32_t kValueSize = 4096;

#define OPT_TYPE_INSERT (1)
#define OPT_TYPE_UPDATE (2)
#define OPT_TYPE_READ (3)
#define OPT_TYPE_SCAN (4)

const char* g_ycsb_workload[] = { "workload/workloada.load", "workload/workloada.run" };

struct ycsb_operator_t {
public:
    uint8_t type_;
    std::string skew_;
    uint32_t other_; // for scan range record

public:
    ycsb_operator_t(uint8_t type, std::string& skew, uint32_t other)
        : type_(type)
        , skew_(skew)
        , other_(other)
    {
    }
};

struct thread_context_t {
public:
    uint32_t thread_id;
    DB* db;
    std::vector<ycsb_operator_t*>* vec_opt;
};

static std::vector<ycsb_operator_t*> g_vec_opt[kNumThread];

// READ usertable user6302928200575776280 [ field0 ]
static inline bool handle_read(std::ifstream& _fin, std::string& key)
{
    std::string _key;
    std::string _blank;

    _fin >> _blank; // usertable
    if (_blank != "usertable") {
        return false;
    }

    _fin >> _key; // key
    key = std::string(_key.begin() + 4, _key.end()); // skip user

    _fin >> _blank; // [
    _fin >> _blank; // field0
    _fin >> _blank; // ]
    return true;
}

// UPDATE usertable user6319145997088671705 [ field0=LbMRolQiuiSYinvnstaHBVUfUyHoeMIU ]
static inline bool handle_update(std::ifstream& _fin, std::string& key, std::string& value)
{
    std::string _key;
    std::string _value;
    std::string _blank;

    _fin >> _blank; // usertable
    if (_blank != "usertable") {
        return false;
    }

    _fin >> _key; // key
    key = std::string(_key.begin() + 4, _key.end()); // skip user

    _fin >> _blank; // [
    _fin >> _value; // value
    value = std::string(_value.begin() + 7, _value.end()); // skip field0=
    _fin >> _blank; // ]
    return true;
}

// INSERT usertable user4787722205795845578 [ field0=YgHLweyJStlEiEkwiKGvYfwCCtWGjVYX ]
static inline bool handle_insert(std::ifstream& _fin, std::string& key, std::string& value)
{
    std::string _key;
    std::string _value;
    std::string _blank;

    _fin >> _blank; // usertable
    if (_blank != "usertable") {
        return false;
    }

    _fin >> _key; // key
    key = std::string(_key.begin() + 4, _key.end()); // skip user

    _fin >> _blank; // [
    _fin >> _value; // value
    value = std::string(_value.begin() + 7, _value.end()); // skip field0=
    _fin >> _blank; // ]
    return true;
}

// SCAN usertable user6297265715691624980 337 [ field0 ]
static inline bool handle_scan(std::ifstream& _fin, std::string& key, uint32_t& range)
{
    std::string _key;
    std::string _blank;

    _fin >> _blank; // usertable
    if (_blank != "usertable") {
        return false;
    }

    _fin >> _key; // key
    key = std::string(_key.begin() + 4, _key.end()); // skip user

    _fin >> range; // range
    _fin >> _blank; // [
    _fin >> _blank; // field0
    _fin >> _blank; // ]
    return true;
}

void read_ycsb_file(const char* name)
{
    bool _res;
    uint32_t _num_opt = 0;
    std::ifstream _fin(name);

    uint8_t _type;
    std::string _line;
    std::string _key;
    std::string _value;
    uint32_t _range;

    if (_fin.is_open()) {
        while (_fin) {
            _fin >> _line;
            if (_line == "READ") {
                _res = handle_read(_fin, _key);
                if (_res) {
                    g_vec_opt[_num_opt % kNumThread].push_back(new ycsb_operator_t(OPT_TYPE_READ, _key, 0));
                }
                // std::cout << "READ " << _key << std::endl;
            } else if (_line == "UPDATE") {
                _res = handle_update(_fin, _key, _value);
                if (_res) {
                    g_vec_opt[_num_opt % kNumThread].push_back(new ycsb_operator_t(OPT_TYPE_UPDATE, _key, 0));
                }
                // std::cout << "UPDATE " << _key << " " << _value << std::endl;
            } else if (_line == "INSERT") {
                _res = handle_insert(_fin, _key, _value);
                if (_res) {
                    g_vec_opt[_num_opt % kNumThread].push_back(new ycsb_operator_t(OPT_TYPE_INSERT, _key, 0));
                }
                // std::cout << "INSERT " << _key << " " << _value << std::endl;
            } else if (_line == "SCAN") {
                _res = handle_scan(_fin, _key, _range);
                if (_res) {
                    g_vec_opt[_num_opt % kNumThread].push_back(new ycsb_operator_t(OPT_TYPE_SCAN, _key, _range));
                }
                // std::cout << "SCAN " << _key << " " << _range << std::endl;
            }
            _num_opt++;
        }
        _fin.close();
    }

    for (int i = 0; i < kNumThread; i++) {
        printf("[%d][SIZE:%zu]\n", i, g_vec_opt[i].size());
    }
}

static inline void generator_kv(std::string& skew, char* key, char* value)
{
    size_t _key_size = skew.size() > kKeySize ? kKeySize : skew.size();
    memset(key, 0, kKeySize);
    memcpy(key, skew.c_str(), _key_size);
    // size_t _value_size = skew.size() > kValueSize ? kValueSize : skew.size();
    // memset(value, 0, kValueSize);
    // memcpy(value, skew.c_str(), _value_size);
}

static void run_thread(thread_context_t* context)
{
    uint32_t _insert_cnt = 0;
    uint32_t _update_cnt = 0;
    uint32_t _read_cnt = 0;
    uint32_t _read_ok_cnt = 0;

    size_t _value_length;
    DB* _db = context->db;
    std::vector<ycsb_operator_t*>* _vec_opt = context->vec_opt;

    Timer _timer;
    printf("[thread_id:%d][num_opt:%zu]\n", context->thread_id, _vec_opt->size());
    _timer.Start();

    for (auto __iter = _vec_opt->begin(); __iter != _vec_opt->end(); __iter++) {
        ycsb_operator_t* __operator = *(__iter);
        char* __key = new char[kKeySize];
        char* __value = nullptr;
        generator_kv(__operator->skew_, __key, __value);
        if (__operator->type_ == OPT_TYPE_INSERT) {
            __value = new char[kValueSize];
            memcpy(__value, __key, kKeySize);
            Slice __dbkey(__key, kKeySize);
            Slice __dbvalue(__value, kValueSize);
            Status __status = _db->Put(WriteOptions(), __dbkey, __dbvalue);
            _insert_cnt++;
            delete __value;
        } else if (__operator->type_ == OPT_TYPE_UPDATE) {
            __value = new char[kValueSize];
            memcpy(__value, __key, kKeySize);
            Slice __dbkey(__key, kKeySize);
            Slice __dbvalue(__value, kValueSize);
            Status __status = _db->Put(WriteOptions(), __dbkey, __dbvalue);
            _update_cnt++;
            delete __value;
        } else if (__operator->type_ == OPT_TYPE_READ) {
            Slice __dbkey(__key, kKeySize);
            std::string __dbvalue;
            Status __status = _db->Get(ReadOptions(), __dbkey, &__dbvalue);
            _read_cnt++;
            if (__status.ok()) {
                _read_ok_cnt++;
            }
        }
        delete __key;
    }
    _timer.Stop();
    printf("[cost:%.2fseconds][iops:%.2f][insert/update:%llu/%llu][read:%llu/%llu]\n",
        _timer.GetSeconds(), 1.0 * _vec_opt->size() / _timer.GetSeconds(), _insert_cnt, _update_cnt, _read_ok_cnt, _read_cnt);
}

void run_workload(const char* ycsb, DB* db)
{
    read_ycsb_file(ycsb);
    std::thread _thread[kNumThread];
    for (uint32_t i = 0; i < kNumThread; i++) {
        thread_context_t* __context = new thread_context_t();
        __context->db = db;
        __context->thread_id = i;
        __context->vec_opt = &g_vec_opt[i];
        _thread[i] = std::thread(run_thread, __context);
    }
    for (uint32_t i = 0; i < kNumThread; i++) {
        _thread[i].join();
    }
}

int main(int argc, char** argv)
{
    DB* _db;
    Options _options;
    _options.create_if_missing = true;

    // compression set
    _options.compression = kNoCompression;

    // memtable
    _options.write_buffer_size = 16UL * 1024UL * 1024 * 1024;

    // cache
    // set block cache size
    std::shared_ptr<Cache> _cache = NewLRUCache(128 * 1024 * 1024);
    BlockBasedTableOptions _table_options;
    _table_options.block_cache = _cache;
    _options.table_factory.reset(NewBlockBasedTableFactory(_table_options));

    // IO handle
    // direct read/write
    _options.use_direct_reads = true;
    _options.use_direct_io_for_flush_and_compaction = true;
    _options.wal_bytes_per_sync = false; // foce sync log

    // compaction
    // _options.level0_file_num_compaction_trigger = 10;
    // level based compaction style
    // *** kCompactionStyleLevel = 0x0,
    // Universal compaction style
    // Not supported in ROCKSDB_LITE.
    // *** kCompactionStyleUniversal = 0x1, (Tiered)
    // FIFO compaction style
    // Not supported in ROCKSDB_LITE
    // *** kCompactionStyleFIFO = 0x2,
    // Disable background compaction. Compaction jobs are submitted
    // via CompactFiles().
    // Not supported in ROCKSDB_LITE
    // *** kCompactionStyleNone = 0x3,
    _options.compaction_style = kCompactionStyleLevel;
    // _options.compaction_options_universal;
    // _options.disable_auto_compactions;
    // _options.compaction_filter;
    // _options.compaction_filter_factory;
    _options.target_file_size_base = 64 * 1024 * 1024; // default
    // _options.target_file_size_multiplier = 1;
    // _options.max_compaction_bytes = 0;
    _options.max_background_compactions = 2; // only one backend compaction thread

    // Open an DB
    rocksdb::DB::Open(_options, "/home/hanshukai/mount/4800/rocksdb", &_db);
    assert(_db != nullptr);

    run_workload(g_ycsb_workload[0], _db);
    run_workload(g_ycsb_workload[1], _db);
    delete _db;
    return 0;
}