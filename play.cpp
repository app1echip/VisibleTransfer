#include <fstream>
#include <vector>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;
class Code4D
{
	int rows, cols, channels;
	Mat _t;
	unsigned bits;
	vector<Mat> frame;

public:
	Code4D(int _rows, int _cols) : _t(_rows, _cols, CV_8UC3, Scalar(255, 255, 255))
	{
		bits = 0;
		cols = _cols;
		rows = _rows;
		for (int i = 0; i < 7; ++i)
			for (int j = 0; j < 7; ++j)
				_t.at<Vec3b>(i, j) = _t.at<Vec3b>(i, cols - 7 + j) = _t.at<Vec3b>(rows - 7 + i, j) = Vec3b(0, 0, 0);
		for (int i = 1; i < 6; ++i)
			for (int j = 1; j < 6; ++j)
				_t.at<Vec3b>(i, j) = _t.at<Vec3b>(i, cols - 7 + j) = _t.at<Vec3b>(rows - 7 + i, j) = Vec3b(255, 255, 255);
		for (int i = 2; i < 5; ++i)
			for (int j = 2; j < 5; ++j)
				_t.at<Vec3b>(i, j) = _t.at<Vec3b>(i, cols - 7 + j) = _t.at<Vec3b>(rows - 7 + i, j) = Vec3b(0, 0, 0);
		frame.push_back(_t.clone());
	}
	void pushByte(unsigned char c)
	{
		for (int i = 0; i < 8; ++i)
		{
			bool is = (c >> i) & 0x01;

			int _rgb = bits % 3;
			unsigned _frame = bits / ((cols * rows - 3 * 8 * 8) * 3);
			if (_frame + 1 > frame.size())
				frame.push_back(_t.clone());

			unsigned zone_a = (cols - 2 * 8) * 8 * 3;
			unsigned zone_b = (rows - 2 * 8) * cols * 3;

			unsigned _c, _r;
			unsigned bit_in_frame = bits % ((cols * rows - 3 * 8 * 8) * 3);
			if (bit_in_frame < zone_a)
				_c = (bit_in_frame / 3) % (cols - 2 * 8) + 8,
				_r = (bit_in_frame / 3) / (cols - 2 * 8);
			else if (bit_in_frame < zone_a + zone_b)
				_c = ((bit_in_frame - zone_a) / 3) % cols,
				_r = ((bit_in_frame - zone_a) / 3) / cols + 8;
			else
				_c = ((bit_in_frame - zone_a - zone_b) / 3) % (cols - 8) + 8,
				_r = ((bit_in_frame - zone_a - zone_b) / 3) / (cols - 8) + rows - 8;
			frame.back().at<Vec3b>(_r, _c).val[_rgb] = is * 255;
			bits++;
		}
	}
	void save(const string &dir)
	{
		for (size_t i = 0; i < frame.size(); i++)
		{
			char buf[20];
			sprintf(buf, "img%03ld.png", i);
			imwrite(dir + string(buf), frame[i]);
		}
	}
};

int main(int argc, char **argv)
{
	int cols, rows;
	sscanf(argv[3], "%d", &cols);
	sscanf(argv[4], "%d", &rows);
	Code4D code(rows, cols);
	ifstream fin(argv[1], ios::binary);
	while (fin.good())
	{
		char byte;
		fin.read(&byte, 1);
		code.pushByte(byte);
	}
	code.save(argv[2]);
	waitKey(0);
	return 0;
}
