#ifndef _DIELECTRIC_MAT_
#define _DIELECTRIC_MAT_

#include "Material.h"

class DielectricMat : public Material
{
private:
	const vec3 _absorptionCoeff;
	const float _refractionIdx;

public:
	DielectricMat(uint id, const vec3& ambientRef, MType mType, const vec3& diffuseRef, const vec3& specularRef, float phongExp, const vec3& absorptionCoeff, float refractionIdx)
	:
	Material(id, ambientRef, mType, diffuseRef, specularRef, phongExp), _absorptionCoeff(absorptionCoeff), _refractionIdx(refractionIdx)
	{}

	const vec3& absorptionCoeff() { return _absorptionCoeff; }
	float refractionIdx() { return _refractionIdx; }

	inline vec3 refract(const vec3 &rayDir, const vec3 &normal);
    inline float attenuateReflected(const vec3& rayDir, const vec3& normal, vec3& out_refractedDir);
private:
	inline float reflectionRatio(float inRefractionIdx, float theta, float phi);

};

inline float DielectricMat::reflectionRatio(float inRefractionIdx, float cosTheta, float cosPhi)
{
	float Rparallel = (_refractionIdx * cosTheta - inRefractionIdx * cosPhi)
						/
						(_refractionIdx * cosTheta + inRefractionIdx * cosPhi);
	
	float Rperpendicular = (inRefractionIdx * cosTheta - _refractionIdx * cosPhi)
							/
							(inRefractionIdx * cosTheta + _refractionIdx * cosPhi);
	
	float Fr = (Rparallel * Rparallel + Rperpendicular * Rperpendicular)
				/ 2;

	return Fr;
}

inline vec3 DielectricMat::refract(const vec3 &rayDir, const vec3 &normal) 
{ 
    float cosi = glm::clamp(dot(rayDir, normal), -1.0f, 1.0f); 
    float etai = 1.0f, etat = this->_refractionIdx; 
    vec3 n = normal; 
    if (cosi < 0) 
    { 
        cosi = -cosi; 
    } 
    else 
    { 
        std::swap(etai, etat); 
        n= -normal; 
    } 
    float eta = etai / etat; 
    float k = 1 - eta * eta * (1 - cosi * cosi);

    return k < 0 ? vec3(0.0f) : eta * rayDir + (eta * cosi - sqrtf(k)) * n; 
} 


inline float DielectricMat::attenuateReflected(const vec3& rayDir, const vec3& normal, vec3& out_refractedDir)
{
    float cosTheta = glm::clamp(dot(rayDir, normal), -1.0f, 1.0f);
    float inRefractionIdx = 1;
    float thisRefractionIdx = this->_refractionIdx;

    vec3 _normal = normal;

    if (cosTheta > 0)
    {
        std::swap(inRefractionIdx, thisRefractionIdx);
        _normal = -_normal;
    }
    else
    {
        cosTheta = -cosTheta;
    }

    float refrIdxRatio = inRefractionIdx / thisRefractionIdx;
    float sinThetaSquared = 1 - cosTheta * cosTheta;

    float sinPhi = refrIdxRatio * sqrtf(std::max(0.0f, sinThetaSquared));
    
    
    if (sinPhi >= 1)
    {
        return 1;
    }
    else
    {
        float k = 1 - (refrIdxRatio * refrIdxRatio) * sinThetaSquared;
        out_refractedDir = k < 0 ? 
                            vec3(0.0f) : 
                            refrIdxRatio * rayDir + (refrIdxRatio * cosTheta - sqrtf(k)) * _normal;

        float cosPhi = std::max(0.0f, 1 - sinPhi * sinPhi);
        cosTheta = fabsf(cosTheta);
        return this->reflectionRatio(inRefractionIdx, cosTheta, cosPhi);
    }

}


#endif