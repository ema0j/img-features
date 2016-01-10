#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include "opencv/cv.h"

#include "Poco/URIStreamOpener.h"
#include "Poco/StreamCopier.h"
#include "Poco/Path.h"
#include "Poco/URI.h"
#include "Poco/Exception.h"
#include "Poco/Net/HTTPStreamFactory.h"

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;
using Poco::URIStreamOpener;
using Poco::StreamCopier;
using Poco::Path;
using Poco::URI;
using Poco::Exception;
using Poco::Net::HTTPStreamFactory;

static bool factoryLoaded = false;

class webImageLoader {
   public :
	   Mat loadFromURL(string url)
	   {
		   //Don't register the factory more than once
           if(!factoryLoaded){
			   HTTPStreamFactory::registerFactory();
               factoryLoaded = true;
           }
 
           //Specify URL and open input stream
           URI uri(url);
           auto_ptr<istream> pStr(URIStreamOpener::defaultOpener().open(uri));

           //Copy image to our string and convert to Mat
           string str;
           StreamCopier::copyToString(*pStr.get(), str);
           vector<char> data( str.begin(), str.end() );
           Mat data_mat(data);
           Mat image(imdecode(data_mat,1));
           return image;
        }
};

Mat stringtoMat(string file);