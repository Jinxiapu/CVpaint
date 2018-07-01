#include "detection.h"
#include <iostream>

int min_Cr = 137, min_Cb = 77;
int max_Cr = 173, max_Cb = 127;
int gaussian_blur_size = 13;

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



bool GetDetectResultBySkin(Mat & frame, DetectResult &dr) {
	GaussianBlur(frame, frame, Size(gaussian_blur_size, gaussian_blur_size), 0);
	Mat ycrcb_image;
	Mat skin_mask = Mat::zeros(frame.size(), CV_8UC1);
	cvtColor(frame, ycrcb_image, CV_BGR2YCrCb);
	inRange(ycrcb_image, Scalar(0, min_Cr, min_Cb), Scalar(255, max_Cr, max_Cb), skin_mask);

	
	vector<vector<Point>> cnts;
	findContours(skin_mask, cnts, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	
	for (int i = 0; i < cnts.size(); i++) {
		
		double area = contourArea(cnts[i]);
		if (area > frame.rows*frame.cols / 50 && area < frame.rows*frame.cols*0.5) {
			vector<vector<Point>> Ploys;
			//approxPolyDP(cnts[i], Ploys.at(0), 20, true);
			//drawContours(frame, Ploys, 0, Scalar(0, 0, 255), 2);
		}
	}
	
	imshow("frame", frame);
	/*Mat element = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
	morphologyEx(skin_mask, skin_mask, MORPH_DILATE, element);*/
	imshow("parameter", skin_mask);
	return false;
}


bool GetDetectResult(Mat & src, DetectResult & dr)//处理主函数
{
	Mat src_YCrCb;
	static Point tmp(0, 0);
	static int uncontinuenum = 0;

	//cvtColor(src, src_YCrCb, CV_RGB2YCrCb);                     // 转化到 YCRCB 空间处理

	//vector<Mat> channels;
	//split(src_YCrCb, channels);//分割通道
	//Mat target = channels[2];
	///*Mat target;
	//cvtColor(src,target,CV_RGB2GRAY);*/
	////		 	medianBlur(target,target,3);     //中值滤波法 
	//GaussianBlur(target, target, Size(5, 5), 0, 0);
	////Mat target=ellipse_detect(src);

	//threshold(target, target, 0, 255, CV_THRESH_OTSU);             // 二值化处理
	////															   //cout<<x<<endl;
	//															   //threshold(target, target, 135, 255,THRESH_BINARY);
																   //bitwise_not(target,target);
	GaussianBlur(src, src, Size(gaussian_blur_size, gaussian_blur_size), 0);
	Mat ycrcb_image;
	Mat target = Mat::zeros(src.size(), CV_8UC1);
	cvtColor(src, ycrcb_image, CV_BGR2YCrCb);
	inRange(ycrcb_image, Scalar(0, min_Cr, min_Cb), Scalar(255, max_Cr, max_Cb), target);

	imshow("tar", target);
	//Mat element = getStructuringElement(MORPH_RECT, Size(10, 10));   // 开运算去除噪点
	//morphologyEx(target, target, MORPH_CLOSE, element);


	vector<vector<Point>> contours;                                 // 找到所有的轮廓
	findContours(target, contours, CV_RETR_EXTERNAL, CHAIN_APPROX_NONE);

	double minarea = 100;//找到面积最大的2个轮廓
	double maxarea = src.rows*src.cols*0.8;
	double max = 0, secondmax = 0;//记录最大的两个区域的面积
	int handface_num[2] = { -1,-1 };
	for (int i = 0; i < contours.size(); i++)
	{
		double area = contourArea(contours[i]);
		if ((area <= minarea) || (area >= maxarea))
			continue;//删除大小区域噪声
		else
		{
			if (area>max)//
			{
				secondmax = max;//最大更新时，原最大成为第二大
				handface_num[1] = handface_num[0];
				max = area;
				handface_num[0] = i;

			}
			else if (area>secondmax)//更新第二大
			{
				secondmax = area;
				handface_num[1] = i;
			}
		}
	}

	//找到手的轮廓
	double maxab = 1.3;//特征值
	int hand_num = -1;
	for (int j = 0; j<2; j++)
	{
		int cont_num = handface_num[j];//取当前轮廓
		if (cont_num<0)
			continue;//取不到则跳过
		vector<Point> poly_cont;
		approxPolyDP(contours[cont_num], poly_cont, 10, true);//拟合多边形

		Moments mu = moments(poly_cont);//计算质心
		Point mc = Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);
		circle(src, mc, 5, Scalar(0, 0, 0), 2);//在图中画重心
		vector<Point>::const_iterator itpo = poly_cont.begin();
		double sum = 0;
		double maxdis = 0;
		while (itpo != (poly_cont.end() - 1))
		{
			Point ptmp = *itpo;   // 将当前迭代器所指向的点保存在pt2中
			double d = (mc.x - ptmp.x)*(mc.x - ptmp.x) + (mc.y - ptmp.y)*(mc.y - ptmp.y);
			d = sqrt(d);// 求当前点到重心的距离
			sum += d;  // 每次将距离都加在sum，以便求总距离
					   //求最大距离所对应的点，把最大距离保存在maxdis
			if (d > maxdis)
				maxdis = d;//记录最大距离
			++itpo;
		}
		double average = sum / (poly_cont.size());//求平均距离
		double ab = maxdis / average;//求比值
		if (ab>maxab)
		{
			maxab = ab;//去最大的比值区域，认为为手部
			hand_num = handface_num[j];
		}
	}
	vector<Point> fingerpoint;
	int finger_num = 0;
	if (hand_num >= 0)	//如果当前帧中找到手部则继续
	{
		vector<Point> hand_cont;
		approxPolyDP(contours[hand_num], hand_cont, 20, true);
		Moments mu = moments(hand_cont);//计算质心
		Point mc = Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);
		if (tmp.x>0 && tmp.y>0)
		{
			double distance = (tmp.x - mc.x)*(tmp.x - mc.x) + (tmp.y - mc.y)*(tmp.y - mc.y);
			distance = sqrt(distance);
			if (distance<30)
			{
				tmp.x = mc.x;
				tmp.y = mc.y;

			}
			else
			{
				uncontinuenum++;
				if (uncontinuenum>5)
				{
					tmp.x = mc.x;
					tmp.y = mc.y;
					uncontinuenum = 0;
				}
				return false;
			}

		}
		else
		{
			tmp.x = mc.x;
			tmp.y = mc.y;
		}

		if (hand_cont.size()>3)
		{
			vector<Point>::const_iterator itp = hand_cont.begin();
			while (itp != hand_cont.end())//设置迭代器对轮廓遍历
			{
				Point recent = *itp;//取当前点
				Point forward, backward;//定义前一个后一个点
				if (itp == (hand_cont.end() - 1))
					forward = *(hand_cont.begin());//自循环
				else
					forward = *(itp + 1);//取前一个点
				if (itp == hand_cont.begin())
					backward = *(hand_cont.end() - 1);//自循环
				else
					backward = *(itp - 1);//取后一个点
				++itp;
				line(src, recent, forward, Scalar(255), 2);//在图中画轮廓
				if (recent.y>mc.y)
					continue;//排除重心一下的点
				double a = (recent.x - forward.x)*(recent.x - forward.x) + (recent.y - forward.y)*(recent.y - forward.y);
				double b = (recent.x - backward.x)*(recent.x - backward.x) + (recent.y - backward.y)*(recent.y - backward.y);
				double c = (forward.x - backward.x)*(forward.x - backward.x) + (forward.y - backward.y)*(forward.y - backward.y);
				double angle = (a + b - c) / (2 * sqrt(a)*sqrt(b));//计算定点角度
				if (angle>0.6)//角度符合规定
				{
					Point averpoint = Point2f((recent.x + forward.x + backward.x) / 3, (recent.y + forward.y + backward.y) / 3);
					double flag = pointPolygonTest(contours[hand_num], averpoint, true);
					if (flag>0)//判断是凹点还是凸点
					{
						fingerpoint.push_back(recent);
						finger_num++;//凸点则认为是手指，并记录
					}
				}

			}

		}
	}
	for (int k = 0; k<finger_num; k++) //画出所识别到的手指点
	{
		circle(src, fingerpoint[k], 5, Scalar(0, 0, 255), 2);
	}

	imshow("frame", src);

	if (finger_num < 3 && finger_num > 1)//手指个数为2则认为是绘画状态
	{
		Point tmp(0, src.rows);
		for (auto p : fingerpoint) {
			if (p.y < tmp.y) {
				tmp = p;
			}
		}
		dr.penpoint = tmp;
		dr.ps = Move;//赋状态

		return true;
	}
	else if (finger_num == 5)//手指为5则认为是控制
	{
		dr.penpoint.x = 0;
		dr.penpoint.y = 0;
		dr.ps = Ctrl;
		return true;
	}
	else
		return false;

}

