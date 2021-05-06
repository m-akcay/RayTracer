#ifndef _BVH_
#define _BVH_

#include "Hittable.h"

class BVH : public Hittable 
{
public:
    static Hittable* construct(std::vector<Hittable*>& objects);
    
    BVH(std::vector<Hittable*>& objects, size_t start, size_t end, u_char axis);
    ~BVH();
    bool hit(const Ray& ray, HitInfo& hitPoint);
    
    Hittable* left;
    Hittable* right;
};

#endif