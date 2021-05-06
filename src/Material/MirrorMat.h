#ifndef _MIRROR_MAT_
#define _MIRROR_MAT_

#include "Material.h"

class MirrorMat : public Material
{
private:
	const vec3 _mirrorRef;

public:
	MirrorMat(uint id, const vec3& ambientRef, MType mType, const vec3& diffuseRef, const vec3& specularRef, const vec3& mirrorRef, float phongExp)
	:
	Material(id, ambientRef, mType, diffuseRef, specularRef, phongExp), _mirrorRef(mirrorRef)
	{}

	//inline void attenuate(const Ray& ray, PointLight* light, HitInfo& outHit, Ray& reflectedRay);
	const vec3& mirrorRef() { return _mirrorRef; } 
};

#endif