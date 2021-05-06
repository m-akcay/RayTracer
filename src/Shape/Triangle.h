#ifndef _TRIANGLE_H_
#define _TRIANGLE_H_

#include "Hittable.h"

class Triangle : public Hittable
{
private:
	const vec3 _v1, _v2, _v3;
	vec3 _normal;
	const vec3 v1v2, v1v3;

public:
	Triangle(uint id, Material* mat,
			const vec3& v1, const vec3& v2, const vec3& v3)
			:
			Hittable(id, mat),
			_v1(v1), _v2(v2), _v3(v3),
			v1v2(v2 - v1), v1v3(v3 - v1),
			_normal(glm::normalize(cross(v2 - v1, v3 - v1)))
	{ 
		vec3 minP = glm::min(glm::min(v1, v2), v3);
		vec3 maxP = glm::max(glm::max(v1, v2), v3);
		this->bbox = AABB(minP, maxP);
	}

	Triangle(uint id, Material* mat, int p1idx, int p2idx, int p3idx,
			const vec3& v1, const vec3& v2, const vec3& v3)
			:
			Hittable(id, mat),
			_v1(v1), _v2(v2), _v3(v3),
			v1v2(v2 - v1), v1v3(v3 - v1),
			_normal(glm::normalize(cross(v2 - v1, v3 - v1)))
	{ 
		this->p1idx = p1idx;
		this->p2idx = p2idx;
		this->p3idx = p3idx;

		vec3 minP = glm::min(glm::min(v1, v2), v3);
		vec3 maxP = glm::max(glm::max(v1, v2), v3);
		this->bbox = AABB(minP, maxP);
	}

	//~Triangle() = default;
	int p1idx, p2idx, p3idx;
	const vec3& v1() { return _v1; }
	const vec3& v2() { return _v2; }
	const vec3& v3() { return _v3; }
	const vec3& normal() { return _normal; }
	void setNormal(const vec3& normal) { this->_normal = normal; }

	inline bool hit(const Ray& ray, HitInfo& outHit);

private:
	inline bool solveEquation(const Ray& ray, float& out_t);
	inline void setHitInfo(const Ray& ray, float t, HitInfo& outHit);
};



bool Triangle::hit(const Ray& ray, HitInfo& outHit)
{
	float t;
	if (solveEquation(ray, t) && t < outHit.tVal - intersectionTestEps)
	{
		setHitInfo(ray, t, outHit);
		return true;
	}

	return false;
}

void Triangle::setHitInfo(const Ray& ray, float t, HitInfo& outHit)
{
	//outHit = HitInfo(ray.getVal(t), _normal, t, _mat);
	outHit.tVal = t;
	outHit.pos = ray.getVal(t);
	outHit.normal = _normal;
	outHit.mat = _mat;
}

bool Triangle::solveEquation(const Ray& ray, float& out_t)
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

	return true;

}

#endif