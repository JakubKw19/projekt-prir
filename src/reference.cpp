#include "compact.hpp"

#include <cassert>
#include <chrono>
#include <cstdint>
#include <stdexcept>

namespace {

void check_n(size_t n) {
    if (n > static_cast<size_t>(UINT32_MAX)) {
        throw std::invalid_argument("n nie miesci sie w uint32_t");
    }
}

}  // namespace

void build_mask(const int32_t* in, size_t n, int32_t lo, int32_t hi, uint8_t* mask) {
    check_n(n);
    assert(n == 0 || (in != nullptr && mask != nullptr));
    for (size_t i = 0; i < n; ++i) {
        mask[i] = static_cast<uint8_t>(in[i] >= lo && in[i] <= hi);
    }
}

uint32_t exclusive_scan(const uint8_t* mask, size_t n, uint32_t* index) {
    check_n(n);
    assert(n == 0 || (mask != nullptr && index != nullptr));
    uint32_t sum = 0;
    for (size_t i = 0; i < n; ++i) {
        index[i] = sum;
        sum += mask[i];
    }
    return sum;
}

void scatter(const int32_t* in, const uint8_t* mask, const uint32_t* index,
             size_t n, int32_t* out) {
    check_n(n);
    assert(n == 0 || (in != nullptr && mask != nullptr && index != nullptr && out != nullptr));
    for (size_t i = 0; i < n; ++i) {
        if (mask[i]) {
            out[index[i]] = in[i];
        }
    }
}

uint32_t compact_reference(const int32_t* in, size_t n, int32_t lo, int32_t hi,
                           int32_t* out, uint8_t* mask, uint32_t* index,
                           PhaseTimes* times) {
    using clock = std::chrono::steady_clock;

    const auto t0 = clock::now();
    build_mask(in, n, lo, hi, mask);
    const auto t1 = clock::now();
    const uint32_t count = exclusive_scan(mask, n, index);
    const auto t2 = clock::now();
    scatter(in, mask, index, n, out);
    const auto t3 = clock::now();

    if (times) {
        const auto sec = [](clock::time_point a, clock::time_point b) {
            return std::chrono::duration<double>(b - a).count();
        };
        times->mask_s = sec(t0, t1);
        times->scan_s = sec(t1, t2);
        times->scatter_s = sec(t2, t3);
    }
    return count;
}
