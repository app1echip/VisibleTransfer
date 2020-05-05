#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <math.h>

using namespace cv;
using namespace std;
//由于透视变换,每块二维码的size可以自己设置
int point_size = 4;
//找到所提取轮廓的中心点


/*
总体上分为三步
1.寻找出四个矩阵
2.确认每个矩阵的位置
3.枯燥且乏味的识别每个点
*/
int main(int argc, char** argv[])
{
	Mat src = imread("C:\\Users\\14769\\Desktop\\qrcodew.jpg");
	Mat src_gray;
	Mat src_new;
	Mat src_all = src.clone();
	Mat threshold_output;
	vector<vector<Point> > contours, contours2;
	vector<Vec4i> hierarchy;
	
	//预处理
	cvtColor(src, src_gray, CV_BGR2GRAY);
	blur(src_gray, src_gray, Size(3, 3)); //模糊，去除毛刺
	threshold(src_gray, threshold_output, 100, 255, THRESH_OTSU);
	//寻找轮廓 
	//第一个参数是输入图像 2值化的
	//第二个参数是内存存储器，FindContours找到的轮廓放到内存里面。
	//第三个参数是层级，**[Next, Previous, First_Child, Parent]** 的vector
	//第四个参数是类型，采用树结构
	//第五个参数是节点拟合模式，这里是全部寻找
	findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CHAIN_APPROX_NONE, Point(0, 0));
	//轮廓筛选
	int c = 0, ic = 0, area = 0;
	int parentIdx = -1;
	for (int i = 0; i < contours.size(); i++)
	{
		//hierarchy[i][2] != -1 表示不是最外面的轮廓
		if (hierarchy[i][2] != -1 && ic == 0)
		{
			parentIdx = i;
			ic++;
		}
		else if (hierarchy[i][2] != -1)
		{
			ic++;
		}
		//最外面的清0
		else if (hierarchy[i][2] == -1)
		{
			ic = 0;
			parentIdx = -1;
		}
		//找到定位点信息
		if (ic >= 2)
		{
			contours2.push_back(contours[parentIdx]);
			ic = 0;
			parentIdx = -1;
		}
	}
	//填充定位点
	for (int i = 0; i < contours2.size(); i++)drawContours(src_all, contours2, i, CV_RGB(0, 255, 0), -1);
	//连接定位点
	int maxx = 1E9 * -1, minx = 1E9;
	int maxsub = 1E9 * -1, minsub = 1E9;
	int lu[2] = { 0 }, ru[2] = { 0 }, ld[2] = { 0 }, rd[2] = { 0 };//left right up down
	Point point[5];
	int size_n;
	//分为四步判断四个点 左上角为所有点和最小 ####右上角 左下角为y-x(x-y?)最大或者最小#####  右下角为和最大
	//这一步很长很乱,懒得用函数,可以直接跳过,具体就是根据特点分块并且存起来用来透视变换
	for (int i = 0; i < contours2.size(); i++)
	{

		for (int j = 0;j < contours2[i].size();j++)
		{
			if (contours2[i][j].x + contours2[i][j].y < minx)
			{
				minx = contours2[i][j].x + contours2[i][j].y;
				lu[0] = i;
				lu[1] = j;
			}
			if (contours2[i][j].x + contours2[i][j].y > maxx)
			{
				maxx = contours2[i][j].x + contours2[i][j].y;
				rd[0] = i;
				rd[1] = j;
			}
			if (contours2[i][j].x - contours2[i][j].y > maxsub)
			{
				maxsub = contours2[i][j].x - contours2[i][j].y;
				ru[0] = i;
				ru[1] = j;
			}
			if (contours2[i][j].x - contours2[i][j].y < minsub)
			{
				minsub = contours2[i][j].x - contours2[i][j].y;
				ld[0] = i;
				ld[1] = j;
			}
		}
		//point[i] = Center_cal(contours2, i);
	}
	vector<Point2f> srcTri(4);
	vector<Point2f> dstTri(4);
	srcTri[0] = Point(contours2[lu[0]][lu[1]].x, contours2[lu[0]][lu[1]].y);
	srcTri[2] = Point(contours2[ru[0]][ru[1]].x, contours2[ru[0]][ru[1]].y);
	srcTri[1] = Point(contours2[ld[0]][ld[1]].x, contours2[ld[0]][ld[1]].y);
	srcTri[3] = Point(contours2[rd[0]][rd[1]].x, contours2[rd[0]][rd[1]].y);
	vector<Point2f>src_coners(4);
	vector<Point2f>dst_coners(4);
	dst_coners[0] = Point2f(0, 0);
	dst_coners[1] = Point2f(0, 48 * 8 * point_size);
	dst_coners[2] = Point2f(48 * 8 * point_size, 0);
	dst_coners[3] = Point2f(48 * 8 * point_size, 48 * 8 * point_size);
	Mat warpMatrix = getPerspectiveTransform(srcTri, dst_coners);
	warpPerspective(src, src_new, warpMatrix, src_new.size(), INTER_LINEAR, BORDER_CONSTANT);
	//到此透视变换完成,此时点(0,0)是二维码左上角,48*8*point_size 是二维码的总宽度,point_size即每个点的宽度
	//line(src, Point(contours2[lu[0]][lu[1]].x, contours2[lu[0]][lu[1]].y), Point(contours2[ru[0]][ru[1]].x, contours2[ru[0]][ru[1]].y), Scalar(0, 0, 255), 2);
	//line(src, Point(contours2[lu[0]][lu[1]].x, contours2[lu[0]][lu[1]].y), Point(contours2[rd[0]][rd[1]].x, contours2[rd[0]][rd[1]].y), Scalar(0, 0, 255), 2);
	// 接下去是枯燥的对点进行识别,我这里设置的单一一个点,为了方便,应该设置一个范围取平均值,同时我给每个点都更改了颜色,有利于判断,后期可以去掉
	int Data;
	int Base;
	for (int i = 0;i <= 7;i++)
	{
		for (int k = 1;k <= 46;k++)
		{
			Data = 0;
			Base = 1;
			for (int j = 0 + k * 8;j <= 7 + k * 8;j++)
			{
				if (src_new.at<Vec3b>((i + 0.5) * point_size, (j + 0.5) * point_size)[0] > 124)
					;else Data += Base;
				Base *= 2;
				//下面三句用来更改颜色
				src_new.at<Vec3b>((i + 0.5) * point_size, (j + 0.5) * point_size)[0] = 120;
				src_new.at<Vec3b>((i + 0.5) * point_size, (j + 0.5) * point_size)[1] = 120;
				src_new.at<Vec3b>((i + 0.5) * point_size, (j + 0.5) * point_size)[2] = 120;
			}
			cout << char(Data);
		}
	}
	//枯燥的对中间部分识别
	for (int i = 8;i <= 55 + 5 * 64;i++)
	{

		for (int k = 0;k <= 47;k++)
		{
			Data = 0;
			Base = 1;
			for (int j = 0 + k * 8;j <= 7 + k * 8;j++)
			{
				if (src_new.at<Vec3b>((i + 0.5) * point_size, (j + 0.5) * point_size)[0] > 124)
					;else Data += Base;
				Base *= 2;
				src_new.at<Vec3b>((i + 0.5) * point_size, (j + 0.5) * point_size)[0] = 120;
				src_new.at<Vec3b>((i + 0.5) * point_size, (j + 0.5) * point_size)[1] = 120;
				src_new.at<Vec3b>((i + 0.5) * point_size, (j + 0.5) * point_size)[2] = 120;
			}
			cout << char(Data);
		}

	}
	//乏味的对下端识别
	for (int i = 56 + 5 * 64;i <= 63 + 5 * 64;i++)
	{

		for (int k = 1;k < 47;k++)
		{
			Data = 0;
			Base = 1;
			for (int j = 0 + k * 8;j <= 7 + k * 8;j++)
			{
				if (src_new.at<Vec3b>((i + 0.5) * point_size, (j + 0.5) * point_size)[0] > 124)
					;else Data += Base;
				Base *= 2;
				src_new.at<Vec3b>((i + 0.5) * point_size, (j + 0.5) * point_size)[0] = 120;
				src_new.at<Vec3b>((i + 0.5) * point_size, (j + 0.5) * point_size)[1] = 120;
				src_new.at<Vec3b>((i + 0.5) * point_size, (j + 0.5) * point_size)[2] = 120;
			}
			cout << char(Data);
		}
	}
	cout << endl;
	//我的代码结束*/
	imshow("结果", src_new);
	imwrite("C:\\Users\\14769\\Desktop\\qrcoder.jpg", src_new);
	waitKey(0);
	return(0);
}