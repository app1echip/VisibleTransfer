//
// Created by hf on 3/8/20.
//

#include "ImageCode.h"

ImageCode::ImageCode(unsigned _row, unsigned _col) : row(_row), col(_col), size(UCHAR_WIDTH * sizeof(fileSize)) {}

uchar &ImageCode::at(uint32_t Id) {
    int singleFrameCapacity = row * col * 3;
    int frameId = Id / singleFrameCapacity;
    int pixelSizeOfLastFrame = (Id - frameId * singleFrameCapacity) / 3;
    int rowId = pixelSizeOfLastFrame / col;
    int colId = pixelSizeOfLastFrame % col;
    int channelId = Id % 3;
    if (frameId > int(frames.size() - 1))
        frames.push_back(Mat(row, col, CV_8UC3));
    return frames[frameId].at<Vec3b>(rowId, colId)[channelId];
}

void ImageCode::setByte(uchar byte, uint32_t Id) {
    for (int i = 0; i < UCHAR_WIDTH; ++i) {
        at(Id + i) = ((byte >> i) & 0x01) * UCHAR_MAX;
    }
}

uchar ImageCode::getByte(uint32_t Id) {
    uchar byte = 0;
    int i;
    for (i = 0; i < UCHAR_WIDTH - 1; ++i) {
        byte += at(Id + i) / UCHAR_MAX ? 0x80 : 0x00;
        byte >>= 1;
    }
    byte += at(Id + i) / UCHAR_MAX ? 0x80 : 0x00;
    return byte;
}

void ImageCode::loadDataFromStream(istream &in) {
    while (in.good()) {
        char byte;
        in.read(&byte, sizeof(byte));
        setByte(byte, size);
        size += 8;
    }
    fileSize = size - sizeof(fileSize) * UCHAR_WIDTH;
    for (int i = 0; i < sizeof(fileSize) * UCHAR_WIDTH; i += UCHAR_WIDTH) {
        uchar *byte = (uchar *) &fileSize + i / UCHAR_WIDTH;
        setByte(*byte, i);
    }
}

void ImageCode::saveToVideo(const string &dst) {
    VideoWriter out(dst, CV_FOURCC('m', 'p', '4', 'v'), 30, Size(dspCol, dspRow));
    Mat black(dspRow, dspCol, CV_8UC3);
    out << black;
    for (auto &frame:frames) {
        Mat border = frame.clone();
        copyMakeBorder(border, border, 1, 1, 1, 1, BORDER_CONSTANT, Scalar(255, 255, 255));
        copyMakeBorder(border, border, 1, 1, 1, 1, BORDER_CONSTANT, Scalar(0, 0, 0));
        Mat display(dspRow, dspCol, CV_8UC3);
        resize(border, display, display.size(), 0, 0, INTER_NEAREST);
        out << display;
    }
    out << black;
}

void ImageCode::saveDataToStream(ostream &out) {
    for (int i = 0; i < sizeof(fileSize) * UCHAR_WIDTH; i += UCHAR_WIDTH) {
        uchar byte = getByte(i);
        uchar *p = (uchar *) &fileSize + i / UCHAR_WIDTH;
        *p = byte;
    }
    for (int i = 0; i < fileSize; i += UCHAR_WIDTH) {
        char byte = (char) getByte(sizeof(fileSize) * UCHAR_WIDTH + i);
        out.write(&byte, sizeof(byte));
    }
}


void ImageCode::loadFromVideo(const string &path) {
    VideoCapture videoCapture(path);
    while (1) {
        Mat raw;
        Mat extracted(row, col, CV_8UC3);
        videoCapture >> raw;
        if (raw.empty()) break;
        bool has = extract(raw, extracted);
    }
}

bool ImageCode::extract(Mat &src, Mat &dst) {
    Mat binary;
    cvtColor(src, binary, COLOR_BGR2GRAY);
    threshold(binary, binary, 0, 255, THRESH_BINARY | THRESH_OTSU);
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(binary, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE, Point());
    int maxId = 0;
    for (int i = 0; i < contours.size(); ++i) {
        double area = contourArea(contours[i]);
        if (area > contourArea(contours[maxId])) { maxId = i; }
    }
    auto &contour = contours[maxId];
    if (contourArea(contour) < 900000) return false;
    Point leftUp, rightUp, leftButton, rightButton;
    int32_t leftUpMin = INT32_MAX, rightUpMax = INT32_MIN, leftButtonMin = INT32_MAX, rightButtonMax = INT32_MIN;
    for (int i = 0; i < contour.size(); ++i) {
        auto &point = contour[i];
        int add = point.x + point.y, sub = point.x - point.y;
        if (add > rightButtonMax) rightButtonMax = add, rightButton = point;
        if (add < leftUpMin) leftUpMin = add, leftUp = point;
        if (sub > rightUpMax) rightUpMax = sub, rightUp = point;
        if (sub < leftButtonMin) leftButtonMin = sub, leftButton = point;
    }
    double scaleCol = double(dspCol) / double(col + 4);
    double scaleRow = double(dspRow) / double(row + 4);
    vector<Point2f> srcPoint = {
            Point2f(leftUp),
            Point2f(rightUp),
            Point2f(leftButton),
            Point2f(rightButton)
    };
    vector<Point2f> dstPoint = {
            Point2f(0, 0),
            Point2f(dspCol - 2 * scaleCol - 1, 0),
            Point2f(0, dspRow - 2 * scaleRow - 1),
            Point2f(dspCol - 2 * scaleCol - 1, dspRow - 2 * scaleRow - 1)
    };
    Mat transform = getPerspectiveTransform(srcPoint, dstPoint);
    Mat warp(dspRow - 2 * scaleRow, dspCol - 2 * scaleCol, CV_8UC3);
    warpPerspective(src, warp, transform, warp.size(), INTER_NEAREST);
    Rect rect(scaleCol - 1, scaleRow - 1, dspCol - 4 * scaleCol, dspRow - 4 * scaleRow);
    Mat crop = warp(rect);
    Mat mat(row, col, CV_8UC3);
    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < col; ++j) {
            double map_x = i * scaleCol + scaleCol / 2;
            double map_y = j * scaleRow + scaleRow / 2;
            auto &target = mat.at<Vec3b>(i, j);
            auto &source = crop.at<Vec3b>(int(map_x), int(map_y));
            target = source;
            //printf("(%d,%d)[%d,%d,%d]<-(%lf,%lf)[%d,%d,%d]\n", i, j, target.val[0], target.val[1], target.val[2], map_x,
            //       map_y, source.val[0], source.val[1], source.val[2]);
            circle(crop, Point(map_y, map_x), 1, Scalar(0, 0, 255));
        }
    }
    namedWindow("crop", WINDOW_NORMAL);
    resizeWindow("crop", 960, 540);
    imshow("crop", crop);
    waitKey();
    namedWindow("mat", WINDOW_NORMAL);
    resizeWindow("mat", 960, 540);
    imshow("mat", mat);
    waitKey();
    return true;
}