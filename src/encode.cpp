#include <cstring>
#include <fstream>
#include "crc.hpp"
#include "mat.hpp"
using namespace std;
using namespace cv;

int main(int argc, char **argv) {
  /* get file_size in bytes */
  ifstream fin(argv[1], ios::binary | ios::in | ios::ate);
  file_size = fin.tellg();
  /* load file into data[] */
  ext_size = file_size;
  while (ext_size % BLOCK_SIZE != 0) ext_size++;
  data = new byte[ext_size];
  fin.seekg(0, ios::beg);
  fin.read((char *)data, file_size);
  memset(data + file_size, 0, ext_size - file_size);

  /* transfer only part of file if too targe */
  time_limit = stoi(argv[3]);
  double precise_max_size = double(time_limit) / 1000.0;
  precise_max_size *= double(MAX_BPS);
  if (double(file_size) > precise_max_size) {
    size_t max_size = size_t(precise_max_size);
    while (max_size % BLOCK_SIZE != 0) max_size--;
    file_size = ext_size = max_size;
  }

  /* write crc 16 to crc[] */
  block_num = ext_size / BLOCK_SIZE;
  crc_size = block_num * CRC_SIZE;
  crc = new byte[crc_size];
  for (size_t i = 0; i < block_num; ++i) {
    uint16_t block_crc = crc_16(data + i * BLOCK_SIZE, BLOCK_SIZE);
    memcpy(crc + i * CRC_SIZE, &block_crc, CRC_SIZE);
  }

  /* get rows and cols of data frames */
  frames = time_limit / (1000.0 / FPS) - 2;
  size_t total_bits_size = (ext_size + crc_size) * CHAR_WIDTH;
  double precise_rows = total_bits_size;
  precise_rows /= frames;
  precise_rows /= double(CHANNEL) * ASPECT;
  precise_rows = sqrt(precise_rows);
  double precise_cols = precise_rows * ASPECT;
  rows = precise_rows;
  cols = precise_cols;
  if (rows < precise_rows) rows++;
  if (cols < precise_cols) cols++;

  /* write file_size, rows and cols to header */
  header = Mat::zeros(HEADER_ROW, HEADER_COL, COLOR);
  size_t p = 0;
  for (size_t i = 0; i < sizeof(file_size); ++i)
    write_byte(&header, p++ * CHAR_WIDTH, *((char *)&file_size + i));
  for (size_t i = 0; i < sizeof(rows); ++i)
    write_byte(&header, p++ * CHAR_WIDTH, *((char *)&rows + i));
  for (size_t i = 0; i < sizeof(cols); ++i)
    write_byte(&header, p++ * CHAR_WIDTH, *((char *)&cols + i));

  /* create $frames of Mat and set each rows and cols */
  mats = new Mat[frames];
  for (int i = 0; i < frames; ++i) mats[i] = Mat::zeros(rows, cols, COLOR);
  /* write data[] and crc[] to mats[] */
  for (size_t i = 0; i < block_num; ++i) {
    /* write one data block */
    size_t block_beg = i * (BLOCK_SIZE + CRC_SIZE);
    for (int j = 0; j < BLOCK_SIZE; j++) {
      size_t byte_beg = block_beg + j;
      write_byte(mats, byte_beg * CHAR_WIDTH, data[i * BLOCK_SIZE + j]);
    }
    /* write one crc block */
    size_t crc_beg = block_beg + BLOCK_SIZE;
    for (int j = 0; j < CRC_SIZE; ++j) {
      size_t byte_beg = crc_beg + j;
      write_byte(mats, byte_beg * CHAR_WIDTH, crc[i * CRC_SIZE + j]);
    }
  }

  /* save header and mats[] to video */
  int vid_format = CV_FOURCC('m', 'p', '4', 'v');
  Size vid_size = Size(DSP_WD, DSP_HT);
  VideoWriter out(argv[2], vid_format, FPS, vid_size);
  /* write one empty frame */
  out << Mat::zeros(vid_size, COLOR);
  Scalar white(UCHAR_MAX, UCHAR_MAX, UCHAR_MAX);
  Scalar black(0, 0, 0);
  /* write header */
  copyMakeBorder(header, header, 1, 1, 1, 1, BORDER_CONSTANT, white);
  copyMakeBorder(header, header, 1, 1, 1, 1, BORDER_CONSTANT, black);
  resize(header, header, vid_size, 0, 0, INTER_NEAREST);
  out << header;
  /* write mats[] */
  for (int i = 0; i < frames; ++i) {
    copyMakeBorder(mats[i], mats[i], 1, 1, 1, 1, BORDER_CONSTANT, white);
    copyMakeBorder(mats[i], mats[i], 1, 1, 1, 1, BORDER_CONSTANT, black);
    resize(mats[i], mats[i], vid_size, 0, 0, INTER_NEAREST);
    out << mats[i];
  }

  delete[] mats;
  delete[] crc;
  delete[] data;

  return 0;
}