// cr_main.cpp

#include "mine_gui.hpp"
#include "mine_gl.hpp"

CreditsScreen::CreditsScreen(ShaderGL* shad_bckg) : shader(shad_bckg) {
	pages.push_back(u8"byZero's Family Game Collection Sampler\n        ���������� ���������� \n                     byZero:\n				  \n  ����� ����   - ������������ ����\n                  ����������������\n                  �������\n				  ����\n				\n  ������ ������ - �������\n                  ������������\n				  ����������� �������\n\n ������ � ����� - sGuzi ��� ������� � ���������\n                  ������ \"������� �����\"\n				  \n                                      ����� -->");
	pages.push_back(u8" ���. ��������� ����� ��� ���������� �� ���\n        \"Half-Life 2\" � \"Quake 2\"\n ������������ ����������:\n  - SDL2\n  - dear imgui,\n  - stb_image\n  - SDL2_mixer\n  - libFLAC\n  - libOgg\n  - ����� �����\n �������� ��������� � ����� license\\n \n ������������ ������ ProggyClear � GNU Unifont\n 				  \n                                      ����� -->");
	pages.push_back(u8"���� �������:\n � ���� ������� (������� ����)\n � ���� ������� (2)\n � 0��0 �0������0 (����� ������� ����)\n � ������ � �����\n � ����� \n				  \n                                      ����� -->");
	pages.push_back(u8"��� ������������ �����������\n � ������� ����������\n � ����� �������\n � ���� ���������� ��� \"�������� ������� �����\"\n � �����\n � ����� ��������� � ���� ������ ��� �����\n \n � ���� ��������� ����� ������� � ��������,  \n ��� ������� �� �� �������� ������� �������! <3\n                                      ����� -->");
	pages.push_back(u8"������� ���� �� �������� � ��������! �� �����\n��������� � ������� ���� ���� � ���� ������,\n� ��� ���� ����� ����, ��� � ����� ����������\n��, ��� ����������.\n\n��� ������ ����� �� ����� �������� � ����, �\n    �� �����������, ����� ���� �������\n  ��������� ����� � ����� ��������� � ����\n      ����� ���� ������������ � �������, \n        �� ��������� �� � ��� ����!\n\n              �� ����� ������!\n\n       (l) byZero games, 2017-2018");
}

static float x = 0.56f, y = 7.22f, z = 3.19f, a = -2.53f, b = 8.01f, c = 1.04f;


void CreditsScreen::Menu() {
	static int page = 0;
	ImGui::Begin("CREDITS", nullptr, ImGuiWindowFlags_NoSavedSettings);
	if (ImGui::Button(u8"< �����")) page = (page > 0) ? page - 1 : page;
	ImGui::SameLine();
	if (ImGui::Button(u8"������ >")) page = (page >= pages.size() - 1) ? page : page + 1;
	ImGui::Separator();
	ImGui::Text(pages[page].c_str());
	//if (page == 4) ImGui::Button("BANNER P L A C E H O L D E R");
	if (ImGui::Button(u8"� ����"))
		EventQueue::AddEvent(new GameEvent(GAME_STOP));
	ImGui::End();
}

void CreditsScreen::Render() {
	static MeshGL cav = MeshGL("models\\cave.obj");
	static TextureGL tex = TextureGL("textures\\cave_light.png");

	shader->Use();
	buf->Use();

	glClearColor(0xFF, 0xFF, 0xFF, 0xff);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	static glm::mat4 model;
	glm::mat4 view = glm::lookAt(glm::vec3(x, y, z), glm::vec3(a, b, c), glm::vec3(0.0, 1.0, 0.0));
	static glm::mat4 projection = glm::perspective(glm::radians(50.0f), (float)FIXED_W / (float)FIXED_H, 0.1f, 100.0f);

	glUniformMatrix4fv(shader->Uniform("model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(shader->Uniform("view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(shader->Uniform("projection"), 1, GL_FALSE, glm::value_ptr(projection));

	tex.Use();
	glUniform1i(shader->Uniform("PlaneTex"), 0);
	cav.Render();

	buf->Default();

	Menu();
}