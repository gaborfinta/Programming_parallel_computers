#include "mf.h"
#include <vector>
#include <algorithm>

float median(int ny, int nx, int y, int x, int hy, int hx, const float* in)
{
    int rowStart = y - hy >= 0 ? y - hy : 0;
    int colStart = x - hx >= 0 ? x - hx : 0;
    int rowEnd = y + hy < ny ? y + hy + 1 : ny;
    int colEnd = x + hx < nx ? x + hx + 1 : nx;

    int windowSize = (rowEnd - rowStart) * (colEnd - colStart);
    
    std::vector<float> window(windowSize);

    if(windowSize == 1)
    {
	window[0] = in[x + y * nx];
    }
    else
    {
	int pos = 0;
	for(int actRow = rowStart; actRow < rowEnd; ++actRow)
	{
	    for(int actCol = colStart; actCol < colEnd; ++actCol)
	    {
		window[pos++] = in[actCol + actRow * nx];
	    }
	}
    }
    if(window.size() % 2 == 0)
    {
	std::nth_element(window.begin(), window.begin() + window.size() / 2 - 1, window.end());
        float firstNum = window[window.size() / 2 - 1];
	std::nth_element(window.begin(), window.begin() + window.size() / 2, window.end());
	float secondNum = window[window.size() / 2];
	return (firstNum + secondNum) / 2.;
    }
    else
    {
	std::nth_element(window.begin(), window.begin() + window.size() / 2, window.end());
	return window[window.size() / 2];
    }    
}

void mf(int ny, int nx, int hy, int hx, const float* in, float* out)
{
    
    for(int y = 0; y < ny; ++y)
    {
	#pragma omp parallel for
	for(int x = 0; x < nx; ++x)
	{
	    out[x + y * nx] = median(ny, nx, y, x, hy, hx, in);
	}
    }
}


