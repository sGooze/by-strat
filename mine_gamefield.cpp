// mine_gamefield.cpp

#include "mine_gui.hpp"
#include "mine_gl.hpp"
// temp?
#include "minefield.hpp"

static ShaderGL mineShader;

GameField::GameField(){
	//mineSky = new SkyboxGL("tdawn", ".png");
	mineShader.SetNewPaths("shaders\\default.vert", "shaders\\default.frag");
}

void MineField::GUIWindow() {
	ImVec2 szv(sz, sz);
	static int winX = size_x * szv.x;
	static int winY = size_y * szv.y;
	static ImVec2 winSize = ImVec2( (winX > FIXED_W - 10) ? FIXED_W - 10 : winX , (winY > FIXED_H - 30) ? FIXED_H - 30 : winY) ;
	ImGui::SetNextWindowSize(ImVec2(), ImGuiCond_Once);
	ImGui::SetNextWindowPosCenter();
	ImGui::Begin("##board", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar );
	ImGui::Text("Field size: %ix%i", size_x, size_y);
	ImGui::Text("Bombs left: "); ImGui::SameLine();
	ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), "%i", bombs_left);
	for (int i = 0; i < size_y; i++) {
		ImGui::BeginGroup();
		for (int j = 0; j < size_x; j++) {
			int id = j * size_x + i, open;
			bool failsafe = false;
			ImGui::PushID(id);
			if (field[j][i].open < 9) {
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.4f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.6f));
				failsafe = true;
			}
			else if (field[j][i].open == 11) {
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.4f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 0.3f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 0.2f));
				failsafe = true;
			}
			ImGui::Button(field[j][i].ToString().c_str(), szv);
			if (ImGui::IsItemHovered()) {
				if (ImGui::IsItemClicked()) {
					open = field[j][i].Open();
					if (open != 9) {
						EventQueue::AddEvent(new AudioEvent_PlaySound("bomb_succ.wav"));
						CheckMines();
					}
					else if (open == 9)
						EventQueue::AddEvent(new AudioEvent_PlaySound("bomb_fail.wav"));
				}
				else if ((ImGui::IsItemClicked(1))&&(field[j][i].IsOpen() > 9)) {
					open = field[j][i].Mark();
					bombs_left += (open == 10) ? 1 : -1;
					CheckMines();
					if (bombs_left == 65535) {
						field[j][i].Mark(); bombs_left++;
					}
					else
						EventQueue::AddEvent(new AudioEvent_PlaySound((open == 10) ? "bomb_unmark.wav" : "bomb_mark.wav"));
				}
			}
			if (failsafe) {
				ImGui::PopStyleColor(3);
			}
			ImGui::PopID();
			ImGui::SameLine();
		}
		ImGui::EndGroup();
	}
	ImGui::Separator();
	if (ImGui::Button("Stop game")) EventQueue::AddEvent(new GameEvent(GAME_STOP));
	ImGui::End();
}

void GameField::RenderBackground() {
	buf->Use();
	glClearColor(0xFF, 0xAA, 0xAA, 0xff);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// setup matrices
	// Vclip=Mprojection*Mview*Mmodel*Vlocal
	//glm::mat4 model;
	//model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	float radius = 10.0f;
	float camX = sin(SDL_GetTicks() / 2000.0) * radius;
	float camY = sin(SDL_GetTicks() / 2000.0) * 2.0f;
	float camZ = cos(SDL_GetTicks() / 2000.0) * radius;
	glm::mat4 view;
	view = glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(camX, camY, camZ), glm::vec3(0.0, 1.0, 0.0));
	//view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
	glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)FIXED_W / (float)FIXED_H, 0.1f, 100.0f);

	// TODO: Background with a field; mouse controls camera tilt; advanced color grading
	/*mineShader.Use();
	// send matrices
	glUniformMatrix4fv(mineShader.Uniform("model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(mineShader.Uniform("view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(mineShader.Uniform("projection"), 1, GL_FALSE, glm::value_ptr(projection));

	//plTexture->Use();
	//glUniform1i(mineShader.Uniform("PlaneTex"), 0);
	// render something??
	*/
	mineSky->Render(glm::mat4(glm::mat3(view)), projection);

	buf->Default();
}

void GameField::Render() {
	RenderBackground();
	field->GUIWindow();
	if (game_was_over) {
			ImGui::OpenPopup("Game over"); game_was_over = false;
	}
	if (ImGui::BeginPopupModal("Game over", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (game_was_won) {
			ImGui::Text("You won!");
			// TODO: GetScore
			ImGui::Text("Final score: %i (%i mines in %.1f seconds, field %ix%i, wrap %s)",
				field->GetScore(), field->GetBombs(), field->GetTime(), 
				field->GetSizeX(), field->GetSizeY(), 
				(field->GetFieldWrap()) ? "enabled" : "disabled");
		}
		else {
			ImGui::Text("You lost...");
		}
		ImGui::Separator();
		ImGui::Text("Do you want to replay the game with the same parameters?\n");

		if (ImGui::Button("Replay", ImVec2(120, 0))) {
			EventQueue::AddEvent(new GameEvent(GAME_STOP));
			EventQueue::AddEvent(new GameChangeEvent(G_MINESWEEP, field->GetParametersArgs()));
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Back to menu", ImVec2(120, 0))) {
			EventQueue::AddEvent(new GameEvent(GAME_STOP));
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

ProcessEventResult GameField::ProcessEvent(SDL_Event& event) {
	if (event.type != SDL_KEYDOWN) return PE_NOP;
	if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) return PE_SWITCH;
	return PE_NOP;
}