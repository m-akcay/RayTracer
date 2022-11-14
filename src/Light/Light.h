#ifndef _LIGHT_H_
#define _LIGHT_H_

#include "../Includes.h"
//#include "../Material/Material.h"

class Light
{
protected:
	const uint _id;
	const vec3 _pos;
	const vec3 _intensity;

public:
	Light(uint id, const vec3& pos, const vec3& intensity)
		:
		_id(id), _pos(pos), _intensity(intensity) { };

	virtual ~Light() {}

	uint id() { return _id; }
	const vec3& pos() const { return _pos; }
	const vec3& intensity() const { return _intensity; }

	//inline void attenuate(const Ray& ray, HitInfo& outHit);
	inline virtual vec3 contribute(const vec3& hitPos, const vec3& lightPos) = 0;
	inline virtual const vec3 directionToLight(const vec3& hitPos, vec3& out_lightPos) const = 0;
};

#endif