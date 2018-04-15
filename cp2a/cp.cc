
#include "cp.h"
#include <cstdlib>
#include <cmath>
#include <vector>
#include <numeric>

double MeanOfRow(const int& nx, const float* data, const int& y)
{
    std::vector<double> means(4, 0.0);
    for(int i = 0; i < nx; ++i)
    {
	means[i % 4] += (double)data[i + y * nx];
    }
    return (means[0] + means[1] + means[2] + means[3]) / (double)nx;
}

double CalculateDenominator(const int& nx, const int& y, const double* normalized)
{
    std::vector<double> denominators(4, 0.0);
    
    for(int x = 0; x < nx; ++x)
    {
	denominators[x % 4] += pow(normalized[x + y * nx], 2);
    }
    return sqrt(denominators[0] + denominators[1] + denominators[2] + denominators[3]);
}

double CalculateSum(const int& nx, const int& y, const int& x, const double* normalized)
{
    std::vector<double> sums(4, 0.0);

    for(int k = 0; k < nx; ++k)
    {
	sums[k % 4] += normalized[k + y * nx] * normalized[k + x * nx];
    }
    return std::accumulate(sums.begin(), sums.end(), 0.);
}

void correlate(int ny, int nx, const float* data, float*result)
{
    double* normalized = (double*)malloc(sizeof(double) * nx * ny);
    //std::vector<double> normalized(nx * ny);

    //first normalization
    
    for(int y = 0; y < ny; ++y)
    {
	double mean = MeanOfRow(nx, data, y);
	for(int x = 0; x < nx; ++x)
	{
	    normalized[x + y * nx] = (double)(data[x + y * nx]) - mean;
	}
    }
    //second normalization

    for(int y = 0; y < ny; ++y)
    {
	double denominator = CalculateDenominator(nx, y, normalized);
      
	for(int x = 0; x < nx; ++x)
	{
	    normalized[x + y * nx] /= denominator;
	}
    }

  
  //Matrix multiplication
    
    for(int y = 0; y < ny; ++y)
    {
        for(int x = y; x < ny; ++x)
	{
	    double sum = CalculateSum(nx, y, x, normalized);
	    result[x + y * ny] = (float)sum;
	}
    }
    free(normalized);
}
