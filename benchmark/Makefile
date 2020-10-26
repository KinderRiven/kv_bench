SRC_LEVELDB := -I../db/leveldb ../db/leveldb/leveldb.cc ../db/leveldb/db.cc
SRC_ROCKSDB := -I../db/rocksdb ../db/rocksdb/rocksdb.cc ../db/rocksdb/db.cc


all: leveldb rocksdb

leveldb:
	g++ -std=c++11 -I../include $(SRC_LEVELDB) test.cc -o leveldb_tester -L../third-party/leveldb -lleveldb -lpthread -lsnappy

rocksdb:
	g++ -std=c++11 -I../include $(SRC_ROCKSDB) test.cc -o rocksdb_tester -L../third-party/rocksdb -lrocksdb -ljemalloc -ldl -lpthread -lsnappy -lgflags -lz -lbz2 -llz4 -lzstd
