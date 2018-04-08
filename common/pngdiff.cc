#include <cassert>
#include <algorithm>
#include <iostream>
#include "pngio.h"
#include "error.h"

static bool compare(int k, const Image8& a, const Image8& b, Image8& diff) {
    if (a.nx != b.nx) {
        error("image width differs");
    }
    if (a.ny != b.ny) {
        error("image height differs");
    }
    if (a.nc != b.nc) {
        error("number of colour channels differs");
    }
    diff.resize_like(a);
    int small = 0;
    int large = 0;
    int largest = 0;
    for (int y = 0; y < a.ny; ++y) {
        for (int x = 0; x < a.nx; ++x) {
            bool issmall = false;
            bool islarge = false;
            for (int c = 0; c < a.nc; ++c) {
                int p = a.get(y, x, c);
                int q = b.get(y, x, c);
                int r = std::abs(p - q);
                if (r == 0) {
                    // Fine
                } else if (r <= k) {
                    issmall = true;
                    ++small;
                } else {
                    islarge = true;
                    ++large;
                }
                largest = std::max(r, largest);
            }
            if (islarge) {
                diff.setlin3(y, x, 1.0, 0.0, 0.0);
            } else if (issmall) {
                diff.setlin3(y, x, 0.0, 0.0, 1.0);
            } else {
                diff.setlin3(y, x, 1.0, 1.0, 1.0);
            }
        }
    }
    if (k) {
        std::cout << small << " differences at most " << k << "\n";
    }
    std::cout << large << " differences larger than " << k << "\n";
    std::cout << "maximum difference is " << largest << "\n";
    return large == 0;
}

int main(int argc, const char** argv) {
    if (argc != 5) {
        error("usage: pngdiff TOLERANCE INPUT1 INPUT2 OUTPUT");
    }
    int k = std::stoi(argv[1]);
    const char* fa = argv[2];
    const char* fb = argv[3];
    const char* fdiff = argv[4];
    Image8 a;
    Image8 b;
    Image8 diff;
    read_image(a, fa, true);
    read_image(b, fb, true);
    bool ok = compare(k, a, b, diff);
    write_image(diff, fdiff);
    if (ok) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}
