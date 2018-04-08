#include "so.h"
#include <algorithm>

void psort(int n, data_t* data) {
    // FIXME: make this more efficient with parallelism
    std::sort(data, data + n);
}
