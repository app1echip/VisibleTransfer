# VisibleTransfer 可测试DEMO
第一个可食用的演示版本
## 如何运行
0. 构建：构建没有特殊的要求。可以使用命令行、CMake或支持CMake的IDE构建；在VS中也可以创建新的VS空白项目，新建一个ImageCode类，将主函数和ImageCode的声明与定义复制进去后构建即可。
1. 使用：为了方便在命令行中把工作目录切换到构建生成的可执行文件(比如`main.exe`)的所在目录，将需要编码的任意文件(比如`in.txt`)和待解码的视频(比如`shot.mp4`)也放在这个目录。
2. 编码：程序接受五个参数，例如：`main.exe encode in.txt 50 50 out.mp4`。第一个参数固定是`encode`，表示进行编码操作；第二个参数代表需要编码的文件名，此处是`in.txt`；第三和四个参数代表二维码尺寸，前一个是宽，后一个是高；第五个参数是输出视频的名称，例如这里叫`out.mp4`。
3. 解码：仍然接受五个参数，例如: `main.exe decode shot.mp4 50 50 out.txt`。第一个参数固定是`decode`，表示进行解码操作；第二个参数代表需要解码的文件名，此处是`shot.txt`；第三和四个参数代表二维码尺寸，与编码时相同;第五个参数是输出文件的名称，比如叫`out.txt`。
## 程序说明
* 编码生成视频画面大小和帧率的默认是1920x1080@10fps，使用`INTER_NEAREST`缩放；
* 默认30fps拍摄从有效的第一帧开始算起，解码时在相邻的三帧中取中间帧；
* 生成的视频有1格大小的白框与1格的黑框保护，解码提取二维码时依靠画面中最大轮廓(即内白外黑的交界处)；
* 视频开头和结尾有1帧黑屏。
## 测试建议
* 以50x50@10fps为例，理论速率约9KB/s,准备一个大小约25KB的文件编码生成约3s的视频；
* 尽可能全屏播放视频并使二维码充满取景框(使得画面中最大的轮廓是二维码框的黑白交界)，保持播放器背景黑色，无UI干扰(例如`mpv Media Player`)；在视频开头暂停，接着打开手机录像，播放视频；
* 充分利用播放器的逐帧查看功能(例如`mpv`暂停时按`,`和`.`切换前后帧)，使用后面的画面进行对焦测光后，再回到开头开始播放录制；
* 初版测试程序预期存在很高不可靠性。