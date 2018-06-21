#include "detection.h"

#include <iostream>
inline double cosAngleABC(Point A, Point B, Point C);
inline int TwoPointsDistance(Point p1, Point p2);
Point ThreePointMean(Point A, Point B, Point C);
inline int TwoPointsDistance(Point p1, Point p2) {
	
	return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}
inline double cosAngleABC(Point A, Point B, Point C) {
	double as, bs, cs;
	as = pow(C.x - B.x, 2) + pow(C.y - B.y, 2);
	bs = pow(A.x - C.x, 2) + pow(C.y - A.y, 2);
	cs = pow(A.x - B.x, 2) + pow(A.y - B.y, 2);
	return (as + cs - bs) / (2 * sqrt(as*cs));
}
Point ThreePointMean(Point A, Point B, Point C) {
	return Point((A.x + B.x + C.x) / 3, (A.y + B.y + C.y) / 3);
}

bool IsHand(const vector<Point> & cnt, double area);
vector<Point> GetPenpointFromHand(Mat & frame, const vector<Point> & origincnt, const vector<Point> & cnt, DetectResult & dr);



bool IsHand(const vector<Point> & cnt, double area) {
	Moments mu = moments(cnt);
	Point mc = Point(mu.m10 / mu.m00, mu.m01 / mu.m00);
	double vertex_mean_distance = 0;
	int max_vertex_distancve = 0;
	for (size_t i = 0; i < cnt.size(); i++) {
		int d = TwoPointsDistance(mc, cnt[i]);
		max_vertex_distancve = max_vertex_distancve > d ? max_vertex_distancve : d;
		vertex_mean_distance += d;
	}
	vertex_mean_distance = vertex_mean_distance / cnt.size();
	double l = arcLength(cnt, true);
	return (max_vertex_distancve / vertex_mean_distance > 1.3)&& (area / l / l < 0.04);
}

vector<Point> GetPenpointFromHand(Mat & frame, const vector<Point> & origincnt, const vector<Point> & cnt, DetectResult & dr) {
	vector<Point> IPoints;
	if (cnt.size() > 3) {
		for (int i = 0; i < cnt.size(); i++) {
			int p1, p2, p3;
			p2 = i;
			p1 = (i == 0) ? (cnt.size() - 1) : (i - 1);
			p3 = (i == (cnt.size() - 1)) ? 0 : (i + 1);
			if (cosAngleABC(cnt[p1], cnt[p2], cnt[p3]) > 0.6
				&& pointPolygonTest(origincnt, ThreePointMean(cnt[p1], cnt[p2], cnt[p3]), false) > 0
				) {
				IPoints.push_back(cnt[i]);
			}
			line(frame, cnt[p1], cnt[p2], Scalar(0, 255, 0), 2);
		}
	}
	if (IPoints.size() == 2) {
		dr.penpoint = IPoints[0].y < IPoints[1].y ? IPoints[0] : IPoints[1];
		dr.ps = Move;
	}
	if (IPoints.size() == 5) {
		dr.ps = Ctrl;
	}
	return IPoints;
}

bool GetDetectResultByYCrCb(Mat & frame, DetectResult & dr) {
	GaussianBlur(frame, frame, Size(21, 21), 0);
	Mat YUVframe;
	cvtColor(frame, YUVframe, CV_BGR2YCrCb);
	vector<Mat> channels;
	split(YUVframe, channels);
	Mat SkinMask;
	threshold(channels[2], SkinMask, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	bitwise_not(SkinMask, SkinMask);
	imshow("skinmask", SkinMask);
	
	vector<vector<Point>> cnts;
	findContours(SkinMask, cnts, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	int max_2_cnt[2] = {-1};
	double max_2_cnt_area[2] = {0};
	for (int i = 0; i < cnts.size(); i++) {
		double area = contourArea(cnts[i]);
		if (area > frame.rows*frame.cols / 50 && area < frame.rows*frame.cols*0.5) {
			if (area > max_2_cnt_area[0]) {
				max_2_cnt[1] = max_2_cnt[0];
				max_2_cnt_area[1] = max_2_cnt_area[0];
				max_2_cnt[0] = i;
				max_2_cnt_area[0] = area;
			}
			else if (area > max_2_cnt_area[1]){
				max_2_cnt[1] = i;
				max_2_cnt_area[1] = area;
			}
		}
	}
	bool have_get_hand = false;
	for (int i = 0; i < 2; i++) {
		if (max_2_cnt[i] > 0 && max_2_cnt[i] < cnts.size()) {
			vector<Point> Ploy;
			approxPolyDP(cnts[max_2_cnt[i]], Ploy, 20, true);
			if (Ploy.size() < 4) {
				break;
			}
			if (IsHand(Ploy, max_2_cnt_area[i])) {
				have_get_hand = true;
				vector<Point> ir = GetPenpointFromHand(frame, cnts[max_2_cnt[i]], Ploy, dr);
				for (auto p : ir) {
					circle(frame, p, 5, Scalar(0, 0, 255), 2);
				}
			}
		}
	}
	
	imshow("point", frame);
	return have_get_hand;
}

bool GetDetectResultByOTSU(Mat & frame, DetectResult & dr)
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

