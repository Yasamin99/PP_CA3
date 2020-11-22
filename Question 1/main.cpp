#include <iostream>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <sys/time.h>
//#include "ipp.h"
#include "x86intrin.h"

using namespace cv;
using namespace std;

int main()
{
	struct timeval start1, end1;
	struct timeval start2, end2;
	//Ipp64u time1, time2;

	cv::Mat img1 = cv::imread("CA03__Q1__Image__01.png", IMREAD_GRAYSCALE);
	cv::Mat img2 = cv::imread("CA03__Q1__Image__02.png", IMREAD_GRAYSCALE);

	unsigned int NCOLS = img1.cols;
	unsigned int NROWS = img1.rows;

	cv::Mat out_img  (img1.rows, img1.cols, CV_8U) ;
	cv::Mat out_img2 (img1.rows, img1.cols, CV_8U) ;

	unsigned char *image1;
	unsigned char *image2;
	unsigned char *out_image;

	image1  = (unsigned char *) img1.data;
	image2  = (unsigned char *) img2.data;
	out_image = (unsigned char *) out_img.data;

	gettimeofday(&start1, NULL);
	for (int row = 0; row < NROWS; row++)
		for (int col = 0; col < NCOLS; col++)
		{
			int index = row * NCOLS + col;
			char diff = *(image1 + index) - *(image2 + index);
			if (diff < 0)
				diff = -diff;
			*(out_image + index) = diff;
			//cout << "a" << diff <<" b" << *(image1 + row * NCOLS + col) - *(image2 + row * NCOLS + col) << endl;
			/*if (diff >= 0)
				*(out_image + row * NCOLS + col) = diff;
			else
				*(out_image + row * NCOLS + col) = -diff;*/
		}
	gettimeofday(&end1, NULL);
	long seconds1 = (end1.tv_sec - start1.tv_sec);
	long micros1 = ((seconds1 * 1000000) + end1.tv_usec) - (start1.tv_usec);
	//time1 = end - start;

	__m128i *pSrc1;
	__m128i *pSrc2;
	__m128i *pRes;
	__m128i m1, m2, m3, m4, m5, m6;

	pSrc1 = (__m128i *) img1.data;
	pSrc2 = (__m128i *) img2.data;
	pRes = (__m128i *) out_img2.data;

	gettimeofday(&start2, NULL);
	m4 = _mm_set1_epi8 ((unsigned char) 0X00);
	for (int i = 0; i < NROWS; i++)
		for (int j = 0; j < NCOLS / 16; j++)
		{
			m1 = _mm_loadu_si128(pSrc1 + i * NCOLS/16 + j) ;
			m2 = _mm_loadu_si128(pSrc2 + i * NCOLS/16 + j) ;
			m3 = _mm_sub_epi8 (m1, m2);
			m5 = _mm_sub_epi8 (m4, m3);
			m6 = _mm_max_epi8 (m3, m5);
			_mm_storeu_si128 (pRes + i * NCOLS/16 + j, m6);
		}
	gettimeofday(&end2, NULL);
	long seconds2 = (end2.tv_sec - start2.tv_sec);
	long micros2 = ((seconds2 * 1000000) + end2.tv_usec) - (start2.tv_usec);
	//time2 = end - start;

	float speedup = (float) (seconds1*1000000 + micros1) / (float) (seconds2*1000000 + micros2);

	//cv::namedWindow("image1", cv::WINDOW_AUTOSIZE );
	//cv::imshow( "image1", img1 ); 				
	//cv::namedWindow("image2", cv::WINDOW_AUTOSIZE );
	//cv::imshow( "image2", img2 );
	cv::namedWindow( "Serial output", cv::WINDOW_AUTOSIZE );
	cv::imshow( "Serial output", out_img ); 
	cv::namedWindow( "Parallel output", cv::WINDOW_AUTOSIZE ); 
	cv::imshow( "Parallel output", out_img2 ); 				                       					

	//printf ("Serial Run time = %d \n", (Ipp32s) time1);
	//printf ("Parallel Run time = %d \n", (Ipp32s) time2);
	printf("Serial Run time = %ld seconds and %ld micro seconds\n\n",seconds1, micros1);
	printf("Parallel Run time = %ld seconds and %ld micro seconds\n\n",seconds2, micros2);
	printf ("Speedup = %4.2f\n", speedup);

	waitKey(0);	
	
    return 0;
}