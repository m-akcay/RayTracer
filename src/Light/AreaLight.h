#ifndef _AREA_LIGHT_H_
#define _AREA_LIGHT_H_

#include "../Includes.h"
#include "Light.h"
//#include "../Material/Material.h"

class AreaLight : public Light
{
private:
	const float _size;
	const vec3 _normal;
public:
	AreaLight(uint id, const vec3& pos, const vec3& intensity, float size, const vec3& normal)
		:
		Light(id, pos, intensity), _size(size), _normal(normal){ }

	uint id() { return _id; }
	const vec3& pos() const { return _pos; }
	const vec3& intensity() const { return _intensity; }
	const float size() { return _size; }
	const vec3& normal() const { return _normal; }

	//inline void attenuate(const Ray& ray, HitInfo& outHit);
	//irradiance = light->intensity() / (distanceToLight * distanceToLight);
	inline vec3 contribute(const vec3& hitPos, const vec3& lightPos) override;
	inline const vec3 directionToLight(const vec3& hitPos, vec3& out_lightPos) const override;
};

inline const vec3 AreaLight::directionToLight(const vec3& hitPos, vec3& out_lightPos) const
{
	vec3 r = _normal;
	vec3 rPrime = r;
	rPrime[Helper::minIdx(r)] = 1.0f;
	vec3 u = glm::normalize(cross(r, rPrime));
	vec3 v = cross(r, u);

	float uRand = Helper::rand(-0.5f, 0.5f);
	float vRand = Helper::rand(-0.5f, 0.5f);

	vec3 randPos = _pos + _size * (u * uRand + v * vRand);
	out_lightPos = randPos;
	return glm::normalize(randPos - hitPos);
}

inline vec3 AreaLight::contribute(const vec3& hitPos, const vec3& lightPos)
{
	vec3 thisPos_minus_hitPos = lightPos - hitPos;
	float distanceToLight = sqrtf(dot(thisPos_minus_hitPos, thisPos_minus_hitPos));

	return this->_intensity / (distanceToLight * distanceToLight);
}

#endif