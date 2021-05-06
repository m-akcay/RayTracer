#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "Includes.h"
#include "ImagePlane.h"
#include "Ray.h"

class Camera
{
private:
	const uint _id;
	const vec3 pos;
	const vec3 u;
	const vec3 v;
	const vec3 gaze;
	//const vec3 m;
	const vec3 q;
	//const float fovY;
	const ImagePlane* imgPlane;
	const float rightMinusLeft_over_width;
	const float topMinusBottom_over_height;

public:
	//Camera() : _id(0) {}
	Camera (uint id, const vec3& pos, const vec3& gaze, const vec3& up,
			const ImagePlane& imgPlane)
			:
			_id(id), pos(pos), gaze(glm::normalize(gaze)), imgPlane(new ImagePlane(imgPlane)),
			u(glm::normalize(cross(up, (-gaze)))), 
			v(glm::normalize(cross(-gaze, u))),
			q(pos + glm::normalize(gaze) * imgPlane.distance() + imgPlane.left() * u + imgPlane.top() * v),
			rightMinusLeft_over_width(
				(imgPlane.right() - imgPlane.left()) / imgPlane.imgSize().x
			),
			topMinusBottom_over_height(
				(imgPlane.top() - imgPlane.bottom()) / imgPlane.imgSize().y
			)
	{}

	// Camera(uint id, const vec3& pos, const vec3& gazePoint, const vec3& up,
	// 		float fovY, const ImagePlane& imgPlane)
	// 		{}

	~Camera() { delete imgPlane; }

	uint id() { return _id; }
	uint imgWidth()  const { return imgPlane->imgSize().x; }
	uint imgHeight() const { return imgPlane->imgSize().y; }
	const string& imgName() { return imgPlane->name(); }

	inline Ray getViewRay (uint x, uint y) const;
};

inline Ray Camera::getViewRay (uint x, uint y) const
{
	float su = (x + 0.5f) * rightMinusLeft_over_width;
	float sv = (y + 0.5f) * topMinusBottom_over_height;
	vec3 s = q + su * u - sv * v;
	vec3 direction = glm::normalize(s - pos);
	return Ray(this->pos, direction);
};

#endif