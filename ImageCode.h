//
// Created by hf on 3/8/20.
//

#ifndef VISIBLETRANSFER_IMAGECODE_H
#define VISIBLETRANSFER_IMAGECODE_H

#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class ImageCode {
    vector<Mat> frames;
    uint32_t size;
    const unsigned row;
    const unsigned col;

    uint32_t fileSize;

    const short dspRow = 1080;
    const short dspCol = 1920;

    uchar &at(uint32_t);

    void setByte(uchar, uint32_t);

    uchar getByte(uint32_t);

public:
    ImageCode(unsigned, unsigned);

    void loadDataFromStream(istream &);

    void saveToVideo(const string &);

    void saveDataToStream(ostream &);

    void loadFromVideo(const string &);

    bool extract(Mat &, Mat &);

    void readSizeInfo();
};


#endif //VISIBLETRANSFER_IMAGECODE_H
