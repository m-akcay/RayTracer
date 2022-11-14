#ifndef _RAY_H_
#define _RAY_H_

#include "glm/common.hpp"
#include "glm/vec3.hpp"

typedef glm::vec3 vec3;

class Ray
{
private:
	vec3 _origin;
	vec3 _direction;
	vec3 _invDir;
	int3 _sign;

public:
	Ray() {};
	Ray(const vec3& origin, const vec3& direction) : _origin(origin), _direction(direction) 
	{
		_invDir = 1.0f / direction; 
        _sign[0] = (_invDir.x < 0); 
        _sign[1] = (_invDir.y < 0); 
        _sign[2] = (_invDir.z < 0); 
	}

	vec3 origin() const { return _origin; }
	vec3 direction() const { return _direction; }
	vec3 invDir() const { return _invDir; }
	int3 sign() const { return _sign; }

	void origin(const vec3& origin) { this->_origin = origin; }
	void direction(const vec3& direction) 
	{ 
		this->_direction = direction;
		this->_invDir = 1.0f / direction;
		_sign[0] = (_invDir.x < 0); 
        _sign[1] = (_invDir.y < 0); 
        _sign[2] = (_invDir.z < 0); 
	}	

	vec3 getVal(float t) const
	{
		return _origin + t * _direction;
	}

	Ray operator *(const mat4& transform)
	{
		vec3 origin = transform * vec4(this->_origin, 1.0f);
		vec3 direction = transform * vec4(this->_direction, 1.0f);
		return Ray(origin, direction);
	}

	// maybe a mistake
	// because origin() and direction() is const
	friend Ray operator *(const mat4& transform, const Ray& ray)
	{
		vec3 origin = transform * vec4(ray.origin(), 1.0f);
		vec3 direction = transform * vec4(ray.direction(), 1.0f);
		return Ray(origin, direction);
	}
	
	friend std::ostream& operator << (std::ostream& out, const Ray& ray)
	{
		out << "orig = " << glm::to_string(ray.origin()) << ")    dir = " << glm::to_string(ray.direction());
		return out;
	}
};

#endif