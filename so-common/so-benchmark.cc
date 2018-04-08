#include "so-gen.h"
#include "so.h"

#include <vector>
#include <random>
#include <iostream>
#include "random.h"
#include "error.h"
#include "timer.h"


static void benchmark(std::size_t len, int iters, gen_type mode)
{
    std::vector<data_t> input(len);
    static constexpr uint64_t seed1 = 57;
    static constexpr uint64_t seed2 = 908;

    for (int iter = 0; iter < iters; ++iter) {
        ppc::random rng(seed1, seed2);
        generate(input.begin(), input.end(), rng, mode);
        {
            std::cout << "so\t" << len << "\t" << std::flush;
            ppc::timer timer;
            psort(len, input.data());
        }
        std::cout << std::endl;
    }
}

int main(int argc, char** argv)
{
    if (argc < 2 || argc > 4) {
        error("usage: so-benchmark <testsize> [mode] [iters]");
    }
    const std::size_t len = std::stol(argv[1]);
    const unsigned mode_i = argc >= 3 ? std::stoi(argv[2]) : 0;
    const int iters = argc >= 4 ? std::stoi(argv[3]) : 1;
    if (mode_i >= gen_types.size()) {
        error("Incorrect test mode.");
    }

    benchmark(len, iters, gen_types[mode_i]);
    return 0;
}
