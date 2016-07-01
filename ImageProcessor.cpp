#include "imageprocessor.h"
#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/stitching/stitcher.hpp>
#include <stdlib.h>
#include <dirent.h>
#include <string>

using namespace std;
using namespace cv;

string result_name = "stitchedResult.jpg";

void ImageProcessor::stitchImages(const char *dirname){
  Mat pano;
  vector<Mat> imgs;

  struct dirent *entry;
  DIR *dp;

  dp = opendir(dirname);

  if (dp == NULL) {
    perror("opendir");
  }

  while((entry = readdir(dp))){
    if (entry->d_name[0] != '.'){
      std::string filenameString(entry->d_name, 0, sizeof(entry->d_name));
      cout << "filename: " << entry->d_name << endl;
      std::string rel_path = "stitch_frames/" + filenameString;
      imgs.push_back(imread(rel_path));
    }
  }

  closedir(dp);

  Stitcher stitcher = Stitcher::createDefault(false);
  Stitcher::Status status = stitcher.stitch(imgs, pano);

  if (status != Stitcher::OK){
     cout << "Can't stitch images, error code = " << int(status) << endl;
  }

  imwrite(result_name, pano);
}
