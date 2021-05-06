#ifndef _POINT_LIGHT_H_
#define _POINT_LIGHT_H_

#include "../Includes.h"
//#include "../Material/Material.h"

class PointLight
{
private:
	const uint _id;
	const vec3 _pos;
	const vec3 _intensity;

public:
	PointLight(uint id, const vec3& pos, const vec3& intensity)
		:
		_id(id), _pos(pos), _intensity(intensity) { };

	uint id() { return _id; }
	const vec3& pos() const { return _pos; }
	const vec3& intensity() const { return _intensity; }

	//inline void attenuate(const Ray& ray, HitInfo& outHit);
};


// void PointLight::attenuate(const Ray& ray, HitInfo& outHit)
// {
// 	vec3 wi, wo, Ld, Ls, halfVec, wiPlusWo, irradiance;
// 	float cosTPrime, cosAPrime, distanceToLight;

// 	wo = -ray.direction;
// 	wi = glm::normalize(this->_pos - outHit.pos);

// 	cosTPrime = glm::max(0.0f, dot(wi, outHit.normal));
// 	distanceToLight = glm::distance(outHit.pos, this->_pos);
// 	irradiance = _intensity / (distanceToLight * distanceToLight);

	
// 	Ld = outHit.mat->diffuseRef() * cosTPrime * irradiance;


// 	wiPlusWo = wi + wo;
// 	halfVec = wiPlusWo / glm::length(wiPlusWo);

// 	cosAPrime = glm::max(0.0f, dot(outHit.normal, halfVec));
// 	Ls = outHit.mat->specularRef() * powf(cosAPrime, outHit.mat->phongExp()) * irradiance;

// 	outHit.color += Ld + Ls;
// }

#endif