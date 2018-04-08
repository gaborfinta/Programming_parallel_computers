#ifndef PPCRANDOM_H
#define PPCRANDOM_H

#include <limits>
#include <array>

namespace ppc {
    /*
        Xoroshiro 128+ PRNG.
        Can be used with C++ random number distributions.
    */
    class random {
    public:
        using result_type = uint64_t;
    private:

        std::array<uint64_t, 2> m_state{};

        static uint64_t rotl(const uint64_t x, int k) {
            return (x << k) | (x >> (64 - k));
        }

        uint64_t next()
        {
            const uint64_t s0 = m_state[0];
            uint64_t s1 = m_state[1];
            const uint64_t result = s0 + s1;

            s1 ^= s0;
            m_state[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14);
            m_state[1] = rotl(s1, 36);

            return result;
        }

    public:
        random(uint64_t seed1, uint64_t seed2): m_state{seed1, seed2} {}
        static constexpr result_type min() { return 0; }
        static constexpr result_type max()
        { 
            return std::numeric_limits<uint64_t>::max();
        }

        result_type operator()()
        {
            return next();
        }
    };
}

#endif
