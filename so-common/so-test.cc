#include "so-gen.h"
#include "so.h"

#include <vector>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <sstream>
#include "error.h"
#include "random.h"
#if defined(_OPENMP)
    #include <omp.h>
#endif


template <typename T, typename rng_type>
bool test(T& input, T& reference_input, int testsize, gen_type mode, rng_type& rng)
{
    generate(input.begin(), input.begin()+testsize, rng, mode);
    std::copy(input.begin(), input.begin()+testsize, reference_input.begin());

    psort(testsize, input.data());
    std::sort(reference_input.begin(), reference_input.begin()+testsize);

    const bool ok = std::equal(input.begin(), input.end(), reference_input.begin());
    return ok;
}

static void exit_fail(int size, gen_type mode, int thread_count)
{
    std::stringstream ss;
    ss << "Test failed for test size " << size 
        << ", mode " << static_cast<int>(mode) 
        << ", thread count " << thread_count << '\n';
    error(ss.str());
}

#define CLEAR "\33[2K\r"

int main(int argc, char** argv)
{
    static constexpr uint64_t seed1 = 230;
    static constexpr uint64_t seed2 = 964;
    ppc::random rng(seed1, seed2);

    if (argc == 1) {
        static constexpr std::array<int, 7> large_test_sizes{63, 811, 966, 2839, 9999, 10000, 100000};

        #if defined(_OPENMP)
        static constexpr int thread_counts[] = {1, 2, 3, 7, 8, 15, 35, 36};
        const int base_thread_count = omp_get_num_threads();
        #else
        static constexpr int thread_counts[] = { 1 };
        #endif
        
        for (auto threads : thread_counts) {
            #if defined(_OPENMP)
            std::printf("Running tests with %d threads:\n", threads);
            omp_set_num_threads(threads);
            #else
            std::printf("Running tests:\n");
            #endif
            for (int testsize = 0; testsize < 20; ++testsize) {
                std::printf(CLEAR "Input size: %6d", testsize);
                std::fflush(stdout);

                std::vector<data_t> input(testsize);
                std::vector<data_t> reference_input(input.size());
                for (int type_i = 0; type_i < (int)gen_types.size(); ++type_i) {
                    const auto mode = gen_types[type_i];
                    for (int iter = 0; iter < 10; ++iter) {
                        const auto ok = test(input, reference_input, testsize, mode, rng);
                        if (!ok) {
                            std::printf(CLEAR);
                            exit_fail(testsize, mode, threads);
                        }
                    }
                }
            }
            for (auto testsize : large_test_sizes) {
                std::printf(CLEAR "Input size: %6d", testsize);
                std::fflush(stdout);

                std::vector<data_t> input(testsize);
                std::vector<data_t> reference_input(input.size());
                for (int type_i = 0; type_i < (int)gen_types.size(); ++type_i) {
                    const auto mode = gen_types[type_i];
                    const auto ok = test(input, reference_input, testsize, mode, rng);
                    if (!ok) {
                        std::printf(CLEAR);
                        exit_fail(testsize, mode, threads);
                    }
                }
            }
            std::printf(CLEAR);
        }
        #if defined(_OPENMP)
        omp_set_num_threads(base_thread_count);
        #endif
        std::printf("Tests passed.\n");
    }
    else if (argc >= 2) {
        const int testsize = std::stoi(argv[1]);
        const unsigned mode_i = argc >= 3 ? std::stoi(argv[2]) : 0;
        const int iters = argc >= 4 ? std::stoi(argv[3]) : 1;
        std::vector<data_t> input(testsize);
        std::vector<data_t> reference_input(testsize);
        if (mode_i >= gen_types.size()) {
            error("Incorrect test mode.\n");
        }
        const auto mode = gen_types[mode_i];

        int num_fails = 0;

        std::printf("Running tests:\n");

        for (int i = 0; i < iters; ++i) {
            std::printf(CLEAR "Iteration: %6d/%d", i+1, iters);
            std::fflush(stdout);

            const auto ok = test(input, reference_input, testsize, mode, rng);
            if (!ok) {
                ++num_fails;
            }
        }

        if (num_fails != 0) {
            std::printf(CLEAR);
            std::stringstream ss;
            ss << "Failed tests " << num_fails << "/" << iters << ".\n";
            error(ss.str());
        }
        std::printf(CLEAR);
        std::cout << "Tests passed.\n";
    }
    else {
        std::cout << "Usage:\n  so-test\n  so-test <testsize> <mode> [<iters>]\n";
        return 1;
    }
    
    return 0;
}

