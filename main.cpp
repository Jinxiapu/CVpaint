#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/video/background_segm.hpp"
#include <stdio.h>
#include <string>
using namespace std;
using namespace cv;



enum PaintState
{
	Move,
	Draw,
	Ctrl
};

static void help()
{
	printf("./CVpaint [video file, else it reads camera 0]\n\n");
}

/*
* Get the Penpoint position from one frame.
* @frame, current frame.
* @ps, the state of the penpoint.
* if cant find penpoint from current frame, return false.
*/
bool getpenpoint(Mat frame, Point & penpoint, PaintState &ps);

int main(int argc, char** argv)
{
	VideoCapture cap;

	CommandLineParser parser(argc, argv, "{help h||}{@input||}");
	if (parser.has("help"))
	{
		help();
		return 0;
	}
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
	
	namedWindow("segmented", 1);
	
/*
	cv::createTrackbar("H_min", "segmented", &H_min, 180);
	cv::createTrackbar("H_max", "segmented", &H_max, 180);
	cv::createTrackbar("S_min", "segmented", &S_min, 255);
	cv::createTrackbar("S_max", "segmented", &S_max, 255);
*/
	Mat Paint(tmp_frame.size(), CV_8UC3, Scalar::all(0));
	Point penpoint;
	PaintState ps;

	for (;;)
	{
		cap >> tmp_frame;
		if (tmp_frame.empty())
			break;
		
		
		if (getpenpoint(tmp_frame, penpoint, ps) && ps == Draw) {
			circle(Paint, penpoint, 20, Scalar(0, 0, 255), FILLED);
		}
		
		
		Mat result;
		addWeighted(tmp_frame, 1, Paint, 1, 0, result);
		imshow("segmented", result);
		
		char keycode = (char)waitKey(30);
		if (keycode == 27) // ESC
			break;
	}
	return 0;
}



bool getpenpoint(Mat frame, Point & penpoint, PaintState &ps)
{
	
	int H_min = 38, H_max = 75, S_min = 200, S_max = 255, V_min = 0, V_max = 255;
	Mat hsv, mask;
	cvtColor(frame, hsv, COLOR_BGR2HSV);
	inRange(hsv, Scalar(H_min, S_min, V_min), Scalar(H_max, S_max, V_max), mask);

	ps = Draw;

	vector<vector<Point> > cnts;
	vector<Point > PossiblePenpoint;

	findContours(mask, cnts, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	for (size_t k = 0; k < cnts.size(); k++) {
		double area = contourArea(cnts[k]);

		Moments M = moments(cnts[k]);
		Point center = Point(M.m10 / M.m00, M.m01 / M.m00);

		if (area > 500) {
			PossiblePenpoint.push_back(center);
			//putText(mask, areanum, center, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 0, 0));
		}
	}
	if (PossiblePenpoint.empty()) {
		return false;
	}
	else {
		penpoint = PossiblePenpoint[0];
		return true;
	}
}