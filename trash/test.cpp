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

    unsigned char* read(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("Failed to open BMP file: " + filename);
        }

        file.read((char*)&bmp_file_header, sizeof(bmp_file_header));

        std::cout<<bmp_file_header.size << " size of header";

        int all_bytes = bmp_file_header.file_size - bmp_file_header.offset_data;
        std::cout<< all_bytes<< " old ";

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

        file.close();
        return pixels;
    }


    
};


int main() {
	BMP_transformator image;
	unsigned char* source = image.read("witcher.bmp");
    delete[] source;
    return 0;
}
