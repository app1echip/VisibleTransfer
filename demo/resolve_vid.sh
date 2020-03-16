#! /bin/bash
# 使用 ./resolve_vid.sh cam.mp4 cam/
# $1 视频文件
# $2 导出目录

ffmpeg -i $1 $2pic%03d.png
