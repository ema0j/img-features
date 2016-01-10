#include "grouping.h"

// maximum number of file name
const int FILE_NAME_MAX = 60;
// groups for the classification by color
string color_match[12] = { "red", "orange", "yellow", "yellow green",
						   "green", "bluish green", "cyan", "navy blue",
						   "blue", "purple", "magenta", "pink" };


string mostUsedColor(Mat h_hist, int hHistSize);
void makecolorfolder();
void makebrightnessfolder(int numfolder);
float avgBrightness(Mat hist, int histSize);
void k_means();


// Gets int type n and returns it after chaging int type to string type 
string intToString(int n)
{
	stringstream s;
	s << n;
	
	return s.str();
}

// Used in K-Means algorithm
// return true if at least one of changed array, return false if all are false
int check(bool *changed, int num){

	int i;
	for( i=0; i<num; i++){
		if ( changed[i] == true )
			return true;	
	}
	return false;
}


// Calculate the number of images
void ReadData::calcnum(){
	// when the type of data is files
	if( flag == IS_FROM_FILES ){
		DIR *dir;
		struct dirent *ent;

		// Open directory and count the number of files in the directory
		// If there is no more file to read, ent is changed to NULL
		if ((dir = opendir (source)) != NULL) {
		  while ((ent = readdir (dir)) != NULL) {
			if(ent == NULL){
				break;
			}
			if(strncmp(ent->d_name, ".", 1)==0 || strncmp(ent->d_name, "..", 2)==0){
				continue;
			}
			num ++;
		  }
		  closedir (dir);
		} else {
		  // Could not open directory
		  perror ("fail to get list of this directory");
		}
	// when the type of data is files
	}else if( flag == IS_FROM_URLS ){
		ifstream urlfile(source);
		string url;

		// Open file and count the number of lines in the file
		if(urlfile.is_open()){
			while(getline(urlfile, url))
				num++;
		}
		urlfile.close();
	}
	
}


// Read data main function
// After getting the number of images, run read data function along data type 
void ReadData::readdata(){
	calcnum();
	if( flag == IS_FROM_FILES )
		ReadData::readfiledata();
	else if( flag == IS_FROM_URLS )
		ReadData::readurldata();

}

// Read data from files which is in the folder given by user
// Save them in Mat type
void ReadData::readfiledata(){
	DIR *dir;
	struct dirent *ent;
	// Set the size of images array with the number of data
	images = new Mat[num];

	int cnt = 0;
	ent = NULL;

	// Make the file "filenames.txt"  
	// to save the filenames and re-use them when making new files for classification
	ofstream filenamesave("./filenames.txt");

	// Open directory and count the number of files in the directory
	// If there is no more file to read, ent is changed to NULL
	if ((dir = opendir (source)) != NULL) {
		while ((ent = readdir(dir)) != NULL){
			if(ent == NULL){
				break;
			}
			if(strncmp(ent->d_name, ".", 1)==0 || strncmp(ent->d_name, "..", 2)==0){
				continue;
			}
			if(cnt == num)
				break;
			
			// set full path of image file
			char fullname[FILE_NAME_MAX] = "";	
			strcat(fullname, source);
			strcat(fullname, "/");
			strcat(fullname, ent->d_name);

			// read the file and save it in Mat type
			Mat image = imread(fullname, 1);
			images[cnt].create(image.rows, image.cols, CV_8UC1);
			images[cnt] = image;
			cout << "imread done " << cnt << endl;
			cnt++;

			// save the file name, not full name, in the "filenames.txt"
			filenamesave << ent->d_name << "\n";
		}
		filenamesave.close();
		closedir(dir);
	}else{
		perror ("fail to get list of this directory");
	}
	cout << "done read files" << endl;
}

// Read data from urls which is in the file given by user
// Save them in Mat type
void ReadData::readurldata(){

	int cnt=0;
	string url="";
	ifstream urlfile(source);

	// Set the size of images array with the number of data
	images = new Mat[num]; 

	// open the file and read the url in line by line
	ifstream urlf(source);
	if(urlf.is_open()){
		while(getline(urlf, url)){
			Mat image;
			cout << url << endl;
			// get image from url and save it in Mat type
			image = stringtoMat(url);
			images[cnt].create(image.rows, image.cols, CV_8UC1);
			images[cnt] = image;
			cnt++;
		}
	}

	urlfile.close();

}

// return images
Mat* ReadData::getimages(){
	return images;
}

// return num
int ReadData::getnum(){
	return num;
}

// return flag
int ReadData::getflag(){
	return flag;
}



// main function in class ColorGroup
// analyze and classify images
void ColorGroup::run(){
	int i;
//	colors = (string*)malloc(sizeof(string)*num);
	// temporary buffer (additional function that changes the size of buffer is needed)
	string colors[100];
	// make the folder to save the result
	makecolorfolder();

	// for each image,
	for(i=0; i<num; i++){
		Mat image, hsv_image;

		image = images[i]; 

		// extract color element of the image
		// if the images doesn't have 3 channels, it's classified to 'etc'
		if (image.channels() == 3){

			// change the mode of image from RGB to HSV
			cvtColor(image, hsv_image, CV_BGR2HSV);

			// Separate the image in 3 places ( H, S, V )
			vector<Mat> hsv_planes;

			split( hsv_image, hsv_planes );

			// make histogram with color element ('h' in hsv means 'hue')
			int hHistSize = 180;
			float hRange[] = {0, 180}, vRange[] = {0, 100};
			const float* hHistRange = { hRange };

			Mat h_hist;

			calcHist( &hsv_planes[0], 1, 0, Mat(), h_hist, 1, &hHistSize, &hHistRange, true, false);

			int hHist_w = 360; int hHist_h = 400;
			int hbin_w = cvRound( (double) hHist_w/hHistSize);

			Mat hHistImage( hHist_h, hHist_w, CV_8UC3, Scalar( 0, 0, 0) );

			normalize(h_hist, h_hist, 0, hHistImage.rows, NORM_MINMAX, -1, Mat() );

			// with histogram of color, get the most used color
			colors[i] = mostUsedColor(h_hist, hHistSize);
			cout << "most used color is " << colors[i] << endl;

		} else {
			colors[i] = "etc";
		}
	}

	// with the color array result, classify and save images in appropriate folder
	if( flag == IS_FROM_FILES ){
		
		int i=0;
		string filename;

		// open "filenames.txt" and get the name of file
		ifstream file;
		file.open("filenames.txt");
		
		// save the images in appropriate folder which is the color array result
		while(!file.eof() && i<num ){
			getline(file, filename);

			char newfile[FILE_NAME_MAX+20] = "./color_result/";
			strcat(newfile, colors[i].c_str());
			strcat(newfile, "/");
			strcat(newfile, filename.c_str());

			cout << "new file adress : " << newfile << endl;
			imwrite(newfile, images[i]);
			i++;
		}

	}else if( flag == IS_FROM_URLS ){
		
		int i;

		// make the file name to save. It counts from 0 in increasing order
		// and save the images in appropriate folder which is the color array result
		const char file[10] = "photo_";
		for( i=0; i<num; i++){
			char newfile[FILE_NAME_MAX+20] = "./color_result/";

			strcat(newfile, colors[i].c_str());
			strcat(newfile, "/");
			strcat(newfile, file);
			if( i < 10 ){
				strcat(newfile, "00");
				strcat(newfile, intToString(i).c_str());
			}else if ( i >= 10 && i < 100 ){
				strcat(newfile, "0");
				strcat(newfile, intToString(i).c_str());
			}else if ( i >= 100 && i < 1000 ){
				strcat(newfile, intToString(i).c_str());
			}
			cout << newfile << endl;
			strcat(newfile, ".jpg");
			imwrite(newfile, images[i]);
		}

	}

	cout << "color grouping finished" << endl;

}

// calculate the most used color from hue histogram
string mostUsedColor(Mat hist, int histSize){

	int i, x, tmp, color_max, idx=0, w_bin = cvRound(180/histSize);
	int color_table[12] = {0};

	// divide whole range of color to 12 section
	// and accumulate the figure of color histogram in each section
	for( i=0; i<histSize; i++){
		tmp = cvRound(hist.at<float>(i-1));
		x = i*w_bin;

		// RED
		if( (x >= 0.0 && x < 7.5) || (x >= 172.5 && x < 180) )
			color_table[0] = color_table[0] + tmp;
		// ORANGE
		else if( x >= 7.5 && x < 22.5 )
			color_table[1] = color_table[1] + tmp;
		// YELLOW
		else if( x >= 22.5 && x < 37.5 )
			color_table[2] = color_table[2] + tmp;
		// YELLOW GREEN
		else if( x >= 37.5 && x < 52.5 )
			color_table[3] = color_table[3] + tmp;
		// GREEN
		else if( x >= 52.5 && x < 67.5 )
			color_table[4] = color_table[4] + tmp;
		// BLUISH GREEN
		else if( x >= 67.5 && x < 82.5 )
			color_table[5] = color_table[5] + tmp;
		// CYAN
		else if( x >= 82.5 && x < 97.5 )
			color_table[6] = color_table[6] + tmp;
		// NAVY BLUE
		else if( x >= 97.5 && x < 112.5 )
			color_table[7] = color_table[7] + tmp;
		// BLUE
		else if( x >= 112.5 && x < 127.5 )
			color_table[8] = color_table[8] + tmp;
		// PURPLE
		else if( x >= 127.5 && x < 142.5 )
			color_table[9] = color_table[9] + tmp;
		// MAGENTA
		else if( x >= 142.5 && x < 157.5 )
			color_table[10] = color_table[10] + tmp;
		// PINK
		else if( x >= 157.5 && x < 172.5 )
			color_table[11] = color_table[11] + tmp;
	}

	// get the maximum among color_table vaule, which is the most used color in the image
	for( i=1; i<12; i++)
		if( color_max < color_table[i]){
			color_max = color_table[i];
			idx = i;
		}
		
	return color_match[idx];
}

// make result folder which is used to put image files after classification
void makecolorfolder(){
	int i, result;
	char *folderpath = "./color_result";

	// make upper folder "color_result"
	result = mkdir( folderpath );
	if(result == -1){
		perror("fail ");
		cout << "errorno : " << errno << endl;
	}

	// make sub-folder with the names of color match
	for ( i=0; i< sizeof(color_match)/sizeof(color_match[0]); i++){
		char subfolderpath[30] = "./color_result/";

		strcat(subfolderpath, color_match[i].c_str());

		result = mkdir( subfolderpath );
		if(result == -1){
			perror("fail ");
			cout << "errorno : " << errno << endl;
		}
	}

	// make another sub-folder "etc" for non-3 channel images
	result = mkdir( "./color_result/etc" );
	if( result == -1){
		perror("fail ");
		cout << "errorno : " << errno << endl;
	}
}

// main function in class BrightnessGroup
// analyze and classify images
void BrightnessGroup::run(){
	int i;
	
	// get the number of clusters from user to set the number of group 
	cout << "How many clusters? ";
	cin >> numcluster; cout << endl;

	// make the folder to save the result
	makebrightnessfolder( numcluster );
	// set the size of brightness array to the number of images
	brightness = (float*)malloc(sizeof(float)*num);

	// for each image,
	for( i=0; i<num; i++){
		Mat image, hsv_image;
		image = images[i]; 

		// extract color element of the image
		// if the images doesn't have 3 channels, it's classified to '-1'
		if (image.channels() == 3){

			// change the mode of image from RGB to HSV
			cvtColor(image, hsv_image, CV_BGR2HSV);

			// Separate the image in 3 places ( H, S, V )
			vector<Mat> hsv_planes;

			split( hsv_image, hsv_planes );

			// make histogram with brightness element ('v' in hsv means 'value')
			int vHistSize = 100;
			float vRange[] = {0, 100};
			const float* vHistRange = { vRange };

			Mat v_hist;
		
			calcHist( &hsv_planes[0], 1, 0, Mat(), v_hist, 1, &vHistSize, &vHistRange, true, false);

			int vHist_w = 300; int vHist_h = 400;
			int vbin_w = cvRound( (double) vHist_w/vHistSize);

			Mat vHistImage( vHist_h, vHist_w, CV_8UC3, Scalar( 0, 0, 0) );

			normalize(v_hist, v_hist, 0, vHistImage.rows, NORM_MINMAX, -1, Mat() );

			// with histogram of brightness, get the average of the brightness
			brightness[i] = avgBrightness(v_hist, vHistSize);
			cout << "The average of brightness is " << brightness[i] << endl;
		}else{
			brightness[i] = -1;
		}
		cout << "next, go to other picture..." << endl;
	}

	// with the average of brightness data, 
	// let the image get its own group by k means clustering
	k_means();

	// with the clusters array result, classify and save images in appropriate folder
	if( flag == IS_FROM_FILES ){
		
		int i=0;
		string filename;

		// open "filenames.txt" and get the name of file
		ifstream file;
		file.open("filenames.txt");
		
		// save the images in appropriate folder which is the clusters array result
		while(!file.eof() && i<num ){
			getline(file, filename);

			char newfile[FILE_NAME_MAX+20] = "./brightness_result/";
			strcat(newfile, intToString(clusters[i]).c_str());
			strcat(newfile, "/");
			strcat(newfile, filename.c_str());

			cout << "new file adress : " << newfile << endl;
			imwrite(newfile, images[i]);
			i++;
		}

	}else if( flag == IS_FROM_URLS ){
		
		int i;

		// make the file name to save. It counts from 0 in increasing order
		// and save the images in appropriate folder which is the color array 
		const char file[10] = "photo_";
		for( i=0; i<num; i++){
			char newfile[FILE_NAME_MAX+20] = "./brightness_result/";

			strcat(newfile, intToString(clusters[i]).c_str());
			strcat(newfile, "/");
			strcat(newfile, file);
			if( i < 10 ){
				strcat(newfile, "00");
				strcat(newfile, intToString(i).c_str());
			}else if ( i >= 10 && i < 100 ){
				strcat(newfile, "0");
				strcat(newfile, intToString(i).c_str());
			}else if ( i >= 100 && i < 1000 ){
				strcat(newfile, intToString(i).c_str());
			}
			cout << newfile << endl;
			strcat(newfile, ".jpg");
			imwrite(newfile, images[i]);
		}

	}

	cout << "brightness grouping finished" << endl;

}



// make result folder which is used to put image files after classification
// for brightness, the folders are made as many as clusters
void makebrightnessfolder(int numfolder){
	int i, result;
	char *folderpath = "./brightness_result";

	// make upper folder "brightness_result"
	result = mkdir( folderpath );
	if(result == -1){
		perror("fail ");
		cout << "errorno : " << errno << endl;
	}

	// make sub-folder with the names of number from 0 to the number of clusters
	for ( i=0; i<numfolder; i++){
		char subfolderpath[30] = "./brightness_result/";

		strcat(subfolderpath, intToString(i).c_str());
		cout << subfolderpath << endl;

		result = mkdir( subfolderpath );
		if(result == -1){
			perror("fail ");
			cout << "errorno : " << errno << endl;
		}
	}

	// make another sub-folder "-1" for non-3 channel images
	result = mkdir( "./brightness_result/-1" );
	if( result == -1){
		perror("fail ");
		cout << "errorno : " << errno << endl;
	}

}

// calculate the average brightness from value histogram
float avgBrightness(Mat hist, int histSize){

	float sum = 0.0;
	float n = 0.0;
	// calculate the average brightness with the whole elements of the image
	for(int i=0; i<histSize; i++){
		sum += i*cvRound(hist.at<float>(i));
		n += cvRound(hist.at<float>(i));
	}
	float avg = sum/n;

	return avg;
}

// K-means algorithm with 1D data.
// let the image get its own group by k means clustering,
// with the average of brightness data
void BrightnessGroup::k_means(){

	cout << "start k means" << endl;	

	int i, j, beforecluster;
	float tmp;
	float sum = 0.0; int count = 0;
	// check if the value of centers are changed or not
	bool *changed = (bool*)malloc(sizeof(bool)*num);
	// save center value
	float *centers = (float*)malloc(sizeof(float)*numcluster);
	// set the size of clusters
	clusters = (int*)malloc(sizeof(int)*num);

	cout << endl;
	//cout << "for check : \n";
	//for(int i=0; i<num; i++)
	//	cout << i << " : " << data[i] << endl;

	// first, initialize the centers with arbitrary brightness value
	for(i=0; i<numcluster; i++)
		centers[i] = brightness[i];
	for(i=0; i<num; i++)
		changed[i] = true;

	// while the cluster is not changed
	while (check(changed, num) ){
		for(i=0; i<num; i++){
			// if brightness is negative, it menas that the image doesn't have 3 channels
			// it's not necessary to be considered
			if(brightness[i] < 0){
				//cout << i << " data check " << brightness[i] << endl;
				clusters[i] = -1;
				changed[i] = false;
			}else{
				beforecluster = clusters[i];
				tmp = abs(brightness[i] - centers[0]);
				clusters[i] = 0;
				for(j=1; j<numcluster; j++){
					if( tmp  > abs(brightness[i] - centers[j]) ){
						tmp = abs(brightness[i] - centers[j]);
						clusters[i] = j;
					}
				}
				//cout << "cluster of " << i << " data : " << clusters[i] << endl;
				// check if the cluster is changed or not
				if(beforecluster == clusters[i])
					changed[i] = false;
				else
					changed[i] = true;
			}
		}
		// recalculate k centers
		for(i=0; i<numcluster; i++){
			sum = 0.0;
			count = 0;
			for(j=0; j<num; j++){
				if( clusters[j] == i){
					sum += brightness[j];
					count++;
				}
			}
			centers[i] = (float)sum/count;
			//cout << " center " << centers[i];
		}
//		cout << endl;
	}

	// print fixed center value
	cout << "center : : " << endl;
	for(i=0; i<numcluster; i++)
		cout << centers[i] << " ";
	cout << endl << endl;

	//cout << "clusters : ";
	//for(i=0; i<num; i++)
	//	cout << i << " gets cluster " << clusters[i] << endl;

	cout << "the end of k means" << endl;

}