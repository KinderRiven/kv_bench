CC = g++
TARGET = db_bench
SRC = ../code/distributions.cc ../code/rocksdb_benchmark_tool.cc
TEST_SRC = qtest.cc
CLIB = -pthread -lpmemkv
STD = -std=c++17
INCLUDE = ../code 
CFLAGS = -Wall -g
PMEMKV	= ../../../build/src

$(TARGET): $(SRC) $(TEST_SRC)
	$(CC) $(CFLAGS) $(SRC) $(TEST_SRC) -o $(TARGET) -I $(INCLUDE) \
	-I ../../../include -I ../../../include/pmdk -I ../../../lib -I ../../../include/components \
	$(CLIB) -L $(PMEMKV)

clean:
	rm -rf *.o $(TARGET)  