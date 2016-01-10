// OpenCV library
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include "opencv/cv.h"
// to open directory and read files in the directory
#include "dirent.h"
// to get image data from url and save them in Mat type
#include "imageUrl.h"

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <direct.h>
#include <errno.h>

using namespace std;
using namespace cv;

#define IS_FROM_FILES   0
#define IS_FROM_URLS    1


// class ReadData
// Read data according to appropriate type (file or url)
// Save them in Mat type to process them with OpenCV
class ReadData
{
public:
	// Calculate the number of images
	void calcnum();

	// Read data functions
	void readfiledata();
	void readurldata();
	void readdata();

	// access data function
	Mat* getimages();
	int getnum();
	int getflag();

	// Initialization - source, flag, num
	ReadData(const char* _source, int _flag){
		source = _source;
		flag = _flag;
		num = 0;
	}

private:
	// source to get data
	const char* source;
	// flag for data type (IS_FROM_FILES : 0, IS_FROM_URLS : 1)
	int flag;
	// number of data
	int num;
	// Images that user put is saved in this array
	Mat* images;
};



// class ColorGroup
// classify the images by color and save them in appropriate color folder
class ColorGroup
{
public :
	// start to classify
	void run();

	// Initialization - images, num, flag
	ColorGroup(Mat* _images, int _num, int _flag){
		images = _images;
		num = _num;
		flag = _flag;
	}
private :
	// images array
	Mat* images;
	// number of images
	int num;
	// flag for data type (IS_FROM_FILES : 0, IS_FROM_URLS : 1)
	int flag;
	// result array after classification
	string* colors;
};

// class BrightnessGroup
// classify the images by brightness and save them in appropriate brightness folder
class BrightnessGroup
{
public :
	// start to classify
	void run();
	// make clusters with brightness value
	void k_means();

	// Initialization - images, num, flag
	BrightnessGroup(Mat* _images, int _num, int _flag){
		images = _images;
		num = _num;
		flag = _flag;
	}

private : 
	// images array
	Mat* images;
	// number of images
	int num;
	// flag for data type (IS_FROM_FILES : 0, IS_FROM_URLS : 1)
	int flag;
	// number of cluster that user type
	int numcluster;
	// result arrays after classification
	float* brightness;
	int* clusters;
};


