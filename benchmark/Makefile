SRC_LEVELDB := -I../db/leveldb ../db/leveldb/leveldb.cc ../db/leveldb/db.cc
SRC_ROCKSDB := -I../db/rocksdb ../db/rocksdb/rocksdb.cc ../db/rocksdb/db.cc
SRC_NOVELSM := -I../db/novelsm ../db/novelsm/novelsm.cc ../db/novelsm/db.cc
SRC_PEBBLESDB := -I../db/pebblesdb ../db/pebblesdb/pebblesdb.cc ../db/pebblesdb/db.cc
SRC_MATRIXKV := -I../db/matrixkv ../db/matrixkv/matrixkv.cc ../db/matrixkv/db.cc


all: leveldb rocksdb novelsm pebblesdb matrixkv

matrixkv:
	g++ -std=c++11 -I../include $(SRC_MATRIXKV) test.cc -o matrixkv_tester -L../third-party/matrixkv -lrocksdb -ljemalloc -ldl -lpthread -lsnappy -lgflags -lz -lbz2 -llz4 -lzstd

pebblesdb:
	g++ -std=c++11 -I../include $(SRC_PEBBLESDB) test.cc -o pebblesdb_tester -L../third-party/pebblesdb -lpebblesdb -lpthread -lsnappy

leveldb:
	g++ -std=c++11 -I../include $(SRC_LEVELDB) test.cc -o leveldb_tester -L../third-party/leveldb -lleveldb -lpthread -lsnappy

novelsm:
	g++ -std=c++11 -I../include $(SRC_NOVELSM) test.cc -o novelsm_tester -L../third-party/novelsm -lleveldb -lpthread -lsnappy -lnuma

rocksdb:
	g++ -std=c++11 -I../include $(SRC_ROCKSDB) test.cc -o rocksdb_tester -L../third-party/rocksdb -lrocksdb -ljemalloc -ldl -lpthread -lsnappy -lgflags -lz -lbz2 -llz4 -lzstd
