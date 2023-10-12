#pragma once
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdint>
#include <algorithm>



struct BMPFileHeader {
    uint16_t file_type{0x4D42};          // File type always BM which is 0x4D42
    uint32_t file_size{0};               // Size of the file (in bytes)
    uint16_t reserved1{0};               // Reserved, always 0
    uint16_t reserved2{0};               // Reserved, always 0
    uint32_t offset_data{0};             // Start position of pixel data (bytes from the beginning of the file)


    uint32_t size{ 0 };                      // Size of this header (in bytes)
    int32_t width{ 0 };                      // width of bitmap in pixels
    int32_t height{ 0 };                     // width of bitmap in pixels
                                              //       (if positive, bottom-up, with origin in lower left corner)
                                              //       (if negative, top-down, with origin in upper left corner)
    uint16_t planes{ 1 };                    // No. of planes for the target device, this is always 1
    uint16_t bit_count{ 0 };                 // No. of bits per pixel
    uint32_t compression{ 0 };               // 0 or 3 - uncompressed. THIS PROGRAM CONSIDERS ONLY UNCOMPRESSED BMP images
    uint32_t size_image{ 0 };                // 0 - for uncompressed images
    int32_t x_pixels_per_meter{ 0 };
    int32_t y_pixels_per_meter{ 0 };
    uint32_t colors_used{ 0 };               // No. color indexes in the color table. Use 0 for the max number of colors allowed by bit_count
    uint32_t colors_important{ 0 };          // No. of colors used for displaying the bitmap. If 0 all colors are required


     uint32_t red_mask{ 0x00ff0000 };         // Bit mask for the red channel
     uint32_t green_mask{ 0x0000ff00 };       // Bit mask for the green channel
     uint32_t blue_mask{ 0x000000ff };        // Bit mask for the blue channel
     uint32_t alpha_mask{ 0xff000000 };       // Bit mask for the alpha channel
     uint32_t color_space_type{ 0x73524742 }; // Default "sRGB" (0x73524742)
     uint32_t unused[16]{ 0 };                // Unused data for sRGB color space
};

#pragma pack(pop)

double * rgb_gauss_sum(int x, int y, int w, int h, unsigned char *data, int padding, double (&kernel)[7][7]){
    double r=0, g=0, b=0;
    for (int j = -3; j < 4; j++){
        for (int i = -3; i < 4; i++){
            int pix_x = x + i;
            int pix_y = y + j;
            if ( !((0 <= pix_x) and (pix_x < w) and (0 <= pix_y) and (pix_y < h)) ){
                pix_x = x;
                pix_y = y;
            }
            // calculate summ
            r += data[3 * (w * pix_y + pix_x) + padding * pix_y] * kernel[i+3][j+3];
            g += data[3 * (w * pix_y + pix_x) + padding * pix_y + 1] * kernel[i+3][j+3];
            b += data[3 * (w * pix_y + pix_x) + padding * pix_y + 2] * kernel[i+3][j+3];
            }
    }

    double *rgb =new double[3]{r ,g ,b};
    return rgb;
}


struct BMP_transformator {
    BMPFileHeader bmp_file_header;

    unsigned char * pixel_data;

    // reading the file

    unsigned char* read(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("Failed to open BMP file: " + filename);
        }

        file.read((char*)&bmp_file_header, sizeof(bmp_file_header));

        std::cout<<bmp_file_header.size << " size of header\n";

        int all_bytes = bmp_file_header.file_size - bmp_file_header.offset_data;

        // int all_bytes = (3 * bmp_file_header.width + ((4 - 3 * bmp_file_header.width % 4) % 4))*bmp_file_header.height;
        // std::cout<<new_bytes<<" new bytes "<< all_bytes<< " old ";

        unsigned char* pixels = new unsigned char[all_bytes];

        file.read(reinterpret_cast<char*>(pixels), all_bytes);

        if (file.bad()) {
            delete[] pixels;
            throw std::runtime_error("Failed to read BMP file: " + filename);
            return 0;
        }

        if (!file.eof() && file.fail()) {
            delete[] pixels;
            throw std::runtime_error("data format error: " + filename);
            return 0;
        }
        // size_t check_bytes = fread(pixels, sizeof(char), all_bytes, bytes_img);

        file.close();
        
        return pixels;
    }

    // writing the file

    unsigned char* write(const std::string& filename, BMPFileHeader bmp_file_header, unsigned char* data)
    {
		std::ofstream file(filename, std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("Failed to open BMP file: " + filename);
        }
        file.write((char*)&bmp_file_header, sizeof(bmp_file_header));

        if (file.bad()) {
            throw std::runtime_error("Failed to write header: " + filename);
            return 0;
        }

        if (!file.eof() && file.fail()) {
            throw std::runtime_error("data format error: " + filename);
            return 0;
        }

        int all_bytes = bmp_file_header.file_size - bmp_file_header.offset_data;
        file.write(reinterpret_cast<char*>(data), all_bytes);

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



    unsigned char* turn_left(BMPFileHeader bmp_file_header, unsigned char* data)
	{
		int w = bmp_file_header.width;
        int h = bmp_file_header.height;

        //calculating old and  new padding of image
        int old_p = (4 - 3 * w % 4) % 4;
        int new_p = (4 - 3 * h % 4) % 4;

        unsigned char* new_data = new unsigned char[(3 * h + new_p) * w];

        //transform image
        int count = 0;

        for (int x = 0; x < w; x++)
        {
            for (int y = 1; y <= h; y++)
            {
                int new_data_pos = 3 * count + new_p * x;
                int old_data_pos = (3 * w + old_p) * h - (y) * (3*w+old_p) + (3 * x);
                std::copy_n(data + old_data_pos, 3, new_data + new_data_pos);
                count++;
            }
        }

		return new_data;
	}

    unsigned char* turn_right(BMPFileHeader bmp_file_header, unsigned char* data)
	{
		int w = bmp_file_header.width;
        int h = bmp_file_header.height;

        //calculate old and new padding of image
        int old_p = (4 - 3 * w % 4) % 4;
        int new_p = (4 - 3 * h % 4) % 4;

        unsigned char* new_data = new unsigned char[(3 * h + new_p) * w];
        int count =0;

        //transform image
        for (int x = w; x > 0; x--)
        {
            for (int y = h; y > 0; y--)
            {
                int new_data_pos = 3 * count + new_p * (w-x);
                int old_data_pos = (3 * w + old_p) * h - (y) * (3 * w + old_p) + (3 * x);
                std::copy_n(data + old_data_pos, 3, new_data + new_data_pos);
                count++;
            }
        }

		return new_data;
	}

    unsigned char* gausian_blur(BMPFileHeader bmp_file_header, unsigned char* data, double sigma){
        int w = bmp_file_header.width;
        int h = bmp_file_header.height;
        int padding = (4 - 3 * w % 4) % 4;

        double kernel[7][7];
        double kernel_sum = 0.0;
        double weight;

        //create gaussian kernel to calculate weights of red green and blue masks of every pixel 
        for (int j=-3; j<4; j++){
            for (int i = -3; i < 4; i++){
                weight = 1.0 / (2.0 * M_PI * sigma * sigma) * exp(-(i*i + j*j) / (2.0 * sigma * sigma));
                kernel_sum += weight;
                kernel[i + 3][j + 3] = weight;
            }   
        }


        unsigned char* new_data = new unsigned char[(3 * bmp_file_header.width + padding) * bmp_file_header.height];

        // calculate the weighted average of the surrounding pixels for each pixel in the picture
        for (int y = 0; y < h; y++){
            for (int x = 0; x < w; x++){
                double *rgb = rgb_gauss_sum(x, y, w, h, data, padding, kernel);
                new_data[3 * (w * y + x) + padding * y] = (int) rgb[0] / kernel_sum;
                new_data[3 * (w * y + x) + padding * y + 1] = (int) rgb[1] / kernel_sum;
                new_data[3 * (w * y + x) + padding * y + 2] = (int) rgb[2] / kernel_sum;
                delete[] rgb;
                }
            }
    return new_data;
    }
};