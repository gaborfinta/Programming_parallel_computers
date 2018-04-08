#include <iostream>
#include <vector>
#include "pngio.h"
#include "error.h"
#include "timer.h"
#include "cp.h"

static void process_image(const Image8& in, Image8& gray, Image8& out) {
    int ny = in.ny;
    int nx = in.nx;
    gray.resize(ny, nx, 1);
    std::vector<float> data(ny * nx);
    std::vector<float> result(ny * ny);
    for (int y = 0; y < in.ny; ++y) {
        for (int x = 0; x < in.nx; ++x) {
            float v = in.getgray(y, x);
            data[x + nx * y] = v;
            gray.setlin(y, x, 0, v);
        }
    }
    std::cout << "cp\t" << in.ny << "\t" << in.nx << "\t" << std::flush;
    { ppc::timer t; correlate(ny, nx, data.data(), result.data()); }
    std::cout << std::endl;
    out.resize(ny, ny, 3);
    for (int j = 0; j < ny; ++j) {
        for (int i = 0; i < ny; ++i) {
            float v = i < j ? result[j + ny * i] : result[i + ny * j];
            if (v >= 0) {
                out.setlin(j, i, 0, 1.0f);
                out.setlin(j, i, 1, 1.0f - v);
                out.setlin(j, i, 2, 1.0f - v);
            } else {
                v = -v;
                out.setlin(j, i, 0, 1.0f - v);
                out.setlin(j, i, 1, 1.0f - v);
                out.setlin(j, i, 2, 1.0f);
            }
        }
    }
}

int main(int argc, const char** argv) {
    if (argc != 4) {
        error("usage: pngcorrelate INPUT OUTPUT1 OUTPUT2");
    }
    const char* fin = argv[1];
    const char* fout1 = argv[2];
    const char* fout2 = argv[3];
    Image8 in;
    Image8 gray;
    Image8 out;
    read_image(in, fin);
    process_image(in, gray, out);
    write_image(gray, fout1);
    write_image(out, fout2);
}
