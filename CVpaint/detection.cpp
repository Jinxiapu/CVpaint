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


bool GetDetectResult(Mat & src, DetectResult & dr)//����������
{
	Mat src_YCrCb;
	static Point tmp(0, 0);
	static int uncontinuenum = 0;

	//cvtColor(src, src_YCrCb, CV_RGB2YCrCb);                     // ת���� YCRCB �ռ䴦��

	//vector<Mat> channels;
	//split(src_YCrCb, channels);//�ָ�ͨ��
	//Mat target = channels[2];
	///*Mat target;
	//cvtColor(src,target,CV_RGB2GRAY);*/
	////		 	medianBlur(target,target,3);     //��ֵ�˲��� 
	//GaussianBlur(target, target, Size(5, 5), 0, 0);
	////Mat target=ellipse_detect(src);

	//threshold(target, target, 0, 255, CV_THRESH_OTSU);             // ��ֵ������
	////															   //cout<<x<<endl;
	//															   //threshold(target, target, 135, 255,THRESH_BINARY);
																   //bitwise_not(target,target);
	GaussianBlur(src, src, Size(gaussian_blur_size, gaussian_blur_size), 0);
	Mat ycrcb_image;
	Mat target = Mat::zeros(src.size(), CV_8UC1);
	cvtColor(src, ycrcb_image, CV_BGR2YCrCb);
	inRange(ycrcb_image, Scalar(0, min_Cr, min_Cb), Scalar(255, max_Cr, max_Cb), target);

	imshow("tar", target);
	//Mat element = getStructuringElement(MORPH_RECT, Size(10, 10));   // ������ȥ�����
	//morphologyEx(target, target, MORPH_CLOSE, element);


	vector<vector<Point>> contours;                                 // �ҵ����е�����
	findContours(target, contours, CV_RETR_EXTERNAL, CHAIN_APPROX_NONE);

	double minarea = 100;//�ҵ��������2������
	double maxarea = src.rows*src.cols*0.8;
	double max = 0, secondmax = 0;//��¼����������������
	int handface_num[2] = { -1,-1 };
	for (int i = 0; i < contours.size(); i++)
	{
		double area = contourArea(contours[i]);
		if ((area <= minarea) || (area >= maxarea))
			continue;//ɾ����С��������
		else
		{
			if (area>max)//
			{
				secondmax = max;//������ʱ��ԭ����Ϊ�ڶ���
				handface_num[1] = handface_num[0];
				max = area;
				handface_num[0] = i;

			}
			else if (area>secondmax)//���µڶ���
			{
				secondmax = area;
				handface_num[1] = i;
			}
		}
	}

	//�ҵ��ֵ�����
	double maxab = 1.3;//����ֵ
	int hand_num = -1;
	for (int j = 0; j<2; j++)
	{
		int cont_num = handface_num[j];//ȡ��ǰ����
		if (cont_num<0)
			continue;//ȡ����������
		vector<Point> poly_cont;
		approxPolyDP(contours[cont_num], poly_cont, 10, true);//��϶����

		Moments mu = moments(poly_cont);//��������
		Point mc = Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);
		circle(src, mc, 5, Scalar(0, 0, 0), 2);//��ͼ�л�����
		vector<Point>::const_iterator itpo = poly_cont.begin();
		double sum = 0;
		double maxdis = 0;
		while (itpo != (poly_cont.end() - 1))
		{
			Point ptmp = *itpo;   // ����ǰ��������ָ��ĵ㱣����pt2��
			double d = (mc.x - ptmp.x)*(mc.x - ptmp.x) + (mc.y - ptmp.y)*(mc.y - ptmp.y);
			d = sqrt(d);// ��ǰ�㵽���ĵľ���
			sum += d;  // ÿ�ν����붼����sum���Ա����ܾ���
					   //������������Ӧ�ĵ㣬�������뱣����maxdis
			if (d > maxdis)
				maxdis = d;//��¼������
			++itpo;
		}
		double average = sum / (poly_cont.size());//��ƽ������
		double ab = maxdis / average;//���ֵ
		if (ab>maxab)
		{
			maxab = ab;//ȥ���ı�ֵ������ΪΪ�ֲ�
			hand_num = handface_num[j];
		}
	}
	vector<Point> fingerpoint;
	int finger_num = 0;
	if (hand_num >= 0)	//�����ǰ֡���ҵ��ֲ������
	{
		vector<Point> hand_cont;
		approxPolyDP(contours[hand_num], hand_cont, 20, true);
		Moments mu = moments(hand_cont);//��������
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
			while (itp != hand_cont.end())//���õ���������������
			{
				Point recent = *itp;//ȡ��ǰ��
				Point forward, backward;//����ǰһ����һ����
				if (itp == (hand_cont.end() - 1))
					forward = *(hand_cont.begin());//��ѭ��
				else
					forward = *(itp + 1);//ȡǰһ����
				if (itp == hand_cont.begin())
					backward = *(hand_cont.end() - 1);//��ѭ��
				else
					backward = *(itp - 1);//ȡ��һ����
				++itp;
				line(src, recent, forward, Scalar(255), 2);//��ͼ�л�����
				if (recent.y>mc.y)
					continue;//�ų�����һ�µĵ�
				double a = (recent.x - forward.x)*(recent.x - forward.x) + (recent.y - forward.y)*(recent.y - forward.y);
				double b = (recent.x - backward.x)*(recent.x - backward.x) + (recent.y - backward.y)*(recent.y - backward.y);
				double c = (forward.x - backward.x)*(forward.x - backward.x) + (forward.y - backward.y)*(forward.y - backward.y);
				double angle = (a + b - c) / (2 * sqrt(a)*sqrt(b));//���㶨��Ƕ�
				if (angle>0.6)//�Ƕȷ��Ϲ涨
				{
					Point averpoint = Point2f((recent.x + forward.x + backward.x) / 3, (recent.y + forward.y + backward.y) / 3);
					double flag = pointPolygonTest(contours[hand_num], averpoint, true);
					if (flag>0)//�ж��ǰ��㻹��͹��
					{
						fingerpoint.push_back(recent);
						finger_num++;//͹������Ϊ����ָ������¼
					}
				}

			}

		}
	}
	for (int k = 0; k<finger_num; k++) //������ʶ�𵽵���ָ��
	{
		circle(src, fingerpoint[k], 5, Scalar(0, 0, 255), 2);
	}

	imshow("frame", src);

	if (finger_num < 3 && finger_num > 1)//��ָ����Ϊ2����Ϊ�ǻ滭״̬
	{
		Point tmp(0, src.rows);
		for (auto p : fingerpoint) {
			if (p.y < tmp.y) {
				tmp = p;
			}
		}
		dr.penpoint = tmp;
		dr.ps = Move;//��״̬

		return true;
	}
	else if (finger_num == 5)//��ָΪ5����Ϊ�ǿ���
	{
		dr.penpoint.x = 0;
		dr.penpoint.y = 0;
		dr.ps = Ctrl;
		return true;
	}
	else
		return false;

}

