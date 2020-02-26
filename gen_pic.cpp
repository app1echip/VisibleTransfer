/* 
编译 $ g++ play.cpp -o play `pkg-config --cflags --libs opencv`
运行 $ ./play ./samplefile ./ 200 100
       argv[1]: 待编码文件
       argv[2]: 导出图片目录（要带 ‘/’ ）
       argv[3],argv[4]: 分辨率，如1920x1080
*/
#include <fstream>
#include <vector>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;
class Code4D
{
	int rows, cols;			// 行数，列数
	Mat _t;				// 模板：带有三个定位点的空白二维码
	unsigned bits;			// 已经使用的数据位数
	vector<Mat> frame;		// 若干个帧

public:
	Code4D(int _rows, int _cols) : _t(_rows, _cols, CV_8UC3, Scalar(255, 255, 255))
		// 构造 Code4D 前生成空二维码模板
	{
		bits = 0;
		cols = _cols;
		rows = _rows;
		
		// 绘制定位点
		for (int i = 0; i < 7; ++i)
			for (int j = 0; j < 7; ++j)
				_t.at<Vec3b>(i, j) = _t.at<Vec3b>(i, cols - 7 + j) = _t.at<Vec3b>(rows - 7 + i, j) = Vec3b(0, 0, 0);
		for (int i = 1; i < 6; ++i)
			for (int j = 1; j < 6; ++j)
				_t.at<Vec3b>(i, j) = _t.at<Vec3b>(i, cols - 7 + j) = _t.at<Vec3b>(rows - 7 + i, j) = Vec3b(255, 255, 255);
		for (int i = 2; i < 5; ++i)
			for (int j = 2; j < 5; ++j)
				_t.at<Vec3b>(i, j) = _t.at<Vec3b>(i, cols - 7 + j) = _t.at<Vec3b>(rows - 7 + i, j) = Vec3b(0, 0, 0);
		
		// 第一帧
		frame.push_back(_t.clone());
	}
	
	// 把 1 Byte = 8 bit 加入图片中
	void pushByte(unsigned char c)
	{
		for (int i = 0; i < 8; ++i)
		{
			bool is = (c >> i) & 0x01;	// is 是c从低到高的地i位

			int _rgb = bits % 3;		// 选择RGB中一个通道
			
			// 超过当前帧容量时创建新帧
			unsigned _frame = bits / ((cols * rows - 3 * 8 * 8) * 3);
			if (_frame + 1 > frame.size())
				frame.push_back(_t.clone());

			// 当前bit在当前帧的坐标
			unsigned _c, _r;
			
			// 从上到下分段讨论计算
			unsigned zone_a = (cols - 2 * 8) * 8 * 3;
			unsigned zone_b = (rows - 2 * 8) * cols * 3;
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
			
			// 四个维度：帧数[frame.back()]、行(_r)、列(_c)、RGB通道(_rgb)
			frame.back().at<Vec3b>(_r, _c).val[_rgb] = is * 255;
			
			bits++;
		}
	}
	
	// 保存到文件
	void save(const string &dir)
	{
		for (size_t i = 0; i < frame.size(); i++)
		{
			// 文件名格式：img000.png
			char buf[20];
			sprintf(buf, "img%03ld.png", i);
			Mat dst;
			copyMakeBorder(frame[i], dst, 1, 1, 1, 1, BORDER_CONSTANT, Scalar(255, 255, 255));
			imwrite(dir + string(buf), dst);
		}
	}
};

int main(int argc, char **argv)
{
	int cols, rows;
	sscanf(argv[3], "%d", &cols);	// cols 是宽
	sscanf(argv[4], "%d", &rows);	// rows 是高
	Code4D code(rows, cols);
	ifstream fin(argv[1], ios::binary);
	while (fin.good())
	{
		char byte;
		fin.read(&byte, 1);	// 从文件流里获得 1 Byte
		code.pushByte(byte);	// 把这一 1 Byte 存入到二维码中
	}
	code.save(argv[2]);		// 保存所有图片
	return 0;
}
