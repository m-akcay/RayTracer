#ifndef _SCENE_H_
#define _SCENE_H_

#include "Includes.h"
#include "Camera.h"
#include "Light/PointLight.h"
//#include "Shape/Hittable.h"
#include "Shape/Triangle.h"
#include "Shape/Mesh.h"
#include "Shape/Sphere.h"
#include "Shape/BVH.h"
#include "Image/Image.h"
#include "Material/MirrorMat.h"
#include "Material/ConductorMat.h"
#include "Material/DielectricMat.h"


class Scene
{
private:
	const vec3 backgroundColor;
	const vec3 ambientLight;
	const float shadowRayEps;
	const float intersectionTestEps;
	const std::vector< Camera* > cameras;
	const std::vector< Material* > materials;
	const std::vector< vec3 > vertices;
	std::vector< Hittable* > objects;
	const std::vector< PointLight* > lights;
	BVH* bvh;
	uint rdepth = 0;

public:
	Scene(const vec3& backgroundColor, const vec3& ambientLight, float shadowRayEps, float intersectionTestEps,
			const std::vector< Camera* >& cameras,
		const std::vector< Material* >& materials,
		const std::vector< vec3 >& vertices,
		const std::vector< Hittable* >& objects,
		const std::vector< PointLight* >& lights)
		:
		backgroundColor(backgroundColor), ambientLight(ambientLight), shadowRayEps(shadowRayEps), intersectionTestEps(intersectionTestEps),
		cameras(cameras), materials(materials), vertices(vertices), objects(objects), lights(lights)
		{

		}
	
	~Scene();

	void render();
	void createBVH();
private:
	vec3 trace0(const Ray& ray, uint rdepth);
	vec3 trace1(const Ray& ray, uint rdepth);
	vec3 trace2(const Ray& ray, uint rdepth);
	vec3 trace3(const Ray& ray, uint rdepth);
	vec3 trace4(const Ray& ray, uint rdepth);
	vec3 trace5(const Ray& ray, uint rdepth);
	vec3 trace6(const Ray& ray, uint rdepth);
	vec3 trace7(const Ray& ray, uint rdepth);
	vec3 rTrace(const Ray& ray, uint rdepth);
	bool inShadow(const HitInfo& hit, const PointLight* light);
//	Scene();
};

#endif