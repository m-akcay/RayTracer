#include "Image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>

Image::Image(uint width, uint height)
    : width(width), height(height)
{
    data = new vec3*[height];

    for (int y = 0; y < height; ++y)
    {
        data[y] = new vec3[width];
    }
    
    pixels = std::vector<vec3>(width * height, vec3(0.0f));
}

Image::~Image()
{
    for (size_t i = 0; i < height; i++)
    {
       delete[] data[i];
        
    }

    delete[] data;

}

//
// Set the value of the pixel at the given column and row
//
void Image::setPixelValue(uint col, uint row, const vec3& color)
{
    data[row][col] = color;
}

void Image::setPixelValue(size_t idx, const vec3& color)
{
    pixels[idx] = color;
}


/* Takes the image name as a file and saves it as a ppm file. */
void Image::saveImage(const char *imageName) const
{
    u_char d[3 * width * height];
    size_t index = 0;    
    for(int y = 0 ; y < height; y++)
	{
		for(int x = 0 ; x < width; x++)
        {
            d[index++] = data[y][x].r;
            d[index++] = data[y][x].g;
            d[index++] = data[y][x].b;
        }
	}
    stbi_write_png(imageName, width, height, 3, d, 3 * width);
    // std::cout << "HERE" << std::endl;
    // for(int y = 0 ; y < height; y++)
	// {
	// 	for(int x = 0 ; x < width; x++)
    //     {
    //         d[index++] = pixels[y * height + x].r;
    //         d[index++] = pixels[y * height + x].g;
    //         d[index++] = pixels[y * height + x].b;

    //     }
	// }
    // stbi_write_png(imageName, width, height, 3, d, 3 * width);

}
