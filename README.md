# VisibleTransfer
Transfer data over visible light.
## Develop environment
Building this project is simple, requiring nothing more than OpenCV packages.
You can compile and link directly with your preferred compiler, eg:
```bash
g++ encode.cpp -o encode `pkg-config --cflags --libs opencv`
```
or use any IDE where CMake takes care of all.
### Linux
This project was initially developed and tested with `libopencv-dev` version 3.2.0 on Ubuntu.
### Windows
## Test
`check` is a simple utility that reads vout.bin, usage:
```bash
./check vout.bin
```
`install.sh` installs `libopencv-dev` on Ubuntu, which provides necessary runtime libraries.