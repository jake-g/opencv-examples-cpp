#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/flann/miniflann.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/photo/photo.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/ml/ml.hpp"
#include "opencv2/highgui/highgui.hpp"
//#include <opencv2/contrib/contrib.hpp>
#include "opencv2/core/core_c.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

using namespace cv;
using namespace std;

#define BRIGHTNESS_FACTOR 75


int main(int argc, char** argv) {
	// Video Processing

	//VideoCapture cap("path/to/the/video.avi"); // open the video file for reading

	VideoCapture cap(0); // open the video camera no.0

	if(!cap.isOpened()) {
		cout << "Cannot open the camera" << endl;
		return -1;
	}

	// cap.set(CV_CAP_PROP_POS_MSEC, 300); // start the video at 300ms

	/*
	// change the property of the VideoCapture object
	bool VideoCapture::set(int propId, double value)

	int propID:
	CV_CAP_PROP_POS_MSEC - current position of the video in milliseconds
	CV_CAP_PROP_POS_FRAMES - current position of the video in frames
	CV_CAP_PROP_FRAME_WIDTH - width of the frame of the video stream
	CV_CAP_PROP_FRAME_HEIGHT - height of the frame of the video stream
	CV_CAP_PROP_FPS - frame rate (frames per second)
	CV_CAP_PROP_FOURCC - four character code  of codec

	double value - This is the new value you are going to assign to the property, specified by the propID

	// get the property of the VideoCapture object
	doubleVideoCapture::get(int propId)
	*/

	double fps =cap.get(CV_CAP_PROP_FPS); // get the frames per seconds of the video
	cout << "Frame per seconds: " << fps << endl;

	double dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH); // get the frame width
	double dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT); // get the frame height
	cout << "Frame Size = " << dWidth << " x " << dHeight << endl;

	Size frameSize(static_cast<int>(dWidth), static_cast<int>(dHeight)); // define the size of the output Window

	// initialize the VideoWriter obejct
//	VideoWriter oVideoWriter("~/Desktop/myVideo.mp4", CV_FOURCC('M', 'P', '4', 'V'), 10, frameSize, true);

	/*
	VideoWriter::VideoWriter(const string& filename, int fourcc, double fps, Size frameSize, bool isColor=true)
	This is the constructor of the VideoWriter class. It initializes the object with following parameters
	const string& filename - Specify the name and the location of the output file. The video stream is written into this file
	int fourcc - specify the 4 character code for the codec which is used to compress the video. Your computer may not be supported some codecs. So, if you fail to save the video, please try other codecs. Here are some popular codecs.
	CV_FOURCC('D', 'I', 'V', '3') for DivX MPEG-4 codec
	CV_FOURCC('M', 'P', '4', '2') for MPEG-4 codec
	CV_FOURCC('D', 'I', 'V', 'X') for DivX codec
	CV_FOURCC('P','I','M','1') for MPEG-1 codec
	CV_FOURCC('I', '2', '6', '3') for ITU H.263 codec
	CV_FOURCC('M', 'P', 'E', 'G') for MPEG-1 codec
	Complete list of codec can be found here
	Here I have used CV_FOURCC('P', 'I', 'M', '1') as the four character code of codec to compress the video. If the output file cannot be opened, you can try different four character code of codecs.
	For Windows users, it is possible to use -1 instead of the above codecs in order to choose compression method and additional compression parameters from a dialog box. It is a best method for Microsoft Windows users.

	double fps - frames per seconds of the video stream. I have used 20. You can try different values. But the codec should support the fps value. So, use an appropriate value.
	Size frameSize - Size object which specify the width and the height of each frame of the video stream.
	bool isColor - If you want to save a color video, pass the value as true. Otherwise false. Remember codec should support whatever value, you pass. In the above example, you have to give true as the 5th argument. Otherwise it will not work.
	*/

//	if(!oVideoWriter.isOpened()) { // if not initialize the VideoWriter successfully, exit the program
//		cout << "ERROR: Failed to write to the video" << endl;
//		return -1;
//	}

	namedWindow("Control", CV_WINDOW_AUTOSIZE); // Create a window called "Control"

	// set the boundry of the threashold
	int iLowH = 60;
	int iHighH = 255;

	int iLowS = 60;
	int iHighS = 255;

	int iLowV = 60;
	int iHighV = 255;



	int iLastX = -1;
	int iLastY = -1;

	namedWindow("MyVideo", CV_WINDOW_AUTOSIZE); //create a window called "MyVideo"
	// namedWindow("Brightness Video", CV_WINDOW_AUTOSIZE);

	Mat imgTmp;
	cap.read(imgTmp);

	Mat imgLines = Mat::zeros(imgTmp.size(), CV_8UC3);

	while(1) {
		Mat frame;
		bool bSuccess = cap.read(frame); // read a new frame from video

		if(!bSuccess) { // if not success, break loop
			cout << "Cannot read the frame from video file" << endl;
			break;
		}

		// Image Processing

		// change the brightness of the video
		// Mat bright_frame = frame + Scalar(BRIGHTNESS_FACTOR, BRIGHTNESS_FACTOR, BRIGHTNESS_FACTOR);

		// Object detection
		Mat imgHSV;

		cvtColor(frame, imgHSV, COLOR_BGR2HSV); // Convert the captered frame from BGR to HSV

		Mat imgThresholded;

		inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); // Threshold the image

		// morphological opening (removes small objects from the foreground)
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5,5)));
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5,5)));

		// morphological closing (removes small holes from the foreground)
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5,5)));
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5,5)));

		// Calculate the moments of the thresholded image
		Moments oMoments = moments(imgThresholded);

		double dM01 = oMoments.m01;
		double dM10 = oMoments.m10;
		double dArea = oMoments.m00;


		//Mat imgLines;
		// if the area <= 10000, I consider that the there are
		// no object in the image and it's because of the noise,
		// the area is not zero
		if(dArea > 10000) {
			// calculate the position of the ball
			int posX = dM10 / dArea;
			int posY = dM01 / dArea;

			if(iLastX >= 0 && iLastY >= 0 && posX >= 0 && posY >= 0) {
				// Draw a red line from the previous point to the current point
				line(imgLines, Point(posX, posY), Point(iLastX, iLastY), Scalar(0, 0, 255), 2);
			}

		}

		imshow("Thresholded Image", imgThresholded);

		frame = frame + imgLines;
		imshow("MyVideo", frame); // show the frame in "MyVideo" window
		//imshow("Brightness Video", bright_frame);

		//oVideoWriter.write(frame); // write the frame into the file
		if(waitKey(30) == 27) { // wait for "esc" key press for 30 ms.
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}

	return 0;
}
