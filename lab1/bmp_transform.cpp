#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include "bmp.h"


int BMP::read(const std::string& filename){
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open BMP file: " + filename);
    }

    file.read(reinterpret_cast<char*>(&bmp_file_header), sizeof(bmp_file_header));

    std::cout<<bmp_file_header.size << " size of header\n";

    int w = bmp_file_header.width;
    int h = bmp_file_header.height;
    int p = (4 - w * 3 % 4) % 4;

    int all_bytes = (3 * w + p) * h;

    unsigned char* new_data = new unsigned char[all_bytes];

    file.read(reinterpret_cast<char*>(new_data), all_bytes);

    if (file.bad()) {
        delete[] new_data;
        throw std::runtime_error("Failed to read BMP file: " + filename);
        return 0;
    }

    if (!file.eof() && file.fail()) {
        delete[] new_data;
        throw std::runtime_error("data format error: " + filename);
        return 0;
    }

    file.close();
    BMP::pixel_data = new_data;

    return 0;
}

int BMP::write(const std::string& filename){
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open BMP file: " + filename);
    }
    file.write(reinterpret_cast<char*>(&bmp_file_header), sizeof(bmp_file_header));

    if (file.bad()) {
        throw std::runtime_error("Failed to write header: " + filename);
        return 0;
    }

    if (!file.eof() && file.fail()) {
        throw std::runtime_error("data format error: " + filename);
        return 0;
    }

    int w = bmp_file_header.width;
    int h = bmp_file_header.height;
    int p = (4 - bmp_file_header.width * 3 % 4) % 4;
    int all_bytes =(3 * w + p) * h;
    file.write(reinterpret_cast<char*>(pixel_data), all_bytes);

    if (file.bad()) {
    throw std::runtime_error("Failed to write data: " + filename);
    return 0;
    }

    if (!file.eof() && file.fail()) {
        throw std::runtime_error("data format error: " + filename);
        return 0;
    }

    std::cout << "Success\n";
    file.close();
    return 0;
}

BMP BMP::turn_left(){

    BMPFileHeader header = bmp_file_header;

    std::swap(header.width, header.height);
    int w = header.width;
    int h = header.height;

    //calculating old and  new padding of image
    int new_p = (4 - 3 * w % 4) % 4;
    int old_p = (4 - 3 * h % 4) % 4;

    unsigned char* new_data = new unsigned char[(3 * w + new_p) * h];

    for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			int pos_from = ((w - 1 - x) * h + y) * 3 + (w - 1 - x) * old_p;
			int pos_to = (y * w + x) * 3 + y * new_p;
			std::copy_n(pixel_data + pos_from, 3, new_data + pos_to);
		}
	}
	return BMP(header, new_data);
}


BMP BMP::turn_right(){
    BMPFileHeader header = bmp_file_header;

    std::swap(header.width, header.height);
    int w = header.width;
    int h = header.height;

    //calculating old and  new padding of image
    int new_p = (4 - 3 * w % 4) % 4;
    int old_p = (4 - 3 * h % 4) % 4;

    unsigned char* new_data = new unsigned char[(3 * w + new_p) * h];

for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			int pos_from = (x * h + h - 1 - y) * 3 + x * old_p;
			int pos_to = (y * w + x) * 3 + y * new_p;
			std::copy_n(pixel_data + pos_from, 3, new_data + pos_to);
		}
	}
	return BMP(header, new_data);
}

BMP BMP::gaussian_blur(double sigma){
    int w = bmp_file_header.width;
    int h = bmp_file_header.height;
    int padding = (4 - 3 * w % 4) % 4;

    double kernel[7][7];
    double kernel_sum = 0.0;
    double weight;

    // create gaussian kernel to calculate weights of red green and blue masks of every pixel 
    for (int j=-3; j<4; j++){
        for (int i = -3; i < 4; i++){

            weight = (1.0 / (2.0 * M_PI * sigma * sigma)) * exp(-(i*i + j*j) / (2.0 * sigma * sigma));
            kernel[i + 3][j + 3] = weight;
        }   
    }
    unsigned char* new_data = new unsigned char[(3 * bmp_file_header.width + padding) * bmp_file_header.height];
    std::copy_n(pixel_data+0, (3 * bmp_file_header.width + padding) * bmp_file_header.height, new_data+0);


    // // calculate the weighted average of the surrounding pixels for each pixel in the picture
    for (int y = 0; y < h; y++){
        for (int x = 0; x < w; x++){
            int xywh[4]={x, y, w, h};
            std::vector<double> rgb= rgb_gauss_sum(xywh, pixel_data, padding, kernel);
            int r = rgb[0]/kernel_sum;
            int g = rgb[1]/kernel_sum;
            int b = rgb[2]/kernel_sum;
            new_data[3 * (w * y + x) + padding * y] = r;
            new_data[3 * (w * y + x) + padding * y + 1] = g;
            new_data[3 * (w * y + x) + padding * y + 2] = b;
            }
        }

    return BMP(bmp_file_header,  new_data);
}

std::vector<double> BMP::rgb_gauss_sum(int (&xywh)[4], unsigned char *data, int padding, double (&kernel)[7][7]){
    double r=0, g=0, b=0;
    int count = 0;
    std::vector<double> rgb = {0,0,0};
    for (int j = -3; j < 4; j++){
        for (int i = -3; i < 4; i++){
            int pix_x = xywh[0] + i;
            int pix_y = xywh[1] + j;
            if ( !((0 <= pix_x) and (pix_x < xywh[2]) and (0 <= pix_y) and (pix_y < xywh[3])) ){
                pix_x = xywh[0];
                pix_y = xywh[1];     
            }
            count+=1;
            // calculate summ
            r += data[3 * (xywh[2] * pix_y + pix_x) + padding * pix_y] * kernel[i+3][j+3];
            g += data[3 * (xywh[2] * pix_y + pix_x) + padding * pix_y + 1] * kernel[i+3][j+3];
            b += data[3 * (xywh[2] * pix_y + pix_x) + padding * pix_y + 2] * kernel[i+3][j+3];
            }
    }
    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;

    return rgb;
}