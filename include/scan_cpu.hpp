#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

// Ekskluzywny skan Blellocha dla n będącego potęgą dwójki.
// Po wywołaniu temp[i] = suma wartości początkowych na pozycjach [0, i).
// Zwracana jest suma całego bloku. Pętla po tid jest równoległa na GPU;
// w tej wersji jest szeregowa, ale w jednym kroku offsetu zapisy nie zachodzą
// na siebie, więc wynik jest identyczny z jądrem CUDA.
inline std::uint32_t blelloch_exclusive_inplace(std::uint32_t* temp, int n) {
    for (int offset = 1; offset < n; offset <<= 1) {
        for (int tid = 0; tid < n; ++tid) {
            const int idx = (tid + 1) * offset * 2 - 1;
            if (idx < n) {
                temp[idx] += temp[idx - offset];
            }
        }
    }

    const std::uint32_t total = temp[n - 1];
    temp[n - 1] = 0;

    for (int offset = n >> 1; offset > 0; offset >>= 1) {
        for (int tid = 0; tid < n; ++tid) {
            const int idx = (tid + 1) * offset * 2 - 1;
            if (idx < n) {
                const std::uint32_t left = temp[idx - offset];
                temp[idx - offset] = temp[idx];
                temp[idx] += left;
            }
        }
    }
    return total;
}

// Model referencyjny potoku GPU: skan lokalny bloków 256, potem ten sam
// skan na sumach bloków i dodanie offsetów. Obsługuje n niebędące wielokrotnością bloku.
inline std::uint32_t hierarchical_exclusive_scan(const std::uint32_t* in, std::uint32_t* out, int n) {
    constexpr int block = 256;
    if (n <= 0) {
        return 0;
    }

    const int blocks = (n + block - 1) / block;
    std::vector<std::uint32_t> sums(static_cast<std::size_t>(blocks));

    for (int b = 0; b < blocks; ++b) {
        std::uint32_t temp[block];
        for (int t = 0; t < block; ++t) {
            const int i = b * block + t;
            temp[t] = (i < n) ? in[i] : 0u;
        }
        const std::uint32_t block_total = blelloch_exclusive_inplace(temp, block);
        for (int t = 0; t < block; ++t) {
            const int i = b * block + t;
            if (i < n) {
                out[i] = temp[t];
            }
        }
        sums[static_cast<std::size_t>(b)] = block_total;
    }

    if (blocks == 1) {
        return sums[0];
    }

    std::vector<std::uint32_t> sum_exclusive(static_cast<std::size_t>(blocks));
    const std::uint32_t grand =
        hierarchical_exclusive_scan(sums.data(), sum_exclusive.data(), blocks);
    for (int b = 0; b < blocks; ++b) {
        const int begin = b * block;
        const int end = std::min(begin + block, n);
        for (int i = begin; i < end; ++i) {
            out[i] += sum_exclusive[static_cast<std::size_t>(b)];
        }
    }
    return grand;
}

inline std::uint32_t naive_exclusive_scan(const std::uint32_t* in, std::uint32_t* out, int n) {
    std::uint32_t sum = 0;
    for (int i = 0; i < n; ++i) {
        out[i] = sum;
        sum += in[i];
    }
    return sum;
}
