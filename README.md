# O05 — Kompakcja i skan prefiksowy

Potok: maska 0/1, skan prefiksowy, przeniesienie zachowanych rekordów do zwartej tablicy. Kolejność zachowanych elementów zostaje.

Stan:

| Część | Stan |
| --- | --- |
| Opis prostymi słowami | [docs/opis.md](docs/opis.md) |
| Plan (szczegóły) | [docs/plan.md](docs/plan.md) |
| Wersja referencyjna (sekwencyjna) | gotowa |
| OpenMP, MPI, CUDA, pomiary, raport | opisane w planie, jeszcze bez kodu |

Referencja filtruje `int32_t` predykatem przedziału domkniętego `[lo, hi]`. Zwraca liczbę dopasowań i tablicę wynikową. Liczy osobno czas maski, skanu i scattera. Alokacja jest poza pomiarem.

## Budowanie

Windows, Visual Studio:

```powershell
.\build.ps1
```

Linux albo macOS:

```bash
make test
```

CMake:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/compact_ref
```

Program kończy się kodem 0, gdy testy przejdą. Wypisuje krótki przykład potoku i czasy dla miliona elementów.
