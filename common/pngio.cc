#include "pngio.h"
#include <iostream>
#include <png.h>
#include "error.h"

void read_image(Image8& im, const char* filename, bool verbose) {
    // Setup
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        error("png_create_read_struct failed");
    }
    png_infop info = png_create_info_struct(png);
    if (!info) {
        error("png_create_info_struct failed");
    }
    if (setjmp(png_jmpbuf(png))) {
        error(filename, "error reading the PNG file");
    }
    FILE* f = fopen(filename, "rb");
    if (!f) {
        error(filename, "cannot open for reading");
    }
    png_init_io(png, f);
    // Read info
    png_read_info(png, info);
    int width = png_get_image_width(png, info);
    int height = png_get_image_height(png, info);
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);
    // Set transformation
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png);
    }
    if (png_get_valid(png, info, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY) {
        png_set_gray_to_rgb(png);
    }
    if (bit_depth == 16) {
        error(filename, "16-bit colours not supported");
    }
    png_color_16 bg = {0, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    png_set_background(png, &bg, PNG_BACKGROUND_GAMMA_SCREEN, 0, 1);
    png_set_interlace_handling(png);
    png_read_update_info(png, info);
    // Read info again
    int rowbytes = png_get_rowbytes(png, info);
    int channels = png_get_channels(png, info);
    if (channels != 3) {
        error(filename, "expected 3 channels");
    }
    if (rowbytes != 3 * width) {
        error(filename, "expected 3 bytes per pixel");
    }
    // Read data
    im.resize(height, width, channels);
    std::vector<uint8_t*> rows;
    for (int y = 0; y < im.ny; ++y) {
        rows.push_back(im.rowptr(y));
    }
    png_read_image(png, rows.data());
    // Done
    png_read_end(png, NULL);
    fclose(f);
    png_destroy_read_struct(&png, &info, NULL);
    if (verbose) {
        // Report
        std::cout << filename << ": " << im.nx << "x" << im.ny << std::endl;
    }
}

void write_image(const Image8& im, const char* filename, bool verbose) {
    // Setup
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        error("png_create_read_struct failed");
    }
    png_infop info = png_create_info_struct(png);
    if (!info) {
        error("png_create_info_struct failed");
    }
    if (setjmp(png_jmpbuf(png))) {
        error(filename, "error writing the PNG file");
    }
    FILE* f = fopen(filename, "wb");
    if (!f) {
        error(filename, "cannot open for writing");
    }
    png_init_io(png, f);
    // Image type
    png_byte color_type;
    if (im.nc == 3) {
        color_type = PNG_COLOR_TYPE_RGB;
    } else if (im.nc == 1) {
        color_type = PNG_COLOR_TYPE_GRAY;
    } else {
        error("only 1 or 3 channels supported");
    }
    // Write info
    png_set_IHDR(
        png, info, im.nx, im.ny, 8,
        color_type, PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE
    );
    png_write_info(png, info);
    // Write data
    std::vector<const uint8_t*> rows;
    for (int y = 0; y < im.ny; ++y) {
        rows.push_back(im.crowptr(y));
    }
    png_write_image(png, const_cast<png_bytepp>(rows.data()));
    // Done
    png_write_end(png, NULL);
    if (fclose(f) != 0) {
        error(filename, "write error");
    }
    png_destroy_write_struct(&png, &info);
    if (verbose) {
        // Report
        std::cout << filename << ": " << im.nx << "x" << im.ny << std::endl;
    }
}
