#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include <cstring>
#include <random>
#include <cstdint>
#include "mf.h"

#define CLEAR "\33[2K\r"
#define RESET "\33[0m"
#define ERROR "\33[31;1m"        

constexpr uint64_t full_test_size = 5e8;

static void generate(int ny, int nx, float* data) {
    std::mt19937 rng;
    std::uniform_real_distribution<double> unif(0.0f, 1.0f);
    for (int y = 0; y < ny; y++) {
        for (int x = 0; x < nx; x++) {
            float value = unif(rng);
            data[y * nx + x] = value;
        }
    }
}

static bool verify_pixel(int y, int x, int ny, int nx, int hy, int hx, const float* input, float* result) {
    int smallcnt = 0;
    int bigcnt = 0;
    int equalcnt = 0;
    float small = -std::numeric_limits<float>::infinity();
    float big = std::numeric_limits<float>::infinity();
    float median = result[x + nx*y];
    for (int j = std::max(0, y-hy); j < std::min(ny, y+hy+1); j++) {
        for (int i = std::max(0, x-hx); i < std::min(nx, x+hx+1); i++) {
            float val = input[i + nx*j];
            if (val == median) {
                equalcnt++;
            } else if (val < median) {
                smallcnt++;
                small = std::max(small, val);
            } else {
                bigcnt++;
                big = std::min(big, val);
            }
        }
    }
    // move some equals to one side to make them the same size
    if(smallcnt < bigcnt) {
        int transfer = bigcnt - smallcnt;
        equalcnt -= transfer;
        smallcnt += transfer;
        small = median;
    } else if(bigcnt < smallcnt) {
        int transfer = smallcnt - bigcnt;
        equalcnt -= transfer;
        bigcnt += transfer;
        big = median;
    }
    // check if we have enough equals in the middle to stop here
    if(equalcnt > 0) {
        return false;
    }
    // check if we weren't in the middle
    if(equalcnt < 0) {
        return true;
    }
    // even counts: compute and check mean
    return (big+small)*0.5f != median;

}

static bool verify(int ny, int nx, int hy, int hx, const float* input, float* result, int* errors) {
    uint64_t operation_count = uint64_t(ny) * nx * (2*hy+1) * (2*hx+1);
    bool has_errors = false;

    if (operation_count <= full_test_size) {
        for (int y = 0; y < ny; y++) {
            for (int x = 0; x < nx; x++) {
                bool error = verify_pixel(y, x, ny, nx, hy, hx, input, result);
                errors[x + nx*y] = error;
                has_errors |= error;
            }
        }
    } else {
        memset(errors, 0, ny*nx*sizeof(errors[0]));
        std::mt19937 rng;
        std::uniform_int_distribution<int> ydist(0, ny-1);
        std::uniform_int_distribution<int> xdist(0, nx-1);
        for (uint64_t i = 0; i < full_test_size; i += (2*hy+1) * (2*hx+1)) {
            int y = ydist(rng);
            int x = xdist(rng);
            bool error = verify_pixel(y, x, ny, nx, hy, hx, input, result);
            errors[x + nx*y] = error;
            has_errors |= error;
        }
    }
    return has_errors;
}

static void print(int ny, int nx, float* input, float* output, int* errors) {
    std::printf("Input\n");
    for (int y = 0; y < ny; y++) {
        for (int x = 0; x < nx; x++) {
            std::printf(" %1.5f", input[y*nx + x]);
        }
        std::printf("\n");
    }

    std::printf("\n");
    std::printf("Output\n");
    for (int y = 0; y < ny; y++) {
        for (int x = 0; x < nx; x++) {
            if (errors[y*nx + x]) {
                std::printf(ERROR " %1.5f" RESET, output[y*nx + x]);
            } else {
                std::printf(" %1.5f", output[y*nx + x]);
            }
        }
        std::printf("\n");
    }
    std::printf("\n");
}

static bool test(int ny, int nx, int hy, int hx, bool verbose) {
    std::vector<float> input(ny * nx);

    generate(ny, nx, input.data());

    std::vector<float> result(ny * nx);
    std::vector<int> errors(ny * nx);
    mf(ny, nx, hy, hx, input.data(), result.data());

    bool error = verify(ny, nx, hy, hx, input.data(), result.data(), errors.data());
    bool pass = !error;

    if (verbose) {
        if (ny < 25 && nx < 25) {
            print(ny, nx, input.data(), result.data(), errors.data());
        }
    }

    return pass;
}

static int testcount = 0;
static int passcount = 0;
static bool has_fails = false;
static struct { int ny, nx, hy, hx; } first_fail;

static void run_test(int ny, int nx, int hy, int hx, bool verbose) {
    // Tell the user where we are going
    std::printf(CLEAR "mf %4d %4d %4d %4d ", ny, nx, hy, hx);
    std::fflush(stdout);
    if (verbose) {
        std::printf("\n");
    }

    bool pass = test(ny, nx, hy, hx, verbose);
    if (!pass) {
        std::printf("FAILED\n");
        if (!has_fails) {
            first_fail.ny = ny;
            first_fail.nx = nx;
            first_fail.hy = hy;
            first_fail.hx = hx;
        }
        has_fails = true;
    } else {
        passcount++;
    }
    testcount++;
}

int main(int argc, const char** argv) {
    if (argc == 1) {
        for (int ny = 1; ny < 10; ny++) {
            for (int nx = 1; nx < 10; nx++) {
                for (int hy = 0; hy < ny / 2 + 1; hy++) {
                    for (int hx = 0; hx < nx / 2 + 10; hx++) {
                        run_test(ny, nx, hy, hx, false);
                    }
                }
            }
        }

        std::vector<int> ns = {11, 23, 37, 107};
        std::vector<int> hs = {7, 17, 21};

        if (!has_fails) {
            for (auto ny : ns) {
                for (auto nx : ns) {
                    for (auto hy : hs) {
                        for (auto hx : hs) {
                            run_test(ny, nx, hy, hx, false);
                        }
                    }
                }
            }
        } else {
            std::printf("Failure in small test cases, skipping big tests\n");
        }

        if (!has_fails) std::printf("\n");
        std::printf("%4d / %4d test passed\n", passcount, testcount);
        if (has_fails) {
            std::printf("To repeat the first failed test with more output, run:\n");
            std::printf("%s %d %d %d %d\n", argv[0], first_fail.ny, first_fail.nx, first_fail.hy, first_fail.hx);
            exit(EXIT_FAILURE);
        }
    } else if (argc == 5) {
        int ny = std::stoi(argv[1]);
        int nx = std::stoi(argv[2]);
        int hy = std::stoi(argv[3]);
        int hx = std::stoi(argv[4]);
        run_test(ny, nx, hy, hx, true);
        if (has_fails) {
            exit(EXIT_FAILURE);
        }
    } else {
        std::printf("Usage:\n");
        std::printf("  %s\n", argv[0]);
        std::printf("  %s <ny> <nx> <hy> <hx>\n", argv[0]);
    }
}
