#ifndef _AABB_
#define _AABB_

#include "../Includes.h"
#include "../Ray.h"

class AABB {
public:
    AABB();
    AABB(const vec3& min, const vec3& max);
    
    inline void expand(const AABB& bbox);
    
    inline float hit(const Ray& ray) const;
    
    vec3 minP;
    vec3 maxP;
    vec3 centroid;

    inline float getCentroid(u_char axis) const;
	inline u_char longestDim() const;
};

inline AABB::AABB()
{
    minP = vec3(INFINITY, INFINITY, INFINITY);
    maxP = vec3(-INFINITY, -INFINITY, -INFINITY);
};

inline AABB::AABB(const vec3& minP, const vec3& maxP)
{
    this->minP = minP;
    this->maxP = maxP;
    this->centroid = (minP + maxP) * 0.5f;
};



inline void AABB::expand(const AABB& bbox) 
{
    minP.x = fmin(minP.x, bbox.minP.x);
    minP.y = fmin(minP.y, bbox.minP.y);
    minP.z = fmin(minP.z, bbox.minP.z);

    maxP.x = fmax(maxP.x, bbox.maxP.x);
    maxP.y = fmax(maxP.y, bbox.maxP.y);
    maxP.z = fmax(maxP.z, bbox.maxP.z);

    this->centroid = (maxP + minP) * 0.5f;
};


inline float AABB::hit(const Ray& ray) const 
{  
    float tmin = -INFINITY;
    float tmax = INFINITY;
    vec3 origin = ray.origin();
    vec3 direction = ray.direction();
    vec3 invDir = ray.invDir();

    for (int i = 0; i < 3; i++) 
    {
        float tmini = (minP[i] - origin[i]) * ray.invDir()[i];
        float tmaxi = (maxP[i] - origin[i]) * ray.invDir()[i];
        if (ray.sign()[i]) 
            std::swap(tmini, tmaxi);
        if (tmini > tmin) 
            tmin = tmini;
        if (tmaxi < tmax) 
            tmax = tmaxi;
        if (tmin > tmax) 
            return INFINITY;
    }

    if (tmin > 0) 
    {
		return tmin;
    } 
    else 
    {
		return tmax;
    }
};

inline float AABB::getCentroid(u_char axis) const
{
    return centroid[axis];
}

inline u_char AABB::longestDim() const
{
	vec3 diff = maxP - minP;
	if (diff.x > diff.y && diff.x > diff.z)
		return 0;
	else if (diff.y > diff.z)
		return 1;
	else
		return 2;
}

#endif