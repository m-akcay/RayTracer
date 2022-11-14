#ifndef _HITTABLE_H_
#define _HITTABLE_H_

#include "../Includes.h"
#include "../Material/Material.h"
#include "AABB.h"

class Mesh;
class Triangle;
class Sphere;

class Hittable
{
private:
	const uint _id;

public:
	constexpr static float intersectionTestEps = 0.000001f;
	constexpr static float shadowRayEps = 0.001f;

	static std::vector< Mesh* > meshes;
	static std::vector< Triangle* > triangles;
	static std::vector< Sphere* > spheres;

	static std::vector< Material* > materials;
	static std::vector< vec3 > vertices;
	static std::vector< vec3 > normals;
	static std::vector< int > indices;
	static std::vector< mat4 > transformations;

	Hittable() : _id(0) {}
	AABB bbox;
	virtual ~Hittable() {}
protected:
	mat4 transform;
	mat4 inverseTransform;
	mat4 inverseTranspose;
	bool hasTransform;
	Material* _mat;

	std::vector< vec3 > baseVertices;
	std::vector< std::vector< size_t> > mIndices;

	Hittable(uint id, Material* mat)
	:
	_id(id), _mat(mat)
	{
		hasTransform = false;
	}

	Hittable(uint id, Material* mat, const mat4& transform) : _id(id), _mat(mat), transform(transform)
	{
		hasTransform = true;
		inverseTransform = glm::inverse(transform);
		inverseTranspose = glm::transpose(inverseTransform);
	}


public:
	uint id() { return _id; }
	Material* mat() { return _mat; }
	const mat4& getTransform() { return this->transform; }
	const std::vector< vec3 >& getBaseVertices() { return baseVertices; }
	const std::vector< std::vector< size_t > >& getIndices() { return mIndices; }
	void setBaseVertices(std::vector< vec3 >& mVertices) { this->baseVertices = mVertices; }
	void setIndices(std::vector< std::vector< size_t > >& mIndices) { this->mIndices = mIndices; }

	virtual bool hit(const Ray& ray, HitInfo& outHit) = 0;
};

#endif