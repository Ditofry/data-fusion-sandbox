#include "delegator.h"
#include "ImageProcessor.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <time.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/stitching/stitcher.hpp"


using namespace std;
using namespace cv;

void testTcpPoll();
void testImageStitch();
void removeFiles();
vector<Mat> retrieveImageFiles(string dir);

int main(int argc, char* argv[]){
  testTcpPoll();
  // testImageStitch();
 return 0;
}

void testTcpPoll(){
  Delegator *d = new Delegator();
  d->spawn_tcp_listener();
  while(true){
    std::cout << "What would you like to do?";
    int choice;
    std::cin >> choice;
    switch (choice){
      case 1:
        d->connection_check();
        break;
      case 2:
        std::cout << d->is_listening();
        break;
      case 3:
        d->tcp_image_poll();
        std::cout << "polled.\n";
        break;
      default:
        std::cout << "Nope, different input, plz";
        break;
    }
  }
}

void testImageStitch(){
  // Clean out stitched images dir
  removeFiles();

  // Grab all images in the stitch_frames directory
  ImageProcessor::stitchImages("stitch_frames");
}

void removeFiles(){
  system("exec rm -r collected_images/*");
}

vector<Mat> retrieveImageFiles(string dir){

}
