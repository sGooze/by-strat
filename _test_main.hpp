#pragma once

#include "common.hpp"
#include "mine_gl.hpp"
#include "c_glmanager.hpp"

class Model3D {
public:
	MeshGL *mesh;
	glm::vec3 color;
	glm::vec3 position;
	glm::vec3 angle;
//public:
	Model3D(std::string& model_path, glm::vec3 position, glm::vec3 angle);
	glm::mat4 GetMatrix() {
		glm::mat4 model;

		model = glm::rotate(model, angle.x, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, angle.y, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, angle.z, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, position);
		return model;
	}
};

class SumShiet {
private:
	bool keysPressed[SDL_NUM_SCANCODES] = {false};
	std::vector<Model3D*> mdlList;
	//CameraGL camera = CameraGL(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	void PollEvents();
	void SimulateFrame();
	void RenderFrame();
public:
	SumShiet();
	bool GameFrame();
};