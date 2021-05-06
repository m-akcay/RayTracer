#ifndef _IMAGE_H_
#define _IMAGE_H_

#include "../Includes.h"

class Image
{
public:
	int width;						
	int height;						
    vec3 **data;
	std::vector<vec3> pixels;

	Image(uint width, uint height);	
	~Image();

    void setPixelValue(uint col, uint row, const vec3& color);  
    void setPixelValue(size_t idx, const vec3& color);  
	void saveImage(const char *imageName) const;
};

#endif
