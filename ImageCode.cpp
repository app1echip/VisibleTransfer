#include "ImageCode.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

using namespace rapidjson;

ImageCode::ImageCode(const string &path)
{
#ifdef WIN32
    FILE *fp = fopen(config.c_str(), "rb");
#else
    FILE *fp = fopen(path.c_str(), "r");
#endif
    char buffer[UINT16_MAX];
    FileReadStream is(fp, buffer, sizeof(buffer));
    fclose(fp);
    Document doc;
    doc.ParseStream(is);

    row = doc["row"].GetInt();
    col = doc["col"].GetInt();
    fps = doc["fps"].GetInt();
    channel = doc["channel"].GetInt();

    displayHeight = doc["displayHeight"].GetInt();
    displayWidth = doc["displayWidth"].GetInt();
    cameraFps = doc["cameraFps"].GetInt();

    fileSize = 0;
    size = sizeof(fileSize) * UCHAR_WIDTH;
}

uchar &ImageCode::at(uint32 index)
{
    int singleFrameCapacity = row * col * channel;
    int frameId = int(index / singleFrameCapacity);
    int pixelsOfLastFrame = int((index - frameId * singleFrameCapacity) / channel);
    int rowId = pixelsOfLastFrame / col;
    int colId = pixelsOfLastFrame % col;
    int channelId = int(index % channel);
    if (frameId > int(dataFrames.size() - 1))
        dataFrames.emplace_back(Mat(row, col, CV_8UC3));
    return dataFrames[frameId].at<Vec3b>(rowId, colId)[channelId];
}

void ImageCode::setByte(uchar byte, uint32 Id)
{
    for (int i = 0; i < UCHAR_WIDTH; ++i)
    {
        at(Id + i) = (byte >> i & 0x01) * UCHAR_MAX;
    }
}

uchar ImageCode::getByte(uint32 Id)
{
    uchar byte = 0;
    int i;
    for (i = 0; i < UCHAR_WIDTH - 1; ++i)
    {
        byte += at(Id + i) / UCHAR_MAX ? 0x80 : 0x00;
        byte >>= 1;
    }
    byte += at(Id + i) / UCHAR_MAX ? 0x80 : 0x00;
    return byte;
}

void ImageCode::loadDataFromStream(istream &in)
{
    while (in.good())
    {
        char byte;
        in.read(&byte, sizeof(byte));
        setByte(byte, size);
        size += UCHAR_WIDTH;
    }
    fileSize = size - sizeof(fileSize) * UCHAR_WIDTH;
    for (int i = 0; i < sizeof(fileSize) * UCHAR_WIDTH; i += UCHAR_WIDTH)
    {
        uchar *byte = (uchar *)&fileSize + i / UCHAR_WIDTH;
        setByte(*byte, i);
    }
}

void ImageCode::saveToVideo(const string &dst)
{
    VideoWriter out(dst, CV_FOURCC('m', 'p', '4', 'v'), fps, Size(displayWidth, displayHeight));
    Mat black(displayHeight, displayWidth, CV_8UC3);
    out << black;
    for (auto &frame : dataFrames)
    {
        Mat border = frame.clone();
        copyMakeBorder(border, border, 1, 1, 1, 1, BORDER_CONSTANT, Scalar(255, 255, 255));
        copyMakeBorder(border, border, 1, 1, 1, 1, BORDER_CONSTANT, Scalar(0, 0, 0));
        Mat display(displayHeight, displayWidth, CV_8UC3);
        resize(border, display, display.size(), 0, 0, INTER_NEAREST);
        out << display;
    }
    out << black;
}

bool ImageCode::extract(Mat &src, Mat &dst)
{
    Mat binary;
    cvtColor(src, binary, COLOR_BGR2GRAY);
    threshold(binary, binary, 0, 255, THRESH_BINARY | THRESH_OTSU);
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(binary, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE, Point());
    int maxId = 0;
    for (int i = 0; i < contours.size(); ++i)
    {
        double area = contourArea(contours[i]);
        if (area > contourArea(contours[maxId]))
        {
            maxId = i;
        }
    }
    auto &contour = contours[maxId];
    if (contourArea(contour) < 900000)
        return false;
    Point leftUp, rightUp, leftButton, rightButton;
    int32_t leftUpMin = INT32_MAX, rightUpMax = INT32_MIN, leftButtonMin = INT32_MAX, rightButtonMax = INT32_MIN;
    for (int i = 0; i < contour.size(); ++i)
    {
        auto &point = contour[i];
        int add = point.x + point.y, sub = point.x - point.y;
        if (add > rightButtonMax)
            rightButtonMax = add, rightButton = point;
        if (add < leftUpMin)
            leftUpMin = add, leftUp = point;
        if (sub > rightUpMax)
            rightUpMax = sub, rightUp = point;
        if (sub < leftButtonMin)
            leftButtonMin = sub, leftButton = point;
    }

    double scaleRow = double(displayHeight) / double(row + 4);
    double scaleCol = double(displayWidth) / double(col + 4);
    vector<Point2f> srcPoint = {
        Point2f(leftUp),
        Point2f(rightUp),
        Point2f(leftButton),
        Point2f(rightButton)

    };
    vector<Point2f> dstPoint = {
        Point2f(0, 0),
        Point2f(displayWidth - 2 * scaleCol - 1, 0),
        Point2f(0, displayHeight - 2 * scaleRow - 1),
        Point2f(displayWidth - 2 * scaleCol - 1, displayHeight - 2 * scaleRow - 1)};
    Mat transform = getPerspectiveTransform(srcPoint, dstPoint);
    Mat warp(displayHeight - 2 * scaleRow, displayWidth - 2 * scaleCol, CV_8UC3);
    warpPerspective(src, warp, transform, warp.size(), INTER_NEAREST);
    Rect rect(scaleCol - 1, scaleRow - 1, displayWidth - 4 * scaleCol, displayHeight - 4 * scaleRow);
    Mat crop = warp(rect);
    dst = Mat::zeros(row, col, CV_8UC3);
    for (int i = 0; i < row; ++i)
    {
        for (int j = 0; j < col; ++j)
        {
            double map_x = j * scaleCol + scaleCol / 2;
            double map_y = i * scaleRow + scaleRow / 2;
            auto &target = dst.at<Vec3b>(i, j);
            auto source = crop.at<Vec3b>(int(map_y), int(map_x));
            for (int i = 0; i < channel; ++i)
            {
                target.val[i] = source.val[i] >= uchar(UCHAR_MAX / 2) ? UCHAR_MAX : 0;
            }
        }
    }
    return true;
}

void ImageCode::loadFromVideo(const string &path)
{
    VideoCapture videoCapture(path);
    int frameCounter = 0;
    int validInFrames = cameraFps / fps;
    int midId = validInFrames / 2;
    while (1)
    {
        Mat raw;
        Mat extracted(row, col, CV_8UC3);
        videoCapture >> raw;
        if (raw.empty())
            break;

        if (extract(raw, extracted))
        {
            string title = format("%03d", frameCounter);
            namedWindow(title, WINDOW_NORMAL);
            resizeWindow(title, 960, 540);
            imshow(title, extracted);
            waitKey();
            if (frameCounter % validInFrames == midId)
                dataFrames.push_back(extracted);
            frameCounter++;
        }
    }
}

void ImageCode::saveDataToStream(ostream &out)
{
    for (int i = 0; i < sizeof(fileSize) * UCHAR_WIDTH; i += UCHAR_WIDTH)
    {
        uchar byte = getByte(i);
        uchar *p = (uchar *)&fileSize + i / UCHAR_WIDTH;
        *p = byte;
    }
    for (int i = 0; i < fileSize; i += UCHAR_WIDTH)
    {
        char byte = (char)getByte(sizeof(fileSize) * UCHAR_WIDTH + i);
        out.write(&byte, sizeof(byte));
    }
}

bool ImageCode::writeHeader()
{
    return false;
}
