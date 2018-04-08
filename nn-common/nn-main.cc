#include <cstdio>
#include <cstdlib>
#include "labels.inl"
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
        fprintf(stderr, "Size differs from expected\n");
        return 1;
    }
    if (fclose(f)) {
        perror(filename);
        return 1;
    }
    return 0;
}

// ------------------------------------------------------------------------

int main(int argc, const char** argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s input.bin\n", argv[0]);
        return 1;
    }

    // Load pre-trained VGG-19 weights.

    g_weights = new float[TOTAL_NUM_WEIGHTS];
    if (read_file("weights.bin", g_weights, sizeof(float), TOTAL_NUM_WEIGHTS)) {
        fprintf(stderr, "There was an error while loading weights.bin. Please run ./setup.sh.\n");
        exit(1);
    }

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

    if (read_file(argv[1], buf0, sizeof(float), 3 * 224 * 224)) {
        fprintf(stderr, "There was an error while loading the image.\n");
        exit(1);
    }

    evalNetwork(buf0);

    // Print top 5 classes.

    for (int n = 0; n < 5; n++)
    {
        int idx = 0;
        for (int i = 1; i < 1000; i++)
            if (buf0[i] > buf0[idx])
                idx = i;

        printf("%6.2f%% [%3d] %s\n", buf0[idx] * 100.f, idx, g_labels[idx]);
        buf0[idx] = -1.f; // Prevent from being picked again.
    }
    printf("\n");
    fflush(stdout);

    // Free memory.

    delete[] g_weights;
    delete[] buf0;

    // Done.

    return 0;
}

// ------------------------------------------------------------------------
