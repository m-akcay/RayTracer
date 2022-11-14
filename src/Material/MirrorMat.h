#ifndef _MIRROR_MAT_
#define _MIRROR_MAT_

#include "Material.h"

class MirrorMat : public Material
{
private:
	const vec3 _mirrorRef;
	const float _roughness;
	inline uint minIdx(const vec3& r);

public:
	MirrorMat(uint id, const vec3& ambientRef, MType mType, const vec3& diffuseRef, const vec3& specularRef, const vec3& mirrorRef, float phongExp, float roughness)
	:
	Material(id, ambientRef, mType, diffuseRef, specularRef, phongExp), _mirrorRef(mirrorRef), _roughness(roughness)
	{}

	//inline void attenuate(const Ray& ray, PointLight* light, HitInfo& outHit, Ray& reflectedRay);
	const vec3& mirrorRef() { return _mirrorRef; } 
	const float roughness() const { return _roughness; }

	inline vec3 reflected(const vec3& inDir, const vec3& normal);
};

inline vec3 MirrorMat::reflected(const vec3& inDir, const vec3& normal)
{
	if (_roughness == 0.0f)
		return glm::reflect(inDir, normal);
	else
	{
		vec3 r = glm::normalize(glm::reflect(inDir, normal));
		vec3 rPrime = r;
		rPrime[minIdx(r)] = 1.0f;

		vec3 u = glm::normalize(cross(r, rPrime));
		vec3 v = cross(r, u);

		float uRand = Helper::rand(-0.5f, 0.5f);
		float vRand = Helper::rand(-0.5f, 0.5f);
		vec3 rReal = glm::normalize(r + _roughness * (uRand * u + vRand * v));
		return rReal;
	}
}

inline uint MirrorMat::minIdx(const vec3& r)
{
	if (fabs(r.x) < fabs(r.y) && fabs(r.x) < fabs(r.z))
		return 0;
	else if (fabs(r.y) < fabs(r.z))
		return 1;
	else
		return 2;
}

#endif