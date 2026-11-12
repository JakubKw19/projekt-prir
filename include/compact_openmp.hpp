
#pragma once

#include "compact.hpp"

// Wersja OpenMP kompakcji.
// Zachowuje kolejność elementów i kontrakt wersji referencyjnej.
// Rozmiar bloku: 4096 elementów.

constexpr size_t OPENMP_BLOCK_SIZE = 4096;

uint32_t compact_openmp(
    const int32_t* in,
    size_t n,
    int32_t lo,
    int32_t hi,
    int32_t* out,
    uint8_t* mask,
    uint32_t* index,
    PhaseTimes* times
);
