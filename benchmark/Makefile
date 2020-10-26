SRC_LEVELDB := ../db/leveldb/leveldb.cc ../db/leveldb/db.cc
SRC_ROCKSDB := ../db/rocksdb/rocksdb.cc ../db/rocksdb/db.cc


all: leveldb rocksdb

leveldb:
	g++ -std=c++11 -I../include -I../db/leveldb $(SRC_LEVELDB) test.cc -o leveldb_tester -L../third-party/leveldb -lleveldb -lpthread -lsnappy

rocksdb:
	g++ -std=c++11 -I../include -I../db/rocksdb $(SRC_ROCKSDB) test.cc -o rocksdb_tester -L../third-party/rocksdb -lrocksdb -ljemalloc -ldl -lpthread -lsnappy -lgflags -lz -lbz2 -llz4 -lzstd
