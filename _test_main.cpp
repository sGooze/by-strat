// test_main.cpp

#include "_test_main.hpp"
#include "mine_gl.hpp"

static glm::mat4 projection;

SumShiet::SumShiet() {
	projection = glm::ortho(0.0f, (float)FIXED_W, (float)FIXED_H, 0.0f, -1.0f, 1.0f);
}

bool SumShiet::GameFrame() {
	static bool helpme = true; static SDL_Event event;
	ImGui_ImplSdlGL3_NewFrame(glWindow);
	while (SDL_PollEvent(&event)) {
		ImGui_ImplSdlGL3_ProcessEvent(&event);
		switch (event.type) {
		case SDL_KEYUP:
			if (event.key.keysym.scancode == SDL_SCANCODE_F10)
				helpme = false;
			break;
		case SDL_WINDOWEVENT:
			if (event.window.type != SDL_WINDOWEVENT_SIZE_CHANGED) break;
			glViewport(0, 0, event.window.data1, event.window.data2);
			projection = glm::ortho(0.0f, (float)event.window.data1, (float)event.window.data2, 0.0f, -1.0f, 1.0f);
			break;
		case SDL_QUIT:
			helpme = false; break;
		}
	}
	ImGui::Begin("meme", nullptr, ImGuiWindowFlags_NoSavedSettings);
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	if (ImGui::Button("End me")) helpme = false;
	ImGui::End();

	// Render
	glClearColor(0x00, 0x00, 0x00, 0xff);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ImGui::Render();
	SDL_GL_SwapWindow(glWindow);
	return helpme;
}