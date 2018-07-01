#include "paint.h"

#ifdef DEBUG
#include <iostream>
#endif // DEBUG

const Scalar Palette[5] = { 
	Scalar(255, 0, 0), 
	Scalar(0, 255, 0), 
	Scalar(0, 0, 255), 
	Scalar(255, 255, 0), 
	Scalar(255, 255, 255) 
};

inline bool PointInCicle(Point p, Point c, int r) {
	return pow(p.x - c.x, 2) + pow(p.y - c.y, 2) < pow(r, 2);
}
inline bool PointInRec(Point p, Point rp1, Point rp2) {
	Point t1, t2;
	/*
	*  t1---------
	*  |         |
	*  |         |
	*  ---------t2
	*/
	t1.x = rp1.x > rp2.x ? rp2.x : rp1.x;
	t1.y = rp1.y > rp2.y ? rp2.y : rp1.y;
	t2.x = rp1.x > rp2.x ? rp1.x : rp2.x;
	t2.y = rp1.y > rp2.y ? rp1.y : rp2.y;
	return (p.x > t1.x) && (p.x < t2.x) && (p.y > t1.y) && (p.y < t2.y);
}


void DrawPaintPanel(Mat &output, int color_circle_radius, int color_circle_space);
int InPaintButtonArea(Point p, int color_circle_radius, int color_circle_space);
void DrawControlPanel(Mat &output);
int InControlButtonArea(Point p, int rows, int cols);

void DrawPaintPanel(Mat &output, int color_circle_radius, int color_circle_space) {
	for (int i = 0; i < 5; i++) {
		circle(output, Point(color_circle_space * (2 * i + 1), color_circle_space), color_circle_radius, Palette[i], FILLED);
	}
	line(output, Point(0, color_circle_space * 2), Point(output.cols, color_circle_space * 2), Scalar(0, 0, 0), 1);
	line(output, Point(color_circle_space * 10, 0), Point(color_circle_space * 10, color_circle_space * 2), Scalar(0, 0, 0), 1);
	putText(output, "ctrl", Point(color_circle_space * 10.5, color_circle_radius*1.8), FONT_HERSHEY_SIMPLEX,2, Scalar(255, 255, 0));
}
int InPaintButtonArea(Point p, int color_circle_radius, int color_circle_space) {
	if (p.x > color_circle_space * 10 && p.y < color_circle_space * 2) {
		return 20;
	}
	for (int i = 0; i < 5; i++) {
		if (PointInCicle(p, Point(color_circle_space * (2 * i + 1), color_circle_space), color_circle_radius)) {
			return 10 + i;
		}
	}
	return -1;
}
void DrawControlPanel(Mat &output) {
	putText(output, "PENCIL", Point(0, output.rows / 5 * 1.2), FONT_HERSHEY_SIMPLEX, 2, Scalar(255, 255, 255));
	for (int i = 0; i < 3; i++) {
		circle(output, Point(output.cols / 6 * (3+i), output.rows / 5 * 1), output.rows / (10+5*i), Scalar(255, 255, 255), FILLED, 4);
	}
	putText(output, "RUBBER", Point(0, output.rows / 5 * 2.7), FONT_HERSHEY_SIMPLEX, 2, Scalar(255, 255, 255));
	for (int i = 0; i < 3; i++) {
		circle(output, Point(output.cols / 6 * (3 + i), output.rows / 2), output.rows / (10 + 5 * i), Scalar(255, 255, 255), FILLED, 4);
	}

	rectangle(output, Point(output.cols / 12 * 1, output.rows/4*2.5), Point(output.cols /8  * 3, output.rows / 7 * 6), Scalar(255, 255, 255), FILLED);
	putText(output, "SAVE", Point(output.cols / 12 * 1, output.rows / 7 * 5.8), FONT_HERSHEY_SIMPLEX, 1.5, Scalar(0, 0, 255));
	rectangle(output, Point(output.cols / 6 * 3, output.rows / 4 * 2.5), Point(output.cols / 8 * 7, output.rows / 7 * 6), Scalar(255, 255, 255), FILLED);
	putText(output, "CLEAR", Point(output.cols / 2, output.rows / 7 * 5.8), FONT_HERSHEY_SIMPLEX, 1.5, Scalar(0, 0, 255));
}
int InControlButtonArea(Point p, int rows, int cols) {
	for (int i = 0; i < 3; i++) {
		if (PointInCicle(p, Point(cols / 6 * (3 + i), rows / 5 * 1), rows / (10 + 5 * i))) {
			return 10 + i;
		}
	}
	for (int i = 0; i < 3; i++) {
		if (PointInCicle(p, Point(cols / 6 * (3 + i), rows / 2), rows / (10 + 5 * i))) {
			return 20 + i;
		}
	}
	if (PointInRec(p, Point(cols / 12, rows / 4 * 2.5), Point(cols / 8 * 3, rows / 7 * 6))) {
		return 30; //SAVE
	}
	if (PointInRec(p, Point(cols / 2, rows / 4 * 2.5), Point(cols / 8 * 7, rows / 7 * 6))) {
		return 40; // CLEAR
	}
	return -1;
}
void PaintPanel(Mat &output, Mat & paint, DetectResult dr) {
	int color_circle_radius = output.rows / 20;
	int color_circle_space = output.cols / 12;
	int basic_size = output.rows / 60;

	DrawPaintPanel(output, color_circle_radius, color_circle_space);


	static size_t counter = CVPaintState.ConfirmTime;
	static int button_id = -1;

	if (dr.penpoint.y < color_circle_space * 2) {
		if (!counter) {
#ifdef DEBUG
			std::cout << "[Paint Ctrl Confirm]ButtonID: " << button_id << endl;
#endif // DEBUG
			if (button_id == 20) {
				CVPaintState.controlOrpaint = true;
			}
			if (button_id >= 10 && button_id < 20) {
				CVPaintState.CurrentPenpointColor = Palette[button_id - 10];
			}
		}
		int inresult = InPaintButtonArea(dr.penpoint, color_circle_radius, color_circle_space);
		if (inresult > 0) {
			if (button_id == inresult) {
				counter--;
			}
			else {
				button_id = inresult;
				counter = CVPaintState.ConfirmTime;
			}
		}
		else {
			button_id = -1;
			counter = CVPaintState.ConfirmTime;
		}
		vector<Point> cursor = { Point(dr.penpoint.x, dr.penpoint.y),
			Point(dr.penpoint.x + basic_size, dr.penpoint.y) ,
			Point(dr.penpoint.x, dr.penpoint.y + basic_size) };
		fillConvexPoly(output, cursor, Scalar(counter * 255 / CVPaintState.ConfirmTime, counter * 255 / CVPaintState.ConfirmTime, 255));
	}
	else {
		button_id = -1;
		counter = CVPaintState.ConfirmTime;
		switch (CVPaintState.CurrentPaintState)
		{
		case PENCIL:
			circle(paint, dr.penpoint, basic_size/2*CVPaintState.CurrentPaintSize, CVPaintState.CurrentPenpointColor, FILLED);
			break;
		case RUBBER:
			rectangle(paint,
				Point(dr.penpoint.x + basic_size * CVPaintState.CurrentPaintSize, dr.penpoint.y),
				Point(dr.penpoint.x, dr.penpoint.y + basic_size * CVPaintState.CurrentPaintSize),
				Scalar(0, 0, 0), FILLED);
			rectangle(output,
				Point(dr.penpoint.x + basic_size * CVPaintState.CurrentPaintSize, dr.penpoint.y),
				Point(dr.penpoint.x, dr.penpoint.y + basic_size * CVPaintState.CurrentPaintSize),
				Scalar(0, 0, 255));
			break;
		default:
			break;
		}
	}
	
}
void ControlPanel(Mat &output, Mat & paint, DetectResult dr) {
	DrawControlPanel(output);
	static size_t counter = CVPaintState.ConfirmTime;
	static int button_id = -1;

	if (!counter) {
#ifdef DEBUG
		std::cout << "[Control Ctrl Confirm]ButtonID: " << button_id << endl;
#endif // DEBUG
		PaintSize S[3] = {LARGE, MIDDLE,  SMALL};
		if (button_id >= 10 && button_id < 20) {
			CVPaintState.CurrentPaintState = PENCIL;
			CVPaintState.CurrentPaintSize = S[button_id - 10];
		}
		if (button_id >= 20 && button_id < 30) {
			CVPaintState.CurrentPaintState = RUBBER;
			CVPaintState.CurrentPaintSize = S[button_id - 20];
		}
		if (button_id == 30) {
			imwrite("./paint.jpg", paint);
		}
		if (button_id == 40) {
			paint.setTo(Scalar(0, 0, 0));
		}
		CVPaintState.controlOrpaint = false;
	}
	int inresult = InControlButtonArea(dr.penpoint, output.rows, output.cols);
	if (inresult > 0) {
		if (button_id == inresult) {
			counter--;
		}
		else {
			button_id = inresult;
			counter = CVPaintState.ConfirmTime;
		}
	}
	else {
		button_id = -1;
		counter = CVPaintState.ConfirmTime;
	}
	vector<Point> cursor = { Point(dr.penpoint.x, dr.penpoint.y),
		Point(dr.penpoint.x + output.rows / 30, dr.penpoint.y) ,
		Point(dr.penpoint.x, dr.penpoint.y + output.rows / 30) };
	fillConvexPoly(output, cursor, Scalar(counter * 255 / CVPaintState.ConfirmTime, counter * 255 / CVPaintState.ConfirmTime, 255));
}