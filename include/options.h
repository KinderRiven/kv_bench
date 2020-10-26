#ifndef INCLUDE_OPTIONS_H_
#define INCLUDE_OPTIONS_H_

namespace kv_benchmark {
class Options {
public:
    Options() : db_path("dbdir"), nvm_path("/home/pmem0") {};

    ~Options(){};

public:
    std::string db_path;

    std::string nvm_path;
};
}

#endif