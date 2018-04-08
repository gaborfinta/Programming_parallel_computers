#ifndef CP_H
#define CP_H

// ny: number of rows in the input matrix.
// nx: number of columns in the input matrix.
// data: input matrix, ny * nx elements.
// out: output matrix, ny * ny elements.
//
// For all 0 <= y < ny and 0 <= x < nx, the element
// at row y and column x is stored in data[x + y*nx].
//
// For all 0 <= j <= i < ny, the correlation between
// input rows i and j needs to be stored in result[i + j*ny].
//
// The elements i < j can be left undefined.

void correlate(int ny, int nx, const float* data, float* result);

constexpr bool STRICT_PRECISION = false;

#endif
