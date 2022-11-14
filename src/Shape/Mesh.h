#ifndef _MESH_H_
#define _MESH_H_

#include "BVH.h"
#include "Triangle.h"

class Mesh : public Hittable
{
private:
	std::vector< Hittable* > faces;
	BVH* bvh;
	inline void createBVH(std::vector< Hittable* >& faces);

public:
	bool isTransformed;
	Mesh(uint id, Material* mat, std::vector< Hittable* >& faces)
	:
	Hittable(id, mat)
	{ 
		this->faces = faces;
		createBVH(this->faces);
		isTransformed = false;
	}

	Mesh(uint id, Material* mat, std::vector< Hittable* >& faces, const mat4& transform)
	:
	Hittable(id, mat, transform)
	{
		this->faces = faces;
		createBVH(this->faces);
		isTransformed = true;
	}

	inline ~Mesh() override;

	inline bool hit(const Ray& ray, HitInfo& outHit) override;
	size_t numOfFaces() { return faces.size(); }
};

inline bool Mesh::hit(const Ray& ray, HitInfo& outHit) 
{
	return this->bvh->hit(ray, outHit);
}

inline void Mesh::createBVH(std::vector< Hittable* >& faces)
{

	float minX, minY, minZ, maxX, maxY, maxZ;
	minX = minY = minZ = INFINITY;
	maxX = maxY = maxZ = -INFINITY;
	vec3 minP(minX, minY, minZ);
	vec3 maxP(maxX, maxY, maxZ);
	AABB base(minP, maxP);
	for (auto face : faces)
	{
		base.expand(face->bbox);
	}

	this->bbox = base;
	
	bvh = static_cast< BVH* > (BVH::construct(faces));
}

Mesh::~Mesh()
{
	// this will be checked
	for (auto face : faces)
		delete face;
}


#endif