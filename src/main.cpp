#include "compact.hpp"
#include "generate.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

namespace {

int g_failed = 0;
int g_passed = 0;

// Niezalezna wyrocznia: zwykla petla, bez maski i skanu.
std::vector<int32_t> oracle(const std::vector<int32_t>& in, int32_t lo, int32_t hi) {
    std::vector<int32_t> out;
    out.reserve(in.size());
    for (int32_t v : in) {
        if (v >= lo && v <= hi) {
            out.push_back(v);
        }
    }
    return out;
}

bool scan_holds(const std::vector<uint8_t>& mask, const std::vector<uint32_t>& index, uint32_t count) {
    uint32_t sum = 0;
    if (mask.size() != index.size()) {
        return false;
    }
    for (size_t i = 0; i < mask.size(); ++i) {
        if (mask[i] > 1 || index[i] != sum) {
            return false;
        }
        sum += mask[i];
    }
    return sum == count;
}

bool run_case(const std::vector<int32_t>& in, int32_t lo, int32_t hi, const char* name) {
    const size_t n = in.size();
    std::vector<uint8_t> mask(n);
    std::vector<uint32_t> index(n);
    std::vector<int32_t> out(n);
    uint32_t count = 0;
    try {
        count = compact_reference(in.empty() ? nullptr : in.data(), n, lo, hi,
                                  out.empty() ? nullptr : out.data(),
                                  mask.empty() ? nullptr : mask.data(),
                                  index.empty() ? nullptr : index.data(),
                                  nullptr);
    } catch (const std::exception& ex) {
        std::fprintf(stderr, "FAIL: %s (%s)\n", name, ex.what());
        ++g_failed;
        return false;
    }

    const std::vector<int32_t> expect_out = oracle(in, lo, hi);
    const bool same_count = count == expect_out.size();
    const bool same_values =
        same_count && std::equal(expect_out.begin(), expect_out.end(), out.begin());
    const bool scan_ok = scan_holds(mask, index, count);
    const bool ok = same_count && same_values && scan_ok;
    if (!ok) {
        std::fprintf(stderr, "FAIL: %s (count=%u oczekiwane=%zu scan=%s)\n", name, count,
                     expect_out.size(), scan_ok ? "ok" : "zly");
        ++g_failed;
        return false;
    }
    ++g_passed;
    return true;
}

void print_example() {
    const int32_t in[] = {3, 1, 4, 1, 5, 9, 2};
    constexpr size_t n = 7;
    constexpr int32_t lo = 1;
    constexpr int32_t hi = 4;
    uint8_t mask[n];
    uint32_t index[n];
    int32_t out[n];
    const uint32_t count = compact_reference(in, n, lo, hi, out, mask, index, nullptr);

    std::cout << "Przyklad: [3, 1, 4, 1, 5, 9, 2], predykat [1, 4]\nmaska: ";
    for (size_t i = 0; i < n; ++i) {
        std::cout << int(mask[i]) << (i + 1 == n ? "" : " ");
    }
    std::cout << "\nindeks:";
    for (size_t i = 0; i < n; ++i) {
        std::cout << " " << index[i];
    }
    std::cout << "\nwynik (" << count << "):";
    for (uint32_t i = 0; i < count; ++i) {
        std::cout << " " << out[i];
    }
    std::cout << "\n";
}

void print_demo() {
    constexpr size_t n = 1000000;
    constexpr int32_t lo = 0;
    constexpr int32_t hi = 99;  // wartosci z [0, 999] -> okolo 10% zachowanych
    std::vector<int32_t> in(n);
    std::vector<uint8_t> mask(n);
    std::vector<uint32_t> index(n);
    std::vector<int32_t> out(n);
    fill_random(in.data(), n, 42u, 0, 999);

    PhaseTimes times{};
    const uint32_t count =
        compact_reference(in.data(), n, lo, hi, out.data(), mask.data(), index.data(), &times);

    const double share = n == 0 ? 0.0 : 100.0 * static_cast<double>(count) / static_cast<double>(n);
    const double mb = static_cast<double>(reference_peak_bytes(n)) / (1024.0 * 1024.0);
    std::cout << "\nDemo n=" << n << " predykat [0, 99], wartosci z [0, 999]\n"
              << "  dopasowania: " << count << " (" << share << "%)\n"
              << "  maska:   " << times.mask_s << " s\n"
              << "  skan:    " << times.scan_s << " s\n"
              << "  scatter: " << times.scatter_s << " s\n"
              << "  razem:   " << times.total_s() << " s\n"
              << "  pamiec szczytowa buforow: " << mb << " MiB\n"
              << "  (czasy bez alokacji)\n";
}

}  // namespace

int main() {
    run_case({}, 0, 10, "pusta tablica");
    run_case({3, 1, 4, 1, 5, 9, 2}, 1, 4, "przyklad z opisu");
    run_case({1}, 1, 1, "jeden element zachowany");
    run_case({1}, 2, 3, "jeden element odrzucony");
    run_case({5, 5, 5, 5}, 5, 5, "zostaja wszystkie");
    run_case({1, 2, 3, 4}, 10, 20, "nie zostaje zaden");
    run_case({1, 2, 3, 4}, 5, 1, "pusty zakres lo > hi");
    run_case({-5, -1, 0, 1, 4}, -1, 1, "liczby ujemne");
    run_case({2, 2, 2, 9, 2}, 2, 2, "stabilna kolejnosc duplikatow");

    // Rozmiary niebedace wielokrotnoscia planowanych blokow OpenMP (4096) i CUDA (256).
    const size_t sizes[] = {255, 256, 257, 1000, 4095, 4096, 4097, 10000};
    for (size_t n : sizes) {
        std::vector<int32_t> in(n);
        fill_random(in.data(), n, 7u, -50, 50);
        const std::string name = "losowe n=" + std::to_string(n);
        run_case(in, -10, 10, name.c_str());
    }

    const size_t runs[] = {1, 64, 1000};
    for (size_t run : runs) {
        std::vector<int32_t> in(20000);
        fill_runs(in.data(), in.size(), run, 1, 9);
        const std::string name = "serie " + std::to_string(run);
        run_case(in, 1, 1, name.c_str());
    }

    {
        std::vector<int32_t> in(1000000);
        fill_random(in.data(), in.size(), 99u, 0, 999);
        run_case(in, 0, 999, "milion, zostaja wszystkie");
        run_case(in, 1, 0, "milion, nie zostaje zaden");
        run_case(in, 0, 499, "milion, okolo polowa");
    }

    std::cout << "Testy: " << g_passed << " OK, " << g_failed << " bledow\n";
    if (g_failed != 0) {
        return 1;
    }

    print_example();
    print_demo();
    return 0;
}
