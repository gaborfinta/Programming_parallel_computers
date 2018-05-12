#include "is.h"
#include "random.h"
#include "timer.h"
#include "image.h"
#include "pngio.h"

#include <vector>
#include <random>

static constexpr float MINDIFF = 0.001f;

static void colours(ppc::random& rng, float a[3], float b[3]) {
    std::uniform_real_distribution<double> unif(0.0f, 1.0f);
    std::bernoulli_distribution coin(0.2);
    
    float maxdiff = 0;
    do {
        bool done = false;
        while (!done) {
            for (int k = 0; k < 3; ++k) {
                a[k] = unif(rng);
                b[k] = coin(rng) ? unif(rng) : a[k];
                if (a[k] != b[k]) {
                    done = true;
                }
            }
        }
        maxdiff = std::max({ 
            std::abs(a[0]-b[0]),
            std::abs(a[1]-b[1]),
            std::abs(a[2]-b[2])
        });
    } while (maxdiff < MINDIFF);
}

static void dump(const float (&a)[3]) {
    std::cout << a[0] << "," << a[1] << "," << a[2];
}

static void dump(const Result& r) {
    std::cout << "  y0 = " << r.y0 << "\n";
    std::cout << "  x0 = " << r.x0 << "\n";
    std::cout << "  y1 = " << r.y1 << "\n";
    std::cout << "  x1 = " << r.x1 << "\n";
    std::cout << "  outer = "; dump(r.outer); std::cout << "\n";
    std::cout << "  inner = "; dump(r.inner); std::cout << "\n";
}

static bool close(float a, float b) {
    return std::abs(a - b) < 0.0001;
}

static bool equal(const float (&a)[3], const float (&b)[3]) {
    return close(a[0], b[0]) 
        && close(a[1], b[1]) 
        && close(a[2], b[2]);
}

static void render_to_file(
    int ny, int nx, const std::vector<float>& buffer,
    const std::string& filename)
{
    Image8 bitmap;
    bitmap.data.resize(buffer.size());
    for (int i = 0; i < (int)buffer.size(); ++i) {
        bitmap.data[i] = buffer[i] * 255.f;
    }
    bitmap.ny = ny;
    bitmap.nx = nx;
    bitmap.nc = 3;
    write_image(bitmap, filename.c_str());
}

static void draw_box(
    int ny, int nx, const Result& r,
    std::vector<float>& buffer, const float (&color)[3])
{
    (void) ny;
    const auto draw_line = [&](int y)
    {
        const auto xbegin = r.x0;
        const auto xend = r.x1;
        for (int x = xbegin; x < xend; ++x) {
            const int pixel_base = 3*(y*nx + x);
            for (int c = 0; c < 3; ++c) {
                buffer[pixel_base+c] = color[c];
            }
        }
    };

    const auto draw_column = [&](int x)
    {
        const auto ybegin = r.y0;
        const auto yend = r.y1;
        for (int y = ybegin; y < yend; ++y) {
            const int pixel_base = 3*(y*nx + x);
            for (int c = 0; c < 3; ++c) {
                buffer[pixel_base+c] = color[c];
            }
        }
    };

    draw_line(r.y0);
    draw_line(r.y1-1);
    draw_column(r.x0);
    draw_column(r.x1-1);
}

static void draw_boxes(
    int ny, int nx, const Result& e, const Result& r,
    const float* data, const std::string& filename)
{
    std::vector<float> buf(3*ny*nx);
    std::copy(data, data+3*ny*nx, buf.data());
    // draw expected
    draw_box(ny, nx, e, buf, {0.f, 1.f, 0.f});
    // draw result
    draw_box(ny, nx, r, buf, {1.f, 0.f, 1.f});
    render_to_file(ny, nx, buf, filename);
}

static void compare(int ny, int nx, const Result& e, const Result& r, const float* input) {
    if (e.y0 == r.y0 
        && e.x0 == r.x0 
        && e.y1 == r.y1 
        && e.x1 == r.x1 
        && equal(e.outer, r.outer) 
        && equal(e.inner, r.inner)) 
    {
        return;
    }
    // Verify boxes
    bool ok = r.y0 < r.y1;
    ok &= r.x0 < r.x1;
    ok &= r.x0 >= 0;
    ok &= r.x1 < nx;
    ok &= r.y0 >= 0;
    ok &= r.y1 < ny;
    
    std::cerr << "Test failed." << std::endl;
    std::cerr << "ny = " << ny << "\n";
    std::cerr << "nx = " << nx << "\n";
    std::cerr << "Expected:\n";
    dump(e);
    std::cerr << "Got:\n";
    dump(r);
    if (ok) {
        draw_boxes(ny, nx, e, r, input, "compare.png");
        std::cerr << "Comparison image written into compare.png\n";
    }
    
    exit(EXIT_FAILURE);
}

static void test(ppc::random& rng, int ny, int nx, int y0, int x0,
    int y1, int x1, bool binary, bool randomColors)
{
    Result e;
    e.y0 = y0;
    e.x0 = x0;
    e.y1 = y1;
    e.x1 = x1;
    if (binary) {
        for (int c = 0; c < 3; ++c) {
            e.inner[c] = 1.0f;
            e.outer[c] = 0.0f;
        }
    }
    else {
        if (randomColors) {
            // Random but distinct colours
            colours(rng, e.inner, e.outer);
        }
        // Test worst-case scenario
        else {
            for (int c = 0; c < 3; ++c) {
                e.inner[c] = 1.0f;
                e.outer[c] = 1.0f;
            }
            e.outer[0] -= MINDIFF;
        }
    }

    
    std::vector<float> data(3*ny*nx);
    for (int y = 0; y < ny; ++y) {
        for (int x = 0; x < nx; ++x) {
            for (int c = 0; c < 3; ++c) {
                bool inside = y0 <= y && y < y1 && x0 <= x && x < x1;
                data[c + 3 * x + 3 * nx * y] = inside ? e.inner[c] : e.outer[c];
            }
        }
    }

    Result r;
    std::cout << "is\t" << ny << '\t' << nx << '\t' << binary << '\t' << randomColors << '\t' << std::flush;
    {
        ppc::timer t;
        r = segment(ny, nx, data.data());
    }
    std::cout << std::endl;
    compare(ny, nx, e, r, data.data());
}

static void test_single(ppc::random& rng, int ny, int nx, bool binary, bool random_only = false)
{
    if (ny*nx <= 2) return;
    
    bool ok = false;
    int y0, x0, y1, x1;
    do {
        // Random box location
        std::uniform_int_distribution<int> dy0(0, ny-1); 
        std::uniform_int_distribution<int> dx0(0, nx-1); 
        y0 = dy0(rng);
        x0 = dx0(rng);

        std::uniform_int_distribution<int> dy1(y0+1, ny);
        std::uniform_int_distribution<int> dx1(x0+1, nx);
        y1 = dy1(rng);
        x1 = dx1(rng);
        // Avoid ambiguous cases
        if      (y0 == 0 && y1 == ny && x0 == 0)  { ok = false; }
        else if (y0 == 0 && y1 == ny && x1 == nx) { ok = false; }
        else if (x0 == 0 && x1 == nx && y0 == 0)  { ok = false; }
        else if (x0 == 0 && x1 == nx && y1 == ny) { ok = false; }
        else { ok = true; }
    } while (!ok);
    
    test(rng, ny, nx, y0, x0, y1, x1, binary, true);
    if (!random_only) {
        test(rng, ny, nx, y0, x0, y1, x1, binary, false);
    } 
}

static void test(ppc::random& rng, int ny, int nx, bool binary) {
    for (int i = 0; i < 10; ++i) {
        test_single(rng, ny, nx, binary);
    }
}

static std::vector<float> generate_gradient(
    int ny, int nx, int x0, int x1, int y0, int y1, int y2,
    float color_outer, float color_inner)
{
    std::vector<float> bitmap(nx*ny*3);
    const float fact = 1.0f / float(y2-y1);

    for (int y = 0; y < ny; ++y) {
        const bool yinside = y >= y0 && y < y1;
        const bool yinside_gradient = y >= y1 && y < y2;
        for (int x = 0; x < nx; ++x) {
            const auto pixel_base = (nx*y + x)*3;
            const bool xinside = x >= x0 && x < x1;
            const bool inside = yinside && xinside;
            const bool inside_gradient = yinside_gradient && xinside;

            if (inside) {
                for (int c = 0; c < 3; ++c) {
                    bitmap[pixel_base+c] = color_inner;
                }
            }
            else if (inside_gradient) {
                const float val = float(y2-y) * fact * (color_inner-color_outer) + color_outer;
                for (int c = 0; c < 3; ++c) {
                    bitmap[pixel_base+c] = val;
                }
            }
            else {
                for (int c = 0; c < 3; ++c) {
                    bitmap[pixel_base+c] = color_outer;
                }
            }
        }
    }
    return bitmap;
}

static Result segment_gradient(
    int ny, int nx, int x0, int x1,
    int y0, int y1, int y2, const float* data)
{
    // We know all the boundaries, except inside the gradient
    __float128 color_outer;
    __float128 color_inner = data[3*(nx*y0 + x0)];

    if (x0 > 0 || y0 > 0) {
        const __float128 gr_color = data[0];
        color_outer = gr_color;
    }
    else if (x1 < nx || y2 < ny) {
        const __float128 gr_color = data[3*(nx*ny)-1];
        color_outer = gr_color;
    }
    else { throw; } // situation should not exist

    const __float128 sumcolor_top = (x1-x0) * (y1-y0) * color_inner;
    __float128 min_sqerror = std::numeric_limits<double>::max();
    Result e;

    // calculate all end positions (naively)
    for (int yend = y1; yend <= y2; ++yend) {
        __float128 sumcolor_inside = sumcolor_top;
        for (int y = y1; y < yend; ++y) {
            const int pixel_base = 3*(nx*y + x0);
            const __float128 gr_color = data[pixel_base];
            sumcolor_inside += (x1-x0) * gr_color;
        }

        __float128 sumcolor_outside = (ny*nx - (x1-x0)*(y2-y0)) * color_outer;
        for (int y = yend; y < y2; ++y) {
            const int pixel_base = 3*(nx*y + x0);
            const __float128 gr_color = data[pixel_base];
            sumcolor_outside += (x1-x0) * gr_color;
        }

        const __float128 pixels_inside = __float128((yend-y0)*(x1-x0));
        const __float128 pixels_outside = __float128(ny*nx) - pixels_inside;

        const __float128 color_inside = sumcolor_inside / pixels_inside;
        const __float128 color_outside = sumcolor_outside / pixels_outside;

        __float128 sqerror_inside = (x1-x0) * (y1-y0) * (color_inner-color_inside)*(color_inner-color_inside);
        for (int y = y1; y < yend; ++y) {
            const int pixel_base = 3*(nx*y + x0);
            const __float128 gr_color = data[pixel_base];
            sqerror_inside += (x1-x0) * (gr_color-color_inside)*(gr_color-color_inside);
        }

        __float128 sqerror_outside = ((ny*nx) - (x1-x0)*(y2-y0)) 
            * (color_outer-color_outside)*(color_outer-color_outside);
        for (int y = yend; y < y2; ++y) {
            const int pixel_base = 3*(nx*y + x0);
            const __float128 gr_color = data[pixel_base];
            sqerror_outside += (x1-x0) * (gr_color-color_outside)*(gr_color-color_outside);
        }

        const __float128 sqerror = 3.0 * (sqerror_inside+sqerror_outside);
        // std::cout << (double)sqerror << '\n';
        if (sqerror < min_sqerror) {
            min_sqerror = sqerror;
            for (int c = 0; c < 3; ++c) {
                e.outer[c] = color_outside;
                e.inner[c] = color_inside;
            }
            e.y0 = y0;
            e.y1 = yend;

            e.x0 = x0;
            e.x1 = x1;
        }
        
    }
    return e;
}

static void test_gradient(ppc::random& rng,
    int ny, int nx, int x0, int x1,
    int y0, int y1, int y2)
{
    float color_outer;
    float color_inner;
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    do {
        color_outer = dist(rng);
        color_inner = dist(rng);
    } while(std::abs(color_outer-color_inner) < MINDIFF);

    const auto data = generate_gradient(
        ny, nx, x0, x1, y0, y1, y2, color_outer, color_inner
    );

    Result e = segment_gradient(ny, nx, x0, x1, y0, y1, y2, data.data());
    Result r;
    std::cout << "is\t" << ny << "\t" << nx << "\t" << 2 << '\t' << 0 << '\t' << std::flush;
    {
        ppc::timer t;
        r = segment(ny, nx, data.data());
    }
    std::cout << '\n';
    compare(ny, nx, e, r, data.data());
}

static void test_gradient(ppc::random& rng, int ny, int nx)
{
    for (int i = 0; i < 10; ++i) {
        // Random box location
        std::uniform_int_distribution<int> dx0(0, nx-1);  int x0 = dx0(rng);
        std::uniform_int_distribution<int> dx1(x0+1, nx); int x1 = dx1(rng);
        std::uniform_int_distribution<int> dy0(0, ny-1);  int y0 = dy0(rng);
        std::uniform_int_distribution<int> dy1(y0+1, ny); int y1 = dy1(rng);
        std::uniform_int_distribution<int> dy2(y1, ny);   int y2 = dy2(rng);
        
        // Avoid ambiguous cases
        if (!(x0 > 0 && x1 < nx && y0 > 0 && y2 < ny)) continue;
        test_gradient(rng, ny, nx, x0, x1, y0, y1, y2);
    }
}

static void do_default_test(ppc::random& rng, bool binary) {
    for (int ny = 1; ny < 60; ++ny) {
        test(rng, ny, 1, binary);
        test(rng, 1, ny, binary);
        if (!binary) {
            test_gradient(rng, ny, 1);
        }
    }
    for (int ny = 2; ny < 60; ny += 13) {
        for (int nx = 2; nx < 60; nx += 7) {
            test(rng, ny, nx, binary);
            if (!binary) {
                test_gradient(rng, ny, nx);
            }
        }
            
    }
    test(rng, 1000, 1, binary);
    test(rng, 1, 1000, binary);
    test(rng, 1000, 2, binary);
    test(rng, 2, 1000, binary);
    test(rng, 100, 50, binary);
}

int main(int argc, char** argv) {

    ppc::random rng(24, 132);
    rng();

    std::vector<std::string> args;
    args.reserve(argc);
    for (int i = 0; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    if (argc == 1) {
        do_default_test(rng, false);
        return 0;
    }
    else if (argc < 2 || argc > 5) {
        std::cerr << "Wrong number of arguments\n";
        return 1;
    }

    int cur_index = 1;

    bool is_binary = false;
    bool is_benchmarktest = false;

    if (args[cur_index] == "binary") {
        is_binary = true;
        ++cur_index;
    }

    if (cur_index == (int)args.size()) {
        do_default_test(rng, true);
        return 0;
    }

    if (args[cur_index] == "benchmarktest") {
        is_benchmarktest = true;
        ++cur_index;
    }

    int ny = 0;
    int nx = 0;

    try {
        ny = std::stoi(args.at(cur_index));
        nx = std::stoi(args.at(cur_index+1));
    } catch (...) {
        std::cerr << "Invalid window size argument\n";
        return 1;
    }
    

    if (is_benchmarktest) {
        test_single(rng, ny, nx, is_binary, true);
        return 0;
    }
    else {
        test(rng, ny, nx, is_binary);
        if (!is_binary) {
            test_gradient(rng, ny, nx);
        }
        return 0;
    }

    {
        std::cout << "Usage:\n"
            << "  is-test [binary]\n"
            << "  is-test [binary] ny nx\n"
            << "  is-test [binary] benchmarktest ny nx\n";
    }
}