#ifndef TIMER_H
#define TIMER_H

#include <fstream>
#include <cstdlib>

#include <iomanip>
#include <iostream>
#include <sys/time.h>
#include <sys/stat.h>

#include <chrono>
#include <vector>

namespace ppc
{

class benchmark_output {
public:

    ~benchmark_output() {
        if (m_times.size() > 0) {
            std::ofstream outfile("benchmark.run");
            for (auto time : m_times) {
                outfile << time << '\n';
            }
        }
    }

    benchmark_output& operator<<(double time) {
        m_times.push_back(time);
        return *this;
    }


private:
    std::vector<double> m_times;
};

static benchmark_output result_output;

class timer {
public:
    using time_point = decltype(std::chrono::high_resolution_clock::now());

    timer()
    {
        write_out = std::getenv("PPC_BENCHMARK") != nullptr;
        start = std::chrono::high_resolution_clock::now();
    }

    ~timer() {
        const auto end = std::chrono::high_resolution_clock::now();
        const double seconds = (end-start).count() / double(1E9);
        if (write_out) {
            result_output << seconds;
        }
        print_formatted(seconds);
    }

private:
    void print_formatted(double sec) {
        std::ios_base::fmtflags oldf = std::cout.flags(std::ios::right | std::ios::fixed);
        std::streamsize oldp = std::cout.precision(3);
        std::cout << sec << '\t' << std::flush;
        std::cout.flags(oldf);
        std::cout.precision(oldp);
        std::cout.copyfmt(std::ios(NULL));
    }

    bool write_out;
    time_point start;
};

}

#endif
