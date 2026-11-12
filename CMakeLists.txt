
cmake_minimum_required(VERSION 3.16)
project(kompakcja LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenMP REQUIRED)

add_executable(compact_ref
    src/main.cpp
    src/reference.cpp
    src/generate.cpp
)

target_include_directories(compact_ref PRIVATE include)

if(MSVC)
    target_compile_options(compact_ref PRIVATE /W4 /utf-8)
else()
    target_compile_options(compact_ref PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(compact_omp
    src/main_openmp.cpp
    src/openmp.cpp
    src/reference.cpp
    src/generate.cpp
)

target_include_directories(compact_omp PRIVATE include)
target_link_libraries(compact_omp PRIVATE OpenMP::OpenMP_CXX)

if(MSVC)
    target_compile_options(compact_omp PRIVATE /W4 /utf-8)
else()
    target_compile_options(compact_omp PRIVATE -Wall -Wextra -Wpedantic)
endif()


# MPI i CUDA dochodza pozniej. Kolejnosc i kontrakt sa w docs/plan.md.
# Nie dodawac tu pustych targetow, dopoki nie ma zrodel.
