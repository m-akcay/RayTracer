#ifndef _MESH_INSTANCE_H_
#define _MESH_INSTANCE_H_

#include "Mesh.h"
#include "BVH.h"
#include "Triangle.h"

class MeshInstance : public Hittable
{
private:
	std::vector< Hittable* > faces;
	BVH* bvh;
	inline void createBVH(std::vector< Hittable* >& faces);

public:
	MeshInstance(uint id, Material* mat, uint baseMeshId, bool resetTransform, std::vector< size_t >& transformationIdx)
	:
	Hittable(id, mat)
	{ 
		//createBVH(faces);
		Mesh* baseMesh = Hittable::meshes[baseMeshId];
		
		setTransformations(transformationIdx);

		mat4 transform = glm::inverse(inverseTransform);
		std::vector< vec3 > vs;
		for (auto v : Hittable::vertices)
		{
			vec3 transformedVert = transform * vec4(v, 1.0f);
			vs.push_back(transformedVert);
		}
	}

	inline ~MeshInstance() override;

	inline bool hit(const Ray& ray, HitInfo& outHit) override;
	size_t numOfFaces() { return faces.size(); }

	inline void setTransformations(std::vector< size_t >& transformationIdx);
};

inline bool MeshInstance::hit(const Ray& ray, HitInfo& outHit) 
{
	return this->bvh->hit(ray, outHit);
}

inline void MeshInstance::setTransformations(std::vector< size_t >& transformationIdx)
{
	mat4 compositeTransform(1.0f);

	for (auto idx : transformationIdx)
	{
		mat4 transform	= Hittable::transformations[idx];
		compositeTransform *= transform;
	}

	this->inverseTransform = glm::inverse(compositeTransform);
}

inline void MeshInstance::createBVH(std::vector< Hittable* >& faces)
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

MeshInstance::~MeshInstance()
{
	// this will be checked
	for (auto face : faces)
		delete face;
}


#endif