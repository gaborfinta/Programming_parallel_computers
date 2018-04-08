#include <cstdio>
#include <cmath>
#include "nn.h"

// ------------------------------------------------------------------------

float* g_weights = NULL;    // store all network weights in one big array.

// ------------------------------------------------------------------------

ConvLayer g_convLayers[16] = {
    { 224,  64,   3,        0,     1728 },
    { 224,  64,  64,     1792,    38656 },    // 2x2 maxpool (224 x 224 -> 112 x 112)
    { 112, 128,  64,    38720,   112448 },
    { 112, 128, 128,   112576,   260032 },    // 2x2 maxpool (112 x 112 -> 56 x 56)
    {  56, 256, 128,   260160,   555072 },
    {  56, 256, 256,   555328,  1145152 },
    {  56, 256, 256,  1145408,  1735232 },
    {  56, 256, 256,  1735488,  2325312 },    // 2x2 maxpool (56 x 56 -> 28 x 28)
    {  28, 512, 256,  2325568,  3505216 },
    {  28, 512, 512,  3505728,  5865024 },
    {  28, 512, 512,  5865536,  8224832 },
    {  28, 512, 512,  8225344, 10584640 },    // 2x2 maxpool (28 x 28 -> 14 x 14)
    {  14, 512, 512, 10585152, 12944448 },
    {  14, 512, 512, 12944960, 15304256 },
    {  14, 512, 512, 15304768, 17664064 },
    {  14, 512, 512, 17664576, 20023872 },    // 2x2 maxpool (14 x 14 -> 7 x 7) -> interpret as flat array
};

DenseLayer g_denseLayers[3] = {
    { 4096, 25088,  20024384, 122784832, false },
    { 4096,  4096, 122788928, 139566144, false },
    { 1000,  4096, 139570240, 143666240, true  },
};

// ------------------------------------------------------------------------

static void evalConv(int idx, const float* bufIn, float* bufOut)
{
    const ConvLayer& layer = g_convLayers[idx];
    const float* W = g_weights + layer.ofsW;
    const float* B = g_weights + layer.ofsB;

    printf("conv %-2d (%3d, %3d, %3d) -> (%3d, %3d, %3d)\n", idx, layer.nIn, layer.sz, layer.sz, layer.nOut, layer.sz, layer.sz);
    fflush(stdout);

    int sz = layer.sz;
    for (int i = 0; i < layer.nOut; i++)
    for (int y = 0; y < sz; y++)
    for (int x = 0; x < sz; x++)
    {
        float sum = B[i];
        for (int j = 0; j < layer.nIn; j++)
        for (int dy = 0; dy < 3; dy++)
        for (int dx = 0; dx < 3; dx++)
        {
            int yy = y + dy - 1;
            int xx = x + dx - 1;
            if (yy >= 0 && yy < sz && xx >= 0 && xx < sz)
                sum += bufIn[sz*sz*j + sz*yy + xx] * W[layer.nIn*3*3*i + 3*3*j + 3*(2-dy) + (2-dx)];
        }
        bufOut[sz*sz*i + sz*y + x] = (sum > 0.f) ? sum : 0.f; // ReLu activation.
    }
}

// ------------------------------------------------------------------------

static void evalDense(int idx, const float* bufIn, float* bufOut)
{
    const DenseLayer& layer = g_denseLayers[idx];
    const float* W = g_weights + layer.ofsW;
    const float* B = g_weights + layer.ofsB;
    float total = 0.f;

    printf("dense %d (%3d) -> (%3d)\n", idx, layer.nIn, layer.nOut);
    fflush(stdout);

    for (int i = 0; i < layer.nOut; i++)
    {
        float sum = B[i];
        for (int j = 0; j < layer.nIn; j++)
            sum += bufIn[j] * W[layer.nIn*i + j];

        if (layer.softmax)
            total += (bufOut[i] = expf(sum));
        else
            bufOut[i] = (sum > 0.f) ? sum : 0.f;
    }

    if (layer.softmax)
        for (int i = 0; i < layer.nOut; i++)
            bufOut[i] *= 1.f / total;
}

// ------------------------------------------------------------------------

#define MAX(a, b) ((a) > (b) ? (a) : (b))
static void maxPool2x2(int sz, int n, const float* bufIn, float* bufOut)
{
    printf("maxpool (%3d, %3d, %3d) -> (%3d, %3d, %3d)\n", n, sz, sz, n, sz/2, sz/2);
    fflush(stdout);

    int h = sz >> 1;
    for (int i = 0; i < n; i++)
    for (int y = 0; y < h; y++)
    for (int x = 0; x < h; x++)
    {
        float v0 = bufIn[sz*sz*i + sz*(y*2)   + (x*2)];
        float v1 = bufIn[sz*sz*i + sz*(y*2)   + (x*2+1)];
        float v2 = bufIn[sz*sz*i + sz*(y*2+1) + (x*2)];
        float v3 = bufIn[sz*sz*i + sz*(y*2+1) + (x*2+1)];
        bufOut[i*h*h + x + h*y] = MAX(MAX(MAX(v0, v1), v2), v3);
    }
}

// ------------------------------------------------------------------------

void evalNetwork(float *buf0) {
    float* buf1 = new float[64 * 224 * 224];

    // Evaluate the network, ping-pong data between buffers.
    printf("Starting inference.\n");
    fflush(stdout);

    evalConv(0, buf0, buf1);
    evalConv(1, buf1, buf0);
    maxPool2x2(224, 64, buf0, buf1);
    evalConv(2, buf1, buf0);
    evalConv(3, buf0, buf1);
    maxPool2x2(112, 128, buf1, buf0);
    evalConv(4, buf0, buf1);
    evalConv(5, buf1, buf0);
    evalConv(6, buf0, buf1);
    evalConv(7, buf1, buf0);
    maxPool2x2(56, 256, buf0, buf1);
    evalConv(8, buf1, buf0);
    evalConv(9, buf0, buf1);
    evalConv(10, buf1, buf0);
    evalConv(11, buf0, buf1);
    maxPool2x2(28, 512, buf1, buf0);
    evalConv(12, buf0, buf1);
    evalConv(13, buf1, buf0);
    evalConv(14, buf0, buf1);
    evalConv(15, buf1, buf0);
    maxPool2x2(14, 512, buf0, buf1);
    evalDense(0, buf1, buf0);
    evalDense(1, buf0, buf1);
    evalDense(2, buf1, buf0);

    printf("Done.\n\n");
    fflush(stdout);

    delete[] buf1;
}

// ------------------------------------------------------------------------

