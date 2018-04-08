#include <iostream>
#include <vector>
#include <random>
#include "error.h"
#include "timer.h"
#include "mf.h"

static void benchmark(int ny, int nx, int hy, int hx) {
    std::mt19937 rng;
    std::uniform_real_distribution<float> u(0.0f, 1.0f);
    std::vector<float> data(ny * nx);
    std::vector<float> result(ny * nx);
    for (int y = 0; y < ny; ++y) {
        for (int x = 0; x < nx; ++x) {
            float v = u(rng);
            data[x + nx * y] = v;
        }
    }
    std::printf("mf %4d %4d %4d %4d ", ny, nx, hy, hx);
    std::fflush(stdout);
    { ppc::timer t; mf(ny, nx, hy, hx, data.data(), result.data()); }
    std::printf("\n");
}

int main(int argc, const char** argv) {
    if (argc < 5 || argc > 6) {
        error("usage: mf-benchmark Y X HY HX [ITERATIONS]");
    }
    int ny = std::stoi(argv[1]);
    int nx = std::stoi(argv[2]);
    int hy = std::stoi(argv[3]);
    int hx = std::stoi(argv[4]);
    int iter = argc == 6 ? std::stoi(argv[5]) : 1;
    for (int i = 0; i < iter; ++i) {
        benchmark(ny, nx, hy, hx);
    }
}
