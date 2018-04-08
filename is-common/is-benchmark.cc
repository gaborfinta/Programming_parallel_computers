#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <random>
#include "random.h"
#include "error.h"
#include "timer.h"
#include "is.h"

static void gen_binary(float* data, int ny, int nx, ppc::random& rng)
{
    std::uniform_int_distribution<int> u(0, 1);
    for (int y = 0; y < ny; ++y) {
        for (int x = 0; x < nx; ++x) {
            const float v = (float)u(rng);
            for (int c = 0; c < 3; ++c) {
                data[c + 3 * x + 3 * nx * y] = v;
            }
        }
    }
}

static void gen_color(float* data, int ny, int nx, ppc::random& rng)
{
    std::uniform_real_distribution<float> u(0.0f, 1.0f);
    for (int y = 0; y < ny; ++y) {
        for (int x = 0; x < nx; ++x) {
            for (int c = 0; c < 3; ++c) {
                float v = u(rng);
                data[c + 3 * x + 3 * nx * y] = v;
            }
        }
    }
}

static void benchmark(int ny, int nx, bool binary) {
    ppc::random rng(664, 555);
    rng();

    std::vector<float> data(ny * nx * 3);

    if (binary) {
        gen_binary(data.data(), ny, nx, rng);
    }
    else {
        gen_color(data.data(), ny, nx, rng);
    }

    std::cout << "is\t" << ny << "\t" << nx << "\t" << std::flush;
    { ppc::timer t; segment(ny, nx, data.data()); }
    std::cout << std::endl;
}

int main(int argc, const char** argv) {
    if (argc < 3 || argc > 5) {
        error("usage: is-benchmark [binary] Y X [ITERATIONS]");
    }
    int head = 1;
    bool is_binary = false;
    if (std::string(argv[1]) == "binary") {
        ++head;
        is_binary = true;
    }

    const int ny = std::stoi(argv[head]);
    const int nx = std::stoi(argv[head+1]);
    const int iter = argc == (head+3) ? std::stoi(argv[head+2]) : 1;
    for (int i = 0; i < iter; ++i) {
        benchmark(ny, nx, is_binary);
    }
}