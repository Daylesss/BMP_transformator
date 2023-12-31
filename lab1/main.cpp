#include <iostream>
#include "bmp.h"

int main() {
	BMP image = BMP();
	image.read("witcher.bmp");
	
	//Rotating image to the left
	BMP turned_left = image.turn_left();
	turned_left.write("left.bmp");

	// image.write("test.bmp");


	// //Rotating image to the right
	// BMP turned_right = image.turn_right();
	// turned_right.write("right.bmp");

	//Putting on gauss filter
	BMP gaussian_filter = image.gaussian_blur(1.75);
	gaussian_filter.write("gaaaaaaus.bmp");

	return 0;
}
