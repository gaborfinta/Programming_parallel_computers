

#include "cp.h"
#include <cstdlib>
#include <cmath>
#include <vector>
#include <numeric>

double MeanOfRow(const int& nx, const float* data, const int& y)
{
    double mean = 0.;
    for(int i = 0; i < nx; ++i)
    {
	mean += (double)data[i + y * nx];
    }
    return mean / (double)nx;
}

double CalculateDenominator(const int& nx, const int& y, const double* normalized)
{
    double denominator = 0.;
    
    for(int x = 0; x < nx; ++x)
    {
	denominator += pow(normalized[x + y * nx], 2);
    }
    return sqrt(denominator);
}

double CalculateSum(const int& nx, const int& y, const int& x, const double* normalized)
{
    double sum = 0.;

    for(int k = 0; k < nx; ++k)
    {
	sum += normalized[k + y * nx] * normalized[k + x * nx];
    }
    return sum;
}

void correlate(int ny, int nx, const float* data, float*result)
{
    double* normalized = (double*)malloc(sizeof(double) * nx * ny);
    //std::vector<double> normalized(nx * ny);

    //first normalization
    #pragma omp parallel for
    for(int y = 0; y < ny; ++y)
    {
	double mean = MeanOfRow(nx, data, y);
	for(int x = 0; x < nx; ++x)
	{
	    normalized[x + y * nx] = (double)(data[x + y * nx]) - mean;
	}
    }
    //second normalization
    #pragma omp parallel for
    for(int y = 0; y < ny; ++y)
    {
	double denominator = CalculateDenominator(nx, y, normalized);
	for(int x = 0; x < nx; ++x)
	{
	    normalized[x + y * nx] /= denominator;
	}
    }

  
  //Matrix multiplication
    #pragma omp parallel for
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
