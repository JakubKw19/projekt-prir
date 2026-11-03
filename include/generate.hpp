#pragma once

#include <cstddef>
#include <cstdint>

// Deterministyczny LCG. Ten sam seed daje te same dane na kazdej maszynie.
void fill_random(int32_t* a, size_t n, uint32_t seed, int32_t lo, int32_t hi);

// Naprzemienne serie wartosci zachowywanej i odrzucanej, dlugosci run_len.
// Ostatnia seria jest ucieta do n. Serie zaczynaja sie od wartosci zachowywanej.
// Przy predykacie, ktory zostawia tylko keep_value, maska to dlugie bloki 1 i 0.
void fill_runs(int32_t* a, size_t n, size_t run_len, int32_t keep_value, int32_t drop_value);
