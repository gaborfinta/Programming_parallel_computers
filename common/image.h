#ifndef IMAGE_H
#define IMAGE_H

#include <algorithm>
#include <vector>
#include <cmath>
#include <cstdint>

constexpr float gam = 2.2f;
constexpr float igam = 1.0f/gam;

template <typename T>
struct Image {
    int ny = 0;  // Height
    int nx = 0;  // Width
    int nc = 0;  // Channels
    std::vector<T> data;

    void resize(int ny_, int nx_, int nc_) {
        ny = ny_;
        nx = nx_;
        nc = nc_;
        data.resize(ny * nx * nc);
    }

    void resize_like(const Image<T>& o) {
        resize(o.ny, o.nx, o.nc);
    }

    T get(int y, int x, int c) const {
        return data[c + nc * (x + nx * y)];
    }

    void set(int y, int x, int c, T v) {
        data[c + nc * (x + nx * y)] = v;
    }

    float getlin(int y, int x, int c) const {
        float v = static_cast<float>(get(y, x, c));
        v /= std::numeric_limits<T>::max();
        return std::pow(v, gam);
    }

    void setlin(int y, int x, int c, float v) {
        v = std::pow(v, igam);
        v = std::min(v, 1.0f);
        v = std::max(v, 0.0f);
        v *= std::numeric_limits<T>::max();
        v = std::round(v);
        set(y, x, c, static_cast<T>(v));
    }

    void setlin3(int y, int x, float r, float g, float b) {
        setlin(y, x, 0, r);
        setlin(y, x, 1, g);
        setlin(y, x, 2, b);
    }

    float getgray(int y, int x) const {
        if (nc == 1) {
            return getlin(y, x, 0);
        } else {
            float r = getlin(y, x, 0);
            float g = getlin(y, x, 1);
            float b = getlin(y, x, 2);
            return 0.2126f * r + 0.7152f * g + 0.0722f * b;
        }
    }

    void getlin(std::vector<float>& result) const {
        result.resize(ny * nx * nc);
        for (int y = 0; y < ny; ++y) {
            for (int x = 0; x < nx; ++x) {
                for (int c = 0; c < nc; ++c) {
                    result[c + nc * x + nc * nx * y] = getlin(y, x, c);
                }
            }
        }
    }

    T* rowptr(int y) {
        return data.data() + y * nx * nc;
    }

    const T* crowptr(int y) const {
        return data.data() + y * nx * nc;
    }
};

typedef Image<uint8_t> Image8;

#endif
