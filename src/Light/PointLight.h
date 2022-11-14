#ifndef _POINT_LIGHT_H_
#define _POINT_LIGHT_H_

#include "../Includes.h"
#include "Light.h"
//#include "../Material/Material.h"

class PointLight : public Light
{
public:
	PointLight(uint id, const vec3& pos, const vec3& intensity)
		:
		Light(id, pos, intensity) { }

	uint id() { return _id; }
	const vec3& pos() const { return _pos; }
	const vec3& intensity() const { return _intensity; }

	//inline void attenuate(const Ray& ray, HitInfo& outHit);
	//irradiance = light->intensity() / (distanceToLight * distanceToLight);
	inline vec3 contribute(const vec3& hitPos, const vec3& lightPos) override;
	inline const vec3 directionToLight(const vec3& hitPos, vec3& out_lightPos) const override;
};


inline const vec3 PointLight::directionToLight(const vec3& hitPos, vec3& out_lightPos) const
{
	out_lightPos = this->_pos;
	return glm::normalize(this->_pos - hitPos);
}


inline vec3 PointLight::contribute(const vec3& hitPos, const vec3& lightPos)
{
	float distanceToLight = glm::distance(lightPos, hitPos);
	return this->_intensity / (distanceToLight * distanceToLight);
}

#endif