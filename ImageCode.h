#ifndef VISIBLETRANSFER_IMAGECODE_H
#define VISIBLETRANSFER_IMAGECODE_H

#include <tiff.h>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class ImageCode
{
private:
    vector<Mat> dataFrames;
    Mat headerFrame;
    uint32 size;
    uchar *rawData;

    int row;
    int col;
    int fps;
    int channel;

    int displayWidth;
    int displayHeight;
    int cameraFps;

    uint32 fileSize;

    uchar &at(uint32);

    void setByte(uchar, uint32);

    uchar getByte(uint32);

    bool extract(Mat &, Mat &);

    bool writeHeader();

    void readHeader();

public:
    explicit ImageCode(const string & = "config.json");

    void loadDataFromStream(istream &);

    void saveToVideo(const string &);

    void saveDataToStream(ostream &);

    void loadFromVideo(const string &);
};

#endif //VISIBLETRANSFER_IMAGECODE_H
