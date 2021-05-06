#include "Scene.h"

void Scene::render()
{
	uint maxNumOfThreads = std::thread::hardware_concurrency();

	for (auto cam : cameras)
	{
		typedef std::chrono::high_resolution_clock high_resolution_clock;
		typedef std::chrono::duration<float> duration;

		std::cout << "CAM_" << cam->id() << std::endl;

		auto startTime = high_resolution_clock::now();

		Image* imPtr = new Image(cam->imgWidth(), cam->imgHeight());
		uint width = cam->imgWidth();
		uint height = cam->imgHeight();
		
		// source:
		// https://medium.com/@phostershop/solving-multithreaded-raytracing-issues-with-c-11-7f018ecd76fa
		// slightly slower but why?

		std::size_t max = width * height;
		std::size_t cores = std::thread::hardware_concurrency();
		volatile std::atomic<std::size_t> count(0);
		std::vector<std::future<void>> future_vector;
		while (cores--)
    	future_vector.emplace_back(
        std::async([=, &count]()
        {
            while (true)
            {
                std::size_t index = count++;
                if (index >= max)
                    break;
                uint x = index % width;
                uint y = index / width;

				Ray ray = cam->getViewRay(x, y);
				//vec3 color = trace0(ray, 0);
				vec3 color = rTrace(ray, 0);
                imPtr->setPixelValue(x, y, color);
            }
        }));

		for (size_t i = 0; i < future_vector.size(); i++)
			future_vector[i].wait();

		// non-threaded
		// for (uint x = 0; x < width; x++)
		// {
		// 	for (uint y = 0; y < height; y++)
		// 	{
		// 		Ray ray = cam->getViewRay(x, y);
		// 		vec3 color = rTrace(ray, ray, 0);
		// 		imPtr->setPixelValue(x, y, color);
		// 		//imPtr->setPixelValue(y * width + x, color);
		// 	}
		// }

		string imName = "out_" + cam->imgName();
		imPtr->saveImage(imName.c_str());
		delete imPtr;

		auto endTime = high_resolution_clock::now();
		duration timeDiff = endTime - startTime;
        std::cout << timeDiff.count() << " seconds" << std::endl << std::endl;

	}
}

vec3 Scene::rTrace(const Ray& ray, uint rdepth)
{
	if (rdepth > 6)
		return backgroundColor;
	HitInfo outHit;
	bool hit = false;

	hit = bvh->hit(ray, outHit);

	if (hit)
	{
		/// TRUE
		bool outside = dot(ray.direction(), outHit.normal) < 0;
		
		if (outside)
			outHit.color = outHit.mat->ambientRef() * ambientLight;

		if (outHit.mat->mType() == MType::MIRROR)
		{
			Ray reflected(outHit.pos + outHit.normal * shadowRayEps, glm::reflect(ray.direction(), outHit.normal));
			MirrorMat* mat = static_cast< MirrorMat* > (outHit.mat);
			outHit.color += mat->mirrorRef() * rTrace(reflected, rdepth + 1);
		}
		else if (outHit.mat->mType() == MType::CONDUCTOR)
		{
			Ray reflected(outHit.pos + outHit.normal * shadowRayEps, glm::reflect(ray.direction(), outHit.normal));
			ConductorMat* mat = static_cast< ConductorMat* > (outHit.mat);
			float cosTheta = dot(-ray.direction(), outHit.normal);
			
			//outHit.color += mat->attenuate(cosTheta) * rTrace(reflected, rdepth + 1);
			outHit.color += mat->attenuate(-ray.direction(), outHit.normal) * rTrace(reflected, rdepth + 1);
		}
		else if (outHit.mat->mType() == MType::DIELECTRIC)
		{
			DielectricMat* mat = static_cast< DielectricMat* > (outHit.mat);
			vec3 refractedDir;
			float kr = mat->attenuateReflected(ray.direction(), outHit.normal, refractedDir);
			vec3 bias = outHit.normal * shadowRayEps;
			vec3 refractedColor(0.0f);
			if (kr < 1)
			{
				vec3 refractedOrig = outside ? outHit.pos - bias : outHit.pos + bias;
				Ray refractedRay(refractedOrig, refractedDir);

				refractedColor = (1 - kr) * rTrace(refractedRay, rdepth + 1);
				if (!outside)
					refractedColor *= glm::exp(-mat->absorptionCoeff() * outHit.tVal);
			}

			vec3 reflectedDir = glm::normalize(glm::reflect(ray.direction(), outHit.normal));
			vec3 reflectedOrig = outside ? outHit.pos + bias : outHit.pos - bias;
			Ray reflectedRay(reflectedOrig, reflectedDir);

			vec3 reflectedColor = kr * rTrace(reflectedRay, rdepth + 1);
		
			outHit.color += reflectedColor + refractedColor;
		}

		for (auto light : lights)
		{
			if (!inShadow(outHit, light) && outside)
			{
				outHit.mat->attenuate(ray, light, outHit);
			}
		}
		
		outHit.color = glm::clamp(outHit.color, vec3(0.0f), vec3(255.0f));

	}

	if (hit)
		return outHit.color;
	else
		return backgroundColor;
}

vec3 Scene::trace0(const Ray& ray, uint rdepth)
{
	HitInfo outHit;
	bool hit = false;

	hit = bvh->hit(ray, outHit);
	//bvhRoot->traverse(bvhRoot);

	if (hit)
	{
		/// TRUE
		bool outside = dot(ray.direction(), outHit.normal) < 0;
		
		if (outside)
			outHit.color = outHit.mat->ambientRef() * ambientLight;

		if (outHit.mat->mType() == MType::MIRROR)
		{
			Ray reflected(outHit.pos + outHit.normal * shadowRayEps, glm::reflect(ray.direction(), outHit.normal));
			MirrorMat* mat = static_cast< MirrorMat* > (outHit.mat);
			outHit.color += mat->mirrorRef() * trace1(reflected, rdepth + 1);
		}
		else if (outHit.mat->mType() == MType::CONDUCTOR)
		{
			Ray reflected(outHit.pos + outHit.normal * shadowRayEps, glm::reflect(ray.direction(), outHit.normal));
			ConductorMat* mat = static_cast< ConductorMat* > (outHit.mat);
			float cosTheta = glm::clamp(dot(-ray.direction(), outHit.normal), -1.0f, 1.0f);
			outHit.color += mat->attenuate(cosTheta) * trace1(reflected, rdepth + 1);
		}
		else if (outHit.mat->mType() == MType::DIELECTRIC)
		{
			DielectricMat* mat = static_cast< DielectricMat* > (outHit.mat);
			
			vec3 refractedDir;
			float kr = mat->attenuateReflected(ray.direction(), outHit.normal, refractedDir);

			vec3 bias = outHit.normal * shadowRayEps;
			vec3 refractedColor(0.0f);
			if (kr < 1)
			{
				
				vec3 refractedOrig = outside ? outHit.pos - bias : outHit.pos + bias;
				Ray refractedRay(refractedOrig, refractedDir);
				refractedColor = (1 - kr) * trace1(refractedRay, rdepth + 1);
				if (!outside)
					refractedColor *= exp(-mat->absorptionCoeff() * outHit.tVal);
			}
			vec3 reflectedDir = glm::normalize(glm::reflect(ray.direction(), outHit.normal));
			vec3 reflectedOrig = outside ? outHit.pos + bias : outHit.pos - bias;
			Ray reflectedRay(reflectedOrig, reflectedDir);
			vec3 reflectedColor = kr * trace1(reflectedRay, rdepth + 1);

			outHit.color += reflectedColor + refractedColor;
		}

		for (auto light : lights)
		{
			if (!inShadow(outHit, light) && outside)
			{
				outHit.mat->attenuate(ray, light, outHit);
			}
		}
		
		outHit.color = glm::clamp(outHit.color, vec3(0.0f), vec3(255.0f));

	}

	if (hit)
		return outHit.color;
	else
		return backgroundColor;
}

vec3 Scene::trace1(const Ray& ray, uint rdepth)
{
	if (rdepth > 6)
		return backgroundColor;
	HitInfo outHit;
	bool hit = false;

	hit = bvh->hit(ray, outHit);
	//bvhRoot->traverse(bvhRoot);

	if (hit)
	{
		/// TRUE
		bool outside = dot(ray.direction(), outHit.normal) < 0;
		
		if (outside)
			outHit.color = outHit.mat->ambientRef() * ambientLight;

		if (outHit.mat->mType() == MType::MIRROR)
		{
			Ray reflected(outHit.pos + outHit.normal * shadowRayEps, glm::reflect(ray.direction(), outHit.normal));
			MirrorMat* mat = static_cast< MirrorMat* > (outHit.mat);
			outHit.color += mat->mirrorRef() * trace2(reflected, rdepth + 1);
		}
		else if (outHit.mat->mType() == MType::CONDUCTOR)
		{
			Ray reflected(outHit.pos + outHit.normal * shadowRayEps, glm::reflect(ray.direction(), outHit.normal));
			ConductorMat* mat = static_cast< ConductorMat* > (outHit.mat);
			float cosTheta = glm::clamp(dot(-ray.direction(), outHit.normal), -1.0f, 1.0f);
			outHit.color += mat->attenuate(cosTheta) * trace2(reflected, rdepth + 1);
		}
		else if (outHit.mat->mType() == MType::DIELECTRIC)
		{
			DielectricMat* mat = static_cast< DielectricMat* > (outHit.mat);
			
			vec3 refractedDir;
			float kr = mat->attenuateReflected(ray.direction(), outHit.normal, refractedDir);

			vec3 bias = outHit.normal * shadowRayEps;
			vec3 refractedColor(0.0f);
			if (kr < 1)
			{
				
				vec3 refractedOrig = outside ? outHit.pos - bias : outHit.pos + bias;
				Ray refractedRay(refractedOrig, refractedDir);
				refractedColor = (1 - kr) * trace2(refractedRay, rdepth + 1);
				if (!outside)
					refractedColor *= exp(-mat->absorptionCoeff() * outHit.tVal);
			}
			vec3 reflectedDir = glm::normalize(glm::reflect(ray.direction(), outHit.normal));
			vec3 reflectedOrig = outside ? outHit.pos + bias : outHit.pos - bias;
			Ray reflectedRay(reflectedOrig, reflectedDir);
			vec3 reflectedColor = kr * trace2(reflectedRay, rdepth + 1);

			outHit.color += reflectedColor + refractedColor;
		}

		for (auto light : lights)
		{
			if (!inShadow(outHit, light) && outside)
			{
				outHit.mat->attenuate(ray, light, outHit);
			}
		}
		
		outHit.color = glm::clamp(outHit.color, vec3(0.0f), vec3(255.0f));

	}

	if (hit)
		return outHit.color;
	else
		return backgroundColor;
}

vec3 Scene::trace2(const Ray& ray, uint rdepth)
{
	if (rdepth > 6)
		return backgroundColor;
	HitInfo outHit;
	bool hit = false;

	hit = bvh->hit(ray, outHit);
	//bvhRoot->traverse(bvhRoot);

	if (hit)
	{
		/// TRUE
		bool outside = dot(ray.direction(), outHit.normal) < 0;
		
		if (outside)
			outHit.color = outHit.mat->ambientRef() * ambientLight;

		if (outHit.mat->mType() == MType::MIRROR)
		{
			Ray reflected(outHit.pos + outHit.normal * shadowRayEps, glm::reflect(ray.direction(), outHit.normal));
			MirrorMat* mat = static_cast< MirrorMat* > (outHit.mat);
			outHit.color += mat->mirrorRef() * trace3(reflected, rdepth + 1);
		}
		else if (outHit.mat->mType() == MType::CONDUCTOR)
		{
			Ray reflected(outHit.pos + outHit.normal * shadowRayEps, glm::reflect(ray.direction(), outHit.normal));
			ConductorMat* mat = static_cast< ConductorMat* > (outHit.mat);
			float cosTheta = glm::clamp(dot(-ray.direction(), outHit.normal), -1.0f, 1.0f);
			outHit.color += mat->attenuate(cosTheta) * trace3(reflected, rdepth + 1);
		}
		else if (outHit.mat->mType() == MType::DIELECTRIC)
		{
			DielectricMat* mat = static_cast< DielectricMat* > (outHit.mat);
			
			vec3 refractedDir;
			float kr = mat->attenuateReflected(ray.direction(), outHit.normal, refractedDir);

			vec3 bias = outHit.normal * shadowRayEps;
			vec3 refractedColor(0.0f);
			if (kr < 1)
			{
				
				vec3 refractedOrig = outside ? outHit.pos - bias : outHit.pos + bias;
				Ray refractedRay(refractedOrig, refractedDir);
				refractedColor = (1 - kr) * trace3(refractedRay, rdepth + 1);
				if (!outside)
					refractedColor *= exp(-mat->absorptionCoeff() * outHit.tVal);
			}
			vec3 reflectedDir = glm::normalize(glm::reflect(ray.direction(), outHit.normal));
			vec3 reflectedOrig = outside ? outHit.pos + bias : outHit.pos - bias;
			Ray reflectedRay(reflectedOrig, reflectedDir);
			vec3 reflectedColor = kr * trace3(reflectedRay, rdepth + 1);

			outHit.color += reflectedColor + refractedColor;
		}

		for (auto light : lights)
		{
			if (!inShadow(outHit, light) && outside)
			{
				outHit.mat->attenuate(ray, light, outHit);
			}
		}
		
		outHit.color = glm::clamp(outHit.color, vec3(0.0f), vec3(255.0f));

	}

	if (hit)
		return outHit.color;
	else
		return backgroundColor;
}

vec3 Scene::trace3(const Ray& ray, uint rdepth)
{
	if (rdepth > 6)
		return backgroundColor;
	HitInfo outHit;
	bool hit = false;

	hit = bvh->hit(ray, outHit);
	//bvhRoot->traverse(bvhRoot);

	if (hit)
	{
		/// TRUE
		bool outside = dot(ray.direction(), outHit.normal) < 0;
		
		if (outside)
			outHit.color = outHit.mat->ambientRef() * ambientLight;

		if (outHit.mat->mType() == MType::MIRROR)
		{
			Ray reflected(outHit.pos + outHit.normal * shadowRayEps, glm::reflect(ray.direction(), outHit.normal));
			MirrorMat* mat = static_cast< MirrorMat* > (outHit.mat);
			outHit.color += mat->mirrorRef() * trace4(reflected, rdepth + 1);
		}
		else if (outHit.mat->mType() == MType::CONDUCTOR)
		{
			Ray reflected(outHit.pos + outHit.normal * shadowRayEps, glm::reflect(ray.direction(), outHit.normal));
			ConductorMat* mat = static_cast< ConductorMat* > (outHit.mat);
			float cosTheta = glm::clamp(dot(-ray.direction(), outHit.normal), -1.0f, 1.0f);
			outHit.color += mat->attenuate(cosTheta) * trace4(reflected, rdepth + 1);
		}
		else if (outHit.mat->mType() == MType::DIELECTRIC)
		{
			DielectricMat* mat = static_cast< DielectricMat* > (outHit.mat);
			
			vec3 refractedDir;
			float kr = mat->attenuateReflected(ray.direction(), outHit.normal, refractedDir);

			vec3 bias = outHit.normal * shadowRayEps;
			vec3 refractedColor(0.0f);
			if (kr < 1)
			{
				
				vec3 refractedOrig = outside ? outHit.pos - bias : outHit.pos + bias;
				Ray refractedRay(refractedOrig, refractedDir);
				refractedColor = (1 - kr) * trace4(refractedRay, rdepth + 1);
				if (!outside)
					refractedColor *= exp(-mat->absorptionCoeff() * outHit.tVal);
			}
			vec3 reflectedDir = glm::normalize(glm::reflect(ray.direction(), outHit.normal));
			vec3 reflectedOrig = outside ? outHit.pos + bias : outHit.pos - bias;
			Ray reflectedRay(reflectedOrig, reflectedDir);
			vec3 reflectedColor = kr * trace4(reflectedRay, rdepth + 1);

			outHit.color += reflectedColor + refractedColor;
		}

		for (auto light : lights)
		{
			if (!inShadow(outHit, light) && outside)
			{
				outHit.mat->attenuate(ray, light, outHit);
			}
		}
		
		outHit.color = glm::clamp(outHit.color, vec3(0.0f), vec3(255.0f));

	}

	if (hit)
		return outHit.color;
	else
		return backgroundColor;
}

vec3 Scene::trace4(const Ray& ray, uint rdepth)
{
	if (rdepth > 6)
		return backgroundColor;
	HitInfo outHit;
	bool hit = false;

	hit = bvh->hit(ray, outHit);
	//bvhRoot->traverse(bvhRoot);

	if (hit)
	{
		/// TRUE
		bool outside = dot(ray.direction(), outHit.normal) < 0;
		
		if (outside)
			outHit.color = outHit.mat->ambientRef() * ambientLight;

		if (outHit.mat->mType() == MType::MIRROR)
		{
			Ray reflected(outHit.pos + outHit.normal * shadowRayEps, glm::reflect(ray.direction(), outHit.normal));
			MirrorMat* mat = static_cast< MirrorMat* > (outHit.mat);
			outHit.color += mat->mirrorRef() * trace5(reflected, rdepth + 1);
		}
		else if (outHit.mat->mType() == MType::CONDUCTOR)
		{
			Ray reflected(outHit.pos + outHit.normal * shadowRayEps, glm::reflect(ray.direction(), outHit.normal));
			ConductorMat* mat = static_cast< ConductorMat* > (outHit.mat);
			float cosTheta = glm::clamp(dot(-ray.direction(), outHit.normal), -1.0f, 1.0f);
			outHit.color += mat->attenuate(cosTheta) * trace5(reflected, rdepth + 1);
		}
		else if (outHit.mat->mType() == MType::DIELECTRIC)
		{
			DielectricMat* mat = static_cast< DielectricMat* > (outHit.mat);
			
			vec3 refractedDir;
			float kr = mat->attenuateReflected(ray.direction(), outHit.normal, refractedDir);

			vec3 bias = outHit.normal * shadowRayEps;
			vec3 refractedColor(0.0f);
			if (kr < 1)
			{
				
				vec3 refractedOrig = outside ? outHit.pos - bias : outHit.pos + bias;
				Ray refractedRay(refractedOrig, refractedDir);
				refractedColor = (1 - kr) * trace5(refractedRay, rdepth + 1);
				if (!outside)
					refractedColor *= exp(-mat->absorptionCoeff() * outHit.tVal);
			}
			vec3 reflectedDir = glm::normalize(glm::reflect(ray.direction(), outHit.normal));
			vec3 reflectedOrig = outside ? outHit.pos + bias : outHit.pos - bias;
			Ray reflectedRay(reflectedOrig, reflectedDir);
			vec3 reflectedColor = kr * trace5(reflectedRay, rdepth + 1);

			outHit.color += reflectedColor + refractedColor;
		}

		for (auto light : lights)
		{
			if (!inShadow(outHit, light) && outside)
			{
				outHit.mat->attenuate(ray, light, outHit);
			}
		}
		
		outHit.color = glm::clamp(outHit.color, vec3(0.0f), vec3(255.0f));

	}

	if (hit)
		return outHit.color;
	else
		return backgroundColor;
}

vec3 Scene::trace5(const Ray& ray, uint rdepth)
{
	if (rdepth > 6)
		return backgroundColor;
	HitInfo outHit;
	bool hit = false;

	hit = bvh->hit(ray, outHit);
	//bvhRoot->traverse(bvhRoot);

	if (hit)
	{
		/// TRUE
		bool outside = dot(ray.direction(), outHit.normal) < 0;
		
		if (outside)
			outHit.color = outHit.mat->ambientRef() * ambientLight;

		if (outHit.mat->mType() == MType::MIRROR)
		{
			Ray reflected(outHit.pos + outHit.normal * shadowRayEps, glm::reflect(ray.direction(), outHit.normal));
			MirrorMat* mat = static_cast< MirrorMat* > (outHit.mat);
			outHit.color += mat->mirrorRef() * trace6(reflected, rdepth + 1);
		}
		else if (outHit.mat->mType() == MType::CONDUCTOR)
		{
			Ray reflected(outHit.pos + outHit.normal * shadowRayEps, glm::reflect(ray.direction(), outHit.normal));
			ConductorMat* mat = static_cast< ConductorMat* > (outHit.mat);
			float cosTheta = glm::clamp(dot(-ray.direction(), outHit.normal), -1.0f, 1.0f);
			outHit.color += mat->attenuate(cosTheta) * trace6(reflected, rdepth + 1);
		}
		else if (outHit.mat->mType() == MType::DIELECTRIC)
		{
			DielectricMat* mat = static_cast< DielectricMat* > (outHit.mat);
			
			vec3 refractedDir;
			float kr = mat->attenuateReflected(ray.direction(), outHit.normal, refractedDir);

			vec3 bias = outHit.normal * shadowRayEps;
			vec3 refractedColor(0.0f);
			if (kr < 1)
			{
				
				vec3 refractedOrig = outside ? outHit.pos - bias : outHit.pos + bias;
				Ray refractedRay(refractedOrig, refractedDir);
				refractedColor = (1 - kr) * trace6(refractedRay, rdepth + 1);
				if (!outside)
					refractedColor *= exp(-mat->absorptionCoeff() * outHit.tVal);
			}
			vec3 reflectedDir = glm::normalize(glm::reflect(ray.direction(), outHit.normal));
			vec3 reflectedOrig = outside ? outHit.pos + bias : outHit.pos - bias;
			Ray reflectedRay(reflectedOrig, reflectedDir);
			vec3 reflectedColor = kr * trace6(reflectedRay, rdepth + 1);

			outHit.color += reflectedColor + refractedColor;
		}

		for (auto light : lights)
		{
			if (!inShadow(outHit, light) && outside)
			{
				outHit.mat->attenuate(ray, light, outHit);
			}
		}
		
		outHit.color = glm::clamp(outHit.color, vec3(0.0f), vec3(255.0f));

	}

	if (hit)
		return outHit.color;
	else
		return backgroundColor;
}

vec3 Scene::trace6(const Ray& ray, uint rdepth)
{
	if (rdepth > 6)
		return backgroundColor;
	HitInfo outHit;
	bool hit = false;

	hit = bvh->hit(ray, outHit);
	//bvhRoot->traverse(bvhRoot);

	if (hit)
	{
		/// TRUE
		bool outside = dot(ray.direction(), outHit.normal) < 0;
		
		if (outside)
			outHit.color = outHit.mat->ambientRef() * ambientLight;

		if (outHit.mat->mType() == MType::MIRROR)
		{
			Ray reflected(outHit.pos + outHit.normal * shadowRayEps, glm::reflect(ray.direction(), outHit.normal));
			MirrorMat* mat = static_cast< MirrorMat* > (outHit.mat);
			outHit.color += mat->mirrorRef() * trace7(reflected, rdepth + 1);
		}
		else if (outHit.mat->mType() == MType::CONDUCTOR)
		{
			Ray reflected(outHit.pos + outHit.normal * shadowRayEps, glm::reflect(ray.direction(), outHit.normal));
			ConductorMat* mat = static_cast< ConductorMat* > (outHit.mat);
			float cosTheta = glm::clamp(dot(-ray.direction(), outHit.normal), -1.0f, 1.0f);
			outHit.color += mat->attenuate(cosTheta) * trace7(reflected, rdepth + 1);
		}
		else if (outHit.mat->mType() == MType::DIELECTRIC)
		{
			DielectricMat* mat = static_cast< DielectricMat* > (outHit.mat);
			
			vec3 refractedDir;
			float kr = mat->attenuateReflected(ray.direction(), outHit.normal, refractedDir);

			vec3 bias = outHit.normal * shadowRayEps;
			vec3 refractedColor(0.0f);
			if (kr < 1)
			{
				
				vec3 refractedOrig = outside ? outHit.pos - bias : outHit.pos + bias;
				Ray refractedRay(refractedOrig, refractedDir);
				refractedColor = (1 - kr) * trace7(refractedRay, rdepth + 1);
				if (!outside)
					refractedColor *= exp(-mat->absorptionCoeff() * outHit.tVal);
			}
			vec3 reflectedDir = glm::normalize(glm::reflect(ray.direction(), outHit.normal));
			vec3 reflectedOrig = outside ? outHit.pos + bias : outHit.pos - bias;
			Ray reflectedRay(reflectedOrig, reflectedDir);
			vec3 reflectedColor = kr * trace7(reflectedRay, rdepth + 1);

			outHit.color += reflectedColor + refractedColor;
		}

		for (auto light : lights)
		{
			if (!inShadow(outHit, light) && outside)
			{
				outHit.mat->attenuate(ray, light, outHit);
			}
		}
		
		outHit.color = glm::clamp(outHit.color, vec3(0.0f), vec3(255.0f));

	}

	if (hit)
		return outHit.color;
	else
		return backgroundColor;
}

vec3 Scene::trace7(const Ray& ray, uint rdepth)
{
	return backgroundColor;
}

bool Scene::inShadow(const HitInfo& hit, const PointLight* light)
{
	vec3 orig = hit.pos + 0.001f * hit.normal;
	vec3 dir = glm::normalize(light->pos() - orig);
	Ray sRay(orig, dir);
	HitInfo shadowHit;
	float lightTVal = glm::distance(light->pos(), orig);
	
	if (bvh->hit(sRay, shadowHit))
	{
		if (shadowHit.tVal > 0 && shadowHit.tVal < lightTVal)
		{
			return true;
		}
	}

	return false;
}


void Scene::createBVH()
{
	bvh = static_cast<BVH*> (BVH::construct(objects));
}


Scene::~Scene()
{
	for (auto cam : cameras)
		delete cam;
	
	for (auto mat : materials)
		delete mat;

	for (auto obj : objects)
		delete obj;

	for (auto light : lights)
		delete light;

	// const std::vector< Material* > materials;
	// const std::vector< vec3 > vertices;
	// const std::vector< Hittable* > objects;
	// const std::vector< PointLight* > lights;
}

