#include <fstream>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

int main()
{
	int code_size = 80;
	Mat mat(code_size + 14, code_size + 14, CV_8UC3, Scalar(255, 255, 255));
	for (int i = 0; i < 7; ++i)
		for (int j = 0; j < 7; ++j)
			mat.at<Vec3b>(i, j) = mat.at<Vec3b>(i, code_size + 13 - j) = mat.at<Vec3b>(code_size + 13 - i, j) = Vec3b(0, 0, 0);
	for (int i = 1; i < 6; ++i)
		for (int j = 1; j < 6; ++j)
			mat.at<Vec3b>(i, j) = mat.at<Vec3b>(i, code_size + 13 - j) = mat.at<Vec3b>(code_size + 13 - i, j) = Vec3b(255, 255, 255);
	for (int i = 2; i < 5; ++i)
		for (int j = 2; j < 5; ++j)
			mat.at<Vec3b>(i, j) = mat.at<Vec3b>(i, code_size + 13 - j) = mat.at<Vec3b>(code_size + 13 - i, j) = Vec3b(0, 0, 0);

	ifstream fin("file", ios::binary);
	int size = 0;
	while (fin.good())
	{
		char byte;
		fin.read(&byte, 1);
		for (int i = 0; i < 8; ++i)
		{
			bool is = (byte >> i) & 0x01;
			int all = 8 * size + i;
			int z = all % 3;
			int y = ((all - z) / 3) % code_size;
			int x = all / (3 * code_size);
			mat.at<Vec3b>(x + 7, y + 7).val[z] = is * 255;
			printf("bit %d: [%d,%d]%d %d\n", all, x, y, z, is);
		}
		size++;
	}

	imshow("img", mat);
	waitKey(0);
	imwrite("img.png", mat);
	return 0;
}
