#! /bin/bash
# 使用 ./gen_vid.sh images/ output.mp4 60
# $1 图片所在文件夹（要/）
# $2 输出视频的位置
# $3 帧率

ffmpeg -framerate $3 -i $1img%03d.png -c:v copy $2
