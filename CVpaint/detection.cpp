#include "detection.h"


bool GetDetectResultByOTSU(Mat frame, DetectResult & dr)
{
	int g_size = 19;
	Mat grayframe, img_bw;
	cvtColor(frame, grayframe, COLOR_BGR2GRAY);
	GaussianBlur(grayframe, grayframe, Size(g_size, g_size), 0);
	threshold(grayframe, img_bw, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	bitwise_not(img_bw, img_bw);
	
	vector<vector<Point> > cnts;

	findContours(img_bw, cnts, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	double max_contour_area = 0;
	size_t finger_contour = -1;

	for (size_t k = 0; k < cnts.size(); k++) {
		double area = contourArea(cnts[k]);
		if (area > max_contour_area) {
			max_contour_area = area;
			finger_contour = k;
		}
	}

	Point tmp_point = cnts[finger_contour][0];
	for (size_t i = 1; i < cnts[finger_contour].size(); i++) {
		if (cnts[finger_contour][i].y < tmp_point.y) {
			tmp_point = cnts[finger_contour][i];
		}
	}
	dr.penpoint = tmp_point;
	dr.ps = Move;

	return true;
}

