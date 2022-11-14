#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "../Includes.h"
#include "../Light/PointLight.h"
#include "../Ray.h"

class Material
{
protected:
	constexpr static float shadowRayEps = 0.001f;
	
	const uint _id;
	const MType _mType;
	const vec3 _ambientRef;
	 vec3 _diffuseRef;
	 vec3 _specularRef;
	float _phongExp;
public:
	Material(uint id, vec3 ambientRef, MType mType, vec3 diffuseRef, vec3 specularRef, 
			float phongExp) 
			: 
			_id(id), _ambientRef(ambientRef), _mType(mType), _diffuseRef(diffuseRef), _specularRef(specularRef), _phongExp(phongExp) {}


	uint id() { return _id; }
	const vec3& ambientRef() const { return _ambientRef; }
	const MType mType() { return _mType; }

	virtual inline void attenuate(const Ray& ray, Light* light, HitInfo& outHit);
	virtual inline void attenuate(const Ray& ray, Light* light, HitInfo& outHit, Ray& reflectedRay, Ray& refractedRay) {};
};


void Material::attenuate(const Ray& ray, Light* light, HitInfo& outHit)
{
	vec3 wi, wo, Ld, Ls, halfVec, wiPlusWo, irradiance, lightPos;
	float cosTPrime, cosAPrime, distanceToLight;

	wo = -ray.direction();
	wi = light->directionToLight(outHit.pos, lightPos);

	cosTPrime = std::max(0.0f, dot(wi, outHit.normal));
	irradiance = light->contribute(outHit.pos, light->pos());

	Ld = this->_diffuseRef * cosTPrime * irradiance;


	wiPlusWo = wi + wo;
	halfVec = wiPlusWo / glm::length(wiPlusWo);

	cosAPrime = glm::max(0.0f, dot(outHit.normal, halfVec));
	Ls = this->_specularRef * powf(cosAPrime, this->_phongExp) * irradiance;

	
	outHit.color += (Ld + Ls);
}

#endif

