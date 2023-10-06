#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdint>


#pragma pack(push, 1)

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


struct BMP_transformator {
    BMPFileHeader bmp_file_header;

    // reading the file

    unsigned char* read(const char* filename)
    {
        FILE* bytes_img = fopen(filename, "rb");
        if (bytes_img)
        {
            size_t check_size = fread(&bmp_file_header, sizeof(char), sizeof(BMPFileHeader), bytes_img);
            if (check_size == sizeof(BMPFileHeader))
            {
                std::cout << check_size << " = size of header\n"<< ((4 - 3 * bmp_file_header.width % 4) % 4) << " = Padding\n";

                int all_bytes = (3 * bmp_file_header.width + ((4 - 3 * bmp_file_header.width % 4) % 4))*bmp_file_header.height;
                unsigned char* pixels = new unsigned char[all_bytes];
                size_t check_bytes = fread(pixels, sizeof(char), all_bytes, bytes_img);

                if (check_bytes == all_bytes)
                {
                    fclose(bytes_img);
                    return pixels;
                }
                else {
                    delete[] pixels;
                    fclose(bytes_img);
                    std::cout << check_bytes << " - read, " << all_bytes << " bytes should be\n";
                    std::cout << "Error reading pixel data\n";
                    return 0;
                }
            }
		    else {
                fclose(bytes_img);
                std::cout << "Error while reading BMPFileHeader!\n";
                std::cout << check_size << " = size of header\n"<<"Should be: "<< sizeof(BMPFileHeader);
                return 0;
		    }
        }
        else {
            std::cout << "Unable to open file\n";
			return 0;
        }
    }

    // writing the file

    unsigned char* write(const char* filename, BMPFileHeader bmp_file_header, unsigned char* data)
    {
		FILE* write_img = fopen(filename, "wb");
		if (write_img)
        {
            size_t check_header = fwrite(&bmp_file_header, sizeof(char), sizeof(BMPFileHeader), write_img);
            if (check_header == sizeof(BMPFileHeader))
            {
                int all_bytes =(3 * bmp_file_header.width + ( 4 - 3 * bmp_file_header.width % 4) % 4)* bmp_file_header.height;

                size_t check_write = fwrite(data, sizeof(char), all_bytes, write_img);
                if (check_write == all_bytes)
                {
                    std::cout << "Success\n";
                    fclose(write_img);
                    return 0;
                }
                else{
                    std::cout << check_write << "bytes were written" << all_bytes << "Should be written\n";
                    fclose(write_img);
                    return 0;
                }
            }
            else{
                fclose(write_img);
                std::cout << "Header is written wrongly\n";
                return 0;
            }
        }
        else{
            std::cout << "File wasn't opened\n";
            return 0;
		}
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
                new_data[3 * count + new_p * x] = data[(3 * w + old_p) * h - (y) * (3*w+old_p) + (3 * x)];
                new_data[3 * count + new_p * x + 1] = data[(3 * w + old_p) * h - (y) * (3*w+old_p) + (3 * x) + 1];
                new_data[3 * count + new_p * x + 2] = data[(3 * w + old_p) * h - (y) * (3*w+old_p) + (3 * x) + 2];

                count++;
            }
        }

		return new_data;
	}

    unsigned char* turn_right(BMPFileHeader bmp_file_header, unsigned char* data)
	{
		int w = bmp_file_header.width;
        int h = bmp_file_header.height;

        //calculate old and  new padding of image
        int old_p = (4 - 3 * w % 4) % 4;
        int new_p = (4 - 3 * h % 4) % 4;

        unsigned char* new_data = new unsigned char[(3 * h + new_p) * w];
        int count = 0;

        //transform image
        for (int x = 0; x < w; x++)
        {
            for (int y = h; y > 0; y--)
            {
                new_data[3 * count + new_p * x] = data[(3 * w + old_p) * h - (y) * (3*w+old_p) + (3 * x)];
                new_data[3 * count + new_p * x + 1] = data[(3 * w + old_p) * h - (y) * (3*w+old_p) + (3 * x) + 1];
                new_data[3 * count + new_p * x + 2] = data[(3 * w + old_p) * h - (y) * (3*w+old_p) + (3 * x) + 2];

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
                double r = 0, g = 0, b = 0;
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
                //calculate the weighted average and assign it to every pixel in new_data
                new_data[3 * (w * y + x) + padding * y] = (int) r / kernel_sum;
                new_data[3 * (w * y + x) + padding * y + 1] = (int) g / kernel_sum;
                new_data[3 * (w * y + x) + padding * y + 2] = (int) b / kernel_sum;
                }
            }
    return new_data;
    }
};

int main() {
	BMP_transformator image;
	unsigned char* source = image.read("witcher.bmp");
    BMPFileHeader header = image.bmp_file_header;
    std::swap(header.width, header.height);
    BMPFileHeader swap_header = header; //a new header is needed to create a picture with changed dimensions
    std::swap(header.width, header.height);

	unsigned char* left_data = image.turn_left(header, source);
	image.write("Turned_left.bmp", swap_header, left_data);
	delete[] left_data;


    unsigned char* right_data = image.turn_right(header, source);
    image.write("Turned_right.bmp", swap_header, right_data);
    delete[] right_data;

    unsigned char* gaus_data = image.gausian_blur(header, source, 1.75);
    image.write("gaus.bmp", header, gaus_data);
    delete[] gaus_data;


    delete[] source;
    return 0;
}
