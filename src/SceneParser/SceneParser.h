#ifndef _SCENE_PARSER_H_
#define _SCENE_PARSER_H_

#include "../Scene.h"

#include "tinyxml2.h"

typedef tinyxml2::XMLElement XMLElement;
typedef tinyxml2::XMLText XMLText;
typedef tinyxml2::XMLDocument XMLDocument;
typedef tinyxml2::XMLError XMLError;
typedef tinyxml2::XMLNode XMLNode;

class SceneParser
{
private:
	static vec3 readVec3();
	static void readCameras(XMLNode* pRoot, std::vector< Camera* >& cameras);
	static void readMaterials(XMLNode* pRoot, std::vector< Material* >& outVal_materials);
	static void readVertices(XMLNode* pRoot, std::vector< vec3 >& outVal_vertices);
	static void readObjects(XMLNode* pRoot, 
							const string& fileName, 
							std::vector< vec3 >& vertices, 
							const std::vector< Material* >& materials, 
							std::vector< Hittable* >& out_objects);
	static void readLights(XMLNode* pRoot, std::vector< Light* >& out_lights, vec3& out_ambientLight);
	static void parsePly(const char* plyPath, Material* mat, std::vector< vec3 >& vertices, std::vector< Hittable* >& out_objects);
	static void parsePly(const char* plyPath, Material* mat, std::vector< vec3 >& vertices,
						std::vector< vec3 >& meshVertices, std::vector< std::vector< size_t > >& indices,
						 std::vector< Hittable* >& out_objects);
	static void parsePly(const char* plyPath, Material* mat, std::vector< vec3 >& vertices, 
						const mat4& transform, std::vector< Hittable* >& out_objects);
	static void parsePly(const char* plyPath, Material* mat, std::vector< vec3 >& vertices, 
						std::vector< vec3 >& meshVertices, std::vector< std::vector< size_t > >& indices,
						const mat4& transform, std::vector< Hittable* >& out_objects);
	static void readTransformations(XMLNode* pRoot);
public:
	static Scene* createScene(const string& fileName); 
};

#endif