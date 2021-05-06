#ifndef _SPHERE_H_
#define _SPHERE_H_

#include "Hittable.h"

class Sphere : public Hittable
{
private:
	const vec3 center;
	const float radius;
	inline void setHitInfo(const Ray& ray, float t, HitInfo& outHit);
public:
	Sphere(uint id, const vec3& center, float radius, Material* mat)
	:
	Hittable(id, mat), center(center), radius(radius)
	{ 
		this->bbox = AABB(center - radius, center + radius);
	}

	//~Sphere() = default;

	inline bool hit(const Ray& ray, HitInfo& outHit);
};

bool Sphere::hit(const Ray& ray, HitInfo& outHit)
{
	float delta, d_dot_d, minusD_dot_oMinusC;
	vec3 origin, direction, oMinusC;
	
	origin = ray.origin();
	direction = ray.direction();
	oMinusC = origin - center;
	
	//d_dot_d = direction.lengthSquare();
	d_dot_d = 1.0f;
	minusD_dot_oMinusC = dot(-direction, oMinusC);


	delta = powf(dot(direction, oMinusC), 2) - d_dot_d * (dot(oMinusC, oMinusC) - radius * radius);

	if (delta < 0)	// no intersection
	{
		return false;
	}
	else if (delta == 0)
	{
		float t = minusD_dot_oMinusC / d_dot_d;
		if (t > 0)
		{
			if (t < outHit.tVal)
			{
				setHitInfo(ray, t, outHit);
				return true;
			}
		}
	}
	else
	{
		float t, t0, t1, rootDelta;
		bool isHit = false;
		rootDelta = sqrtf(delta);
		t0 = (minusD_dot_oMinusC - rootDelta) / d_dot_d;
		t1 = (minusD_dot_oMinusC + rootDelta) / d_dot_d;

		if (t0 > 0 &&
			(t1 < 0 || t0 < t1))
		{
			if (t0 < outHit.tVal - intersectionTestEps)
			{
				setHitInfo(ray, t0, outHit);
				return true;
			}
		}
		else if(t1 > 0 && 
				(t0 < 0 || t1 < t0))
		{
			if (t1 < outHit.tVal - intersectionTestEps)
			{
				setHitInfo(ray, t1, outHit);
				return true;
			}
		}
	}

	return false;
}


void Sphere::setHitInfo(const Ray& ray, float t, HitInfo& outHit)
{
	vec3 hitp = ray.getVal(t);
	vec3 normal = (hitp - center) / radius;
	// std::cout << "s0" << std::endl;
	// outHit = HitInfo(hitp, normal, t, _mat);
	// std::cout << "s1" << std::endl;
	// if (outHit.mat == NULL)
	// 	std::cout << glm::to_string(outHit.normal)<< std::endl;
	outHit.pos = hitp;
	outHit.normal = normal;
	outHit.tVal = t;
	outHit.mat = _mat;
}


#endif