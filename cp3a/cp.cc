
#include "cp.h"
#include <cstdlib>
#include <cmath>
#include <vector>
#include <numeric>
#include <limits>
#include "vector.h"

constexpr int nDouble = 4;
constexpr int nParallelOps = 10;

double MeanOfRow(const int& nx, const double4_t* workingData, const int& y, const int& nVectors, const int& nNewX)
{
    double4_t* partialSums = double4_alloc(nParallelOps);
    double4_t zeroVec = {0., 0., 0., 0.};
    for(int i = 0; i < nParallelOps; ++i)
    {
	partialSums[i] = zeroVec;
    }
    for(int x = 0; x < nVectors; x += nParallelOps)
    {
	for(int i = 0; i < nParallelOps; ++i)
	{
	    partialSums[i] += workingData[x + i + nNewX * y];
	}
    }

    double ret = 0.;
    for(int i = 0; i < nParallelOps; ++i)
    {
	for(int j = 0; j < nDouble; ++j)
	{
	    ret += partialSums[i][j];
	}
    }

    

    free(partialSums);
    return ret / (double)nx;
}

double CalculateDenominator(const int& y, const double4_t* workingData, const int& nVectors, const int& nNewX)
{
    double4_t* denominators = double4_alloc(nParallelOps);
    for(int i = 0; i < nParallelOps; ++i)
    {
	for(int j = 0; j < nDouble; ++j)
	{
	    denominators[i][j] = 0.0;
	}
    } 

    for(int x = 0; x < nVectors; x += nParallelOps)
    {
	for(int i = 0; i < nParallelOps; ++i)
	{
	    denominators[i] += workingData[x + i + y * nNewX] * workingData[x + i + y * nNewX];
	}
    }

    double ret = 0.;
    for(int i = 0; i < nParallelOps; ++i)
    {
	for(int j = 0; j < nDouble; ++j)
	{
	    ret += denominators[i][j];
	}
    }
    free(denominators);
    return sqrt(ret);
}

void CalculateSum(const int& x, const double4_t* workingData, const int& nNewX, double4_t* partialSums, const int& run, double4_t* reusableCells)
{
    double4_t zeroVec = {0., 0., 0., 0.};
    for(int i = 0; i < nParallelOps * nParallelOps; ++i)
    {
	    partialSums[i] = zeroVec;
    }
    
    for(int partialRow = 0; partialRow < nParallelOps; ++partialRow)
    {
	for(int partialCol = 0; partialCol < nParallelOps; ++partialCol)
	{
	    for(int i = 0; i < nParallelOps; ++i)
	    {
		partialSums[partialCol + partialRow * nParallelOps] +=
		    reusableCells[i + partialRow * nParallelOps] * workingData[i + run  + (partialCol + x) * nNewX];
	    }
	}
    }
    
    int k = 2;
    k = k + 2;
}

void correlate(int ny, int nx, const float* data, float*result)
{
    int nVectors = (nx + nDouble - 1) / nDouble;
    int nExtendedRow = (nVectors + nParallelOps - 1) / nParallelOps;
    int nNewX = nExtendedRow * nParallelOps;

    int nExtendedCol = (ny + nParallelOps - 1) / nParallelOps;
    int nNewY = nExtendedCol * nParallelOps;

    double4_t* workingData = double4_alloc(nNewY * nNewX);
    
    //copying data to vectors
    #pragma omp parallel for
    for(int y = 0; y < ny; ++y)
    {
        for(int x = 0; x < nNewX; ++x)
	{
		for(int actDouble = 0; actDouble < nDouble; ++actDouble)
		{
			int actCol = actDouble + x * nDouble;
	 	        workingData[x + nNewX * y][actDouble] = actCol < nx ? (double)data[y * nx + actCol] : 0.;
		}
	}
    }
    double4_t zeroVec = {0., 0., 0., 0.};
    for(int y = ny; y < nNewY; ++y)
    {
	for(int x = 0; x < nNewX; ++x)
	{
	    workingData[x + nNewX * y] = zeroVec;
	}
    }
    
    //first normalization
    #pragma omp parallel for
    for(int y = 0; y < ny; ++y)
    {
        double mean = MeanOfRow(nx, workingData, y, nVectors, nNewX);
	double4_t meanVec = {mean, mean, mean, mean};
	
        for(int actVector = 0; actVector < nVectors - (nx % nDouble == 0 ? 0 : 1); ++actVector)
	{
            workingData[actVector + nNewX * y] -= meanVec;
	}
	for(int actDouble = 0; actDouble < nx % nDouble; ++actDouble)
	{
            workingData[(y + 1) * nNewX - (nNewX - nVectors) - 1][actDouble] -= mean;
	}
	
    }
    //second normalization
    #pragma omp parallel for
    for(int y = 0; y < ny; ++y)
    {
        double denominator = CalculateDenominator(y, workingData, nVectors, nNewX);
	double4_t denomVec = { denominator, denominator, denominator, denominator};
        for(int actVector = 0; actVector < nVectors - (nx % nDouble == 0 ? 0 : 1); ++actVector)
	{
            workingData[actVector + nNewX * y] /= denomVec;
	}
	for(int actDouble = 0; actDouble < nx % nDouble; ++actDouble)
	{
            workingData[(y + 1) * nNewX - (nNewX - nVectors) - 1][actDouble] /= denominator;
	}
    }


    //Matrix multiplication
    
    double* res = (double*)malloc(sizeof(double) * nNewY * nNewY);
    for(int y = 0; y < nNewY; ++y)
    {
        for(int x = 0; x < nNewY; ++x)
	{
	    res[x + nNewY * y] = 0.;
	}
    }
    
    for(int run = 0; run < nNewX; run += nParallelOps)
    {
        #pragma omp parallel for schedule(static, 1)
	for(int y = 0; y < nNewY; y += nParallelOps)
	{
	    double4_t reusableCells[nParallelOps * nParallelOps];
	    std::vector<int> indices;
	    //copy values to local variable to reuse
	    for(int row = 0; row < nParallelOps; ++row)
	    {
		for(int col = 0; col < nParallelOps; ++col)
		{
		    reusableCells[col + row * nParallelOps] = workingData[run + col + (y + row) * nNewX];
		}
	    }
	    for(int x = y; x < nNewY; x += nParallelOps)
	    {
		double4_t partialRes[nParallelOps * nParallelOps];
		CalculateSum(x, workingData,nNewX, partialRes, run, reusableCells);
		//copy the results to res array
		for(int i = 0; i < nParallelOps; ++i)
		{
		    for(int j = 0; j < nParallelOps; ++j)
		    {
			double sum = 0.;
			for(int addParts = 0; addParts < nDouble; ++addParts)
			{
			    sum += partialRes[j + i * nParallelOps][addParts];
			}
			res[j + x + (y + i) * nNewY] += sum;
		    }
		}
	    }
	}
    }
    
    #pragma omp parallel for
    for(int y = 0; y < ny; ++y)
    {
	for(int x = 0; x < ny; ++x)
	{
	    result[x + y * ny] = res[x + y * nNewY];
	}
    }
    
    free(workingData);
    free(res);
}





