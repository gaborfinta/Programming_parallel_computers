
#include "cp.h"
#include <cstdlib>
#include <cmath>
#include <vector>

double MeanOfRow(const int& nx, const float* data, const int& y)
{
	double mean = data[y * nx];
	for(int i = 1; i < nx; ++i)
	{
		mean += (double)data[i + y * nx];
	}
	return mean/(double)nx;
}


void correlate(int ny, int nx, const float* data, float* result)
{
  double* normalized = (double*)malloc(sizeof(double) * nx * ny);
  //std::vector<double> normalized(nx * ny);

  //first normalization
  double mean = 0;
  for(int y = 0; y < ny; ++y)
  {
    mean = data[y * nx];
    for(int i = 1; i < nx; ++i)
    {
        mean += (double)data[i + y * nx];
    }
    mean = mean / (double)nx;
    //double mean = MeanOfRow(nx, data, y);
    for(int x = 0; x < nx; ++x)
    {
      normalized[x + y * nx] = (double)(data[x + y * nx]) - mean;
    }
  }

  
  //second normalization
  double denominator = 0.;
  for(int y = 0; y < ny; ++y)
  {
    denominator = 0.;
    for(int x = 0; x < nx; ++x)
    {
      denominator += pow(normalized[x + y * nx], 2);
    }
    denominator = sqrt(denominator);
    for(int x = 0; x < nx; ++x)
    {
      normalized[x + y * nx] = normalized[x + y * nx] / denominator;
    }
  }

  //Transpose
  /*std::vector<double> transposed(nx * ny);
  for(int y = 0; y < ny; ++y)
  {
    for(int x = 0; x < nx; ++x)
    {
      transposed[y + x * ny] = normalized[x + y * nx];
    }
    }*/
  //Matrix multiplication
  double sum = 0.;
   for(int y = 0; y < ny; ++y)
   {
    for(int x = y; x < ny; ++x)
    {
      sum = 0.;
      for(int k = 0; k < nx; ++k)
      {
	sum += normalized[k + y * nx] * normalized[k + x * nx]; //transposed[x + k * ny]; 
      }
      result[x + y * ny] = (float)sum;
    }
   }
   free(normalized);
}

