#include "rocksdb.h"
#include "rocksdb/table.h"

using namespace kv_benchmark;

RocksDB::RocksDB(kv_benchmark::Options& options)
{
    rocksdb::Options _options;
    _options.create_if_missing = true;

    // compression set
    _options.compression = rocksdb::kNoCompression;

    // memtable
    // write buffer size
    _options.write_buffer_size = 64 * 1024 * 1024;

    // cache
    // set block cache size
    std::shared_ptr<rocksdb::Cache> _cache = rocksdb::NewLRUCache(128 * 1024 * 1024);
    rocksdb::BlockBasedTableOptions _table_options;
    _table_options.block_cache = _cache;
    _options.table_factory.reset(NewBlockBasedTableFactory(_table_options));

    // IO handle
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
    _options.target_file_size_multiplier = 1;
    _options.max_compaction_bytes = 0;
    _options.max_background_compactions = 1; // only one backend compaction thread

    // Open an DB
    rocksdb::DB::Open(_options, options.db_path, &db_);
    assert(db_ != nullptr);
}

RocksDB::~RocksDB()
{
    printf("RocksDB::Close\n");
    db_->Close();
}

bool RocksDB::Put(char* key, size_t key_length, char* value, size_t value_length)
{
    rocksdb::Slice _key(key, key_length);
    rocksdb::Slice _value(value, value_length);
    rocksdb::Status _status = db_->Put(rocksdb::WriteOptions(), _key, _value);
    return (_status.ok() == true) ? true : false;
}

bool RocksDB::Delete(char* key, size_t key_length)
{
    rocksdb::Slice _key(key, key_length);
    rocksdb::Status _status = db_->Delete(rocksdb::WriteOptions(), _key);
    return (_status.ok() == true) ? true : false;
}

bool RocksDB::Update(char* key, size_t key_length, char* value, size_t value_length)
{
    rocksdb::Slice _key(key, key_length);
    rocksdb::Slice _value(value, value_length);
    rocksdb::Status _status = db_->Put(rocksdb::WriteOptions(), _key, _value);
    return (_status.ok() == true) ? true : false;
}

bool RocksDB::Get(char* key, size_t key_length, char* value, size_t& value_length)
{
    rocksdb::Slice _key(key, key_length);
    std::string _value;
    rocksdb::Status _status = db_->Get(rocksdb::ReadOptions(), _key, &_value);
    value_length = _value.size();
    memcpy(value, _value.data(), value_length);
    return (_status.ok() == true) ? true : false;
}

bool RocksDB::Scan(char* key, size_t key_length, size_t scan_count)
{
}

void RocksDB::Print()
{
}