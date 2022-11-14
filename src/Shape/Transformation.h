#ifndef _TRANSFORMATION_H_
#define _TRANSFORMATION_H_

#include "../Includes.h"

class Transformation
{
private:
	static std::vector< mat4* > translations;
	static std::vector< mat4* > rotations;
	static std::vector< mat4* > scalings;
public:
	static void addTransformation(TransformationType ttype, mat4* transformation)
	{
		switch (ttype)
		{
		case TRANSLATION:
			translations.push_back(transformation);
			break;
		
		case SCALING:
			scalings.push_back(transformation);
			break;
		
		case ROTATION:
			rotations.push_back(transformation);
			break;
		default:
			return;
		}
	}

	static mat4* getTransformation(TransformationType ttype, uint id)
	{
		switch (ttype)
		{
		case TRANSLATION:
			return translations[id];
		
		case SCALING:
			return scalings[id];
		
		case ROTATION:
			return rotations[id];
		default:
			return nullptr;
		}
	}

	static mat4* getTransformation(char ttype, uint id)
	{
		switch (ttype)
		{
		case 't':
			return translations[id];
		
		case 's':
			return scalings[id];
		
		case 'r':
			return rotations[id];
		default:
			return nullptr;
		}
	}

	static mat4 getCompositeTransformation(const char* transformationString)
	{
		std::stringstream ss(transformationString);
		std::istream_iterator<std::string> begin(ss);
		std::istream_iterator<std::string> end;
		std::vector<std::string> vstrings(begin, end);

		mat4 transform(1.0f);

		for (auto tstr : vstrings)
		{
			char ttype;
			int tid;
			sscanf(tstr.c_str(), "%c%d", &ttype, &tid);
			
			transform *= *getTransformation(ttype, tid - 1);
		}

		return transform;
	}
};

#endif