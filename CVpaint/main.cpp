#include <stdio.h>
#include <string>
#include <iostream>


#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

#include "CVPaint.h"
#include "paint.h"
#include "detection.h"

_CVPaintState CVPaintState = {
	true,
	Scalar(255, 255, 255), 
	PENCIL, 
	LARGE, 
	20,
};

static void help()
{
	printf("./CVpaint [video file, else it reads camera 0]\n\n");
}


int main(int argc, char** argv)
{
	VideoCapture cap;

#ifdef SAVE_VIDEO
	VideoWriter vwriter;
	if (vwriter.open("./vtest.avi", -1,
		1000 / 30,
		Size(cap.get(CV_CAP_PROP_FRAME_WIDTH), cap.get(CV_CAP_PROP_FRAME_HEIGHT)))) {
		cout << "Video has been successfully initialized." << endl;
	}
#endif // SAVE_VIDEO

	

	CommandLineParser parser(argc, argv, "{help h||}{@input||}");
	if (parser.has("help"))
	{
		help();
		return 0;
	}
	//string input = "C:\\Users\\Jinxiapu\\Pictures\\1.mp4";
	string input = parser.get<std::string>("@input");
	if (input.empty())
		cap.open(0);
	else
		cap.open(input);

	if (!cap.isOpened())
	{
		printf("\nCan not open camera or video file\n");
		return -1;
	}

	Mat tmp_frame;
	cap >> tmp_frame;
	if (tmp_frame.empty())
	{
		printf("can not read data from the video source\n");
		return -1;
	}


	Mat Paint(tmp_frame.size(), CV_8UC3, Scalar::all(0));
	namedWindow("output", 1);
	cvCreateTrackbar("max_Cr", "output", &max_Cr, 255);
	cvCreateTrackbar("max_Cb", "output", &max_Cb, 255);
	cvCreateTrackbar("min_Cr", "output", &min_Cr, 255);
	cvCreateTrackbar("min_Cb", "output", &min_Cb, 255);

	while(true)
	{
		cap >> tmp_frame;
		if (tmp_frame.empty())
			break;

		DetectResult dr;
		Mat output;
		tmp_frame.copyTo(output);

		bool detectresult = GetDetectResult(tmp_frame, dr);
		if (!detectresult) {
			putText(output, "failed.", Point(10, tmp_frame.rows - 10), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255));
		}
		
		if (CVPaintState.controlOrpaint) {
			ControlPanel(output, Paint, dr);
		}
		else {
			PaintPanel(output, Paint, dr);
			addWeighted(output, 1, Paint, 1, 0, output);
		}

		

		imshow("output", output);
#ifdef SAVE_VIDEO
		vwriter.write(output);
#endif // SAVE_VIDEO
		
		char keycode = (char)waitKey(FrameInterval);
		if (keycode == 27) // ESC
			break;
	}
	cap.release();
#ifdef SAVE_VIDEO
	vwriter.release();
#endif // SAVE_VIDEO
	return 0;
}