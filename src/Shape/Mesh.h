#ifndef _MESH_H_
#define _MESH_H_

#include "Triangle.h"

class Mesh : public Hittable
{
private:
	const std::vector<Triangle> faces;

public:
	//BVH* bvh;
	Mesh(uint id, Material* mat, const std::vector<Triangle>& faces)
	:
	Hittable(id, mat),
	faces(faces)
	{ 
		//bvh =  (BVH*) BVH::construct(faces);
	}
	//~Mesh() = default;

	inline bool hit(const Ray& ray, HitInfo& outHit);
	size_t numOfFaces() { return faces.size(); }
};

bool Mesh::hit(const Ray& ray, HitInfo& outHit)
{
	bool _hit = false;
	
	for (auto face : faces)
	{
		if (face.hit(ray, outHit))
		{
			_hit = true;
		}
	}

	return _hit;
}


#endif