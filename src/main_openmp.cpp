
#include "compact.hpp"
#include "compact_openmp.hpp"
#include "generate.hpp"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <vector>

int main() {
    const size_t n = 1000000;
    const int32_t lo = 0;
    const int32_t hi = 99;

    std::vector<int32_t> in(n);
    std::vector<int32_t> out_ref(n);
    std::vector<int32_t> out_omp(n);

    std::vector<uint8_t> mask_ref(n);
    std::vector<uint8_t> mask_omp(n);

    std::vector<uint32_t> index_ref(n);
    std::vector<uint32_t> index_omp(n);

    fill_random(in.data(), n, 42u, 0, 999);

    PhaseTimes ref_times{};
    PhaseTimes omp_times{};

    const uint32_t ref_count = compact_reference(
        in.data(), n, lo, hi,
        out_ref.data(), mask_ref.data(),
        index_ref.data(), &ref_times
    );

    const uint32_t omp_count = compact_openmp(
        in.data(), n, lo, hi,
        out_omp.data(), mask_omp.data(),
        index_omp.data(), &omp_times
    );

    const bool correct =
        ref_count == omp_count &&
        std::equal(
            out_ref.begin(),
            out_ref.begin() + ref_count,
            out_omp.begin()
        );

    std::cout << "OpenMP - kompakcja\n";
    std::cout << "Liczba elementow: " << n << "\n";
    std::cout << "Rozmiar bloku: " << OPENMP_BLOCK_SIZE << "\n";
    std::cout << "Wynik: " << (correct ? "POPRAWNY" : "BLAD") << "\n";
    std::cout << "Dopasowania: " << omp_count << "\n";

    std::cout << "\nReferencja:\n";
    std::cout << "  maska:    " << ref_times.mask_s << " s\n";
    std::cout << "  skan:     " << ref_times.scan_s << " s\n";
    std::cout << "  scatter:  " << ref_times.scatter_s << " s\n";
    std::cout << "  razem:    " << ref_times.total_s() << " s\n";

    std::cout << "\nOpenMP:\n";
    std::cout << "  maska:    " << omp_times.mask_s << " s\n";
    std::cout << "  skan:     " << omp_times.scan_s << " s\n";
    std::cout << "  scatter:  " << omp_times.scatter_s << " s\n";
    std::cout << "  razem:    " << omp_times.total_s() << " s\n";

    return correct ? 0 : 1;
}
