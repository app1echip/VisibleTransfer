#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, char const *argv[])
{
    Mat img = imread("cam/pic001.png");
    cvtColor(img, img, COLOR_BGR2GRAY);
    threshold(img, img, 0, 255, THRESH_BINARY | THRESH_OTSU);
    vector<vector<Point>> contours;
    vector<Vec4i> hireachy;
    findContours(img, contours, hireachy, RETR_LIST, CHAIN_APPROX_SIMPLE, Point());
    vector<vector<Point2f>> points;
    for (size_t i = 0; i < contours.size(); i++)
    {
        if (contourArea(contours[i]) < 100.0)
            continue;
        RotatedRect rect = minAreaRect(contours[i]);
        float w = rect.size.width;
        float h = rect.size.height;
        float rate = min(w, h) / max(w, h);
        if (rate > 0.85 && w < img.cols / 4 && h < img.rows / 4)
        {
        // 按照轮廓中心点位置将所有轮廓分类
        // 一个类别里有三个轮廓的既是定位点
            bool rep = false;
            for (auto &p : points)
            {
                float shift_x = p[0].x - rect.center.x;
                float shift_y = p[0].y - rect.center.y;
                if ((shift_x > -2 && shift_x < 2) && (shift_y > -2 && shift_y < 2))
                    p.push_back(rect.center), rep = true;
            }
            if (!rep)
            {
                vector<Point2f> list;
                list.push_back(rect.center);
                points.push_back(list);
            }
        }
    }
    for (auto &p : points)
    {
        if (p.size() >= 3)
            cout << p.size() << endl;
    }

    return 0;
}
