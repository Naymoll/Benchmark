#include <vector>
#include <ctime>
#include <cstdlib>
#include <algorithm>

#include <benchmark/benchmark.h>

constexpr unsigned LINE_SIZE = 64;
constexpr unsigned KB = 1024;
constexpr unsigned MB = KB * KB;
constexpr int ITERATIONS = 10000;

constexpr unsigned long long int operator "" _KB(unsigned long long int val) {
    return val * KB;
}

constexpr unsigned long long int operator "" _MB(unsigned long long int val) {
    return val * MB;
}

struct Mixed {
    int warm;
    int cold[15];
};

struct Warm {
    int warm;
};

template<class T>
static void data_benchmark(benchmark::State &state) {
    const size_t size = state.range(0);
    auto array = new T[size];

    int tmp = 0;
    for (auto _: state) {
        for (auto j = 0; j < ITERATIONS; ++j) {
            for (auto ptr = array; ptr != (array + size); ptr += 1) {
                benchmark::DoNotOptimize((*ptr).warm += 1);
                benchmark::DoNotOptimize(tmp = (*ptr).warm);
            }
        }
    }

    state.SetBytesProcessed(int64_t(state.iterations()) *
                            int64_t(size * LINE_SIZE) * ITERATIONS);

    delete[] array;
}

struct Space {
    bool b0;
    int i;
    bool b1;
    double test;
};

struct LessSpace {
    bool b0;
    bool b1;
    int i;
    double test;
};

#pragma pack(push, 1)
struct Pack {
    bool b0;
    int i;
    bool b1;
    double test;
};
#pragma pack(pop)

template<class T>
static void struct_size_benchmark(benchmark::State &state) {
    size_t size = state.range(0);
    auto array = new T[size];

    double tmp = 0.0;
    for (auto _: state) {
        for (auto j = 0; j < ITERATIONS; ++j) {
            for (auto ptr = array; ptr != (array + size); ptr += 1) {
                benchmark::DoNotOptimize((*ptr).test += 1.0);
                benchmark::DoNotOptimize(tmp = (*ptr).test);
            }
        }
    }

    state.SetBytesProcessed(int64_t(state.iterations()) *
                            int64_t(size * LINE_SIZE) * ITERATIONS);

    delete[] array;
}

struct MaxPad {
    int padding[19];
    int test;
};

struct AvgPad {
    int padding[17];
    int test;
};

struct MinPad {
    int padding[15];
    int test;
};

template<class T>
static void padding_benchmark(benchmark::State &state) {
    size_t size = state.range(0);
    auto array = new T[size];

    int tmp = 0;
    for (auto _: state) {
        for (auto j = 0; j < ITERATIONS; ++j) {
            for (auto ptr = array; ptr != (array + size); ptr += 1) {
                benchmark::DoNotOptimize((*ptr).test += 1);
                benchmark::DoNotOptimize(tmp = (*ptr).test);
            }
        }
    }

    state.SetBytesProcessed(int64_t(state.iterations()) *
                            int64_t(size * LINE_SIZE) * ITERATIONS);

    delete[] array;
}

static std::vector<int> random_array(size_t size) {
    std::vector<int> array(size);

    srand(time(nullptr));
    for (size_t i = 0; i < size; i++) {
        array[i] = rand();
    }

    return array;
}

static void bubble_benchmark(benchmark::State &state) {
    size_t size = state.range(0);
    std::vector<int> array = random_array(size);

    for (auto _: state) {
        for (size_t i = 0; i < size - 1; i++) {
            for (size_t j = i + 1; j < size; j++) {
                if (array[i] < array[j]) {
                    std::swap(array[i], array[j]);
                }
            }

        }
    }

    state.SetComplexityN(size);
}

static void quick_benchmark(benchmark::State &state) {
    size_t size = state.range(0);
    std::vector<int> array = random_array(size);

    for (auto _: state) {
        std::sort(array.begin(), array.end());
    }

    state.SetComplexityN(size);
}

int main(int argc, char **argv) {
    std::vector<long int> runs = {
            1_KB,
            2_KB,
            4_KB,
            8_KB,
            16_KB,
            24_KB,
            28_KB,
            32_KB,
            34_KB,
            36_KB,
            40_KB,
            48_KB,
            64_KB,
    };

    for (auto run : runs) {
        benchmark::RegisterBenchmark("warm", data_benchmark<Warm>)->ArgName("size")->Arg(run);
        benchmark::RegisterBenchmark("mixed", data_benchmark<Mixed>)->ArgName("size")->Arg(run);
    }

    for (auto run : runs) {
        benchmark::RegisterBenchmark("space", struct_size_benchmark<Space>)->ArgName("size")->Arg(run);
        benchmark::RegisterBenchmark("less space", struct_size_benchmark<LessSpace>)->ArgName("size")->Arg(run);
        benchmark::RegisterBenchmark("pack", struct_size_benchmark<Pack>)->ArgName("size")->Arg(run);
    }

    for (auto run : runs) {
        benchmark::RegisterBenchmark("max padding", padding_benchmark<MaxPad>)->ArgName("size")->Arg(run);
        benchmark::RegisterBenchmark("avg padding", padding_benchmark<AvgPad>)->ArgName("size")->Arg(run);
        benchmark::RegisterBenchmark("min padding", padding_benchmark<MinPad>)->ArgName("size")->Arg(run);
    }

    benchmark::RegisterBenchmark("bubble sort", bubble_benchmark)->RangeMultiplier(2)->Range(1 << 10,
                                                                                             1 << 16)->Complexity();
    benchmark::RegisterBenchmark("quick sort", quick_benchmark)->RangeMultiplier(2)->Range(1 << 10,
                                                                                           1 << 16)->Complexity();

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
}