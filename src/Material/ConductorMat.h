#ifndef _CONDUCTOR_MAT_
#define _CONDUCTOR_MAT_

#include "Material.h"

class ConductorMat : public Material
{
private:
	const vec3 _mirrorRef;
	const float _refractionIdx;
	const float _absorptionIdx;
	const float nSquared_plus_kSquared;
public:
	ConductorMat(uint id, vec3 ambientRef, MType mType, vec3 diffuseRef, vec3 specularRef, 
			float phongExp, const vec3& mirrorRef, float refractionIdx, float absorptionIdx)
	:
	Material(id, ambientRef, mType, diffuseRef, specularRef, phongExp), _mirrorRef(mirrorRef), _refractionIdx(refractionIdx), _absorptionIdx(absorptionIdx),
	nSquared_plus_kSquared((refractionIdx * refractionIdx) + (absorptionIdx * absorptionIdx))
	{
		// std::cout << _refractionIdx << "  " << _absorptionIdx << std::endl;
		// std::cout << (refractionIdx * refractionIdx) + (absorptionIdx * absorptionIdx) << std::endl;
		// std::cout << nSquared_plus_kSquared << std::endl;
	}

	const vec3& mirrorRef() { return _mirrorRef; }
	float refractionIdx() { return _refractionIdx; }
	float absorptionIdx() { return _absorptionIdx; }
	inline vec3 attenuate(float cosTheta);
	inline vec3 attenuate(const vec3& rayDir, const vec3& normal);

private:
	inline float reflectionRatio(float cosTheta);
	inline float reflectionRatio(const vec3& rayDir, const vec3& normal);
};


inline float ConductorMat::reflectionRatio(float cosTheta)
{
	// float cosTheta = glm::cos(theta);
	cosTheta = glm::clamp(cosTheta, -1.0f, 1.0f);
	float cosThetaSquared = cosTheta * cosTheta;
	float n2_cosTheta_times2 = this->_refractionIdx * cosTheta * 2;
	float Rs = (nSquared_plus_kSquared - n2_cosTheta_times2 + cosThetaSquared)
				/
				(nSquared_plus_kSquared + n2_cosTheta_times2 + cosThetaSquared);

	float Rp = (nSquared_plus_kSquared * cosThetaSquared - n2_cosTheta_times2 + 1)
				/
				(nSquared_plus_kSquared * cosThetaSquared + n2_cosTheta_times2 + 1);

	float Fr = (Rs + Rp) * 0.5f;
	return Fr;
}

inline float ConductorMat::reflectionRatio(const vec3& rayDir, const vec3& normal)
{
	// float cos = fmin(dot(rayDir, normal), 1.0);
	// float r0 = (-1 + _refractionIdx) / (1 + _refractionIdx);
	// r0 *= r0;
	// return r0 + (1 - r0) * pow((1-cos), 5);

	float dp = glm::clamp(dot(rayDir, normal), -1.0f, 1.0f);
	float rpar2 = (nSquared_plus_kSquared*dp*dp - 2*_refractionIdx*dp + 1)
					/
					(nSquared_plus_kSquared*dp*dp + 2*_refractionIdx*dp + 1)
					;
	float rperp2 = (nSquared_plus_kSquared + dp*dp - 2*_refractionIdx*dp)
					/
					(nSquared_plus_kSquared + dp*dp + 2*_refractionIdx*dp)
					;
	return (rpar2 + rperp2) * 0.5f;
}

inline vec3 ConductorMat::attenuate(float cosTheta)
{
	return this->_mirrorRef * this->reflectionRatio(cosTheta);
}

inline vec3 ConductorMat::attenuate(const vec3& rayDir, const vec3& normal)
{
	return this->_mirrorRef * this->reflectionRatio(rayDir, normal);
}

#endif