#ifndef _IMAGE_PLANE_H_
#define _IMAGE_PLANE_H_

#include "Includes.h"

class ImagePlane
{
private:
	const float _left, _right, _top, _bottom;
	const float _distance;
	const vec2 _imgSize;
	const string _name;

public:
	ImagePlane(float left, float right, float bottom, float top, 
				float distance,
				const vec2& imgSize,
				const string& name) 
				: 
		_left(left), _right(right), _bottom(bottom), _top(top),
		_distance(distance),
		_imgSize(imgSize),
		_name(name) { };
	
	float left() const { return _left; }
	float right() const { return _right; }
	float bottom() const { return _bottom; }
	float top() const { return _top; }

	const vec2& imgSize() const { return _imgSize; }
	float distance() const { return _distance; }
	const string& name() const { return _name; }
};

#endif