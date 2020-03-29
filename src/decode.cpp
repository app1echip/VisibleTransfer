#include <cstring>
#include <fstream>
#include "crc.hpp"
#include "mat.hpp"
using namespace std;
using namespace cv;

/* try find valid pattern and return correct Mat */
void find_pattern(VideoCapture& in, Mat& out);

int main(int argc, char** argv) {
  VideoCapture in(argv[1]);
  /* read header */
  header = Mat::zeros(HEADER_ROW, HEADER_COL, COLOR);
  find_pattern(in, header);
  
  /* read file_size, rows and cols from header */
  size_t p = 0;
  for (size_t i = 0; i < sizeof(file_size); ++i)
    read_byte(&header, p++ * CHAR_WIDTH, (byte*)&file_size + i);
  for (size_t i = 0; i < sizeof(rows); ++i)
    read_byte(&header, p++ * CHAR_WIDTH, (byte*)&rows + i);
  for (size_t i = 0; i < sizeof(cols); ++i)
    read_byte(&header, p++ * CHAR_WIDTH, (byte*)&cols + i);
  /* get frame number */
  ext_size = file_size;
  while (ext_size % BLOCK_SIZE != 0) ext_size++;
  block_num = ext_size / BLOCK_SIZE;
  crc_size = block_num * CRC_SIZE;
  size_t total_bits_size = (ext_size + crc_size) * CHAR_WIDTH;
  double precise_frames =
      double(total_bits_size) / double(rows * cols * CHANNEL);
  frames = precise_frames;
  if (precise_frames > frames) frames++;

  /* read into mats[] */
  mats = new Mat[frames];
  for (int i = 0; i < frames; ++i) {
    mats[i] = Mat::zeros(rows, cols, COLOR);
    find_pattern(in, mats[i]);
  }
  /* split mats[] into data[] and crc[] */
  data = new byte[ext_size];
  crc = new byte[crc_size];
  memset(data + file_size, 0, ext_size - file_size);
  for (size_t i = 0; i < block_num; ++i) {
    /* read one data block */
    size_t block_beg = i * (BLOCK_SIZE + CRC_SIZE);
    for (int j = 0; j < BLOCK_SIZE; j++) {
      size_t byte_beg = block_beg + j;
      read_byte(mats, byte_beg * CHAR_WIDTH, data + i * BLOCK_SIZE + j);
    }
    /* read one crc block */
    size_t crc_beg = block_beg + BLOCK_SIZE;
    for (int j = 0; j < CRC_SIZE; ++j) {
      size_t byte_beg = crc_beg + j;
      read_byte(mats, byte_beg * CHAR_WIDTH, crc + i * CRC_SIZE + j);
    }
  }

  /* save binary file */
  ofstream out(argv[2], ios::binary);
  out.write((char*)data, file_size);

  /* check crc and save result */
  verify = new byte[ext_size];
  for (size_t i = 0; i < block_num; ++i) {
    bool same = crc_16(data + i * BLOCK_SIZE, BLOCK_SIZE) ==
                *(uint16_t*)(crc + i * CRC_SIZE);
    memset(verify + i * BLOCK_SIZE, same * 0xff, BLOCK_SIZE);
  }
  ofstream vout(argv[3], ios::binary);
  vout.write((char*)verify, file_size);

  delete[] verify;
  delete[] data;
  delete[] crc;
  delete[] mats;

  return 0;
}

/* try find valid pattern and return correct Mat */
void find_pattern(VideoCapture& in, Mat& out) {
  int valid = 0;
  int dup = CAM_FPS / FPS;
  Mat bin;
  while (valid < dup) {
    Mat raw;
    in >> raw;
    if (raw.empty()) return;
    /* find all contours */
    cvtColor(raw, bin, COLOR_BGR2GRAY);
    threshold(bin, bin, 0, UCHAR_MAX, THRESH_BINARY | THRESH_OTSU);
    vector<vector<Point>> con;
    vector<Vec4i> hie;
    findContours(bin, con, hie, RETR_LIST, CHAIN_APPROX_SIMPLE, Point());

    /* find max contour, namely the code */
    int max_id;
    double max_area = 0;
    double current_area;
    for (int i = 0; i < con.size(); ++i) {
      current_area = contourArea(con[i]);
      if (current_area > max_area) max_area = current_area, max_id = i;
    }

    /* return false if max contour area too small or frame redundent */
    /* correct code if valid */
    if (max_area > 850000 && valid++ == dup / 2) {
      /* find corners */
      Point pts[4];
      int32_t m[4] = {INT32_MAX, INT32_MIN, INT32_MAX, INT32_MIN};
      for (int i = 0; i < con[max_id].size(); ++i) {
        auto& pt = con[max_id][i];
        int add = pt.x + pt.y, sub = pt.x - pt.y;
        if (add < m[0]) m[0] = add, pts[0] = pt;
        if (sub > m[1]) m[1] = sub, pts[1] = pt;
        if (sub < m[2]) m[2] = sub, pts[2] = pt;
        if (add > m[3]) m[3] = add, pts[3] = pt;
      }

      /* wrap image */
      const float ht_dpi = float(DSP_HT) / float(out.rows + 4);
      const float wd_dpi = float(DSP_WD) / float(out.cols + 4);
      const float warp_ht = float(DSP_HT) * (1.0 - 2.0 / float(out.rows + 4));
      const float warp_wd = float(DSP_WD) * (1.0 - 2.0 / float(out.cols + 4));
      vector<Point2f> corner;
      for (int i = 0; i < 4; ++i) corner.emplace_back(pts[i]);
      vector<Point2f> origin;
      origin.emplace_back(Point2f(0, 0));
      origin.emplace_back(Point2f(warp_wd, 0));
      origin.emplace_back(Point2f(0, warp_ht));
      origin.emplace_back(Point2f(warp_wd, warp_ht));
      Mat transform = getPerspectiveTransform(corner, origin);
      Mat warp(int(warp_ht), int(warp_wd), COLOR);
      
      /* pick color */
      for (int i = 0; i < out.rows; ++i) {
        for (int j = 0; j < out.cols; ++j) {
          float map_x = (j + 1.5) * wd_dpi;
          float map_y = (i + 1.5) * ht_dpi;
          auto& out_px = out.at<Vec3b>(i, j);
          auto raw_px = warp.at<Vec3b>(int(map_y), int(map_x));
          for (int i = 0; i < CHANNEL; ++i)
            out_px.val[i] = raw_px.val[i] >= CHAR_MAX ? UCHAR_MAX : 0x00;
        }
      }
    }
  }
}