#include "distributions.h"

namespace ROCKSDB_BENCHMARK_NAMESPACE {

Status GenerateTwoTermExpKeys::InitiateExpDistribution(
    int64_t flags_num, int64_t flags_keyrange_num, int64_t total_keys,
    double prefix_a, double prefix_b, double prefix_c, double prefix_d)
{
    keyrange_rand_max_ = flags_num;
    int64_t amplify = 0;
    int64_t keyrange_start = 0;
    initiated_ = true;
    if (flags_keyrange_num <= 0) {
        keyrange_num_ = 1;
    } else {
        keyrange_num_ = flags_keyrange_num;
    }
    keyrange_size_ = total_keys / keyrange_num_;
    // Calculate the key-range shares size based on the input parameters
    for (int64_t pfx = keyrange_num_; pfx >= 1; pfx--) {
        // Step 1. Calculate the probability that this key range will be
        // accessed in a query. It is based on the two-term expoential
        // distribution
        double keyrange_p = prefix_a * std::exp(prefix_b * pfx) + prefix_c * std::exp(prefix_d * pfx);
        if (keyrange_p < std::pow(10.0, -16.0)) {
            keyrange_p = 0.0;
        }
        // Step 2. Calculate the amplify
        // In order to allocate a query to a key-range based on the random
        // number generated for this query, we need to extend the probability
        // of each key range from [0,1] to [0, amplify]. Amplify is calculated
        // by 1/(smallest key-range probability). In this way, we ensure that
        // all key-ranges are assigned with an Integer that  >=0
        if (amplify == 0 && keyrange_p > 0) {
            amplify = static_cast<int64_t>(std::floor(1 / keyrange_p)) + 1;
        }

        // Step 3. For each key-range, we calculate its position in the
        // [0, amplify] range, including the start, the size (keyrange_access)
        KeyrangeUnit p_unit;
        p_unit.keyrange_start = keyrange_start;
        if (0.0 >= keyrange_p) {
            p_unit.keyrange_access = 0;
        } else {
            p_unit.keyrange_access = static_cast<int64_t>(std::floor(amplify * keyrange_p));
        }
        p_unit.keyrange_keys = keyrange_size_;
        keyrange_set_.push_back(p_unit);
        keyrange_start += p_unit.keyrange_access;
    }
    keyrange_rand_max_ = keyrange_start;

    // Step 4. Shuffle the key-ranges randomly
    // Since the access probability is calculated from small to large,
    // If we do not re-allocate them, hot key-ranges are always at the end
    // and cold key-ranges are at the begin of the key space. Therefore, the
    // key-ranges are shuffled and the rand seed is only decide by the
    // key-range hotness distribution. With the same distribution parameters
    // the shuffle results are the same.
    Random64 rand_loca(keyrange_rand_max_);
    for (int64_t i = 0; i < flags_keyrange_num; i++) {
        int64_t pos = rand_loca.Next() % flags_keyrange_num;
        assert(i >= 0 && i < static_cast<int64_t>(keyrange_set_.size()) && pos >= 0 && pos < static_cast<int64_t>(keyrange_set_.size()));
        std::swap(keyrange_set_[i], keyrange_set_[pos]);
    }

    // Step 5. Recalculate the prefix start postion after shuffling
    int64_t offset = 0;
    for (auto& p_unit : keyrange_set_) {
        p_unit.keyrange_start = offset;
        offset += p_unit.keyrange_access;
    }

    return Status::OK();
}

int64_t GenerateTwoTermExpKeys::DistGetKeyID(int64_t ini_rand, double key_dist_a,
    double key_dist_b)
{

    int64_t keyrange_rand = ini_rand % keyrange_rand_max_;

    // Calculate and select one key-range that contains the new key
    // using binary search
    int64_t start = 0, end = static_cast<int64_t>(keyrange_set_.size());
    while (start + 1 < end) {
        int64_t mid = start + (end - start) / 2;
        assert(mid >= 0 && mid < static_cast<int64_t>(keyrange_set_.size()));
        if (keyrange_rand < keyrange_set_[mid].keyrange_start) {
            end = mid;
        } else {
            start = mid;
        }
    }
    int64_t keyrange_id = start;

    // Select one key in the key-range and compose the keyID
    int64_t key_offset = 0, key_seed;
    if (key_dist_a == 0.0 && key_dist_b == 0.0) {
        key_offset = ini_rand % keyrange_size_;
    } else {
        key_seed = static_cast<int64_t>(
            ceil(std::pow((ini_rand / key_dist_a), (1 / key_dist_b))));
        Random64 rand_key(key_seed);
        key_offset = static_cast<int64_t>(rand_key.Next()) % keyrange_size_;
    }
    return keyrange_size_ * keyrange_id + key_offset;
}

Status QueryDecider::Initiate(std::vector<double> ratio_input)
{
    int range_max = 1000;
    double sum = 0.0;
    for (auto& ratio : ratio_input) {
        sum += ratio;
    }
    range_ = 0;
    for (auto& ratio : ratio_input) {
        range_ += static_cast<int>(ceil(range_max * (ratio / sum)));
        type_.push_back(range_);
        ratio_.push_back(ratio / sum);
    }
    return Status::OK();
}

int QueryDecider::GetType(int64_t rand_num)
{
    if (rand_num < 0) {
        rand_num = rand_num * (-1);
    }
    assert(range_ != 0);
    int pos = static_cast<int>(rand_num % range_);
    for (int i = 0; i < static_cast<int>(type_.size()); i++) {
        if (pos < type_[i]) {
            return i;
        }
    }
    return 0;
}

int64_t ParetoCdfInversion(double u, double theta, double k, double sigma)
{
    double ret;
    if (k == 0.0) {
        ret = theta - sigma * std::log(u);
    } else {
        ret = theta + sigma * (std::pow(u, -1 * k) - 1) / k;
    }
    return static_cast<int64_t>(ceil(ret));
}
// The inverse function of power distribution (y=ax^b)
int64_t PowerCdfInversion(double u, double a, double b)
{
    double ret;
    ret = std::pow((u / a), (1 / b));
    return static_cast<int64_t>(ceil(ret));
}

// Add the noise to the QPS
double AddNoise(double origin, double noise_ratio, double flags_sine_a)
{
    if (noise_ratio < 0.0 || noise_ratio > 1.0) {
        return origin;
    }
    int band_int = static_cast<int>(flags_sine_a);
    double delta = (rand() % band_int - band_int / 2) * noise_ratio;
    if (origin + delta < 0) {
        return origin;
    } else {
        return (origin + delta);
    }
}

Slice CompressibleString(Random* rnd, double compressed_fraction,
    int len, std::string* dst)
{
    int raw = static_cast<int>(len * compressed_fraction);
    if (raw < 1)
        raw = 1;
    std::string raw_data;
    RandomString(rnd, raw, &raw_data);

    // Duplicate the random data until we have filled "len" bytes
    dst->clear();
    while (dst->size() < (unsigned int)len) {
        dst->append(raw_data);
    }
    dst->resize(len);
    return Slice(*dst);
}

Slice RandomString(Random* rnd, int len, std::string* dst)
{
    dst->resize(len);
    for (int i = 0; i < len; i++) {
        (*dst)[i] = static_cast<char>(' ' + rnd->Uniform(95)); // ' ' .. '~'
    }
    return Slice(*dst);
}
}
