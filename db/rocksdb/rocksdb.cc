#include "rocksdb.h"
#include "rocksdb/table.h"

using namespace kv_benchmark;

RocksDB::RocksDB(kv_benchmark::Options& options)
{
    rocksdb::Options _options;
    _options.create_if_missing = true;
    _options.compression = rocksdb::kNoCompression;
    _options.write_buffer_size = 64 * 1024 * 1024;

    // set block cache
    std::shared_ptr<rocksdb::Cache> _cache = rocksdb::NewLRUCache(8 * 1024 * 1024);
    rocksdb::BlockBasedTableOptions _table_options;
    _table_options.block_cache = _cache;
    _options.table_factory.reset(NewBlockBasedTableFactory(_table_options));

    // direct read/write
    _options.use_direct_reads = true;
    _options.use_direct_io_for_flush_and_compaction = true;

    // Open an DB
    rocksdb::DB::Open(_options, options.db_path, &db_);
    assert(db_ != nullptr);
}

RocksDB::~RocksDB()
{
    delete db_;
}

bool RocksDB::Put(char* key, size_t key_length, char* value, size_t value_length)
{
    rocksdb::Slice _key(key, key_length);
    rocksdb::Slice _value(value, value_length);
    rocksdb::Status _status = db_->Put(rocksdb::WriteOptions(), _key, _value);
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