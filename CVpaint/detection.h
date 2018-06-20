#pragma once

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/video/background_segm.hpp"

#include "CVPaint.h"

/*
* Get the Penpoint position from one frame.
* @frame, current frame.
* @ps, the state of the penpoint.
* if cant find penpoint from current frame, return false.
*/

bool GetDetectResultByOTSU(Mat frame, DetectResult & dr);