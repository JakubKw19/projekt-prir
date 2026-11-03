#pragma once

#include <cstdint>

// Rekord 32-bajtowy: drugi rozmiar elementu (zakres dla dwóch osób).
// key jest polem predykatu, payload ma być przenoszony bez zmian.
struct Record {
    std::int32_t key;
    unsigned char payload[28];
};

static_assert(sizeof(Record) == 32, "Record musi mieć 32 bajty");
static_assert(alignof(Record) <= 4, "Record nie może mieć niespodziewanego wyrównania");

#ifdef __CUDACC__
#define COMPACT_HD __host__ __device__
#else
#define COMPACT_HD
#endif

// Predykat zakresu domkniętego. Dla rekordu patrzy wyłącznie na key.
struct InRange {
    std::int32_t lo;
    std::int32_t hi;

    COMPACT_HD bool operator()(std::int32_t value) const {
        return value >= lo && value <= hi;
    }

    COMPACT_HD bool operator()(const Record& record) const {
        return record.key >= lo && record.key <= hi;
    }
};

enum class Pattern : int {
    Random = 0,
    Runs = 1
};
