# O05 — Kompakcja i skan prefiksowy

Zadanie: z tablicy `int32_t` o długości `n` wybrać elementy spełniające predykat przedziału domkniętego `[lo, hi]` i zapisać je w osobnym buforze wyjściowym bez dziur, ze zachowaną kolejnością względną (stabilność). Zwracana jest też liczba dopasowań `count`.

Przykład dla `lo = 1`, `hi = 4`:

```text
in      3  1  4  1  5  9  2
mask    1  1  1  1  0  0  1
index   0  1  2  3  4  4  4
out     3  1  4  1  2          count = 5
```

Filtrowanie da się napisać jedną pętlą. Tu rozbijamy to na trzy jawne fazy (maska → skan wyłączny → scatter), bo ten sam układ da się później zrównoleglić w OpenMP, MPI i CUDA, a poszczególne fazy da się mierzyć osobno.

---

## Wersja referencyjna

Wersja referencyjna (`compact_reference` w `src/reference.cpp`) to sekwencyjna implementacja potoku. Nie używa wątków ani GPU — pojedyncze pętle `for` po `[0, n)`. Służy jako:

1. definicja poprawnego zachowania algorytmu,
2. baza do testów regresji (każdy wariant równoległy musi dać bitowo ten sam `out[0..count)`),
3. punkt odniesienia przy pomiarach czasu.

### Fazy

**`build_mask`** — dla każdego `i` ustawia `mask[i] ∈ {0,1}` według `lo <= in[i] <= hi`. Typ maski: `uint8_t`.

**`exclusive_scan`** — liczy skan wyłączny maski:

```text
index[i] = suma mask[0] + … + mask[i-1]
```

Zwracana wartość to suma całej maski, czyli `count`. Dzięki skanowi wyłącznemu pierwszy zachowany element trafia pod indeks 0; `index[i]` jest już adresem docelowym w `out`, zanim doliczymy bieżący bit maski.

**`scatter`** — jeśli `mask[i] == 1`, wykonuje `out[index[i]] = in[i]`. Elementy z maską 0 są pomijane. Stabilność wynika z tego, że przechodzimy `i` rosnąco, a `index` jest niemalejący.

`compact_reference` woła te trzy funkcje po kolei. Opcjonalnie wypełnia `PhaseTimes` (czasy ścienne faz bez alokacji). Bufory (`mask`, `index`, `out`) alokuje wywołujący; `out` ma pojemność `n` (górne oszacowanie), zajęte jest tylko `count` elementów. Kompakcja w miejscu nie wchodzi w zakres.

Szczyt jawnych buforów: `n * (4 + 1 + 4 + 4)` bajtów (`in` + `mask` + `index` + `out`), bez narzutu alokatora — wzór w `reference_peak_bytes`.

### Testy

`src/main.cpp` porównuje wynik z niezależną wyrocznią (zwykła pętla bez maski/skanu) oraz sprawdza niezmiennik skanu. Pokryte m.in.: `n = 0`, zero / wszystkie dopasowania, `lo > hi`, ujemne wartości, rozmiary wokół potęg dwójki (255…4097), serie i losowe dane przy różnej selektywności. Generator: LCG + `fill_runs` (`src/generate.cpp`).

Budowa: Windows `.\build.ps1` (MSVC), Linux/macOS `make test`. Program kończy się kodem 0 przy przejściu testów.

API (niezmienne dla `int32_t` w kolejnych wariantach) jest w `include/compact.hpp`.

---

## Warianty docelowe

| Wariant | Stan | Krótko |
| --- | --- | --- |
| Referencja | zrobiona | sekwencyjny potok + testy + czasy faz |
| OpenMP | brak | bloki po 4096, skan lokalny, offsety bloków, scatter bez atomików |
| MPI | brak | fragment na rangę, `MPI_Exscan`, złożenie przez `Gatherv` |
| CUDA | brak | własne jądra (skan kafelkowy); Thrust tylko jako kolumna porównawcza |
| Pomiary + raport | brak | siatka `n` / selektywność / serie → CSV, potem `docs/raport.md` |

Kolejność implementacji: OpenMP → MPI → CUDA → systematyczne pomiary → raport. Szczegóły algorytmów, podział pamięci i metodyka pomiarów: [`plan.md`](plan.md).

Opcjonalnie (zakres drugiej osoby): rekord 32 B (`Record` / predykat tylko po `key`) albo wariant OpenMP bez materializacji pełnej tablicy `index[n]`.
