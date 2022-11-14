#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <memory>
#include <vector>
#include <random>

#include <algorithm>
#include <iterator>
#include <forward_list>

#include <thread>
#include <atomic>
#include <future>

#include "glm/vec3.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/vector_angle.hpp"
#include "glm/gtx/compatibility.hpp"
#include "glm/gtx/transform.hpp"

typedef std::string string;
typedef unsigned int uint;

typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;

typedef glm::int3 int3;

typedef glm::mat2 mat2;
typedef glm::mat3 mat3;
typedef glm::mat4 mat4;

//#define clamp glm::clamp
#define cross glm::cross
#define dot glm::dot
//#define normalize glm::normalize


#ifndef _HIT_INFO_
#define _HIT_INFO_

class Material;

enum MType
{
	SPECULAR,
	MIRROR,
	CONDUCTOR,
	DIELECTRIC
};

enum TransformationType
{
	TRANSLATION,
	SCALING,
	ROTATION
};

class Helper
{
public:
	static inline float rand()
	{
		static std::uniform_real_distribution<float> distribution(0.0, 1.0);
		static std::mt19937 generator;
		return distribution(generator);
	}

	static inline float rand(float min, float max)
	{
		static std::uniform_real_distribution<float> distribution(min, max);
		static std::mt19937 generator;
		return distribution(generator);
	}

	static inline uint minIdx(const vec3& r)
	{
		if (fabs(r.x) < fabs(r.y) && fabs(r.x) < fabs(r.z))
			return 0;
		else if (fabs(r.y) < fabs(r.z))
			return 1;
		else
			return 2;
	}
};


class HitInfo
{
public:
	vec3 pos;
	vec3 normal;
	float tVal;
	Material* mat;
	vec3 color = vec3(0.0f);
	HitInfo() 
	:
	pos(vec3(0.0f)), normal(0.0f), tVal(INFINITY), mat(NULL)
	{}

	HitInfo(const vec3& pos, const vec3& normal, float tVal, Material* mat)
	:
	pos(pos), normal(normal), tVal(tVal), mat(mat)
	{
	}

};
#endif

