#include "pngio.h"
#include "error.h"
#include "timer.h"
#include "mf.h"

static void process(const Image8& in, Image8& out1, Image8& out2, int k) {
    out1.resize_like(in);
    out2.resize_like(in);
    std::vector<float> vin(in.nx * in.ny);
    std::vector<float> vout(in.nx * in.ny);
    std::cout << "mf\t" << in.ny << "\t" << in.nx << std::flush;
    for (int c = 0; c < in.nc; ++c) {
        for (int y = 0; y < in.ny; ++y) {
            for (int x = 0; x < in.nx; ++x) {
                vin[x + in.nx * y] = in.getlin(y, x, c);
            }
        }
        std::cout << "\t";
        { ppc::timer t; mf(in.ny, in.nx, k, k, vin.data(), vout.data()); }
        for (int y = 0; y < in.ny; ++y) {
            for (int x = 0; x < in.nx; ++x) {
                float a = vin[x + in.nx * y];
                float b = vout[x + in.nx * y];
                float r = 0.5f + (a - b);
                out1.setlin(y, x, c, b);
                out2.setlin(y, x, c, r);
            }
        }
    }
    std::cout << "\n";
}

int main(int argc, const char** argv) {
    if (argc != 5) {
        error("usage: pngmf WINDOW INPUT OUTPUT1 OUTPUT2");
    }
    int k = std::stoi(argv[1]);
    if (k <= 0) {
        error("window size has to be positive");
    }
    const char* fin = argv[2];
    const char* fout1 = argv[3];
    const char* fout2 = argv[4];
    Image8 in;
    Image8 out1;
    Image8 out2;
    read_image(in, fin);
    process(in, out1, out2, k);
    write_image(out1, fout1);
    write_image(out2, fout2);
}
