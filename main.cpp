#include <memory>
#include <vector>

#include <benchmark/benchmark.h>

constexpr unsigned LINE_SIZE = 64;
constexpr unsigned KB = 1024;
constexpr unsigned MB = KB*KB;

struct alignas(LINE_SIZE) MemoryLine {
    uint8_t stub[LINE_SIZE];
};

constexpr unsigned long long int operator "" _KB(unsigned long long int val) {
    return val * KB;
}

constexpr unsigned long long int operator "" _MB(unsigned long long int val) {
    return val * MB;
}

constexpr int ITERATIONS = 10000;
constexpr int MEM_BUF_SIZE = 64_MB;

static void cache_benchmark(benchmark::State &state) {
    auto mem_buf_up = std::make_unique<MemoryLine[]>(MEM_BUF_SIZE / sizeof(MemoryLine));
    auto array = reinterpret_cast<uint8_t*>(mem_buf_up.get());

    int tmp = 0;
    const size_t bytes_to_read = state.range(0);
    const size_t step = state.range(1);
    for (auto _: state) {
        for (auto j = 0; j < ITERATIONS; ++j) {
            size_t bytes_read = 0;
            for (auto ptr = array; bytes_read < bytes_to_read; ptr += (4 * step)) {
                benchmark::DoNotOptimize(tmp = *ptr);
                benchmark::DoNotOptimize(tmp = *(ptr + step));
                benchmark::DoNotOptimize(tmp = *(ptr + 2 * step));
                benchmark::DoNotOptimize(tmp = *(ptr + 3 * step));
                bytes_read += 4 * LINE_SIZE;
            }
        }
    }

    state.SetBytesProcessed(int64_t(state.iterations()) *
                            int64_t(bytes_to_read) * ITERATIONS);
}

struct Mixed {
    int warm;
    int cold[15];
};

struct Warm {
    int warm;
};

template<class T>
static void warm_mixed_benchmark(benchmark::State &state) {
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
                            int64_t(size * 64) * ITERATIONS);

    delete[] array;
}

struct Space {
    bool b0;
    int i;
    bool b1;
    double d;
};

struct LessSpace {
    bool b0;
    bool b1;
    int i;
    double d;
};

#pragma pack(push, 1)
struct Pack {
    bool b0;
    int i;
    bool b1;
    double d;
};
#pragma pack(pop)

template<class T>
static void space_benchmark(benchmark::State &state) {
    size_t size = state.range(0);
    auto array = new T[size];

    double tmp = 0.0;
    for (auto _: state) {
        for (auto j = 0; j < ITERATIONS; ++j) {
            for (auto ptr = array; ptr != (array + size); ptr += 1) {
                benchmark::DoNotOptimize((*ptr).d += 1.0);
                benchmark::DoNotOptimize(tmp = (*ptr).d);
            }
        }
    }

    state.SetBytesProcessed(int64_t(state.iterations()) *
                            int64_t(size * 64) * ITERATIONS);

    delete[] array;
}


static void split_benchmark(benchmark::State &state) {
    int tmp = 0;
    const size_t bytes_to_read = state.range(0);
    const size_t step = state.range(1);

    if (bytes_to_read <= 32_KB) {
        auto mem_buf_up = std::make_unique<MemoryLine[]>(MEM_BUF_SIZE / sizeof(MemoryLine));
        auto array = reinterpret_cast<uint8_t*>(mem_buf_up.get());

        for (auto _: state) {
            for (auto j = 0; j < ITERATIONS; ++j) {
                size_t bytes_read = 0;
                for (auto ptr = array; bytes_read < bytes_to_read; ptr += (4 * step)) {
                    benchmark::DoNotOptimize(tmp = *ptr);
                    benchmark::DoNotOptimize(tmp = *(ptr + step));
                    benchmark::DoNotOptimize(tmp = *(ptr + 2 * step));
                    benchmark::DoNotOptimize(tmp = *(ptr + 3 * step));
                    bytes_read += 4 * LINE_SIZE;
                }
            }
        }
    } else {
        auto mem_buf_up = std::make_unique<MemoryLine[]>(32_KB / sizeof(MemoryLine));
        auto array = reinterpret_cast<uint8_t*>(mem_buf_up.get());
        auto end = array + 32_KB;

        for (auto _: state) {
            for (auto j = 0; j < ITERATIONS; ++j) {
                size_t bytes_read = 0;

                for (auto ptr = array; bytes_read < bytes_to_read; ) {
                    benchmark::DoNotOptimize(tmp = *ptr);
                    benchmark::DoNotOptimize(tmp = *(ptr + step));
                    benchmark::DoNotOptimize(tmp = *(ptr + 2 * step));
                    benchmark::DoNotOptimize(tmp = *(ptr + 3 * step));
                    bytes_read += 4 * LINE_SIZE;

                    ptr += (4 * step);
                    if (ptr >= end) {
                        ptr = array;
                    }
                }
            }
        }
    }


    state.SetBytesProcessed(int64_t(state.iterations()) *
                            int64_t(bytes_to_read) * ITERATIONS);
}

int main(int argc, char** argv) {
    std::vector<std::pair<const char*, long int>> runs = {
            {"1 KB",   1_KB},
            {"2 KB",   2_KB},
            {"4 KB",   4_KB},
            {"8 KB",   8_KB},
            {"16 KB",  16_KB},
            {"24 KB",  24_KB},
            {"28 KB",  28_KB},
            {"32 KB",  32_KB},
            {"34 KB",  34_KB},
            {"36 KB",  36_KB},
            {"40 KB",  40_KB},
            {"48 KB",  48_KB},
            {"64 KB",  64_KB},
            {"128 KB", 128_KB},
            {"256 KB", 256_KB},
            {"384 KB", 384_KB},
            {"448 KB", 448_KB},
            {"480 KB", 480_KB},
            {"512 KB", 512_KB},
            {"544 KB", 544_KB},
            {"608 KB", 608_KB},
            {"640 KB", 640_KB},
            {"768 KB", 768_KB},
            {"896 KB", 896_KB},
            {"960 KB", 960_KB},
            {"1 MB",   1_MB},
            {"1.0625 MB",1088_KB},
            {"1.125 MB",1152_KB},
            {"1.25 MB",1280_KB},
            {"1.5 MB", 1536_KB},
            {"2 MB",   2_MB},
            {"2.5 MB", 2056_KB},
            {"3 MB",   3_MB},
            {"4 MB",   4_MB},
            {"6 MB",   6_MB},
            {"8 MB",   8_MB},
            {"12 MB",  12_MB},
            {"16 MB",  16_MB},
    };

    for (auto& r : runs) {
        benchmark::RegisterBenchmark("cache_benchmark", cache_benchmark)->ArgName(r.first)->Args(
                {r.second, LINE_SIZE})->Threads(1);
    }
    for (auto step = LINE_SIZE; step < 18 * LINE_SIZE; step += LINE_SIZE) {
        benchmark::RegisterBenchmark("steps", cache_benchmark)->ArgName(
                std::to_string(int(step / LINE_SIZE)) + std::string(" lines"))->Args(
                {1_MB, step})->Threads(1);
    }

    for (auto& r : runs) {
        benchmark::RegisterBenchmark("split_benchmark", split_benchmark)->ArgName(r.first)->Args(
                {r.second, LINE_SIZE})->Threads(1);
    }

    for (auto run : runs) {
        if (run.second > 64_KB) {
            break;
        }
        benchmark::RegisterBenchmark("warm", warm_mixed_benchmark<Warm>)->ArgName("size")->Arg(run.second);
        benchmark::RegisterBenchmark("mixed", warm_mixed_benchmark<Mixed>)->ArgName("size")->Arg(run.second);
    }

    for (auto run : runs) {
        if (run.second > 64_KB) {
            break;
        }

        benchmark::RegisterBenchmark("space", space_benchmark<Space>)->ArgName("run")->Arg(run.second);
        benchmark::RegisterBenchmark("less space", space_benchmark<LessSpace>)->ArgName("size")->Arg(run.second);
        benchmark::RegisterBenchmark("pack", space_benchmark<Pack>)->ArgName("size")->Arg(run.second);
    }

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
}