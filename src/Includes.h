#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <memory>
#include <vector>

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

