#include <cassert>
#include "pngio.h"
#include "error.h"
#include "timer.h"
#include "is.h"

inline bool inside(int v, int a, int b) {
    return a <= v && v < b;
}

inline bool boundary(int v, int a, int b) {
    return v == a || v == b - 1;
}

bool is_inside(const Result& r, int y, int x) {
    return inside(y, r.y0, r.y1) && inside(x, r.x0, r.x1);
}

bool at_boundary(const Result& r, int y, int x, int d) {
    if (boundary(y, r.y0-d, r.y1+d)) {
        return inside(x, r.x0-d, r.x1+d);
    } else if (boundary(x, r.x0-d, r.x1+d)) {
        return inside(y, r.y0-d, r.y1+d);
    } else {
        return false;
    }
}

bool at_inner_boundary(const Result& r, int y, int x) {
    return at_boundary(r, y, x, 0);
}

bool at_outer_boundary(const Result& r, int y, int x) {
    return at_boundary(r, y, x, 1);
}

static void process(const Image8& in, Image8& out1, Image8& out2) {
    assert(in.nc == 3);
    Result r;
    std::vector<float> data;
    in.getlin(data);
    std::cout << "is\t" << in.ny << "\t" << in.nx << "\t" << std::flush;
    { ppc::timer t; r = segment(in.ny, in.nx, data.data()); }
    std::cout << std::endl;
    out1.resize_like(in);
    out2.resize_like(in);
    for (int y = 0; y < in.ny; ++y) {
        for (int x = 0; x < in.nx; ++x) {
            if (is_inside(r, y, x)) {
                out1.setlin3(y, x, r.inner[0], r.inner[1], r.inner[2]);
            } else {
                out1.setlin3(y, x, r.outer[0], r.outer[1], r.outer[2]);
            }
            if (at_inner_boundary(r, y, x)) {
                out2.setlin3(y, x, 1.0f, 1.0f, 1.0f);
            } else if (at_outer_boundary(r, y, x)) {
                out2.setlin3(y, x, 0.0f, 0.0f, 0.0f);
            } else {
                for (int c = 0; c < in.nc; ++c) {
                    out2.set(y, x, c, in.get(y, x, c));
                }
            }
        }
    }
}


int main(int argc, const char** argv) {
    if (argc != 4) {
        error("usage: pngsegment INPUT OUTPUT1 OUTPUT2");
    }
    const char* fin = argv[1];
    const char* fout1 = argv[2];
    const char* fout2 = argv[3];
    Image8 in;
    Image8 out1;
    Image8 out2;
    read_image(in, fin);
    process(in, out1, out2);
    write_image(out1, fout1);
    write_image(out2, fout2);
}
