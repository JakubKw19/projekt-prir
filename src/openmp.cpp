
#include "compact_openmp.hpp"

#include <cassert>
#include <chrono>
#include <stdexcept>
#include <vector>
#include <cstdint>
#include <climits>

namespace {

void check_n(size_t n) {
    if (n > static_cast<size_t>(UINT32_MAX)) {
        throw std::invalid_argument("n nie miesci sie w uint32_t");
    }
}

} // namespace

uint32_t compact_openmp(
    const int32_t* in,
    size_t n,
    int32_t lo,
    int32_t hi,
    int32_t* out,
    uint8_t* mask,
    uint32_t* index,
    PhaseTimes* times
) {
    using clock = std::chrono::steady_clock;

    check_n(n);

    assert(n == 0 ||
           (in != nullptr && out != nullptr &&
            mask != nullptr && index != nullptr));

    if (n == 0) {
        if (times) {
            times->mask_s = 0.0;
            times->scan_s = 0.0;
            times->scatter_s = 0.0;
        }
        return 0;
    }

    const size_t block_count =
        (n + OPENMP_BLOCK_SIZE - 1) / OPENMP_BLOCK_SIZE;

    // Liczba elementów zachowanych w każdym bloku.
    std::vector<uint32_t> block_counts(block_count);

    // Offset każdego bloku w tablicy wynikowej.
    std::vector<uint32_t> block_offsets(block_count);

    // --------------------------------------------------
    // 1. Budowanie maski równolegle
    // --------------------------------------------------

    auto t0 = clock::now();

    #pragma omp parallel for schedule(static)
    for (std::int64_t i = 0; i < static_cast<std::int64_t>(n); ++i) {
        mask[i] = static_cast<uint8_t>(
            in[i] >= lo && in[i] <= hi
        );
    }

    auto t1 = clock::now();

    // --------------------------------------------------
    // 2. Skan lokalny każdego bloku
    // --------------------------------------------------

    #pragma omp parallel for schedule(static)
    for (std::int64_t b = 0;
         b < static_cast<std::int64_t>(block_count);
         ++b) {

        const size_t begin =
            static_cast<size_t>(b) * OPENMP_BLOCK_SIZE;

        const size_t end =
            (begin + OPENMP_BLOCK_SIZE < n)
                ? begin + OPENMP_BLOCK_SIZE
                : n;

        uint32_t local_sum = 0;

        for (size_t i = begin; i < end; ++i) {
            // Skan wyłączny: indeks przed dodaniem maski[i].
            index[i] = local_sum;
            local_sum += mask[i];
        }

        block_counts[b] = local_sum;
    }

    // --------------------------------------------------
    // 3. Offsety bloków
    // --------------------------------------------------

    uint32_t total = 0;

    for (size_t b = 0; b < block_count; ++b) {
        block_offsets[b] = total;
        total += block_counts[b];
    }

    // Dodanie offsetu bloku do lokalnych indeksów.
    // Każdy wątek zapisuje do innego fragmentu index[].

    #pragma omp parallel for schedule(static)
    for (std::int64_t b = 0;
         b < static_cast<std::int64_t>(block_count);
         ++b) {

        const size_t begin =
            static_cast<size_t>(b) * OPENMP_BLOCK_SIZE;

        const size_t end =
            (begin + OPENMP_BLOCK_SIZE < n)
                ? begin + OPENMP_BLOCK_SIZE
                : n;

        const uint32_t offset = block_offsets[b];

        for (size_t i = begin; i < end; ++i) {
            index[i] += offset;
        }
    }

    auto t2 = clock::now();

    // --------------------------------------------------
    // 4. Scatter równolegle, bez atomików
    // --------------------------------------------------

    #pragma omp parallel for schedule(static)
    for (std::int64_t b = 0;
         b < static_cast<std::int64_t>(block_count);
         ++b) {

        const size_t begin =
            static_cast<size_t>(b) * OPENMP_BLOCK_SIZE;

        const size_t end =
            (begin + OPENMP_BLOCK_SIZE < n)
                ? begin + OPENMP_BLOCK_SIZE
                : n;

        const uint32_t offset = block_offsets[b];

        for (size_t i = begin; i < end; ++i) {
            if (mask[i]) {
                out[index[i]] = in[i];
            }
        }
    }

    auto t3 = clock::now();

    if (times) {
        const auto seconds = [](clock::time_point a,
                                clock::time_point b) {
            return std::chrono::duration<double>(b - a).count();
        };

        times->mask_s = seconds(t0, t1);
        times->scan_s = seconds(t1, t2);
        times->scatter_s = seconds(t2, t3);
    }

    return total;
}
