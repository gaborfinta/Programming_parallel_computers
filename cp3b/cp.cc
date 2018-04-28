
#include "cp.h"
#include <cstdlib>
#include <cmath>
#include <vector>
#include <numeric>
#include <limits>
#include "vector.h"

constexpr int nFloat = 8;
constexpr int nParallelOps = 10;

float MeanOfRow(const int& nx, const float8_t* workingData, const int& y, const int& nVectors, const int& nNewX)
{
    float8_t* partialSums = float8_alloc(nParallelOps);
    float8_t zeroVec = {0., 0., 0., 0., 0., 0., 0., 0.};
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

    float ret = 0.;
    for(int i = 0; i < nParallelOps; ++i)
    {
	for(int j = 0; j < nFloat; ++j)
	{
	    ret += partialSums[i][j];
	}
    }

    

    free(partialSums);
    return ret / (float)nx;
}

float CalculateDenominator(const int& y, const float8_t* workingData, const int& nVectors, const int& nNewX)
{
    float8_t denominators[nParallelOps];// = float8_alloc(nParallelOps);
    for(int i = 0; i < nParallelOps; ++i)
    {
	for(int j = 0; j < nFloat; ++j)
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

    float ret = 0.;
    for(int i = 0; i < nParallelOps; ++i)
    {
	for(int j = 0; j < nFloat; ++j)
	{
	    ret += denominators[i][j];
	}
    }
//    free(denominators);
    return sqrt(ret);
}

void CalculateSum(const int& x, const float8_t* workingData, const int& nNewX, float8_t* partialSums, const int& run, float8_t* reusableCells)
{
    float8_t zeroVec = {0., 0., 0., 0., 0., 0., 0., 0.};
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
    int nVectors = (nx + nFloat - 1) / nFloat;
    int nExtendedRow = (nVectors + nParallelOps - 1) / nParallelOps;
    int nNewX = nExtendedRow * nParallelOps;

    int nExtendedCol = (ny + nParallelOps - 1) / nParallelOps;
    int nNewY = nExtendedCol * nParallelOps;

    float8_t* workingData = float8_alloc(nNewY * nNewX);
    
    //copying data to vectors
    #pragma omp parallel for
    for(int y = 0; y < ny; ++y)
    {
        for(int x = 0; x < nNewX; ++x)
	{
		for(int actFloat = 0; actFloat < nFloat; ++actFloat)
		{
			int actCol = actFloat + x * nFloat;
	 	        workingData[x + nNewX * y][actFloat] = actCol < nx ? data[y * nx + actCol] : 0.;
		}
	}
    }
    float8_t zeroVec = {0., 0., 0., 0., 0., 0., 0., 0.};
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
        float mean = MeanOfRow(nx, workingData, y, nVectors, nNewX);
	float8_t meanVec = {mean, mean, mean, mean, mean, mean, mean, mean};
	
        for(int actVector = 0; actVector < nVectors - (nx % nFloat == 0 ? 0 : 1); ++actVector)
	{
            workingData[actVector + nNewX * y] -= meanVec;
	}
	for(int actFloat = 0; actFloat < nx % nFloat; ++actFloat)
	{
            workingData[(y + 1) * nNewX - (nNewX - nVectors) - 1][actFloat] -= mean;
	}
	
    }
    //second normalization
    #pragma omp parallel for
    for(int y = 0; y < ny; ++y)
    {
        float denominator = CalculateDenominator(y, workingData, nVectors, nNewX);
	float8_t denomVec = { denominator, denominator, denominator, denominator, denominator, denominator, denominator, denominator};
        for(int actVector = 0; actVector < nVectors - (nx % nFloat == 0 ? 0 : 1); ++actVector)
	{
            workingData[actVector + nNewX * y] /= denomVec;
	}
	for(int actFloat = 0; actFloat < nx % nFloat; ++actFloat)
	{
            workingData[(y + 1) * nNewX - (nNewX - nVectors) - 1][actFloat] /= denominator;
	}
    }


    //Matrix multiplication
    
    float* res = (float*)malloc(sizeof(float) * nNewY * nNewY);
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
	    float8_t reusableCells[nParallelOps * nParallelOps];
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
		float8_t partialRes[nParallelOps * nParallelOps];
		CalculateSum(x, workingData,nNewX, partialRes, run, reusableCells);
		//copy the results to res array
		for(int i = 0; i < nParallelOps; ++i)
		{
		    for(int j = 0; j < nParallelOps; ++j)
		    {
			float sum = 0.;
			for(int addParts = 0; addParts < nFloat; ++addParts)
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





