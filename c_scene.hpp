// c_scene.hpp
//	PURPOSE: Animated background class
#pragma once

#include "common.hpp"
#include "mine_gl.hpp"

class Scene {
private:
	SkyboxGL skybox;
	CameraGL camera;
	//ShaderGL shd;
public:
	//Scene(SkyboxGL sky, CameraGL cam) : skybox(sky), camera(cam) {};
	Scene(const std::string& def);
	//~Scene();
	void Render();
};