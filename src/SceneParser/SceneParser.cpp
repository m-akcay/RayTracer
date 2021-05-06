#include "SceneParser.h"
#include "../Material/MirrorMat.h"
#include "../Material/ConductorMat.h"
#include "../Material/DielectricMat.h"
#include "happly.h"
#include <sstream>


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
	std::vector< PointLight* > lights;


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

		//cameras.push_back(new Camera(id, imageName, pos, gaze, up, imgPlane));
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

				materialElement = pMaterial->FirstChildElement("MirrorReflectance");
				str = materialElement->GetText();
				sscanf(str, "%f %f %f", &x, &y, &z);
				mirrorRef = vec3(x, y, z);

				out_materials.push_back(new MirrorMat(id, ambientRef, MType::MIRROR, diffuseRef, specularRef, mirrorRef, phongExp));
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
				//std::cout << glm::to_string(absorptionCoeff) << "   " << refractionIdx << std::endl;
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

void SceneParser::readObjects(XMLNode* pRoot, const string& fileName, const std::vector< vec3 >& vertices, const std::vector< Material* >& materials, std::vector< Hittable* >& out_objects)
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

		out_objects.push_back(new Sphere(id, vertices[cIndex - 1], R, materials[matIndex - 1]));

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

		out_objects.push_back(new Triangle(id, materials[matIndex - 1], 
								vertices[p1Index - 1], vertices[p2Index - 1], vertices[p3Index - 1]));

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

		eResult = pObject->QueryIntAttribute("id", &id);
		objElement = pObject->FirstChildElement("Material");
		eResult = objElement->QueryIntText(&matIndex);
		objElement = pObject->FirstChildElement("Faces");
		eResult = objElement->QueryStringAttribute("plyFile", &ply);
		
		
		if (eResult == XMLError::XML_SUCCESS)
		{
			// vector<string> strings;
			// istringstream f("denmark;sweden;india;us");
			// string s;    
			// while (getline(f, s, ';')) {
			// 	cout << s << endl;
			// 	strings.push_back(s);
			// }
			//string plyPath = "../" + 
			std::vector< size_t > indices;
			std::vector< vec3 > mVertices;
			std::istringstream stream(fileName);
			string plyPath;
			getline(stream, plyPath, '/');
			plyPath += "/" + string(ply);
			//std::cout << plyPath << std::endl;
			parsePly(plyPath.c_str(), materials[matIndex - 1], out_objects);
		}
		else
		{
			///////////////////////////////////////////////////////////////////////////////////////////
			// std::vector< Triangle* > tris;
			// std::vector< vec3 > normals(vertices.size(), vec3(0.0f));
			// std::vector< size_t > normalTriCount(vertices.size(), 1); 

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

				Triangle* tri = new Triangle(0, materials[matIndex - 1], p1Index, p2Index, p3Index, vertices[p1Index - 1], vertices[p2Index - 1], vertices[p3Index - 1]);
				out_objects.push_back(tri);
				//tris.push_back(tri);
				// normals[p1Index - 1] += tri->normal(); 
				// normals[p2Index - 1] += tri->normal();
				// normals[p3Index - 1] += tri->normal();
				// normalTriCount[p1Index - 1]++; 
				// normalTriCount[p2Index - 1]++;
				// normalTriCount[p3Index - 1]++;
			}	
			
			// for (size_t i = 0; i < normals.size(); i++)
			// {
			// 	normals[i] = normals[i] / (float) normalTriCount[i];
			// 	normals[i] = glm::normalize(normals[i]);
			// }

			// for (auto tri : tris)
			// {
			// 	vec3 n1 = normals[tri->p1idx - 1];
			// 	vec3 n2 = normals[tri->p2idx - 1];
			// 	vec3 n3 = normals[tri->p3idx - 1];

			// 	vec3 normal = (n1 + n2 + n3) / 3.0f;
			// 	tri->setNormal(normal);
			// 	out_objects.push_back(tri);
			// }

		}

		//out_objects.push_back(new Mesh(id, materials[matIndex - 1], faces));
		//out_objects.push_back(new Triangle(id, material[matIndex - 1],))
		//std::cout << faces.size() << std::endl;
		pObject = pObject->NextSiblingElement("Mesh");
	}
}


void SceneParser::readLights(XMLNode* pRoot, std::vector< PointLight* >& out_lights, vec3& out_ambientLight)
{
	int id;
	vec3 position;
	vec3 intensity;
	float x, y, z;
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
}


void SceneParser::parsePly(const char* plyPath, Material* mat, std::vector< Hittable* >& out_objects)
{
	//size_t startIdx = out_objects.size();
	//std::vector< Triangle* > tris;

	happly::PLYData plyIn(plyPath);
	std::vector< vec3 > vertices;
	
// Get mesh-style data from the object
	std::vector<std::array<double, 3>> vPos = plyIn.getVertexPositions();
	std::vector<std::vector<size_t>> fInd = plyIn.getFaceIndices<size_t>();

	for (auto v : vPos)
	{
		//std::cout << v[0] << " " << v[1] << " " << v[2] << std::endl;
		vertices.push_back(vec3(v[0], v[1], v[2]));
	}

	// std::vector< vec3 > normals(vertices.size(), vec3(0.0f));
	// std::vector< size_t > normalTriCount(vertices.size(), 1); 

	for (auto i : fInd)
	{
		//std::cout << i[0] << " " << i[1] << " " << i[2] << std::endl;
		vec3 v1 = vertices[i[0]];
		vec3 v2 = vertices[i[1]];
		vec3 v3 = vertices[i[2]];

		// normalTriCount[i[0]]++;
		// normalTriCount[i[1]]++;
		// normalTriCount[i[2]]++;

		Triangle* tri = new Triangle(0, mat, v1, v2, v3);
		// normals[i[0]] += tri->normal();
		// normals[i[1]] += tri->normal();
		// normals[i[2]] += tri->normal();

		out_objects.push_back(tri);
		//tris.push_back(tri);
	}

	//std::cout << fInd.size() << "    " << vertices.size() << std::endl;

	// for (size_t i = 0; i < normals.size(); i++)
	// {
	// 	// float multiplyValue = 1 / (float) normalTriCount;
	// 	normals[i] = normals[i] / (float)normalTriCount[i];
	// 	normals[i] = glm::normalize(normals[i]);
	// }
	

	// for (size_t i = 0; i < fInd.size(); i++)
	// {
	// 	vec3 n1 = normals[fInd[i][0]];
	// 	vec3 n2 = normals[fInd[i][1]];
	// 	vec3 n3 = normals[fInd[i][2]];

	// 	vec3 normal = (n1 + n2 + n3) / 3.0f;
	// 	tris[i]->setNormal(normal);
	// 	out_objects.push_back(tris[i]);
	// } 
	
	

	//std::cout << "end" << std::endl;
}


