SRC_LEVELDB := -I../db/leveldb ../db/leveldb/leveldb.cc ../db/leveldb/db.cc
SRC_ROCKSDB := -I../db/rocksdb ../db/rocksdb/rocksdb.cc ../db/rocksdb/db.cc
SRC_NOVELSM := -I../db/novelsm ../db/novelsm/novelsm.cc ../db/novelsm/db.cc

all: leveldb rocksdb novelsm

leveldb:
	g++ -std=c++11 -I../include $(SRC_LEVELDB) test.cc -o leveldb_tester -L../third-party/leveldb -lleveldb -lpthread -lsnappy

leveldb:
	g++ -std=c++11 -I../include $(SRC_NOVELSM) test.cc -o novelsm_tester -L../third-party/novelsm -lleveldb -lpthread -lsnappy

rocksdb:
	g++ -std=c++11 -I../include $(SRC_ROCKSDB) test.cc -o rocksdb_tester -L../third-party/rocksdb -lrocksdb -ljemalloc -ldl -lpthread -lsnappy -lgflags -lz -lbz2 -llz4 -lzstd
