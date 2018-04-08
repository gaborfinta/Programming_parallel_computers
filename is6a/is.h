#ifndef IS_H
#define IS_H

// *** Encoding of colours ***
//
// A colour consists of three components: red, green, blue.
// Each component is a float in the range [0.0, 1.0].
// Components are numbered red = 0, green = 1, blue = 2.

// *** Result ***
//
// Data structure for storing a segmentation.
//
// y0, x0, y1, x1: location of the rectangle
// outer: colour of the region outside the rectangle
// inner: colour of the region inside the rectangle
//
// The rectangle covers:
// - x coordinates: x0 ... x1-1
// - y coordinates: y0 ... y1-1

struct Result {
    int y0;
    int x0;
    int y1;
    int x1;
    float outer[3];
    float inner[3];
};

// *** segment ***
//
// nx, ny: image dimensions, width x pixels and height y pixels.
// data: input image.
//
// Colour component c of pixel (x,y) for
// 0 <= c < 3, 0 <= x < nx, and 0 <= y < ny
// is located at data[c + 3 * x + 3 * nx * y]
//
// Return value: optimal segmentation, with:
// 0 <= y0 < y1 <= ny
// 0 <= x0 < x1 <= nx

Result segment(int ny, int nx, const float* data);

#endif