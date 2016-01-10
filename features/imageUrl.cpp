#include "imageUrl.h"

Mat stringtoMat(string file)
{
    Mat image;

    if(file.compare(file.size()-4,4,".gif")==0)
    {
        cerr<<"UNSUPPORTED_IMAGE_FORMAT";
        return image;
    }
 
	else if(file.compare(0,7,"http://")==0)  // Valid URL only if it starts with "http://"
    {
        webImageLoader loader;
        image = loader.loadFromURL(file);
        if(!image.data)
            cerr<<"INVALID_IMAGE_URL";
        return image;
    }
	else if(file.compare(0,8,"https://")==0) // Valid URL only if it starts with "https://"
	{
		webImageLoader loader;
		string newfile = "http://"+file.substr(8);
		image = loader.loadFromURL(newfile);
		if(!image.data)
			cerr<<"INVALID_IMAGE_URL";
		return image;
	}
    else
    {
        image = imread(file,1); // Try if the image path is in the local machine
        if(!image.data)
			cerr<<"IMAGE_DOESNT_EXIST";
            return image;
     }

}
