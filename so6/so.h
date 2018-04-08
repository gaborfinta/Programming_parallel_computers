#ifndef SO_H
#define SO_H

// Data type (64-bit unsigned integers).

typedef unsigned long long data_t;

// Sort the first n elements of data.
//
// The result should be equivalent to:
//   std::sort(data, data+n);

void psort(int n, data_t* data);

#endif