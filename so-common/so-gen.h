#include "so.h"

#include <array>
#include <cassert>

enum class gen_type : int {
    rand = 0,
    rand_small,
    constant,
    incr,
    decr
};

static constexpr std::array<gen_type, 5> gen_types {
    gen_type::rand,
    gen_type::rand_small,
    gen_type::constant,
    gen_type::incr,
    gen_type::decr
};

static constexpr data_t MAGIC = 12345;
static constexpr uint64_t seed1 = 230;
static constexpr uint64_t seed2 = 964;

template <typename iter, typename rng_type>
static void generate(iter begin, iter end, rng_type& rng, gen_type type)
{
    int64_t i = 0;
    const auto n = std::distance(begin, end);

    switch (type) {
    case gen_type::rand:
        for (auto it = begin; it != end; ++it) {
            *it = rng();
        }
        break;
    case gen_type::rand_small:
        for (auto it = begin; it != end; ++it) {
            *it = rng() & 3;
        }
        break;
    case gen_type::constant:
        std::fill(begin, end, MAGIC);
        break;
    case gen_type::incr:
        for (auto it = begin; it != end; ++it) {
            *it = MAGIC + i++;
        }
        break;
    case gen_type::decr:
        for (auto it = begin; it != end; ++it) {
            *it = MAGIC + n - i++;
        }
        break;
    default:
        assert(false);
    }
}

