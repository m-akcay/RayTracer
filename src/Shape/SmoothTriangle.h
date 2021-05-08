#ifndef _SMOOTH_TRIANGLE_H_
#define _SMOOTH_TRIANGLE_H_

#include "Hittable.h"

class SmoothTriangle : public Hittable
{
private:
	size_t _v1idx, _v2idx, _v3idx;
	const vec3 _v1, _v2, _v3;
	const vec3 v1v2, v1v3;
	vec3 _n1, _n2, _n3;

public:
	SmoothTriangle(uint id, Material* mat,
			size_t v1idx, size_t v2idx, size_t v3idx)
			:
			Hittable(id, mat),
			_v1(vertices[v1idx]), _v2(vertices[v2idx]), _v3(vertices[v3idx]),
			v1v2(vertices[v2idx] - vertices[v1idx]), v1v3(vertices[v3idx] - vertices[v1idx]),
			_v1idx(v1idx), _v2idx(v2idx), _v3idx(v3idx)
	{ 
		vec3 minP = glm::min(glm::min(_v1, _v2), _v3);
		vec3 maxP = glm::max(glm::max(_v1, _v2), _v3);
		this->bbox = AABB(minP, maxP);
	}

	void setNormals()
	{
		_n1 = normals[_v1idx];
		_n2 = normals[_v2idx];
		_n3 = normals[_v3idx];
	}
	//~SmoothTriangle() = default;
	size_t v1idx() { return _v1idx; }
	size_t v2idx() { return _v2idx; }	
	size_t v3idx() { return _v3idx; }
	const vec3& v1() { return _v1; }
	const vec3& v2() { return _v2; }
	const vec3& v3() { return _v3; }

	const vec3& n1() { return _n1; }
	const vec3& n2() { return _n2; }
	const vec3& n3() { return _n3; }

	inline bool hit(const Ray& ray, HitInfo& outHit);
private:
	inline bool solveEquation(const Ray& ray, float& out_u, float& out_v, float& out_t);
	inline void setHitInfo(const Ray& ray, float u, float v, float t, HitInfo& outHit);
};



bool SmoothTriangle::hit(const Ray& ray, HitInfo& outHit)
{
	float u, v, t;
	if (solveEquation(ray, u, v, t) && t < outHit.tVal - intersectionTestEps)
	{
		setHitInfo(ray, u, v, t, outHit);
		return true;
	}

	return false;
}

void SmoothTriangle::setHitInfo(const Ray& ray, float u, float v, float t, HitInfo& outHit)
{
	//outHit = HitInfo(ray.getVal(t), _normal, t, _mat);
	outHit.tVal = t;
	outHit.pos = ray.getVal(t);
	// outHit.normal = glm::normalize(_n1 * u + _n2 * v + _n3);
	outHit.normal = glm::normalize(_n2 * u + _n3 * v + _n1);
	outHit.mat = _mat;
}

bool SmoothTriangle::solveEquation(const Ray& ray, float& out_u, float& out_v, float& out_t)
{	
	// Moller-Trumbore intersection algorithm
	// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
	// vec3 v0v1 = -v1v2;
	// vec3 v0v2 = -v1v3;
	vec3 pvec = cross(ray.direction(), v1v3);
	float det = dot(v1v2, pvec);
	if (fabs(det) < 0)
		return false;
	
	float invDet = 1 / det;
	vec3 tvec = ray.origin() - _v1;
	float u = dot(tvec, pvec) * invDet;
	if (u < 0 || u > 1)
		return false;
	
	vec3 qvec = cross(tvec, v1v2);
	float v = dot(ray.direction(), qvec) * invDet;
	if (v < 0 || u + v > 1)
		return false;

	out_t = dot(v1v3, qvec) * invDet;
	if (out_t < 0)
		return false;

	out_u = u;
	out_v = v;

	return true;
}

#endif