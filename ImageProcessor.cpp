#include "imageprocessor.h"
#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

bool try_use_gpu = false;
vector<Mat> imgs;
string result_name = "result.jpg";

ImageProcessor::ImageProcessor()
{
    // TODO: we should keep these attrs in a struct that we can
    // pair with any given set of images
    bool try_use_gpu = false;
    vector<Mat> imgs;
    string result_name = "result.jpg";


}

void ImageProcessor::printUsage()
{
    cout <<
        "Rotation model images stitcher.\n\n"
        "stitching img1 img2 [...imgN]\n\n"
        "Flags:\n"
        "  --try_use_gpu (yes|no)\n"
        "      Try to use GPU. The default value is 'no'. All default values\n"
        "      are for CPU mode.\n"
        "  --output <result_img>\n"
        "      The default is 'result.jpg'.\n";
}


int ImageProcessor::parseImgArgs(int argc, char** argv)
{
    if (argc == 1)
    {
        printUsage();
        return -1;
    }
    for (int i = 1; i < argc; ++i)
    {
        if (string(argv[i]) == "--help" || string(argv[i]) == "/?")
        {
            printUsage();
            return -1;
        }
        else if (string(argv[i]) == "--try_use_gpu")
        {
            if (string(argv[i + 1]) == "no")
                try_use_gpu = false;
            else if (string(argv[i + 1]) == "yes")
                try_use_gpu = true;
            else
            {
                cout << "Bad --try_use_gpu flag value\n";
                return -1;
            }
            i++;
        }
        else if (string(argv[i]) == "--output")
        {
            result_name = argv[i + 1];
            i++;
        }
        else
        {
            Mat img = imread(argv[i]);
            if (img.empty())
            {
                cout << "Can't read image '" << argv[i] << "'\n";
                return -1;
            }
            imgs.push_back(img);
        }
    }
    return 0;
}

void ImageProcessor::stitchImage(){
   Mat pano;
   Stitcher stitcher = Stitcher::createDefault(try_use_gpu);
   Stitcher::Status status = stitcher.stitch(imgs, pano);

   if (status != Stitcher::OK)
   {
       cout << "Can't stitch images, error code = " << int(status) << endl;
       return -1;
   }

   imwrite(result_name, pano);
}
