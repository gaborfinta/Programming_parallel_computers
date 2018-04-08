#ifndef PNGIO_H
#define PNGIO_H

#include "image.h"

void read_image(Image8& im, const char* filename, bool verbose = false);
void write_image(const Image8& im, const char* filename, bool verbose = false);

#endif
