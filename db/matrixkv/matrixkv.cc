/*
 * @Author: your name
 * @Date: 2021-03-18 11:55:19
 * @LastEditTime: 2021-03-26 10:49:43
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /kv_bench/db/matrixkv/matrixkv.cc
 */
#include "matrixkv.h"
#include "rocksdb/nvm_option.h"
#include "rocksdb/options.h"
#include "rocksdb/table.h"

using namespace kv_benchmark;

MatrixKV::MatrixKV(kv_benchmark::Options& options)
{
    rocksdb::Options _options;
    _options.create_if_missing = true;
    _options.compression = rocksdb::kNoCompression;

    rocksdb::NvmSetup* _nvm_opt = new rocksdb::NvmSetup();
    _nvm_opt->pmem_path = "/home/pmem0";

    _nvm_opt->Level0_column_compaction_trigger_size = 2UL * 1024 * 1024 * 1024;
    _nvm_opt->Level0_column_compaction_slowdown_size = 2UL * 1024 * 1024 * 1024 + 512UL * 1024 * 1024;
    _nvm_opt->Level0_column_compaction_stop_size = 3UL * 1024 * 1024 * 1024;
    //column compaction时没有L1文件交集时,至少选择L0数据量进行column compaction的文件个数
    _nvm_opt->Column_compaction_no_L1_select_L0 = 4;
    //column compaction时有L1文件交集时,至少选择L0数据量进行column compaction的文件个数
    _nvm_opt->Column_compaction_have_L1_select_L0 = 2;

    _nvm_opt->use_nvm_module = true;
    _options.nvm_setup.reset(_nvm_opt);

    // set block cache
    std::shared_ptr<rocksdb::Cache> _cache = rocksdb::NewLRUCache(8 * 1024 * 1024);
    rocksdb::BlockBasedTableOptions _table_options;
    _table_options.block_cache = _cache;
    _options.table_factory.reset(NewBlockBasedTableFactory(_table_options));

    _options.write_buffer_size = options.write_buffer_size;

    // direct read/write
    _options.use_direct_reads = true;
    _options.use_direct_io_for_flush_and_compaction = true;

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
    _options.compaction_style = rocksdb::kCompactionStyleLevel;
    // _options.compaction_options_universal;
    // _options.disable_auto_compactions;
    // _options.compaction_filter;
    // _options.compaction_filter_factory;
    _options.target_file_size_base = 64 * 1024 * 1024; // default
    // _options.target_file_size_multiplier = 1;
    // _options.max_compaction_bytes = 0;
    _options.max_background_compactions = options.num_backend_thread; // only one backend compaction thread

    rocksdb::DB::Open(_options, options.db_path, &db_);
    assert(db_ != nullptr);
}

MatrixKV::~MatrixKV()
{
    delete db_;
}

void MatrixKV::Close()
{
    printf("MatrixKV::Close\n");
    db_->Close();
}

bool MatrixKV::Put(char* key, size_t key_length, char* value, size_t value_length)
{
    rocksdb::Slice _key(key, key_length);
    rocksdb::Slice _value(value, value_length);
    rocksdb::Status _status = db_->Put(rocksdb::WriteOptions(), _key, _value);
    return (_status.ok() == true) ? true : false;
}

bool MatrixKV::Delete(char* key, size_t key_length)
{
    rocksdb::Slice _key(key, key_length);
    rocksdb::Status _status = db_->Delete(rocksdb::WriteOptions(), _key);
    return (_status.ok() == true) ? true : false;
}

bool MatrixKV::Update(char* key, size_t key_length, char* value, size_t value_length)
{
    rocksdb::Slice _key(key, key_length);
    rocksdb::Slice _value(value, value_length);
    rocksdb::Status _status = db_->Put(rocksdb::WriteOptions(), _key, _value);
    return (_status.ok() == true) ? true : false;
}

bool MatrixKV::Get(char* key, size_t key_length, char* value, size_t& value_length)
{
    rocksdb::Slice _key(key, key_length);
    std::string _value;
    rocksdb::Status _status = db_->Get(rocksdb::ReadOptions(), _key, &_value);
    value_length = _value.size();
    memcpy(value, _value.data(), value_length);
    return (_status.ok() == true) ? true : false;
}

bool MatrixKV::Scan(char* key, size_t key_length, size_t scan_count)
{
}

void MatrixKV::Print()
{
}