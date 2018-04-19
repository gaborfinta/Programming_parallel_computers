
#include "cp.h"
#include <cstdlib>
#include <cmath>
#include <vector>
#include <numeric>

constexpr int nPad = 4;

double MeanOfRow(const int& nx, const float* data, const int& y)
{
    //std::vector<double> means(4, 0.0);
    double* sums = (double*)malloc(sizeof(double) * nPad);
    for(int i = 0; i < nPad; ++i)
    {
	sums[i] = 0.0;
    }
    for(int i = 0; i < nx; ++i)
    {
	sums[i % nPad] += (double)data[i + y * nx];
    }
    double sum = std::accumulate(sums, sums + nPad, 0.);
    free(sums);
    return sum / (double)nx;
} 

double CalculateDenominator(const int& nx, const int& y, const std::vector<double>& normalized, const int& nNewX)
{
    double* denominators = (double*)malloc(sizeof(double) * nPad);
    //std::vector<double> denominators(4, 0.0);
    for(int i = 0; i < nPad; ++i)
    {
	denominators[i] = 0.0;
    }
    for(int x = 0; x < nx; ++x)
    {
	denominators[x % nPad] += pow(normalized[x + y * nNewX], 2);
    }
    double denominator = std::accumulate(denominators, denominators + nPad, 0.);
    free(denominators);
    return sqrt(denominator);
}

double CalculateSum(const int& nx, const int& y, const int& x, const std::vector<double>& normalized, const int& nNewX)
{
    //std::vector<double> sums(4, 0.0);
    double* sums = (double*)malloc(sizeof(double) * nPad);
    for(int i = 0; i < nPad; ++i)
    {
	sums[i] = 0.0;
    }
    for(int k = 0; k < nx/*nx*/; k += nPad)
    {
	for(int i = 0; i < nPad; ++i)
	{
	    sums[i] += normalized[k + i + y * nNewX] * normalized[k + i + x * nNewX];
	}
	//sums[k % 4] += normalized[k + y * nx] * normalized[k + x * nx];
    }
    double sum = std::accumulate(sums, sums + nPad, 0.);
    free(sums);
    return sum;
}

void correlate(int ny, int nx, const float* data, float*result)
{
    int nPadded = (nx + nPad - 1) / nPad;
    int nNewX = nPadded * nPad;
    //double* normalized = (double*)malloc(sizeof(double) * nNewX * ny);
    std::vector<double> normalized(nNewX * ny, 0.0);

    //first normalization
    
    for(int y = 0; y < ny; ++y)
    {
	double mean = MeanOfRow(nx, data, y);
	for(int x = 0; x < nx; ++x)
	{
	    normalized[x + y * nNewX] = (double)(data[x + y * nx]) - mean;
	}
    }
    //second normalization

    for(int y = 0; y < ny; ++y)
    {
	double denominator = CalculateDenominator(nx, y, normalized, nNewX);
      
	for(int x = 0; x < nx; ++x)
	{
	    normalized[x + y * nNewX] /= denominator;
	}
    }

  
  //Matrix multiplication
    
    for(int y = 0; y < ny; ++y)
    {
        for(int x = y; x < ny; ++x)
	{
	    double sum = CalculateSum(nx, y, x, normalized, nNewX);
	    result[x + y * ny] = (float)sum;
	}
    }
    //free(normalized);
}
