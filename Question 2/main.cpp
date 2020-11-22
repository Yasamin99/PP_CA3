//#include <opencv4/opencv2/opencv.hpp>
//#include <opencv4/opencv2/highgui.hpp>
//#include <opencv4/opencv2/imgproc/imgproc.hpp>
//#include <opencv4/opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include "x86intrin.h"
#include <math.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include <sys/time.h>
#include <bits/stdc++.h>
#include <stdio.h>

using std::cout;
using std::endl;

int main()
{
    timeval startTime, endTime;
    double serialTime, parallelTime;

    cv::Mat img1 = cv::imread("CA03__Q2__Image__01.png", cv::IMREAD_GRAYSCALE);
    cv::Mat img2 = cv::imread("CA03__Q2__Image__02.png", cv::IMREAD_GRAYSCALE);

    unsigned int NCOLS = img1.cols;
	unsigned int NROWS = img1.rows;

    unsigned int img2Rows = img2.rows;
    unsigned int img2Columns = img2.cols;

	cv::Mat outputSerial(NROWS, NCOLS, CV_8U);
    cv::Mat outputParallel(NROWS, NCOLS, CV_8U);

    unsigned char *img1Pointer;
    unsigned char *img2Pointer;
	unsigned char *outputSerialPointer;
    unsigned char *outputParallelPointer;

	img1Pointer = (unsigned char *) img1.data;
    img2Pointer = (unsigned char *) img2.data;
	outputSerialPointer = (unsigned char *) outputSerial.data;

    //Serial Implementation
    gettimeofday(&startTime, NULL);
    for (int row = 0; row < NROWS; row++)
		for (int col = 0; col < NCOLS; col++)
			if (row < img2Rows && col < img2Columns)
            {
                unsigned char pixel1 = *(img1Pointer + row * NCOLS + col);
                unsigned char pixel2 = *(img2Pointer + row * img2Columns + col) / 2;
                unsigned char addResult = (pixel1 > 0xFF - pixel2) ? 0xFF : pixel1 + pixel2;
                *(outputSerialPointer + row * NCOLS + col) = addResult;
            }
            else
                *(outputSerialPointer + row * NCOLS + col) = *(img1Pointer + row * NCOLS + col);

    gettimeofday(&endTime, NULL); 
    serialTime = (endTime.tv_sec - startTime.tv_sec) * 1e6; 
    serialTime = (serialTime + (endTime.tv_usec - startTime.tv_usec)) * 1e-6;

    //Parallel Implementation
    __m128i *pSrc1;
    __m128i *pSrc2;
	__m128i *pRes;
	__m128i m1, m2, m3, m4;

	pSrc1 = (__m128i *) img1.data;
    pSrc2 = (__m128i *) img2.data;
	pRes = (__m128i *) outputParallel.data;

    gettimeofday(&startTime, NULL);
    m4 = _mm_set1_epi8 ((unsigned char) 0X7F);
	for (int i = 0; i < NROWS; i++)
    {
		for (int j = 0; j < NCOLS / 16; j += 1)
		{
			m1 = _mm_loadu_si128(pSrc1 + i * NCOLS/16 + j);
            if(i < img2Rows && j < img2Columns/16)
            {
                m2 = _mm_loadu_si128(pSrc2 + i * img2Columns/16 + j);
                m2 = _mm_srli_epi16(m2, 1);
                m3 = _mm_and_si128(m2, m4);
                m1 = _mm_adds_epu8(m1, m3);
            }
			_mm_storeu_si128 (pRes + i * NCOLS/16 + j, m1);
		}
    }
    gettimeofday(&endTime, NULL); 
    parallelTime = (endTime.tv_sec - startTime.tv_sec) * 1e6; 
    parallelTime = (parallelTime + (endTime.tv_usec - startTime.tv_usec)) * 1e-6;
    cv::namedWindow("serial output", cv::WINDOW_AUTOSIZE);
    cv::imshow("serial output", outputSerial);
    cv::waitKey(0);
    cv::namedWindow("parallel output", cv::WINDOW_AUTOSIZE);
    cv::imshow("parallel output", outputParallel);
    cv::waitKey(0);

    cout << "Serial Time: " << std::fixed << serialTime << std::setprecision(6) << " sec" << endl; 
    cout << "Parallel Time: " << std::fixed << parallelTime << std::setprecision(6) << " sec" << endl; 
    cout << "SPEED UP: " << serialTime / parallelTime << endl;

    return 0;
}
