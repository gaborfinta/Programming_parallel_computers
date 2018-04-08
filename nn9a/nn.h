
// ------------------------------------------------------------------------

#define TOTAL_NUM_WEIGHTS 143667240
extern float* g_weights;

// ------------------------------------------------------------------------

struct ConvLayer
{
    int  sz;                // size of input/output feature maps
    int  nOut;              // number of feature maps out
    int  nIn;               // number of feature maps in
    int  ofsW;              // offset to weight tensor [nOut][nIn][3][3]
    int  ofsB;              // offset to bias vector   [nOut]
};

struct DenseLayer
{
    int  nOut;              // length of output vector
    int  nIn;               // length of input vector
    int  ofsW;              // offset to weight matrix [nOut][nIn]
    int  ofsB;              // offset to bias vector   [nOut]
    bool softmax;           // do softmax activation? if false, use ReLU.
};

// ------------------------------------------------------------------------

extern ConvLayer g_convLayers[16];
extern DenseLayer g_denseLayers[3];

// ------------------------------------------------------------------------

void evalNetwork(float *buf0);
