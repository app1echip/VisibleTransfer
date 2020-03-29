#pragma once
#include <climits>
#include <cstdint>
#include <opencv2/opencv.hpp>

typedef unsigned char byte;
#define MAX_BPS 10240 /* max speed in B/s */
#define FPS 10.0      /* image code fps */
#define HEADER_ROW 7  /* header cols */
#define HEADER_COL 7  /* header rows */
#define ASPECT 2.0    /* image code aspect ratio (rows/cols) */
#define CHANNEL 3     /* pixel color format */
#define COLOR CV_8UC3 /* pixel color format */
#define DSP_WD 1920   /* output and input video width in pixels */
#define DSP_HT 1080   /* output and input video height in pixels */
#define CAM_FPS 30    /* input video fps */
#define BLOCK_SIZE 4  /* data block size in bytes */
#define CRC_SIZE 2    /* crc block size in bytes */
size_t file_size;     /* raw binary file size in bytes */
int rows;             /* image code rows */
int cols;             /* image code cols */
int frames;           /* image code frames */
size_t ext_size;      /* extended file size in bytes */
size_t crc_size;      /* entire crc size in bytes */
size_t block_num;     /* number of blocks */
byte *data;           /* binary file */
byte *crc;            /* entire file crc */
byte *verify;         /* indicates reliability */
cv::Mat header;       /* header that indicates file_size, rows and cols */
cv::Mat *mats;        /* mats storage for binary file */
int time_limit;       /* generated video length in milliseconds */