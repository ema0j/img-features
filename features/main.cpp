#include <stdio.h>
#include <iostream>
#include "grouping.h"

using namespace std;


int main(int argc, char *argv[])
{
	// Features detection and classification
	// Here, user can choose classification by color or by brightness.
	// More features can be added.
	cout << "Features detection and classification" << endl;
	cout << "1. by color" << endl;
	cout << "2. by brightness" << endl;
	cout << endl;

	string source;
	int result, data, ans;
	
	// Choose the classification (color, brightness, ...)
	cout << "Choose the number : ";
	cin >> ans;
	cout << endl;

	// User can choose the type of data among file or image url
	cout << "choose the type of data (0: images folder, 1: images urls) : ";
	cin >> data; cout << endl;

	// ex : file folder path(./images) or urls file name(./urls/urls.txt)
	cout << "Type folder or urls file : ";
	cin >> source; cout<<endl;

	// read data from the source and save them in Mat type
	ReadData r = ReadData(source.c_str(), data);
	r.readdata();

	// Classification by color
	if( ans == 1 ){
		// set colorgroup and run
		ColorGroup cg = ColorGroup(r.getimages(), r.getnum(), r.getflag());
		cg.run();
		cout << "Colorgroup DONE" << endl;
	// Classification by brightness
	}else if( ans == 2 ){
		// set brightnessgroup and run
		BrightnessGroup bg = BrightnessGroup(r.getimages(), r.getnum(), r.getflag());
		bg.run();
		cout << "Brightnessgroup DONE" << endl;
	}
	
	cout << "Features DONE" << endl;
	return 0;
}