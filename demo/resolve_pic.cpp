#include <fstream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

//默认定位点8格
#define LENGTH 100		//二维码格数
#define W 1				//白色边框的格数*每格所占像素
#define PIX 1			//每格所占像素

void decode(Mat src) {
	ofstream fout("C:/Users/Count Zero/Desktop/0.txt",ios::out);
	string bit[2][2][2];		//存码

	/*bgr*/
	//bit[0][0][0] = "000";	//黑
	//bit[0][0][1] = "100";	//蓝
	//bit[0][1][0] = "010";	//绿
	//bit[0][1][1] = "110";	//青
	//bit[1][0][0] = "001";	//红
	//bit[1][0][1] = "101";	//品红
	//bit[1][1][0] = "011";	//黄
	//bit[1][1][1] = "111";	//白
	
	/*rgb*/
	bit[0][0][0] = "000";	//黑
	bit[0][0][1] = "001";	//蓝
	bit[0][1][0] = "010";	//绿
	bit[0][1][1] = "011";	//青
	bit[1][0][0] = "100";	//红
	bit[1][0][1] = "101";	//品红
	bit[1][1][0] = "110";	//黄
	bit[1][1][1] = "111";	//白

	int count = 7;
	char buffer[8];	//暂存8bit
	char ch = 0;	//字对应的ascii
	string code;	//暂存每格的码

	for (int i = W; i < LENGTH + W; i += PIX) {
		Vec3b * p = src.ptr<Vec3b>(i);	//行指针
		for (int j = W; j < LENGTH + W; j += PIX) {
			if ((i < 8 * PIX + W && j < 8 * PIX + W) || (i > LENGTH - 9 * PIX + W && j < 8 * PIX + W) || (i < 8 * PIX + W && j > LENGTH - 9 * PIX + W))continue;	//跳过定位点
			code = bit[(p[j][0] + 127) / 255][(p[j][1] + 127) / 255][(p[j][2] + 127) / 255];	//表驱动,不用这个我读不出一格的码。。。
			for (int k = 0; k < 3; k++) {
				buffer[count--] = code[k];
				//cout << "buffer" << '[' << count+1 << ']' << ':' << buffer[count+1] << endl;	//测试
				if (count < 0){ 	//凑齐8bit				
					for (count = 0; count < 8; count++)
						ch = ch * 2 + (buffer[count] - '0');	//二进制转十进制(ASCII)	
				fout << ch;	//写入		
				ch = 0; count = 7;	//重置参数	
				}
			}
		}
	}	
	fout.close();
}

int main(int argc, char **argv)
{
	Mat src = imread("C:/Users/Count Zero/Desktop/img000.png");
	decode(src);
	system("pause");
	return 0;
}
