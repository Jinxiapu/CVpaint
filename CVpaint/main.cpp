#include "opencv2/opencv.hpp"
using namespace cv;
int main(int, char**)
{
	VideoCapture cap(0); // 使用默认摄像头
	if (!cap.isOpened())  // 检查摄像头是否成功打开
		return -1;
	Mat edges;
	namedWindow("edges", 1); //建立一个窗口
	for (;;)
	{
		Mat frame;
		cap >> frame; // 获得摄像头的一帧
					  //下面三句代码是利用canny算子进行边缘检测
		cvtColor(frame, edges, COLOR_BGR2GRAY);
		GaussianBlur(edges, edges, Size(7, 7), 1.5, 1.5);
		Canny(edges, edges, 0, 30, 3);
		imshow("edges", edges);  //在名字为“edges”的窗口显示
		if (waitKey(30) >= 0) break;
	}
	// the camera will be deinitialized automatically in VideoCapture destructor
	return 0;
}

