#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <chrono>
#include <array>
#include <vector>

namespace ppc {
    using clock = std::chrono::high_resolution_clock;
    using time_point = decltype(clock::now());

    template <int sz>
    struct stopwatch_fixed {
        std::array<time_point, sz> m_timePoints;
        int m_numTimePoints = 0;

        void record() {
            if (m_numTimePoints == sz) {
                throw std::runtime_error("Recorded time point count exceeds storage size");
            }
            m_timePoints[m_numTimePoints++] = clock::now();
        }

        void print() const {
            std::printf("\nMeasured time diffs:\n");
            for (int i = 0; i < m_numTimePoints - 1; ++i) {
                const double timeDiff = (m_timePoints[i+1] - m_timePoints[i]).count() / double(1E9);
                std::printf("%d-%d: %lf s\n", i, i+1, timeDiff);
            }
        }
    };

    // Same as stopwatch_fixed, but unlimited timepoints and dynamic memory allocation
    struct stopwatch_dynamic {
        std::vector<time_point> m_timePoints;

        void record() {
            m_timePoints.push_back(clock::now());
        }

        void print() const {
            std::printf("\nMeasured time diffs:\n");
            for (int i = 0; i < (int)m_timePoints.size() - 1; ++i) {
                const double timeDiff = (m_timePoints[i+1] - m_timePoints[i]).count() / double(1E9);
                std::printf("%d-%d: %lf s\n", i, i+1, timeDiff);
            }
        }
    };

    using stopwatch = stopwatch_fixed<32>;
}

#endif