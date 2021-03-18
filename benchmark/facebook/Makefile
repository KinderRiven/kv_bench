SRC_LEVELDB := -I../../db/leveldb ../../db/leveldb/leveldb.cc ../../db/leveldb/db.cc
SRC_ROCKSDB := -I../../db/rocksdb ../../db/rocksdb/rocksdb.cc ../../db/rocksdb/db.cc
SRC_NOVELSM := -I../../db/novelsm ../../db/novelsm/novelsm.cc ../../db/novelsm/db.cc
SRC_PEBBLESDB := -I../../db/pebblesdb ../../db/pebblesdb/pebblesdb.cc ../../db/pebblesdb/db.cc
SRC_MATRIXKV := -I../../db/matrixkv ../../db/matrixkv/matrixkv.cc ../../db/matrixkv/db.cc
SRC_YCSB := main.cc workload_generator.cc distributions.cc rocksdb_benchmark_tool.cc

all: leveldb rocksdb novelsm pebblesdb matrixkv detail

detail:
	g++ -std=c++11 detail.cc -o detail

matrixkv:
	g++ -std=c++11 -I../../include $(SRC_MATRIXKV) $(SRC_YCSB) -o matrixkv_tester -L../../third-party/matrixkv -lrocksdb -lpmem -ljemalloc -ldl -lpthread -lsnappy -lgflags -lz -lbz2 -llz4 -lzstd

pebblesdb:
	g++ -std=c++11 -I../../include $(SRC_PEBBLESDB) $(SRC_YCSB) -o pebblesdb_tester -L../../third-party/pebblesdb -lpebblesdb -lpthread -lsnappy

leveldb:
	g++ -std=c++11 -I../../include $(SRC_LEVELDB) $(SRC_YCSB) -o leveldb_tester -L../../third-party/leveldb -lleveldb -lpthread -lsnappy

novelsm:
	g++ -std=c++11 -I../../include $(SRC_NOVELSM) $(SRC_YCSB) -o novelsm_tester -L../../third-party/novelsm -lleveldb -lpthread -lsnappy -lnuma

rocksdb:
	g++ -std=c++11 -I../../include $(SRC_ROCKSDB) $(SRC_YCSB) -o rocksdb_tester -L../../third-party/rocksdb -lrocksdb -ljemalloc -ldl -lpthread -lsnappy -lgflags -lz -lbz2 -llz4 -lzstd
