#include <iostream>
#include <chrono>
//#include "src/Includes.h"
#include "src/SceneParser/SceneParser.h"

int main(int nargs, const char* args[])
{
    // shared_ptr<Scene> newScene = SceneParser::createScene("hw1/cornellbox.xml");
    Scene* newScene = SceneParser::createScene(args[1]);
    std::cout << args[1] << std::endl;
    if (newScene != NULL)
    {
        newScene->createBVH();
        newScene->render();
        delete newScene;
    }
    else
        std::cout << "null file" << std::endl;
    
    return 0;
}
