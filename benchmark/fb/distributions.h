#pragma once

#include <vector>
#include <assert.h>
#include <memory>
#include <algorithm>

#include "random.h"
#include "slice.h"
#include "status.h"


namespace ROCKSDB_BENCHMARK_NAMESPACE{
  // KeyrangeUnit is the struct of a keyrange. It is used in a keyrange vector
  // to transfer a random value to one keyrange based on the hotness.
  struct KeyrangeUnit {
    int64_t keyrange_start;
    int64_t keyrange_access;
    int64_t keyrange_keys;
  };

  // generate random string:
  Slice RandomString(Random *rnd, int len, std::string *dst);

  Slice CompressibleString(Random *rnd, double compressed_fraction,
                           int len, std::string *dst);

class GenerateTwoTermExpKeys {
  public:
    int64_t keyrange_rand_max_;
    int64_t keyrange_size_;
    int64_t keyrange_num_;
    bool initiated_;
    std::vector<KeyrangeUnit> keyrange_set_;

    GenerateTwoTermExpKeys() {
        initiated_ = false;
    }

    ~GenerateTwoTermExpKeys() {}

    // Initiate the KeyrangeUnit vector and calculate the size of each 
    // KeyrangeUnit.
    Status InitiateExpDistribution(int64_t flags_num, int64_t flags_keyrange_num, int64_t total_keys,
                                   double prefix_a, double prefix_b, double prefix_c, double prefix_d);

    // Generate the Key ID according to the input ini_rand
    int64_t DistGetKeyID(int64_t ini_rand, double key_dist_a, double key_dist_b);
};

class QueryDecider {
  public:
   std::vector<int> type_;
   std::vector<double> ratio_;
   int range_;

   QueryDecider() {}
   ~QueryDecider() {}

   Status Initiate(std::vector<double> ratio_input);
   int GetType(int64_t rand_num);
};

// inverse function of Pareto distribution
int64_t ParetoCdfInversion(double u, double theta, double k, double sigma);

// inverse function of power distribution (y = ax^b)
int64_t PowerCdfInversion(double u, double a, double b);

// Add the noise to the QPS
double AddNoise(double origin, double noise_ratio, double flags_sine_a);

// following are random generating distributions:

enum DistributionType : unsigned char {
  kFixed = 0,
  kUniform,
  kNormal
};

static enum DistributionType flags_value_size_distribution_type_e = kFixed;

class BaseDistribution {
 public:
  BaseDistribution(unsigned int min, unsigned int max) :
    min_value_size_(min),
    max_value_size_(max) {}
  virtual ~BaseDistribution() {}

  unsigned int Generate() {
    auto val = Get();
    if (NeedTruncate()) {
      val = std::max(min_value_size_, val);
      val = std::min(max_value_size_, val);
    }
    return val;
  }
 private:
  virtual unsigned int Get() = 0;
  virtual bool NeedTruncate() {
    return true;
  }
  unsigned int min_value_size_;
  unsigned int max_value_size_;
};

//Uniform Distribution
class UniformDistribution
    : public BaseDistribution,
      public std::uniform_int_distribution<unsigned int> {
 public:
  UniformDistribution(unsigned int min_, unsigned int max_) :
    BaseDistribution(min_, max_),
    std::uniform_int_distribution<unsigned int>(min_, max_),
    gen_(rd_()) {}
 private:
  virtual unsigned int Get() override {
    return (*this)(gen_);
  }
  virtual bool NeedTruncate() override {
    return false;
  }
  std::random_device rd_;
  std::mt19937 gen_;
};

// Normal Distribution
class NormalDistribution
    : public BaseDistribution, 
      public std::normal_distribution<double> {
 public:
  NormalDistribution(unsigned int min_, unsigned int max_) :
    BaseDistribution(min_, max_),
    // 99.7% values within the range [min, max].
    std::normal_distribution<double>((double)(min_ + max_) / 2.0 /*mean*/,
                                     (double)(max_ - min_) / 6.0 /*stddev*/),
    gen_(rd_()) {}
 private:
  virtual unsigned int Get() override {
    return static_cast<unsigned int>((*this)(gen_));
  }
  std::random_device rd_;
  std::mt19937 gen_;
};

// Fix Distribution
class FixedDistribution : public BaseDistribution
{
 public:
  FixedDistribution(unsigned int size) :
    BaseDistribution(size, size),
    size_(size) {}
 private:
  virtual unsigned int Get() override {
    return size_;
  }
  virtual bool NeedTruncate() override {
    return false;
  }
  unsigned int size_;
};

// Helper for quickly generating random data
class RandomGenerator {
 private:
  std::string data_;
  unsigned int pos_;
  std::unique_ptr<BaseDistribution> dist_;

 public:

  RandomGenerator(unsigned int flags_value_size_min, unsigned int flags_value_size_max,
                  unsigned int value_size, double flags_compression_ratio = 0.0) {
    auto max_value_size = flags_value_size_max;
    switch (flags_value_size_distribution_type_e)
    {
    case kUniform:
      dist_.reset(new UniformDistribution(flags_value_size_min,
                                          flags_value_size_max));
      break;
    
    case kNormal:
      dist_.reset(new NormalDistribution(flags_value_size_min,
                                         flags_value_size_max));
      break;
    default:
      dist_.reset(new FixedDistribution(value_size));
      max_value_size = value_size;
    }

    Random rnd(301);
    std::string piece;
    while (data_.size() < (unsigned)std::max(1048576, (int)max_value_size)) {
      // Add a short fragment that is as compressible as specified
      // by FLAGS_compression_ratio.
      CompressibleString(&rnd, flags_compression_ratio, 100, &piece);
      data_.append(piece);
    }
    pos_ = 0;
  }

  Slice Generate(unsigned int len) {
    assert(len <= data_.size());
    if (pos_ + len > data_.size()) {
      pos_ = 0;
    }
    pos_ += len;
    return Slice(data_.data() + pos_ - len, len);
  }

  Slice Generate() {
    auto len = dist_->Generate();
    return Generate(len);
  }
};
}
