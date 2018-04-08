#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <algorithm>
#include <cassert>

#include "cp.h"
#include "error.h"

constexpr float allowed_error = STRICT_PRECISION
    ? std::numeric_limits<float>::epsilon() * 0.6
    : 1e-5;
constexpr float gvfa_limit = 1e-3;

typedef long double pfloat;

static void generate(int ny, int nx, float* data) {
    std::mt19937 rng;
    std::uniform_real_distribution<double> unif(0.0f, 1.0f);
    std::bernoulli_distribution coin(0.2);
    for (int y = 0; y < ny; ++y) {
        if (y > 0 && coin(rng)) {
            // Introduce some correlations
            int row = std::min(static_cast<int>(y * unif(rng)), y - 1);
            double offset = 2.0 * (unif(rng) - 0.5f);
            double mult = 2.0 * unif(rng);
            for (int x = 0; x < nx; ++x) {
                double error = 0.1 * unif(rng);
                data[x + nx * y] = mult * data[x + nx * row] + offset + error;
            }
        } else {
            // Generate random data
            for (int x = 0; x < nx; ++x) {
                double v = unif(rng);
                data[x + nx * y] = v;
            }
        }
    }
}


// Generates random rows in a random 3D subspace
static void generate_subspace(int ny, int nx, float* data) {
    std::mt19937 rng;
    std::normal_distribution<float> d;

    std::vector<float> a(nx);
    std::vector<float> b(nx);
    std::vector<float> c(nx);
    for(int x = 0; x < nx; x++) {
        a[x] = d(rng);
        b[x] = d(rng);
        c[x] = d(rng);
    }

    for(int y = 0; y < ny; y++) {
        float am = d(rng);
        float bm = d(rng);
        float cm = d(rng);
        float len = std::sqrt(am*am + bm*bm + cm*cm);
        am /= len; bm /= len; cm /= len;
        for(int x = 0; x < nx; x++) {
            data[x + nx * y] = am * a[x] + bm * b[x] + cm * c[x];
        }
    }
}

static void generate_normal(int ny, int nx, float* data) {
    std::mt19937 rng;
    std::normal_distribution<float> d;
    std::generate(data, data+nx*ny, [&]{ return d(rng); });
}

static void generate_special(int ny, int nx, float* data) {
    std::mt19937 rng;
    const float a = std::numeric_limits<float>::max();
    std::uniform_real_distribution<double> unif(-a, a);
    std::generate(data, data+nx*ny, [&]{ return unif(rng); });
}

static void generate_measurement(int ny, int nx, float* data) {
    std::mt19937 rng;
    std::uniform_real_distribution<double> unif(0.0, 1.0);
    std::bernoulli_distribution coin(0.5);
    std::vector<double> target(nx);
    std::generate(target.begin(), target.end(), [&]{ return unif(rng); });
    for(int j=0; j<ny; j++) {
        // parameters for y = ax + b
        double a = 4.0 * unif(rng) + 0.25; // [0.25, 4.25]
        if(coin(rng)) a = -a;
        double b = 4.0 * unif(rng) - 2.0; // [-2.0, 2.0]
        double error = unif(rng);
        for(int i=0; i<nx; i++) {
            // generate related data with error
            data[j*nx+i] = a * target[i] + b + (unif(rng)-0.5)*2.0*error;
        }
    }
}

static float verify(int ny, int nx, const float* data, const float* result) {
    bool nans = false;
    double worst = 0.0;

    std::vector<pfloat> normalized(ny*nx);

    for(int j = 0; j < ny; j++) {
        pfloat s = 0.0;
        pfloat ss = 0.0;
        for(int i = 0; i < nx; i++) {
            pfloat x = data[j * nx + i];
            s += x;
            ss += x * x;
        }
        pfloat mean = s / nx;
        pfloat mult = 1.0 / (pfloat) std::sqrt((long double) (ss - s*mean));
        for(int i = 0; i < nx; i++) {
            normalized[j * nx + i] = (data[j * nx + i] - mean) * mult;
        }
    }
    
    pfloat err_sqsum = 0.0;

    for (int j = 0; j < ny; ++j) {
        for (int i = j; i < ny; ++i) {
            float q = result[i + ny * j];
            if (q != q) { // q is NaN
                nans = true;
                continue;
            }
            pfloat temp = 0.0;
            for (int x = 0; x < nx; ++x) {
                pfloat a = normalized[x + nx * i];
                pfloat b = normalized[x + nx * j];
                temp += a * b;
            }
            pfloat err = q - temp;
            err_sqsum += err * err;
            double abserr = std::abs((double) err);
            //std::cout << abserr << std::endl;

            worst = std::max(abserr, worst);
        }
    }
    if (nans) {
        worst = 0.0/0.0;
    }
    return worst;
}

// Does 'iter' iterations of Freivald's algorithm and returns the largest
// difference over all vector elements and iterations.
static float verify_gvfa(int ny, int nx, const float* data, const float* result, int iter) {
    std::vector<double> normalized(ny*nx);
    
    for(int j = 0; j < ny; j++) {
        double s = 0.0;
        for(int i = 0; i < nx; i++) {
            double x = data[j * nx + i];
            s += x;
        }
        double mean = s / nx;
        double ss = 0.0;
        for(int i = 0; i < nx; i++) {
            double x = data[j * nx + i];
            x -= mean;
            normalized[j * nx + i] = x;
            ss += x * x;
        }
        double mult = 1.0 / std::sqrt(ss);
        for(int i = 0; i < nx; i++) {
            normalized[j * nx + i] *= mult;
        }
    }

    std::mt19937 rng;
    std::normal_distribution<double> d;

    float worst = 0.0f;
    for(int k = 0; k < iter; k++) {
        std::vector<double> x(ny);
        for(int j = 0; j < ny; j++) {
            x[j] = d(rng);
        }

        std::vector<double> ATx(nx);
        for(int i = 0; i < nx; i++) {
            double temp = 0.0;
            for(int j = 0; j < ny; j++) {
                temp += normalized[j * nx + i] * x[j];
            }
            ATx[i] = temp;
        }

        std::vector<double> AATx(ny);
        for(int j = 0; j < ny; j++) {
            double temp = 0.0;
            for(int i = 0; i < nx; i++) {
                temp += normalized[j * nx + i] * ATx[i];
            }
            AATx[j] = temp;
        }

        std::vector<double> Bx(ny);
        for(int j = 0; j < ny; j++) {
            double temp = 0.0;
            for(int i = 0; i < ny; i++) {
                temp += result[std::min(i,j) * ny + std::max(i,j)] * x[i];
            }
            Bx[j] = temp;
        }

        float iter_worst = 0.0f;
        for(int j = 0; j < ny; j++) {
            float diff = AATx[j] - Bx[j];
            float err = std::abs(diff);
            if(err != err) return err; // NaN
            iter_worst = std::max(err, iter_worst);
        }
        worst = std::max(iter_worst, worst);
    }
    return worst;
}
static void print(int ny, int nx, const float *matrix) {
    for(int j = 0; j < ny; j++) {
        for(int i = 0; i < nx; i++) {
            float x = matrix[j*nx+i];
            if(std::abs(x) < 10.0) {
                printf("% -7.3f", x);
            } else {
                printf("% -7.0e", x);
            }
        }
        std::cout << '\n';
    }
}

static bool test(int ny, int nx, int mode, bool verbose) {
    std::vector<float> data(ny * nx);
    switch(mode) {
        case 0: generate(ny, nx, data.data()); break;
        case 1: generate_normal(ny, nx, data.data()); break;
        case 2: generate_subspace(ny, nx, data.data()); break;
        case 3: generate_measurement(ny, nx, data.data()); break;
        case 4: generate_special(ny, nx, data.data()); break;
        default: error("unknown MODE");
    }
    std::vector<float> result(ny * ny);
    correlate(ny, nx, data.data(), result.data());

    float gvfa_error = verify_gvfa(ny, nx, data.data(), result.data(), 20);
    bool pass = gvfa_error < gvfa_limit;
    if((double)nx * ny * ny < 1e8) {
        float max_error = verify(ny, nx, data.data(), result.data());
        bool full_pass = max_error < allowed_error;
        // gvfa should never claim error if the result is actually ok
        assert(pass || !full_pass);
        pass = full_pass;
        std::cout << std::fixed << std::setprecision(3);
        std::cout << '\t' << max_error / allowed_error << '\t';
    } else {
        std::cout << std::fixed << std::setprecision(3);
        std::cout << '\t' << gvfa_error / gvfa_limit << '\t';
    }

    if(verbose) {
        if(ny < 25 && nx < 25) {
            std::cout << "\ninput:\n";
            print(ny, nx, data.data());
        }
        if(ny < 25) {
            std::cout << "\noutput:\n";
            print(ny, ny, result.data());
        }
        
    } else {
        /*
        std::cout << std::scientific << std::setprecision(4)
            << stats.max_error << "\t"
            << stats.gvfa_max << "\t";
            */
    }
    return pass;
}

static bool has_fails = false;
static struct { int ny; int nx; int mode; } first_fail = {};
static int passcount = 0;
static int testcount = 0;

// To be used in batch mode to keep track of test suite progress
static void run_test(int ny, int nx, int mode, bool verbose) {
    std::cout << "cp-test "
        << std::setw(4) << ny << ' '
        << std::setw(4) << nx << ' '
        << std::setw(1) << mode << ' '
        << std::flush;
    bool pass = test(ny, nx, mode, verbose);
    std::cout << (pass ? "OK\n" : "ERR\n");
    if(pass) {
        passcount++;
    } else if(!has_fails) {
        has_fails = true;
        first_fail.ny = ny;
        first_fail.nx = nx;
        first_fail.mode = mode;
    }
    testcount++;
}

int main(int argc, const char** argv) {
    if(argc == 1) {
        for(int ny=2; ny<10; ny++)
        for(int nx=2; nx<10; nx++)
            run_test(ny, nx, 2, false);

        std::vector<int> modes = {0,1,2,3};
        std::vector<int> nxs = {10,50,100,200,500,1000};
        std::vector<int> nys = {2,5,10,100,200};
        for(int ny : nys)
        for(int nx : nxs)
        for(int mode : modes)
            run_test(ny, nx, mode, false);

        for(int ny=42; ny<50; ny++)
        for(int nx=100; nx<108; nx++)
            run_test(ny, nx, 2, false);

        if(STRICT_PRECISION) {
            int mode = 4;
            for(int x=2; x < 100; x*=3) {
                run_test(x, x, mode, false);
            }
        }

        std::cout << passcount << "/" << testcount << " tests passed.\n";
        if(has_fails) {
            std::cout 
                << "To repeat the first failed test with more output, run:\n"
                << argv[0] << " "
                << first_fail.ny << " "
                << first_fail.nx << " "
                << first_fail.mode << std::endl;
            exit(EXIT_FAILURE);
        }
    } else if(argc == 4) {
        int ny = std::stoi(argv[1]);
        int nx = std::stoi(argv[2]);
        int mode = std::stoi(argv[3]);
        run_test(ny, nx, mode, true);
        if(has_fails) {
            exit(EXIT_FAILURE);
        }
    } else {
        std::cout << "Usage:\n  cp-test\n  cp-test <ny> <nx> <mode>\n";
    }
}
