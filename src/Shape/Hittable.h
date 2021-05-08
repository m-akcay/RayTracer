#ifndef _HITTABLE_H_
#define _HITTABLE_H_

#include "../Includes.h"
#include "../Material/Material.h"
#include "AABB.h"


class Hittable
{
private:
	const uint _id;

public:
	constexpr static float intersectionTestEps = 0.000001f;
	constexpr static float shadowRayEps = 0.001f;
	static std::vector< Material* > materials;
	static std::vector< vec3 > vertices;
	static std::vector< vec3 > normals;
	static std::vector< int > indices;

	Hittable() : _id(0) {}
	AABB bbox;
	virtual ~Hittable() {}
protected:
	Material* _mat;
	Hittable(uint id, Material* mat)
	:
	_id(id), _mat(mat)
	{}


public:
	uint id() { return _id; }
	Material* mat() { return _mat; }

	virtual bool hit(const Ray& ray, HitInfo& outHit) = 0;
};

#endif