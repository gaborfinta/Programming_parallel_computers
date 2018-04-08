#include "is.h"

Result segment(int ny, int nx, const float* data) {
    // FIXME
    Result result { ny/3, nx/3, 2*ny/3, 2*nx/3, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f} };
    return result;
}
