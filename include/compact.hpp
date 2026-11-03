#pragma once

#include <cstddef>
#include <cstdint>

// Wersja referencyjna potoku kompakcji.
// Kolejnosc jest stabilna: zachowane elementy zostaja w tej samej kolejnosci
// wzgledem siebie.
//
// Potok:
//   1. maska[i] = 1, gdy lo <= in[i] <= hi, w przeciwnym razie 0
//   2. index[i] = liczba jedynek na pozycjach [0, i)   (skan wylaczny)
//   3. jezeli maska[i] == 1, to out[index[i]] = in[i]
//
// Skan wylaczny (exclusive) daje indeks miejsca docelowego jeszcze przed
// doliczeniem biezacego elementu. Dzieki temu pierwszy zachowany rekord
// trafia pod indeks 0.
//
// Kontrakt:
//   - n == 0: wskazniki moga byc puste, wynikiem jest 0
//   - n > 0: in, out, mask i index wskazuja na n elementow
//   - out ma pojemnosc n (gorne oszacowanie; zajete jest tylko count elementow)
//   - n miesci sie w uint32_t, bo suma maski jest licznikiem 32-bitowym
//   - czasy nie obejmuja alokacji buforow

struct PhaseTimes {
    double mask_s;
    double scan_s;
    double scatter_s;

    double total_s() const { return mask_s + scan_s + scatter_s; }
};

// Szczyt jawnych buforow referencji: wejscie + maska + indeksy + wyjscie o pojemnosci n.
// Bez narzutu alokatora.
inline size_t reference_peak_bytes(size_t n) {
    return n * (sizeof(int32_t) + sizeof(uint8_t) + sizeof(uint32_t) + sizeof(int32_t));
}

void build_mask(const int32_t* in, size_t n, int32_t lo, int32_t hi, uint8_t* mask);

// Zwraca sume maski, czyli liczbe zachowanych elementow.
uint32_t exclusive_scan(const uint8_t* mask, size_t n, uint32_t* index);

void scatter(const int32_t* in, const uint8_t* mask, const uint32_t* index,
             size_t n, int32_t* out);

// Pelny potok. Zwraca liczbe dopasowan. times moze byc nullptr.
uint32_t compact_reference(const int32_t* in, size_t n, int32_t lo, int32_t hi,
                           int32_t* out, uint8_t* mask, uint32_t* index,
                           PhaseTimes* times);
