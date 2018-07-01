#pragma once
/*
* Global Struct and Enum.
* Global variables.
*/


#include "opencv2/imgproc.hpp"

#define DEBUG
//#define SAVE_VIDEO

using namespace std;
using namespace cv;

enum PenPointState
{
	Move = 0,
	Ctrl
};

enum PaintState {
	PENCIL,
	RUBBER
};

enum PaintSize {
	SMALL=1,
	MIDDLE,
	LARGE
};
const string PenPointState_str[2] = { "Move", "Ctrl" };

struct DetectResult
{
	PenPointState ps;
	Point penpoint;
};

const int FrameInterval = 30;

struct _CVPaintState {
	// true means in control state.
	// flase means in paint state.
	bool controlOrpaint;
	Scalar CurrentPenpointColor;
	PaintState CurrentPaintState;
	PaintSize CurrentPaintSize;
	const size_t ConfirmTime;
};
extern _CVPaintState CVPaintState;

extern int min_Cr, min_Cb;
extern int max_Cr, max_Cb;