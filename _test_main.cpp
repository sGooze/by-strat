// test_main.cpp

#include "_test_main.hpp"
#include "mine_gl.hpp"

static bool gameIsRunning = true;
static ShaderGL shader;

static MeshManager allMeshes;
static TextureManager shTex;

Model3D::Model3D(std::string& model_path, glm::vec3 position, glm::vec3 angle) : position(position) {
	this->angle = glm::vec3(glm::radians(angle.x), glm::radians(angle.y), glm::radians(angle.z));
	color = glm::vec3(frand(), frand(), frand());
	allMeshes.Load(model_path);
	mesh = &(allMeshes.Get(model_path));
}

SumShiet::SumShiet() {
	//projection = glm::ortho(0.0f, (float)FIXED_W, (float)FIXED_H, 0.0f, -1.0f, 1.0f);
	//projection = glm::perspective(glm::radians(90.0f), (float)FIXED_W / (float)FIXED_H, 3.1f, 5000.0f);
	//shader = ShaderGL("default");
	shader.SetNewPaths("shaders\\test.vert", "shaders\\test.frag");
	shTex.Load("pickup.png");
}

void SumShiet::PollEvents() {
	static SDL_Event event;
	while (SDL_PollEvent(&event)) {
		ImGui_ImplSdlGL3_ProcessEvent(&event);
		switch (event.type) {
		case SDL_KEYDOWN:
			keysPressed[event.key.keysym.scancode] = true;
			break;
		case SDL_KEYUP:
			if (event.key.keysym.scancode == SDL_SCANCODE_F10)
				gameIsRunning = false;
			keysPressed[event.key.keysym.scancode] = false;
			break;
		case SDL_WINDOWEVENT:
			if (event.window.type != SDL_WINDOWEVENT_SIZE_CHANGED) break;
			glViewport(0, 0, event.window.data1, event.window.data2);
			//projection = glm::ortho(0.0f, (float)event.window.data1, (float)event.window.data2, 0.0f, -1.0f, 1.0f);
			break;
		case SDL_QUIT:
			gameIsRunning = false; break;
		}
	}
}


// Projection shit
static glm::mat4 projection, view;

void SumShiet::SimulateFrame() {
	ImGui::Begin("meme", nullptr, ImGuiWindowFlags_NoSavedSettings);
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::Separator();
	static char model_name[256] = {'\0'};
	static float model_pos[3] = { 0 };
	static float model_angle[3] = {0};
	ImGui::Text("Load new model");
	ImGui::InputText("Filename", model_name, 256);
	ImGui::InputFloat3("Position", model_pos);
	ImGui::InputFloat3("Angle", model_angle);
	if (ImGui::Button("Load")) {
		mdlList.push_back(new Model3D(std::string(model_name),
			glm::vec3(model_pos[0], model_pos[1], model_pos[2]),
			glm::vec3(model_angle[0], model_angle[1], model_angle[2])));
		model_name[0] = '\0';
		model_pos[0] = model_pos[1] = model_pos[2] = 0.0f;
		model_angle[0] = model_angle[1] = model_angle[2] = 0.0f;
	}

	if (ImGui::Button("End me")) gameIsRunning = false;
	ImGui::End();
}

void SumShiet::RenderFrame() {
	static TextureGL& picktex = shTex.Get("pickup.png");

	ImGui::Begin("Setup", nullptr, ImGuiWindowFlags_NoSavedSettings);
	{
		ImGui::Text("Camera setup");
		static glm::vec3 camera, lookat, up = glm::vec3(0.0f, 1.0f, 0.0f);
		ImGui::DragFloat3("Camera", glm::value_ptr(camera));
		ImGui::DragFloat3("Lookat", glm::value_ptr(lookat));
		view = glm::lookAt(camera, lookat, up);
		ImGui::Separator();

		ImGui::Text("Projection setup");
		static bool isPersp = true;
		static float fov = 90.0f;
		static glm::vec4 rectangle = glm::vec4(0, 800.0f, 600.0f, 0);
		static glm::vec2 zzone = glm::vec2(-10.0f, 10.0f);
		ImGui::Checkbox("Perspective?", &isPersp);
		if (isPersp) ImGui::DragFloat("FOV", &fov);
		ImGui::DragFloat4("Properties", glm::value_ptr(rectangle));
		ImGui::DragFloat2("Z cutoff", glm::value_ptr(zzone));
		projection = (isPersp)
			? glm::perspective(glm::radians(fov), rectangle.y / rectangle.z, zzone.x, zzone.y)
			: glm::ortho(rectangle.x, rectangle.y, rectangle.z, rectangle.w, zzone.x, zzone.y);
	}
	ImGui::End();

	glClearColor(0x00, 0x00, 0x00, 0xff);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader.Use();
	glm::mat4 model;
	glUniformMatrix4fv(shader.Uniform("model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(shader.Uniform("view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(shader.Uniform("projection"), 1, GL_FALSE, glm::value_ptr(projection));
	picktex.Use(0);
	for (auto& mdl : mdlList) {
		model = mdl->GetMatrix();
		glUniformMatrix4fv(shader.Uniform("model"), 1, GL_FALSE, glm::value_ptr(model));
		mdl->mesh->Render(GL_LINE_LOOP);
	}

	ImGui::Render();
	SDL_GL_SwapWindow(glWindow);
}

bool SumShiet::GameFrame() {
	/* TODO:
	/ 1. JSON - тест с загрузкой, обработкой и выгрузкой
	/ 2. Тестовая штука с моделями и разными режимами проекций
	/ 3. Отрисовка спрайтов
	*/
	ImGui_ImplSdlGL3_NewFrame(glWindow);
	PollEvents();
	SimulateFrame();
	RenderFrame();
	
	return gameIsRunning;
}