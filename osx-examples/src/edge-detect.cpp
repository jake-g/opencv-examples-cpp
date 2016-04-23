#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
using namespace cv;

int low_thresh = 100;
int high_thresh = 200;
int kSize = 5; // kernel size

int main(int, char**)
{
	namedWindow( "Edges", CV_WINDOW_NORMAL );
	CvCapture* capture = cvCaptureFromCAM(-1);

	cv::Mat frame; cv::Mat out; cv::Mat gray;

	while(1) {
		frame = cvQueryFrame( capture );

		GaussianBlur( frame, out, Size(kSize, kSize), 0, 0 );
		cvtColor( frame , gray, CV_BGR2GRAY, 3 ); // produces a one-channel image (CV_8UC1)
		Canny( gray, out, low_thresh, high_thresh); // the result goes to out2 again,but since it is still one channel it is fine

		if( !frame.data ) break;
		imshow( "Edges", out );

		if( cvWaitKey(33) == 27 ) break;
	}
	return 0;
}
