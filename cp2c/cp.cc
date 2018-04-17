
#include "cp.h"
#include <cstdlib>
#include <cmath>
#include <vector>
#include <numeric>
#include <limits>
#include "vector.h"

constexpr int nDouble = 4;

double MeanOfRow(const int& nx, const double4_t* workingData, const int& y, const int& nVectors)
{
    double4_t mean = workingData[nVectors * y];

    for(int x = 1; x < nVectors; ++x)
    {
        mean += workingData[x + nVectors * y];
    }

    double ret = 0.;
    for(int i = 0; i < nDouble; ++i)
    {
	ret += mean[i];
    }
    
    return ret / (double)nx;
}

double CalculateDenominator(const int& nx, const int& y, const double4_t* workingData, const int& nVectors)
{
    double4_t denominator = {0., 0., 0., 0.}; 

    for(int x = 0; x < nVectors - 1; ++x)
    {
        denominator += workingData[x + y * nVectors] * workingData[x + y * nVectors];
    }
    
    double ret = 0.;
    for(int i = 0; i < nDouble; ++i)
    {
	ret += denominator[i];
    }
    
    for(int x = 0; x < (nx % 4 == 0 ? 4 : nx % nDouble); ++x)
    {
        ret += workingData[(y + 1) * nVectors - 1][x] * workingData[(y + 1) * nVectors - 1][x];
    }
    
    return sqrt(ret);
}

double CalculateSum(const int& y, const int& x, const double4_t* workingData, const int& nVectors)
{   
    double4_t sum = workingData[nVectors * y] * workingData[nVectors * x];
asm("#foo");
    for(int k = 1; k < nVectors; ++k)
    {
        sum += workingData[k + nVectors * y] * workingData[k + nVectors * x];
    }
asm("#bar");
    double ret = 0.;
    for(int i = 0; i < nDouble; ++i)
    {
	ret += sum[i];
    }
    
    return ret;
}

void correlate(int ny, int nx, const float* data, float*result)
{
    int nVectors = (nx + nDouble - 1) / nDouble;

    double4_t* workingData = double4_alloc(ny*nVectors);
    
    //copying data to vectors
    for(int y = 0; y < ny; ++y)
    {
        for(int x = 0; x < nVectors; ++x)
	{
	    for(int actDouble = 0; actDouble < nDouble; ++actDouble)
	    {
		int actCol = actDouble + x * nDouble;
                workingData[x + nVectors * y][actDouble] = actCol < nx ? data[y * nx + actCol] : 0.;
	    }
	}
    }
    
    //first normalization
    for(int y = 0; y < ny; ++y)
    {
        double mean = MeanOfRow(nx, workingData, y, nVectors);
	double4_t meanVec = {mean, mean, mean, mean};
	
        for(int actVector = 0; actVector < nVectors - (nx % nDouble == 0 ? 0 : 1); ++actVector)
	{
            workingData[actVector + nVectors * y] -= meanVec;
	}
	for(int actDouble = 0; actDouble < nx % nDouble; ++actDouble)
	{
            workingData[(y + 1) * nVectors - 1][actDouble] -= mean;
	}
	
    }
    //second normalization
    for(int y = 0; y < ny; ++y)
    {
        double denominator = CalculateDenominator(nx, y, workingData, nVectors);
	double4_t denomVec = { denominator, denominator, denominator, denominator};
        for(int actVector = 0; actVector < nVectors - (nx % nDouble == 0 ? 0 : 1); ++actVector)
	{
            workingData[actVector + nVectors * y] /= denomVec;
	}
	for(int actDouble = 0; actDouble < nx % nDouble; ++actDouble)
	{
            workingData[(y + 1) * nVectors - 1][actDouble] /= denominator;
	}
    }

  //Matrix multiplication

    for(int y = 0; y < ny; ++y)
    {
	for(int x = y; x < ny; ++x)
	{
            double sum = CalculateSum(y, x, workingData, nVectors);
	    result[x + y * ny] = (float)sum;
	}
    }
    free(workingData);
}
