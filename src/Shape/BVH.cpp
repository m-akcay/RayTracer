#include "BVH.h"

#include "BVH.h"
#include <iostream>

BVH::~BVH() 
{
    if (left != NULL) 
        delete left;
    if (right != NULL) 
        delete right;
};

Hittable* BVH::construct(std::vector<Hittable*>& objects)
{
    size_t size = objects.size();
    if (size == 0)
        return NULL; 
    else if (size == 1)
        return objects[0];
    else 
        return new BVH(objects, 0, size, 0);
}

BVH::BVH(std::vector<Hittable*>& objects, size_t start, size_t end, u_char axis)
{
    float centroid;
    size_t mid = start;
    Hittable* object;
    this->left = NULL;
    this->right = NULL;

    for (size_t index = start; index < end; index++) 
    {
        this->bbox.expand(objects[index]->bbox);
    }
    
    centroid = this->bbox.getCentroid(axis);
	u_char longestAxis = this->bbox.longestDim();
    
    for (size_t index = start; index < end; index++) 
    {
        object = objects[index];
        
        if (object->bbox.getCentroid(axis) < centroid) 
            std::swap(objects[index], objects[mid++]);
    }
    if (mid == start || mid == end) 
        mid = (start + end) / 2;
    
    if (start == mid - 1) 
        left = objects[start];
    else 
        left = new BVH(objects, start, mid, longestAxis);

    if (mid == end - 1) 
        right = objects[mid];
    else 
        right = new BVH(objects, mid, end, longestAxis);
}

bool BVH::hit(const Ray& ray, HitInfo& outHit)
{
    float boxHit_tVal = bbox.hit(ray);

    if (boxHit_tVal < 0 || boxHit_tVal == INFINITY) 
        return false;
    
    bool isHit = false;
    HitInfo leftHitPoint, rightHitPoint;

    if (left->hit(ray, leftHitPoint) && 
        leftHitPoint.tVal > 0 &&
        leftHitPoint.tVal < outHit.tVal) 
    {
        outHit = leftHitPoint;
        isHit = true;
    }
    
    if (right->hit(ray, rightHitPoint) && 
        rightHitPoint.tVal > 0 &&
        rightHitPoint.tVal < outHit.tVal) 
    {
        outHit = rightHitPoint;
        isHit = true;
    }

    return isHit;
}

