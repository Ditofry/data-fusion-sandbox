#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H


class ImageProcessor
{
public:
    ImageProcessor();
    bool try_use_gpu = false;

    void printUsage();
    void stitchImage();
    int parseImgArgs(int argc, char** argv);
};

#endif // IMAGEPROCESSOR_H
