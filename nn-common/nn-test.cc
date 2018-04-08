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

static void runTest(std::string file, std::vector<std::pair<int, float>> expected) {
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

    printf("Running test %s:\n", file.c_str());
    fflush(stdout);

    evalNetwork(buf0);

    printf("Expected\n");
    for (size_t i = 0; i < expected.size(); i++) {
        printf("%6.2f%% [%3d] %s\n", expected[i].second * 100.f, expected[i].first, g_labels[expected[i].first]);
    }

    bool has_error = false;
    printf("Got\n");
    for (size_t i = 0; i < expected.size(); i++) {
        int idx = 0;
        for (int i = 1; i < 1000; i++)
            if (buf0[i] > buf0[idx])
                idx = i;

        printf("%6.2f%% [%3d] %s", buf0[idx] * 100.f, idx, g_labels[idx]);

        if (idx != expected[i].first || abs(buf0[idx] - expected[i].second) > 0.001) {
            has_error = true;
            printf("  <-");
        }
        buf0[idx] = -1.f;
        printf("\n");
    }

    if (has_error) {
        error("Test failed");
    }

    printf("\n");

    // Free memory.

    delete[] buf0;
}

// ------------------------------------------------------------------------

int main(int argc, const char** argv)
{
    (void) argv;
    if (argc != 1) {
        error("usage: nn-test");
        return 1;
    }

    // Load pre-trained VGG-19 weights.

    g_weights = new float[TOTAL_NUM_WEIGHTS];
    if (read_file("weights.bin", g_weights, sizeof(float), TOTAL_NUM_WEIGHTS)) {
        error("There was an error while loading weights.bin. Please run ./setup.sh.\n");
    }

    // Run the test
    runTest("examples/nn1.bin", {{483, .8628}, {449, .0324}, {497, .0274}, {975, .0254}, {698, .0178}});
    runTest("examples/nn2.bin", {{968, .2103}, {504, .1604}, {725, .0745}, {470, .0703}, {868, .0668}});

    // Free memory.

    delete[] g_weights;

    // Done.

    return 0;
}

// ------------------------------------------------------------------------
