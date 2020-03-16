#include <iostream>
#include "ImageCode.h"

int main(int argc, char **argv) {
    if (argc != 6) return -1;

    string mode = string(argv[1]);
    string src = string(argv[2]);
    int col = stoi(argv[3]);
    int row = stoi(argv[4]);
    string dst = string(argv[5]);

    ImageCode imageCode(row, col);

    if (mode == "encode") {
        ifstream fin(src, ios::binary);
        imageCode.loadDataFromStream(fin);
        imageCode.saveToVideo(dst);
    } else if (mode == "decode") {
        imageCode.loadFromVideo(argv[2]);
        ofstream out(dst, ios::binary);
        imageCode.saveDataToStream(out);
    }
    return 0;
}
