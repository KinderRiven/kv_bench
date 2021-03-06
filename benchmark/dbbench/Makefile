CC := g++

LIB_PATH := /home/hanshukai/SplitKV/third-party

# third-party lib
LIB_DPDK_PATH := /home/hanshukai/import_libs/spdk/dpdk/build/lib
LIB_SPDK_PATH := /home/hanshukai/import_libs/spdk/build/lib
LIB_PMDK_PATH := /home/hanshukai/import_libs/pmdk/src/nondebug
LIB_TBB_PATH := /home/hanshukai/SplitKV/third-party/tbb

LINK_DPDK := -Wl,--whole-archive -L$(LIB_DPDK_PATH) \
    -lrte_eal -lrte_mempool -lrte_ring -lrte_mbuf -lrte_mempool_ring -lrte_pci \
    -lrte_bus_pci -lrte_kvargs -lrte_vhost -lrte_net -lrte_hash -lrte_telemetry \
    -lrte_cryptodev -lrte_power -lrte_rcu

LINK_SPDK := -Wl,--whole-archive -L$(LIB_SPDK_PATH) \
    -lspdk_env_dpdk  -lspdk_env_dpdk_rpc -lspdk_json \
    -lspdk_jsonrpc -lspdk_rpc \
    -lspdk_event_net -lspdk_event_vmd -lspdk_event \
    -lspdk_thread -lspdk_sock_posix -lspdk_sock -lspdk_notify -lspdk_net -lspdk_nvme \
    -lspdk_log -lspdk_trace -lspdk_util -lspdk_conf -lspdk_vmd

LINK_PMDK := -L$(LIB_PMDK_PATH) -lpmem -lpmemobj -lpmempool

LINK_FLAGS := -L$(LIB_TBB_PATH) -lpthread -lrt -lnuma -ldl -luuid -lm -lisal -ltbb

# LevelDB
INC_LEVELDB := -I../../db/leveldb -I../../include
SRC_LEVELDB := ../../db/leveldb/leveldb.cc ../../db/leveldb/db.cc
LIB_LEVELDB := $(LIB_PATH)/leveldb
LINK_LEVELDB := -L$(LIB_LEVELDB) -lleveldb -ljemalloc -lsnappy 

# RocksDB
INC_ROCKSDB := -I../../db/rocksdb -I../../include
SRC_ROCKSDB := ../../db/rocksdb/rocksdb.cc ../../db/rocksdb/db.cc
LIB_ROCKSDB := $(LIB_PATH)/rocksdb
LINK_ROCKSDB := -L$(LIB_ROCKSDB) -lrocksdb -ljemalloc -lsnappy -lgflags -lz -lbz2 -llz4 -lzstd

# NoveLSM
INC_NOVELSM := -I../../db/novelsm -I../../include
SRC_NOVELSM := ../../db/novelsm/novelsm.cc ../../db/novelsm/db.cc
LIB_NOVELSM := $(LIB_PATH)/novelsm
LINK_NOVELSM := -L$(LIB_NOVELSM) -lleveldb -ljemalloc -lsnappy 

# PebblesDB
INC_PEBBLESDB := -I../../db/pebblesdb -I../../include
SRC_PEBBLESDB := ../../db/pebblesdb/pebblesdb.cc ../../db/pebblesdb/db.cc
LIB_PEBBLESDB := $(LIB_PATH)/pebblesdb
LINK_PEBBLESDB := -L$(LIB_PEBBLESDB) -lpebblesdb -ljemalloc -lsnappy -lgflags -lz -lbz2 -llz4 -lzstd

# MatrixKV
INC_MATRIXKV := -I../../db/matrixkv -I../../include
SRC_MATRIXKV := ../../db/matrixkv/matrixkv.cc ../../db/matrixkv/db.cc
LIB_MATRIXKV := $(LIB_PATH)/matrixkv
LINK_MATRIXKV := -L$(LIB_MATRIXKV) -lrocksdb -ljemalloc -lsnappy -lgflags -lz -lbz2 -llz4 -lzstd $(LINK_PMDK)

# SILK
INC_SILK := -I../../db/silk -I../../include
SRC_SILK := ../../db/silk/silk.cc ../../db/silk/db.cc
LIB_SILK := $(LIB_PATH)/silk
LINK_SILK := -L$(LIB_SILK) -lrocksdb -ljemalloc -lsnappy -lgflags -lz -lbz2 -llz4 -lzstd

# SplitKV

# SpanDB

all: clean leveldb novelsm pebblesdb rocksdb matrixkv silk

leveldb:
	$(CC) --std=c++11 main.cc workload_generator.cc -o leveldb_tester $(INC_LEVELDB) $(SRC_LEVELDB) $(LINK_LEVELDB) $(LINK_FLAGS)

rocksdb:
	$(CC) --std=c++11 main.cc workload_generator.cc -o rocksdb_tester $(INC_ROCKSDB) $(SRC_ROCKSDB) $(LINK_ROCKSDB) $(LINK_FLAGS)

pebblesdb:
	$(CC) --std=c++11 main.cc workload_generator.cc -o pebblesdb_tester $(INC_PEBBLESDB) $(SRC_PEBBLESDB) $(LINK_PEBBLESDB) $(LINK_FLAGS)

silk:
	$(CC) --std=c++11 main.cc workload_generator.cc -o silk_tester $(INC_SILK) $(SRC_SILK) $(LINK_SILK) $(LINK_FLAGS)

matrixkv:
	$(CC) --std=c++11 main.cc workload_generator.cc -o matrixkv_tester $(INC_MATRIXKV) $(SRC_MATRIXKV) $(LINK_MATRIXKV) $(LINK_FLAGS)

novelsm:
	$(CC) --std=c++11 main.cc workload_generator.cc -o novelsm_tester $(INC_NOVELSM) $(SRC_NOVELSM) $(LINK_NOVELSM) $(LINK_FLAGS)

clean:
	rm -rf rocksdb_tester pebblesdb_tester matrixkv_tester novelsm_tester leveldb_tester silk_tester

export:
	export LD_LIBRARY_PATH=/home/hanshukai/import_libs/pmdk/src/nondebug:/home/hanshukai/import_libs/spdk/build/lib:/home/hanshukai/import_libs/spdk/dpdk/build/lib:/home/hanshukai/SplitKV/third-party/tbb
