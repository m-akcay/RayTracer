#include "SceneParser.h"
#include "../Material/MirrorMat.h"
#include "../Material/ConductorMat.h"
#include "../Material/DielectricMat.h"
#include "happly.h"
#include "../Shape/SmoothTriangle.h"
#include "../Shape/Transformation.h"
#include "../Light/AreaLight.h"
#include <sstream>


std::vector< vec3 > Hittable::vertices;
std::vector< vec3 > Hittable::normals;

std::vector< Mesh* >     Hittable::meshes;
std::vector< Triangle* > Hittable::triangles;
std::vector< Sphere* >   Hittable::spheres;

std::vector< mat4* > Transformation::translations;
std::vector< mat4* > Transformation::rotations;
std::vector< mat4* > Transformation::scalings;


Scene* SceneParser::createScene(const string& fileName)
{
	const char *str;
	XMLDocument xmlDoc;
	XMLError eResult;
	XMLElement *pElement;
	uint elemCount;

	float x, y, z;
	vec3 backgroundColor;
	vec3 ambientLight;
	float intersectionTestEps = 0.000001f;
	float shadowRayEps = 0.001f;
	std::vector< Camera* > cameras;
	std::vector< Material* > materials;
	std::vector< vec3 > vertices;
	std::vector< Hittable* > objects;
	std::vector< Light* > lights;


	eResult = xmlDoc.LoadFile(fileName.c_str());

	XMLNode *pRoot = xmlDoc.FirstChild();

	pElement = pRoot->FirstChildElement("BackgroundColor");
	str = pElement->GetText();
	sscanf(str, "%f %f %f", &x, &y, &z);
	backgroundColor = vec3(x, y, z);
	pElement = pRoot->FirstChildElement("ShadowRayEpsilon");
	if(pElement != nullptr)
		pElement->QueryFloatText(&shadowRayEps);
	//Hittable::shadowRayEps = shadowRayEps;

	pElement = pRoot->FirstChildElement("IntersectionTestEpsilon");
	if(pElement != nullptr)
		eResult = pElement->QueryFloatText(&intersectionTestEps);
	//Hittable::intersectionTestEps = intersectionTestEps;

	// Parse cameras
	readCameras(pRoot, cameras);
	//std::cout << "CAM_0->width->" << cameras[0]->imgWidth() << std::endl;
	
	//Parse materals
	readMaterials(pRoot, materials);
	//std::cout << "MAT_3->diffuseRef->" << glm::to_string(materials[3]->diffuseRef()) << std::endl;

	// Parse vertex data
	readVertices(pRoot, vertices);
	Hittable::vertices = vertices;

	readTransformations(pRoot);
	// // Parse objects
	
	readObjects(pRoot, fileName, vertices, materials, objects);
	

	// // Parse lights
	readLights(pRoot, lights, ambientLight);

	size_t totalTris = 0;
	for (auto obj : objects)
	{
		Triangle* tri = dynamic_cast<Triangle*> (obj);
		if (tri)
			totalTris++;
	}
	std::cout << std::endl << "number of triangles -> " << totalTris << std::endl << std::endl;

	try
	{
		return new Scene(backgroundColor, ambientLight, shadowRayEps, intersectionTestEps,
									cameras, materials, vertices, objects, lights);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return nullptr;
	}
	
	
}

void SceneParser::readCameras(XMLNode* pRoot, std::vector< Camera* >& out_cameras)
{
	XMLElement* pElement = pRoot->FirstChildElement("Cameras");
	XMLElement *pCamera = pElement->FirstChildElement("Camera");
	XMLElement *camElement;
	XMLError eResult;
	const char* str;

	while (pCamera != nullptr)
	{
        int id;
        char imageName[64];
		float x, y, z;
        vec3 pos, gaze, up;
		float left, right, bottom, top;
		int width, height;
		float distance;
		const char* str2;
		eResult = pCamera->QueryIntAttribute("id", &id);
		eResult = pCamera->QueryStringAttribute("type", &str2);

		if (eResult == XMLError::XML_SUCCESS)
		{
			if (string(str2) == "lookAt")
			{
				float fovY;
				vec3 gazePoint;

				camElement = pCamera->FirstChildElement("Position");
				str = camElement->GetText();
				sscanf(str, "%f %f %f", &x, &y, &z);
				pos = vec3(x, y, z);

				camElement = pCamera->FirstChildElement("GazePoint");
				str = camElement->GetText();
				sscanf(str, "%f %f %f", &x, &y, &z);
				gazePoint = vec3(x, y, z);

				camElement = pCamera->FirstChildElement("FovY");
				eResult = camElement->QueryFloatText(&fovY);

				camElement = pCamera->FirstChildElement("Up");
				str = camElement->GetText();
				sscanf(str, "%f %f %f", &x, &y, &z);
				up = vec3(x, y, z);

				camElement = pCamera->FirstChildElement("NearDistance");
				eResult = camElement->QueryFloatText(&distance);
				
				camElement = pCamera->FirstChildElement("ImageResolution");	
				str = camElement->GetText();
				sscanf(str, "%d %d", &width, &height);

				camElement = pCamera->FirstChildElement("ImageName");
				str = camElement->GetText();

				float radians = fovY * M_PI / 180;
				float theta = radians;
				top = tan(fovY / 2) * distance;
				bottom = -top;
				right = ((float)width / (float)height) * top;
				left = -right;

				gaze = glm::normalize(gazePoint - pos);

				strcpy(imageName, str);
				out_cameras.push_back( new Camera(id, pos, gaze, up, 
														ImagePlane(left, right, bottom, top, distance, vec2(width, height), string(imageName))));
			}
			pCamera = pCamera->NextSiblingElement("Camera");
			continue;
		}
		
		camElement = pCamera->FirstChildElement("Position");
		str = camElement->GetText();
		sscanf(str, "%f %f %f", &x, &y, &z);
		pos = vec3(x, y, z);
		camElement = pCamera->FirstChildElement("Gaze");
		str = camElement->GetText();
		sscanf(str, "%f %f %f", &x, &y, &z);
		gaze = vec3(x, y, z);
		camElement = pCamera->FirstChildElement("Up");
		str = camElement->GetText();
		sscanf(str, "%f %f %f", &x, &y, &z);
		up = vec3(x, y, z);
		camElement = pCamera->FirstChildElement("NearPlane");
		str = camElement->GetText();
		sscanf(str, "%f %f %f %f", &left, &right, &bottom, &top);
		camElement = pCamera->FirstChildElement("NearDistance");
		eResult = camElement->QueryFloatText(&distance);
		camElement = pCamera->FirstChildElement("ImageResolution");	
		str = camElement->GetText();
		sscanf(str, "%d %d", &width, &height);
		camElement = pCamera->FirstChildElement("ImageName");
		str = camElement->GetText();
		strcpy(imageName, str);

		out_cameras.push_back( new Camera(id, pos, gaze, up, 
														ImagePlane(left, right, bottom, top, distance, vec2(width, height), string(imageName))));

		pCamera = pCamera->NextSiblingElement("Camera");
	}
}

void SceneParser::readMaterials(XMLNode* pRoot, std::vector< Material* >& out_materials)
{
	XMLElement* pElement = pRoot->FirstChildElement("Materials");
	XMLElement* pMaterial = pElement->FirstChildElement("Material");
	XMLElement* materialElement;
	XMLError eResult;
	const char* str;

	while(pMaterial != nullptr)
	{
		int id;
		float x, y, z;
		vec3 ambientRef, diffuseRef, specularRef;
		int phongExp;
		const char* mType;

		eResult = pMaterial->QueryIntAttribute("id", &id);
		materialElement = pMaterial->FirstChildElement("AmbientReflectance");
		str = materialElement->GetText();
		sscanf(str, "%f %f %f", &x, &y, &z);
		ambientRef = vec3(x, y, z);
		materialElement = pMaterial->FirstChildElement("DiffuseReflectance");
		str = materialElement->GetText();
		sscanf(str, "%f %f %f", &x, &y, &z);
		diffuseRef = vec3(x, y, z);
		materialElement = pMaterial->FirstChildElement("SpecularReflectance");
		str = materialElement->GetText();
		sscanf(str, "%f %f %f", &x, &y, &z);
		specularRef = vec3(x, y, z);

		materialElement = pMaterial->FirstChildElement("PhongExponent");
		if (materialElement != nullptr)
			materialElement->QueryIntText(&phongExp);
		else
			phongExp = 0;

		eResult = pMaterial->QueryStringAttribute("type", &mType);
		if (eResult == tinyxml2::XML_SUCCESS)
		{
			string _mType(mType);
			if (_mType == "mirror")
			{
				vec3 mirrorRef;
				float roughness;

				materialElement = pMaterial->FirstChildElement("MirrorReflectance");
				str = materialElement->GetText();
				sscanf(str, "%f %f %f", &x, &y, &z);
				mirrorRef = vec3(x, y, z);
				materialElement = pMaterial->FirstChildElement("Roughness");
				if (materialElement != nullptr)
					materialElement->QueryFloatText(&roughness);
				else
					roughness = 0.0f;
				//std::cout << "roughness->" << roughness << std::endl;
				out_materials.push_back(new MirrorMat(id, ambientRef, MType::MIRROR, diffuseRef, specularRef, mirrorRef, phongExp, roughness));
			}
			else if (_mType == "conductor")
			{
				vec3 mirrorRef;
				float refractionIdx, absorptionIdx;
				
				materialElement = pMaterial->FirstChildElement("MirrorReflectance");
				str = materialElement->GetText();
				sscanf(str, "%f %f %f", &x, &y, &z);
				mirrorRef = vec3(x, y, z);

				materialElement = pMaterial->FirstChildElement("RefractionIndex");
				materialElement->QueryFloatText(&refractionIdx);

				materialElement = pMaterial->FirstChildElement("AbsorptionIndex");
				materialElement->QueryFloatText(&absorptionIdx);

				out_materials.push_back(new ConductorMat(id, ambientRef, MType::CONDUCTOR, diffuseRef, specularRef, phongExp, mirrorRef, refractionIdx, absorptionIdx));
				//std::cout << glm::to_string(mirrorRef) << "   " << refractionIdx << " " << absorptionIdx << std::endl;
			}
			else if (_mType == "dielectric")
			{
				float refractionIdx;
				vec3 absorptionCoeff;
				
				materialElement = pMaterial->FirstChildElement("AbsorptionCoefficient");
				str = materialElement->GetText();
				sscanf(str, "%f %f %f", &x, &y, &z);
				absorptionCoeff = vec3(x, y, z);

				materialElement = pMaterial->FirstChildElement("RefractionIndex");
				materialElement->QueryFloatText(&refractionIdx);

				out_materials.push_back(new DielectricMat(id, ambientRef, MType::DIELECTRIC, diffuseRef, specularRef, phongExp, absorptionCoeff, refractionIdx));
			}
		}
		else
			out_materials.push_back(new Material(id, ambientRef, MType::SPECULAR, diffuseRef, specularRef, phongExp));

		pMaterial = pMaterial->NextSiblingElement("Material");
	}
}

void SceneParser::readVertices(XMLNode* pRoot, std::vector< vec3 >& out_vertices)
{
	XMLElement* pElement = pRoot->FirstChildElement("VertexData");
	float x, y, z;
	int cursor = 0;
	vec3 tmpPoint;
	const char* str = pElement->GetText();
	while(str[cursor] == ' ' || str[cursor] == '\t' || str[cursor] == '\n')
		cursor++;
	while(str[cursor] != '\0')
	{
		for(int cnt = 0 ; cnt < 3 ; cnt++)
		{
			if(cnt == 0)
				x = atof(str + cursor);
			else if(cnt == 1)
				y = atof(str + cursor);
			else
				z = atof(str + cursor);
			while(str[cursor] != ' ' && str[cursor] != '\t' && str[cursor] != '\n')
				cursor++; 
			while(str[cursor] == ' ' || str[cursor] == '\t' || str[cursor] == '\n')
				cursor++;
		}
		tmpPoint = vec3(x, y, z);
		out_vertices.push_back(tmpPoint);
	}
}

void SceneParser::readTransformations(XMLNode* pRoot)
{
	XMLElement* pElement = pRoot->FirstChildElement("Transformations");
	if (pElement == nullptr)
		return;
	XMLError eResult;
	XMLElement *objElement;
	const char* str;

	XMLElement* pObject = pElement->FirstChildElement("Translation");
	while(pObject != nullptr)
	{
		int id;
		float x, y, z;

		eResult = pObject->QueryIntAttribute("id", &id);
		str = pObject->GetText();
		sscanf(str, "%f %f %f", &x, &y, &z);
		glm::mat4* translationMat = new glm::mat4(1.0f);
		*translationMat = glm::translate(*translationMat, vec3(x, y, z));
		Transformation::addTransformation(TRANSLATION, translationMat);

		pObject = pObject->NextSiblingElement("Translation");
	}

	pObject = pElement->FirstChildElement("Scaling");
	while(pObject != nullptr)
	{
		int id;
		float x, y, z;

		eResult = pObject->QueryIntAttribute("id", &id);
		str = pObject->GetText();
		sscanf(str, "%f %f %f", &x, &y, &z);
		glm::mat4* scalingMat = new glm::mat4(1.0f);
		*scalingMat = glm::scale(*scalingMat, vec3(x, y, z));
		Transformation::addTransformation(SCALING, scalingMat);

		pObject = pObject->NextSiblingElement("Scaling");
	}

	pObject = pElement->FirstChildElement("Rotation");
	while(pObject != nullptr)
	{
		int id;
		float x, y, z, degs;

		eResult = pObject->QueryIntAttribute("id", &id);
		str = pObject->GetText();
		sscanf(str, "%f %f %f %f", &degs, &x, &y, &z);
		glm::mat4* rotationMat = new glm::mat4(1.0f);
		*rotationMat = glm::rotate(*rotationMat, glm::radians(degs), vec3(x, y, z));
		Transformation::addTransformation(ROTATION, rotationMat);

		pObject = pObject->NextSiblingElement("Rotation");
	}

	// mat4 composite(1.0f);
	// for (int i = 0; i < 3; i++)
	// {
	// 	//std::cout << glm::to_string(*Transformation::getTransformation(TRANSLATION, i)) << std::endl;
	// 	composite *= *Transformation::getTransformation(TRANSLATION, i);
	// }
	// std::cout << glm::to_string(composite) << std::endl;
	// //std::cout << glm::to_string(*Transformation::getTransformation(ROTATION, 0)) << std::endl;


	// exit(0);
}


void SceneParser::readObjects(XMLNode* pRoot, const string& fileName, std::vector< vec3 >& vertices, const std::vector< Material* >& materials, std::vector< Hittable* >& out_objects)
{
	XMLElement* pElement = pRoot->FirstChildElement("Objects");
	XMLError eResult;
	// Parse spheres
	XMLElement *pObject = pElement->FirstChildElement("Sphere");
	XMLElement *objElement;
	const char* str;
	const char* ply;

	while(pObject != nullptr)
	{
		int id;
		int matIndex;
		int cIndex;
		float R;

		eResult = pObject->QueryIntAttribute("id", &id);
		objElement = pObject->FirstChildElement("Material");
		eResult = objElement->QueryIntText(&matIndex);
		objElement = pObject->FirstChildElement("Center");
		eResult = objElement->QueryIntText(&cIndex);
		objElement = pObject->FirstChildElement("Radius");
		eResult = objElement->QueryFloatText(&R);

		out_objects.push_back(new Sphere(id, cIndex - 1, R, materials[matIndex - 1]));

		pObject = pObject->NextSiblingElement("Sphere");
	}

	// Parse triangles
	pObject = pElement->FirstChildElement("Triangle");
	while(pObject != nullptr)
	{
		int id;
		int matIndex;
		int p1Index;
		int p2Index;
		int p3Index;

		eResult = pObject->QueryIntAttribute("id", &id);
		objElement = pObject->FirstChildElement("Material");
		eResult = objElement->QueryIntText(&matIndex);
		objElement = pObject->FirstChildElement("Indices");
		str = objElement->GetText();
		sscanf(str, "%d %d %d", &p1Index, &p2Index, &p3Index);

		bool transformed = false;
		objElement = pObject->FirstChildElement("Transformations");
		mat4 transform;
		if (objElement != nullptr)
		{
			transformed = true;
			str = objElement->GetText();
			transform = Transformation::getCompositeTransformation(str);
		}
		
		if (transformed)
		{
			vec3 v1, v2, v3;
			v1 = transform * vec4(vertices[p1Index - 1], 1.0f);
			v2 = transform * vec4(vertices[p2Index - 1], 1.0f);
			v3 = transform * vec4(vertices[p3Index - 1], 1.0f);
			out_objects.push_back(new Triangle(id, materials[matIndex - 1], 
								v1, v2, v3));
		}
		else
		{
			out_objects.push_back(new Triangle(id, materials[matIndex - 1], 
								vertices[p1Index - 1], vertices[p2Index - 1], vertices[p3Index - 1]));
<<<<<<< Updated upstream

=======
		}
		
		//out_objects.push_back(new SmoothTriangle(id, materials[matIndex - 1], p1Index - 1, p2Index - 1, p3Index - 1));
		
>>>>>>> Stashed changes
		pObject = pObject->NextSiblingElement("Triangle");
	}

	// Parse meshes
	pObject = pElement->FirstChildElement("Mesh");
	while(pObject != nullptr)
	{
		int id;
		int matIndex;
		int p1Index;
		int p2Index;
		int p3Index;
		int cursor = 0;
		int vertexOffset = 0;
		std::vector<Triangle*> faces;
		float minx, miny, minz, maxx, maxy, maxz;
		minx = miny = minz = INFINITY;
		maxx = maxy = maxz = -INFINITY;
		
		bool smoothShading = false, transformed = false;

		eResult = pObject->QueryIntAttribute("id", &id);
		eResult = pObject->QueryStringAttribute("shadingMode", &str);
		if (eResult == XMLError::XML_SUCCESS)
		{
			string shadingMode(str);
			if (shadingMode == "smooth")
				smoothShading = true;
		}
		objElement = pObject->FirstChildElement("Material");
		eResult = objElement->QueryIntText(&matIndex);
<<<<<<< Updated upstream
		objElement = pObject->FirstChildElement("Faces");
		eResult = objElement->QueryStringAttribute("plyFile", &ply);

=======
		
		std::vector< Hittable* > tris;
		//std::vector< vec3 > transformedVertices;
		objElement = pObject->FirstChildElement("Transformations");
		mat4 transform;
		if (objElement != nullptr)
		{
			transformed = true;
			str = objElement->GetText();
			transform = Transformation::getCompositeTransformation(str);
		}

		objElement = pObject->FirstChildElement("Faces");
		eResult = objElement->QueryStringAttribute("plyFile", &ply);
		std::vector< vec3 > mVertices;
		std::vector< std::vector< size_t > > indices;
>>>>>>> Stashed changes
		if (eResult == XMLError::XML_SUCCESS)
		{
			std::istringstream stream(fileName);
			string plyPath;
			getline(stream, plyPath, '/');
			plyPath += "/inputs/" + string(ply);

			// TODO 
			// if shadingMode is smooth
			// call smoothParsePly

			
			if (transformed)
				parsePly(plyPath.c_str(), materials[matIndex - 1], vertices, mVertices, indices, transform, tris);
			else
				parsePly(plyPath.c_str(), materials[matIndex - 1], vertices, tris);
		}
		else
		{
<<<<<<< Updated upstream
			std::vector< Hittable* > tris;
=======
			if (transformed)
			{
				for (auto v : vertices)
				{
					vec3 transformedV = transform * vec4(v, 1.0f);
					mVertices.push_back(transformedV);
				}
			}
>>>>>>> Stashed changes
			
			objElement->QueryIntAttribute("vertexOffset", &vertexOffset);
			str = objElement->GetText();
			while(str[cursor] == ' ' || str[cursor] == '\t' || str[cursor] == '\n')
				cursor++;
			while(str[cursor] != '\0')
			{
				for(int cnt = 0 ; cnt < 3 ; cnt++)
				{
					if(cnt == 0)
						p1Index = atoi(str + cursor) + vertexOffset;
					else if(cnt == 1)
						p2Index = atoi(str + cursor) + vertexOffset;
					else
						p3Index = atoi(str + cursor) + vertexOffset;
					while(str[cursor] != ' ' && str[cursor] != '\t' && str[cursor] != '\n')
						cursor++; 
					while(str[cursor] == ' ' || str[cursor] == '\t' || str[cursor] == '\n')
						cursor++;
				}
				
				Triangle* tri;
				if (transformed)
					tri = new Triangle(0, materials[matIndex - 1], p1Index, p2Index, p3Index, mVertices[p1Index - 1], mVertices[p2Index - 1], mVertices[p3Index - 1]);
				else
					tri = new Triangle(0, materials[matIndex - 1], p1Index, p2Index, p3Index, vertices[p1Index - 1], vertices[p2Index - 1], vertices[p3Index - 1]);

				tris.push_back(tri);
			}	
		}
			
<<<<<<< Updated upstream
			Mesh* mesh = new Mesh(id, materials[matIndex - 1], tris);
			out_objects.push_back(mesh);
=======
		std::cout << tris.size() << std::endl;

		Mesh* mesh = new Mesh(id, materials[matIndex - 1], tris);
		mesh->setBaseVertices(mVertices);
		mesh->setIndices(indices);
		out_objects.push_back(mesh);
		Hittable::meshes.push_back(mesh);

		pObject = pObject->NextSiblingElement("Mesh");
	}
	
	pObject = pElement->FirstChildElement("MeshInstance");
	while(pObject != nullptr)
	{
		int id, matIndex;
		int baseMeshId;
		bool resetTransform;
		eResult = pObject->QueryIntAttribute("id", &id);
		eResult = pObject->QueryIntAttribute("baseMeshId", &baseMeshId);
		eResult = pObject->QueryBoolAttribute("resetTransform", &resetTransform);
		objElement = pObject->FirstChildElement("Material");
		eResult = objElement->QueryIntText(&matIndex);
		
		Mesh* baseMesh = Hittable::meshes[baseMeshId - 1];
		std::vector< vec3 > transformedVertices;
		
		objElement = pObject->FirstChildElement("Transformations");
		mat4 transform;
		if (objElement != nullptr)
		{
			str = objElement->GetText();
			transform = Transformation::getCompositeTransformation(str);
			// for (auto v : vertices)
			// {
			// 	vec3 transformedV = transform * vec4(v, 1.0f);
			// 	transformedVertices.push_back(transformedV);
			// }
		}

		if (!resetTransform)
			transform = baseMesh->getTransform() * transform;

		std::vector< vec3 > mVertices = baseMesh->getBaseVertices();
		std::vector< std::vector< size_t > > indices = baseMesh->getIndices();
		std::vector< Hittable* > tris;
		for (auto v : baseMesh->getBaseVertices())
		{
			vec3 transformedV = transform * vec4(v, 1.0f);
			transformedVertices.push_back(transformedV);
		}

		for (auto i : indices)
		{
			vec3 v1 = transformedVertices[i[0]];
			vec3 v2 = transformedVertices[i[1]];
			vec3 v3 = transformedVertices[i[2]];

			Triangle* tri = new Triangle(0, materials[matIndex - 1], i[0], i[1], i[2], v1, v2, v3);
			
			tris.push_back(tri);
>>>>>>> Stashed changes
		}
		std::cout << "P3" << std::endl;

<<<<<<< Updated upstream
		pObject = pObject->NextSiblingElement("Mesh");
=======
		Mesh* mesh = new Mesh(id, materials[matIndex - 1], tris);
		out_objects.push_back(mesh);
		Hittable::meshes.push_back(mesh);
		std::cout << "P4" << std::endl;

		pObject = pObject->NextSiblingElement("MeshInstance");
>>>>>>> Stashed changes
	}
}


void SceneParser::readLights(XMLNode* pRoot, std::vector< Light* >& out_lights, vec3& out_ambientLight)
{
	int id;
	float x, y, z;
	vec3 position;
	
	XMLElement* pElement = pRoot->FirstChildElement("Lights");
	XMLError eResult;

	XMLElement *pLight = pElement->FirstChildElement("AmbientLight");
	XMLElement *lightElement;
	
	const char* str = pLight->GetText();
	sscanf(str, "%f %f %f", &x, &y, &z);
	out_ambientLight = vec3(x, y, z);
	
	pLight = pElement->FirstChildElement("PointLight");
	while(pLight != nullptr)
	{
		vec3 intensity;

		eResult = pLight->QueryIntAttribute("id", &id);
		lightElement = pLight->FirstChildElement("Position");
		str = lightElement->GetText();
		sscanf(str, "%f %f %f", &x, &y, &z);
		position = vec3(x, y, z);
		lightElement = pLight->FirstChildElement("Intensity");
		str = lightElement->GetText();
		sscanf(str, "%f %f %f", &x, &y, &z);
		intensity = vec3(x, y, z);

		out_lights.push_back(new PointLight(id, position, intensity));

		pLight = pLight->NextSiblingElement("PointLight");
	}

	pLight = pElement->FirstChildElement("AreaLight");
	while(pLight != nullptr)
	{
		vec3 normal;
		vec3 radiance;
		float size;

		eResult = pLight->QueryIntAttribute("id", &id);
		lightElement = pLight->FirstChildElement("Position");
		str = lightElement->GetText();
		sscanf(str, "%f %f %f", &x, &y, &z);
		position = vec3(x, y, z);
		lightElement = pLight->FirstChildElement("Radiance");
		str = lightElement->GetText();
		sscanf(str, "%f %f %f", &x, &y, &z);
		radiance = vec3(x, y, z);
		lightElement = pLight->FirstChildElement("Normal");
		str = lightElement->GetText();
		sscanf(str, "%f %f %f", &x, &y, &z);
		normal = vec3(x, y, z);
		lightElement = pLight->FirstChildElement("Size");
		lightElement->QueryFloatText(&size);

		out_lights.push_back(new AreaLight(id, position, radiance, size, normal));

		pLight = pLight->NextSiblingElement("PointLight");
	}
}


void SceneParser::parsePly(const char* plyPath, Material* mat, std::vector< vec3 >& vertices, std::vector< Hittable* >& out_objects)
{
	happly::PLYData plyIn(plyPath);
	
	// Get mesh-style data from the object
	std::vector<std::array<double, 3>> vPos = plyIn.getVertexPositions();
	std::vector<std::vector<size_t>> fInd = plyIn.getFaceIndices<size_t>();

	// vertices already contains something
	size_t baseVertIdx = vertices.size();

	for (auto v : vPos)
	{
<<<<<<< Updated upstream
=======
		// vec3 rvert = rotmat * vec4(v[0], v[1], v[2], 1.0f);
		// vertices.push_back(rvert);
		// Hittable::vertices.push_back(rvert);
>>>>>>> Stashed changes
		vertices.push_back(vec3(v[0], v[1], v[2]));
		Hittable::vertices.push_back(vec3(v[0], v[1], v[2]));
	}

<<<<<<< Updated upstream
=======

>>>>>>> Stashed changes
	for (auto i : fInd)
	{
		vec3 v1 = vertices[baseVertIdx + i[0]];
		vec3 v2 = vertices[baseVertIdx + i[1]];
		vec3 v3 = vertices[baseVertIdx + i[2]];

		Triangle* tri = new Triangle(0, mat, i[0], i[1], i[2], v1, v2, v3);
		
		out_objects.push_back(tri);
	}

}

void SceneParser::parsePly(const char* plyPath, Material* mat, std::vector< vec3 >& vertices, 
						std::vector< vec3 >& meshVertices, std::vector< std::vector< size_t > >& indices,
						 std::vector< Hittable* >& out_objects)
{
	happly::PLYData plyIn(plyPath);
	
	// Get mesh-style data from the object
	std::vector<std::array<double, 3>> vPos = plyIn.getVertexPositions();
	std::vector<std::vector<size_t>> fInd = plyIn.getFaceIndices<size_t>();

	// vertices already contains something
	size_t baseVertIdx = vertices.size();

	for (auto v : vPos)
	{
		meshVertices.push_back(vec3(v[0], v[1], v[2]));
		vertices.push_back(vec3(v[0], v[1], v[2]));
		Hittable::vertices.push_back(vec3(v[0], v[1], v[2]));
	}

<<<<<<< Updated upstream
		Triangle* tri = new Triangle(0, mat, v1, v2, v3);
=======

	for (auto i : fInd)
	{
		indices.push_back(i);

		vec3 v1 = meshVertices[i[0]];
		vec3 v2 = meshVertices[i[1]];
		vec3 v3 = meshVertices[i[2]];

		Triangle* tri = new Triangle(0, mat, i[0], i[1], i[2], v1, v2, v3);
		
>>>>>>> Stashed changes
		out_objects.push_back(tri);
	}
}

<<<<<<< Updated upstream
=======


void SceneParser::parsePly(const char* plyPath, Material* mat, std::vector< vec3 >& vertices, 
							const mat4& transform, std::vector< Hittable* >& out_objects)
{
	happly::PLYData plyIn(plyPath);
	
	// Get mesh-style data from the object
	std::vector<std::array<double, 3>> vPos = plyIn.getVertexPositions();
	std::vector<std::vector<size_t>> fInd = plyIn.getFaceIndices<size_t>();

	// vertices already contains something
	size_t baseVertIdx = vertices.size();

	for (auto v : vPos)
	{
		vec3 rvert = transform * vec4(v[0], v[1], v[2], 1.0f);
		vertices.push_back(rvert);
		Hittable::vertices.push_back(rvert);
		// vertices.push_back(vec3(v[0], v[1], v[2]));
		// Hittable::vertices.push_back(vec3(v[0], v[1], v[2]));
	}


	for (auto i : fInd)
	{
		vec3 v1 = vertices[baseVertIdx + i[0]];
		vec3 v2 = vertices[baseVertIdx + i[1]];
		vec3 v3 = vertices[baseVertIdx + i[2]];

		Triangle* tri = new Triangle(0, mat, i[0], i[1], i[2], v1, v2, v3);
		
		out_objects.push_back(tri);
	}
>>>>>>> Stashed changes
}


void SceneParser::parsePly(const char* plyPath, Material* mat, std::vector< vec3 >& vertices, 
						std::vector< vec3 >& meshVertices, std::vector< std::vector< size_t > >& indices,
						const mat4& transform, std::vector< Hittable* >& out_objects)
{
	happly::PLYData plyIn(plyPath);
	
	// Get mesh-style data from the object
	std::vector<std::array<double, 3>> vPos = plyIn.getVertexPositions();
	std::vector<std::vector<size_t>> fInd = plyIn.getFaceIndices<size_t>();

	// vertices already contains something
	size_t baseVertIdx = vertices.size();

	for (auto v : vPos)
	{
		meshVertices.push_back(vec3(v[0], v[1], v[2]));
		
		vec3 rvert = transform * vec4(v[0], v[1], v[2], 1.0f);
		vertices.push_back(rvert);
		Hittable::vertices.push_back(rvert);
		// vertices.push_back(vec3(v[0], v[1], v[2]));
		// Hittable::vertices.push_back(vec3(v[0], v[1], v[2]));
	}


	for (auto i : fInd)
	{
		indices.push_back(i);
		vec3 v1 = vertices[baseVertIdx + i[0]];
		vec3 v2 = vertices[baseVertIdx + i[1]];
		vec3 v3 = vertices[baseVertIdx + i[2]];

		Triangle* tri = new Triangle(0, mat, i[0], i[1], i[2], v1, v2, v3);
		
		out_objects.push_back(tri);
	}
}
