#include "codedef.h"

/* access one specific bit by reference */
uchar &at_bit(cv::Mat *mats, size_t pos) {
  int rows = mats->rows;
  int cols = mats->cols;
  int frame_bit_capactiy = rows * cols * CHANNEL;
  int frame_id = pos / frame_bit_capactiy;
  int pixel_id = (pos - frame_id * frame_bit_capactiy) / CHANNEL;
  int row_id = pixel_id / cols;
  int col_id = pixel_id % cols;
  int channel_id = pos % CHANNEL;
  return (mats + frame_id)->at<cv::Vec3b>(row_id, col_id)[channel_id];
}

/* write one byte to specific position in mats */
void write_byte(cv::Mat *mats, size_t pos, byte b) {
  for (int i = 0; i < CHAR_WIDTH; ++i)
    at_bit(mats, pos++) = (b >> i & 0x01) * UCHAR_MAX;
}

/* read one byte from specific position in mats */
void read_byte(cv::Mat *mats, size_t pos, byte *b) {
  *b = 0x00;
  for (int i = 0; i < CHAR_WIDTH - 1; ++i) {
    *b += at_bit(mats, pos++) == UCHAR_MAX ? 0x80 : 0x00;
    *b >>= 1;
  }
  *b += at_bit(mats, pos++) == UCHAR_MAX ? 0x80 : 0x00;
}