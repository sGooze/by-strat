// sn_gui.cpp
//	gui for snek game

#include "sn_main.hpp"
#include "mine_gl.hpp"

static float s = 0.0075f;
float pix_verts[]{
	1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	0.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	0.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
};

bool exitAttempt = false;

bool fullscreen = false;

static camPos menu_def = camPos(9.24f, 7.35f, 8.04f, 0.0f, 2.6f, 0.0f);
static camPos ingame_def = camPos(0.0f, 5.62f, 4.81f, 0.0f, 2.6f, -4.35f);
static camPos curCam = menu_def;

static FramebufferGL *curBuf = nullptr;
//////////////////////////////////////////////////////////////////////

ProcessEventResult SnekGame::ProcessEvent(SDL_Event& event) {
	if ((instance != nullptr) && (event.type == SDL_KEYDOWN))
		instance->Move(event.key.keysym.scancode);

	if (event.type == SDL_KEYUP)
		switch (event.key.keysym.scancode) {
		case SDL_SCANCODE_ESCAPE:
			exitAttempt = !exitAttempt;
			break;
		case SDL_SCANCODE_F:
			fullscreen = !fullscreen;
		}
	return PE_NOP;
}

void SnekGame::SnekEvents() {
	SnekEvent* ev;
	while (EventQueue::PollEvent(EVENT_SENK, (Event**)&ev)) {
		switch (ev->purpose) {
		case SNEK_START:
			if (instance != nullptr) { delete instance; instance = nullptr; }
			instance = new SnGameInstance(((SnekEvent_StartGame*)ev)->x, ((SnekEvent_StartGame*)ev)->y, ((SnekEvent_StartGame*)ev)->speed, ((SnekEvent_StartGame*)ev)->live, ((SnekEvent_StartGame*)ev)->wall);
			curCam = ingame_def;
			instance->SpawnTreat();
			EventQueue::AddEvent(new AudioEvent_PlayMusic("nesong2.ogg", 100, 100));
			break;
		case SNEK_LOSE:
			if (instance != nullptr) { 
				snekSkor.Add(player_name, instance->score);
				delete instance; instance = nullptr; 
			}
			if (frand() >= 0.5f)
				EventQueue::AddEvent(new AudioEvent_PlayMusic("bckdrone2.ogg", 100, 100));
			else
				EventQueue::AddEvent(new AudioEvent_PlayMusic("snbckg1.flac", 100, 100));
			curCam = menu_def;
			break;
		case SNEK_SOUND:
			EventQueue::AddEvent(new AudioEvent_PlaySound(((SnekEvent_Sound*)ev)->name));
			break;
		case SNEK_EXIT:
			exitAttempt = true;
			break;
		}
		delete ev;
	}
}

void SnekNotes(bool *close) {
	ImGui::Begin("Notes: Snek", close, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text(u8"SNEK 2\n  The return of the son\n   of the snek's mother's husband, part 5:\n        The Revenge of Snek's Wife\n		\nУправление:\n Стрелки ··· Передвижение\n F ········· Полный экран\n\nЦель:\n Ж р и   и л и   у м р и");
	ImGui::End();
}

void SnekGame::Render() {
	static bool leaders = false, notes = false;
	snThinks.Think();
	SnekEvents();

	curBuf = (fullscreen) ? buf : &bufScreen;

	static int gx = 15, gy = 15, gl = 3, gs = 5;
	static bool gw = false;
	if (instance == nullptr) {
		RenderBackground();
		ImGui::SetNextWindowSize(ImVec2(400, 220));
		ImGui::Begin("snk_intro", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

		ImGui::DragInt("X", &gx, 0.1f, 10, 40);
		ImGui::DragInt("Y", &gy, 0.1f, 10, 40);
		ImGui::DragInt("Lives", &gl, 0.1f, 1, 10);
		ImGui::DragInt("Speed", &gs, 0.1f, 1, 100);
		ImGui::Checkbox("Border wall", &gw);
		if (ImGui::Button("Start"))
			EventQueue::AddEvent(new SnekEvent_StartGame(gx, gy, gs, gl, gw));
		if (ImGui::Button("Leaderboards"))
			leaders = !leaders;
		if (ImGui::Button("Notes"))
			notes = !notes;
		if (ImGui::Button("Back"))
			EventQueue::AddEvent(new GameEvent(GAME_STOP));
		ImGui::End();
		if (leaders) snekSkor.Window(&leaders);
		if (notes) SnekNotes(&notes);
		return;
	}
	RenderGame();
}

void SnekGame::RenderGame() {
	static ShaderGL shader = ShaderGL("pixel");
	static MeshGL& pix = snMesh.Get("pix");

	static int32_t up_prev = 0;
	if (!exitAttempt) {
		if (SDL_GetTicks() - up_prev >= (uint32_t)(1000 / instance->speed)) {
			instance->Update();
			up_prev = SDL_GetTicks();
		}
	}
	else {
		ImGui::OpenPopup("Exit Snek");
		exitAttempt = false;
	}
	if (ImGui::BeginPopupModal("Exit Snek", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Are you sure you want to exit?");
		ImGui::Separator();
		if (ImGui::Button("Yes", ImVec2(120, 0))) {
			//EventQueue::AddEvent(new SnekEvent(SNEK_LOSE));
			delete instance; instance = nullptr;
			ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
			return;
		}
		ImGui::SameLine();
		if (ImGui::Button("No", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	static glm::mat4 unit;
	float ratio, invr;
	// TODO: Replace invr with check against bigger side of the screen
	if (fullscreen) {
		ratio = (float)FIXED_W / (float)FIXED_H;
		invr = 1 / ratio;
	}
	else {
		ratio = (float)SCRW / (float)SCRH;
		invr = ratio;
	}
	s = invr * (1.0f / (float)std::max(instance->x, instance->y));
	glm::mat4 project = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f / ratio, 0.0f, 100.0f);
	glm::mat4 scalem = glm::scale(unit, glm::vec3(s, s, s));
	curBuf->Use();

	glClearColor(0x00, 0xAA, 0xAA, 0xff);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader.Use();
	glm::vec4 col = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glUniformMatrix4fv(shader.Uniform("scale"), 1, GL_FALSE, glm::value_ptr(scalem));
	glUniformMatrix4fv(shader.Uniform("project"), 1, GL_FALSE, glm::value_ptr(project));
	glUniform4fv(shader.Uniform("pixColor"), 1, glm::value_ptr(col));

	glDisable(GL_DEPTH_TEST);
	for (int f = 0; f < instance->x; f++) {
		for (int g = 0; g < instance->y; g++) {
			glUniform2f(shader.Uniform("pos"), (float)f, (float)g);
			pix.Render();
		}
	}
	col = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glUniform4fv(shader.Uniform("pixColor"), 1, glm::value_ptr(col));
	for (auto& x : instance->player.GetBody()) {
		glUniform2f(shader.Uniform("pos"), x.fGetX(), x.fGetY());
		pix.Render();
	}
	for (object* x : instance->GetObjectList()) {
		SDL_Rect box = x->GetBBox();
		if ((box.h > 1) || (box.w > 1)) {
			col = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
			glUniform4fv(shader.Uniform("pixColor"), 1, glm::value_ptr(col));
			for (int i = box.x; i < box.x + box.w; i++)
				for (int j = box.y; j < box.y + box.h; j++) {
					glUniform2f(shader.Uniform("pos"), (float)i, (float)j);
					pix.Render();
				}
			continue;
		}
		col = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);
		glUniform4fv(shader.Uniform("pixColor"), 1, glm::value_ptr(col));
		glUniform2f(shader.Uniform("pos"), x->fGetX(), x->fGetY());
		pix.Render();
	}
	curBuf->Default();
	glEnable(GL_DEPTH_TEST);

	if (!fullscreen)
		RenderBackground();
	ImGui::SetNextWindowSize(ImVec2(200, 100));
	ImGui::Begin("snk_gui", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
	//ImGui::DragFloat("Scale", &s, 0.001f, -1.0f, 1.0f);
	//ImGui::Separator();
	//ImGui::Text("Scale: %f", s);
	ImGui::Text("Score: %i", instance->score);
	ImGui::Text("Lives: %i", instance->lives);
	ImGui::Text("Speed: %i\n", instance->speed);
	ImGui::Text("F: Toggle fullscreen");

	ImGui::End();
}

void SnekGame::RenderBackground() {
	static ShaderGL bckSh = ShaderGL("default");
	static MeshGL& wal = snMesh.Get("snek_scene_wall.obj");
	static MeshGL& cab = snMesh.Get("snek_scene_cab.obj");
	static MeshGL& scr = snMesh.Get("snek_scene_screen.obj");
	static TextureGL& map_cab = snTex.Get("lightmap_cab.png");
	static TextureGL& map_wal = snTex.Get("lightmap_walls.png");
	buf->Use();

	glClearColor(0x00, 0x00, 0x00, 0xff);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	static glm::mat4 model;
	glm::mat4 view = curCam.lookAt();
	static glm::mat4 projection = glm::perspective(glm::radians(55.0f), (float)FIXED_W / (float)FIXED_H, 0.1f, 100.0f);

	bckSh.Use();
	// send matrices
	glUniformMatrix4fv(bckSh.Uniform("model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(bckSh.Uniform("view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(bckSh.Uniform("projection"), 1, GL_FALSE, glm::value_ptr(projection));

	map_wal.Use();
	glUniform1i(bckSh.Uniform("PlaneTex"), 0);
	wal.Render();
	map_cab.Use();
	cab.Render();
	bufScreen.GetTexture().Use();
	scr.Render();

	buf->Default();
}