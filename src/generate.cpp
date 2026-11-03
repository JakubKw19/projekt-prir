#include "generate.hpp"

#include <cassert>

void fill_random(int32_t* a, size_t n, uint32_t seed, int32_t lo, int32_t hi) {
    assert(n == 0 || a != nullptr);
    assert(lo <= hi);
    uint32_t state = seed == 0 ? 1u : seed;
    const uint32_t span = static_cast<uint32_t>(static_cast<int64_t>(hi) - lo + 1);
    for (size_t i = 0; i < n; ++i) {
        state = state * 1664525u + 1013904223u;
        a[i] = static_cast<int32_t>(lo + static_cast<int32_t>(state % span));
    }
}

void fill_runs(int32_t* a, size_t n, size_t run_len, int32_t keep_value, int32_t drop_value) {
    assert(n == 0 || a != nullptr);
    if (run_len == 0) {
        run_len = 1;
    }
    for (size_t i = 0; i < n; ++i) {
        const bool keep_run = ((i / run_len) % 2) == 0;
        a[i] = keep_run ? keep_value : drop_value;
    }
}
