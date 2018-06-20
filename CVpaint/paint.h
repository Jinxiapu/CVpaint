#pragma once

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

#include "CVPaint.h"

void ControlPanel(Mat &output, Mat & paint, DetectResult dr);
void PaintPanel(Mat &output, Mat & paint, DetectResult dr);