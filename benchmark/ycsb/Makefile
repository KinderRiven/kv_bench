### RocksDB ###
SRC_ROCKSDB := -I../../db/rocksdb ../../db/rocksdb/rocksdb.cc ../../db/rocksdb/db.cc
LIB_ROCKSDB := -L/home/hanshukai/LightRocksDB -lrocksdb -ljemalloc -ldl -lpthread -lsnappy -lgflags -lz -lbz2 -llz4 -lzstd
# LIB_ROCKSDB := -L../../third-party/rocksdb

### MatrixKV ###
SRC_MATRIXKV := -I../../db/matrixkv ../../db/matrixkv/matrixkv.cc ../../db/matrixkv/db.cc
LIB_MATRIXKV := -L../../third-party/matrixkv -lrocksdb -lpmem -ljemalloc -ldl -lpthread -lsnappy -lgflags -lz -lbz2 -llz4 -lzstd

### SILK ###
SRC_SILK := -I../../db/silk ../../db/silk/silk.cc ../../db/silk/db.cc
LIB_SILK := -L/home/hanshukai/SILK-USENIXATC2019 -lrocksdb -ljemalloc -ldl -lpthread -lsnappy -lgflags -lz -lbz2 -llz4 -lzstd
# LIB_ROCKSDB := -L../../third-party/rocksdb


### LevelDB ###
SRC_LEVELDB := -I../../db/leveldb ../../db/leveldb/leveldb.cc ../../db/leveldb/db.cc

### NoveLSM ###
SRC_NOVELSM := -I../../db/novelsm ../../db/novelsm/novelsm.cc ../../db/novelsm/db.cc

### PebblesDB ###
SRC_PEBBLESDB := -I../../db/pebblesdb ../../db/pebblesdb/pebblesdb.cc ../../db/pebblesdb/db.cc


###
SRC_YCSB := main.cc workload_generator.cc workload_ycsb.c
LIB := -L../../third-party/tbb -ltbb

all: rocksdb matrixkv detail

detail:
	g++ -std=c++11 detail.cc -o detail

rocksdb:
	g++ -std=c++11 -I../../include $(SRC_ROCKSDB) $(SRC_YCSB) -o rocksdb_tester $(LIB) $(LIB_ROCKSDB)

matrixkv:
	g++ -std=c++11 -I../../include $(SRC_MATRIXKV) $(SRC_YCSB) -o matrixkv_tester $(LIB) $(LIB_MATRIXKV)

silk:
	g++ -std=c++11 -I../../include $(SRC_SILK) $(SRC_YCSB) -o matrixkv_tester $(LIB) $(LIB_SILK)

pebblesdb:
	g++ -std=c++11 -I../../include $(SRC_PEBBLESDB) $(SRC_YCSB) -o pebblesdb_tester -L../../third-party/pebblesdb -lpebblesdb -lpthread -lsnappy

leveldb:
	g++ -std=c++11 -I../../include $(SRC_LEVELDB) $(SRC_YCSB) -o leveldb_tester -L../../third-party/leveldb -lleveldb -lpthread -lsnappy

novelsm:
	g++ -std=c++11 -I../../include $(SRC_NOVELSM) $(SRC_YCSB) -o novelsm_tester -L../../third-party/novelsm -lleveldb -lpthread -lsnappy -lnuma