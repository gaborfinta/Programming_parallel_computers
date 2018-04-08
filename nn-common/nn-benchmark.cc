#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <cassert>
#include <cstring>
#include "labels.inl"
#include "error.h"
#include "timer.h"
#include "nn.h"

// ------------------------------------------------------------------------

static int read_file(const char* filename, void* p, size_t size, size_t nitems)
{
    FILE* f = fopen(filename, "rb");
    if (!f) {
        perror(filename);
        return 1;
    }
    if (fread(p, size, nitems, f) != nitems) {
        perror(filename);
        return 1;
    }
    if (fclose(f)) {
        perror(filename);
        return 1;
    }
    return 0;
}

// ------------------------------------------------------------------------

static void runBenchmark(std::string file) {
    // Allocate memory for two maximal size buffers (64 x 224 x 224).

    float* buf0 = new float[64 * 224 * 224];

    // Load pre-processed image into buf0, indexed as [channel][y][x].
    //
    // If you want to load a different image, note that VGG-19 requires color
    // (103.939, 116.779, 123.68) to be subtracted from raw input values
    // that are originally in range 0--255. Thus the dynamic range of input
    // of data to the network should be:
    //   R: [-103.939 .. 151.061]
    //   G: [-116.779 .. 138.221]
    //   B: [-123.680 .. 131.320]

    // Copy the input
    if (read_file(file.c_str(), buf0, sizeof(float), 3 * 224 * 224)) {
        error(std::string("There was an error while loading image ") + file);
    }

    printf("Running benchmark %s:\n", file.c_str());

    {
        ppc::timer t;
        evalNetwork(buf0);
        printf("Time: ");
    }

    printf("\n\n");

    // Free memory.

    delete[] buf0;
}

// ------------------------------------------------------------------------

int main(int argc, const char** argv)
{
    if (argc != 1 && argc != 2) {
        error("usage: nn-bencmark [ITERATIONS]");
        return 1;
    }

    int iterations = 1;
    if (argc == 2) {
        iterations = atoi(argv[1]);
    }

    // Load pre-trained VGG-19 weights.

    g_weights = new float[TOTAL_NUM_WEIGHTS];
    if (read_file("weights.bin", g_weights, sizeof(float), TOTAL_NUM_WEIGHTS)) {
        error("There was an error while loading weights.bin. Please run ./setup.sh.\n");
    }

    // Run the benchmark
    for (int i = 0; i < iterations; i++) {
        runBenchmark("examples/nn1.bin");
        runBenchmark("examples/nn2.bin");
    }

    // Free memory.

    delete[] g_weights;

    // Done.

    return 0;
}

// ------------------------------------------------------------------------
