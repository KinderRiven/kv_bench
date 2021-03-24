SRC_LEVELDB := -I../../db/leveldb ../../db/leveldb/leveldb.cc ../../db/leveldb/db.cc
SRC_ROCKSDB := -I../../db/rocksdb ../../db/rocksdb/rocksdb.cc ../../db/rocksdb/db.cc
SRC_NOVELSM := -I../../db/novelsm ../../db/novelsm/novelsm.cc ../../db/novelsm/db.cc
SRC_PEBBLESDB := -I../../db/pebblesdb ../../db/pebblesdb/pebblesdb.cc ../../db/pebblesdb/db.cc
SRC_MATRIXKV := -I../../db/matrixkv ../../db/matrixkv/matrixkv.cc ../../db/matrixkv/db.cc
SRC_YCSB := main.cc workload_generator.cc workload_ycsb.c

LIB_ROCKSDB := -L/home/hanshukai/rocksdb
# LIB_ROCKSDB := -L../../third-party/rocksdb
LIB_MATRIXKV := -L../../third-party/matrixkv

LIB := -L../../third-party/tbb

all: leveldb rocksdb novelsm pebblesdb matrixkv detail

detail:
	g++ -std=c++11 detail.cc -o detail

rocksdb:
	g++ -std=c++11 -I../../include $(SRC_ROCKSDB) $(SRC_YCSB) -o rocksdb_tester $(LIB) $(LIB_ROCKSDB) -lrocksdb -ljemalloc -ldl -lpthread -lsnappy -lgflags -lz -lbz2 -llz4 -lzstd -ltbb

matrixkv:
	g++ -std=c++11 -I../../include $(SRC_MATRIXKV) $(SRC_YCSB) -o matrixkv_tester $(LIB) $(LIB_MATRIXKV) -lrocksdb -lpmem -ljemalloc -ldl -lpthread -lsnappy -lgflags -lz -lbz2 -llz4 -lzstd

pebblesdb:
	g++ -std=c++11 -I../../include $(SRC_PEBBLESDB) $(SRC_YCSB) -o pebblesdb_tester -L../../third-party/pebblesdb -lpebblesdb -lpthread -lsnappy

leveldb:
	g++ -std=c++11 -I../../include $(SRC_LEVELDB) $(SRC_YCSB) -o leveldb_tester -L../../third-party/leveldb -lleveldb -lpthread -lsnappy

novelsm:
	g++ -std=c++11 -I../../include $(SRC_NOVELSM) $(SRC_YCSB) -o novelsm_tester -L../../third-party/novelsm -lleveldb -lpthread -lsnappy -lnuma