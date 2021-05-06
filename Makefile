all:
	g++ main.cpp src/Scene.cpp src/SceneParser/SceneParser.cpp src/Shape/BVH.cpp src/Image/Image.cpp src/SceneParser/tinyxml2.cpp -O3 -std=c++11 -pthread -o raytracer

clear:
	rm -rf raytracer