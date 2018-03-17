// mine_mainmenu.cpp

#include "mine_gui.hpp"
#include "mine_gl.hpp"

char player_name[128] = "BigBoy";

ProcessEventResult MainMenu::ProcessEvent(SDL_Event& event) {
	/*if (gameAboutToStart) {
		gameAboutToStart = false;
		return PE_NEWGAME;
	}*/
	return PE_NOP;
}

static uint8_t gmode = 0; // Game Mode: used for choosing the game setup menu

void MainNotes(bool*);

void MainMenu::Render() {
	static bool noquit = true, names = true, debg = false, metrics = false, notes = false;

	if (!noquit) {
		ImGui::SetNextWindowPosCenter();
		ImGui::Begin("Confirm", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
		ImGui::Text(u8"Спасибо за игру!\n\nЭто ДЕМО было продуктом дружного творческого союза\n\n==================================================\n B Y Z E R O   B U S I N E S S  S O L U T I O N S\n                     inc.\n==================================================\n\nНаш коллектив:\nМаксим \"Санбез\"\nСтёпа \"DJ Грув\" Грубзбзпс\n\nПодробности - в разделе \"Credits\".\nВы точно хотите выйти?");
		ImGui::Separator();
		if (ImGui::Button(u8"Да")) {
			EventQueue::AddEvent(new GameEvent(GAME_EXIT));
		}
		ImGui::SameLine();
		if (ImGui::Button(u8"Нет")) {
			noquit = true;
		}
		ImGui::End();
		return;
	}
	if (gmode != 0) {
		// Selecting a game menu
		MinesweepMenu();
		return;
	}
	static ImGuiWindowFlags menu_flag = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
	ImGui::SetNextWindowSize(ImVec2(500, 175));
	ImGui::SetNextWindowPosCenter();
	ImGui::Begin("Main menu", &noquit, menu_flag);
	ImGui::Text(u8"Ваше имя: "); ImGui::SameLine();
	if (names) {
		ImGui::Text(player_name); ImGui::SameLine();
		if (ImGui::Button(u8"Изменить")) names = false;
	}
	else {
		ImGui::PushItemWidth(95.0f);
		ImGui::InputText("", player_name, IM_ARRAYSIZE(player_name)); ImGui::SameLine();
		ImGui::PopItemWidth();
		if (ImGui::Button("OK"))
			names = true;
	}
	ImGui::Separator();
	// Select game
	static TextureGL tbtn = TextureGL("textures\\btns2.png", false);
	{
		ImVec2 sz = ImVec2(75.0f, 75.0f);
		//if (ImGui::Button("sweeps", sz))
		ImGui::PushID("g_sap");
		if (ImGui::ImageButton((ImTextureID)tbtn.ID(), sz, ImVec2(0, 0), ImVec2(0.29296875f, 0.29296875f)))
			gmode = 1;
		ImGui::PopID();
		ImGui::SameLine();

		//if (ImGui::Button("snakes", sz)) 
		ImGui::PushID("g_snek");
		if (ImGui::ImageButton((ImTextureID)tbtn.ID(), sz, ImVec2(0.29296875f, 0), ImVec2(0.29296875f * 2, 0.29296875f)))
		{
			//gmode = 2;
			EventQueue::AddEvent(new GameChangeEvent(G_SNEKE));
		}
		ImGui::PopID();
		ImGui::SameLine();

		ImGui::PushID("g_shut");
		if (ImGui::ImageButton((ImTextureID)tbtn.ID(), sz, ImVec2(0.29296875f, 0.29296875f), ImVec2(0.29296875f * 2, 0.29296875f * 2)))
			EventQueue::AddEvent(new GameChangeEvent(G_SHUT));
		ImGui::PopID();
		ImGui::SameLine();

		//if (ImGui::Button("credits", sz))
		ImGui::PushID("g_cred");
		if (ImGui::ImageButton((ImTextureID)tbtn.ID(), sz, ImVec2(0.29296875f * 2, 0), ImVec2(0.29296875f * 3, 0.29296875f)))
			EventQueue::AddEvent(new GameChangeEvent(G_CREDS));
		ImGui::PopID();
		ImGui::SameLine();

		ImGui::PushID("g_quit");
		if (ImGui::ImageButton((ImTextureID)tbtn.ID(), sz, ImVec2(0.0f, 0.29296875f), ImVec2(0.29296875f, 0.29296875f * 2)))
			noquit = false;
		ImGui::PopID();
	}
	ImGui::Separator();
	const char pop[4] = "POP";
	if (ImGui::Button(u8"Трофеи")) {
		//notes = true;
		ImGui::OpenPopup(pop);
	} ImGui::SameLine();
	if (ImGui::BeginPopup(pop)) {
		ImGui::Text(u8"Трофейная комната временно закрыта. Зайдите попозже!");
		if (ImGui::Button("OK", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
			notes = false;
		}
		ImGui::EndPopup();
	}
#ifdef _DEBUG
	ImGui::Checkbox("show debugs", &debg);
	ImGui::SameLine();
	//ImGui::Checkbox("show metrics", &metrics);
#endif // _DEBUG
	ImGui::Text("a byzero \"gaming collection\" (c) 2017-2018");
	ImGui::End();
	/*if (ImGui::BeginPopupModal(pop, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text(u8"Трофейная комната временно закрыта. Зайдите попозже!");
		if (ImGui::Button("OK", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
			notes = false;
		}
		ImGui::EndPopup();
	}*/
	if (debg) ImGui::ShowTestWindow(&debg);
	if (notes) MainNotes(&notes);
}

void Tooltip(const std::string& header, const std::string& desc) {
	ImGui::BeginTooltip();
	ImGui::PushTextWrapPos(450.0f);
	ImGui::Text(header.c_str());
	ImGui::TextDisabled(desc.c_str());
	//ImGui::TextUnformatted(text.c_str());
	ImGui::PopTextWrapPos();
	ImGui::EndTooltip();
}

void MainNotes(bool *close) {
	ImGui::Begin("Trophy room", close, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
	//ImGui::Text("a by zero gaming presentation");
	//Gui::Text("DEVELOPED BY:");
	//Gui::Text("          BYZERO");
	//Gui::Text("-------------");
	//Gui::Text("sgooze: \"programming\", gameplay, crying");
	//Gui::Text("sunbat: art-direction, testing, solid ideas which are too");
	//Gui::Text("        good to be implemented");
	//Gui::Text("\n\nWe are very sorry");
	//Gui::Text(u8"Проверка на юникод");
	static TextureGL tex = TextureGL("textures\\achives2.png", false);
	static TextureGL tex2 = TextureGL("textures\\achives.png", false);
	ImVec2 sz = ImVec2(128, 128);
	float offset_x = 64.0f / 256.0f, offset_y = 64.0f / 256.0f;
	TextureGL* ox;

	ImGui::Text("Minesweep");
	ImGui::PushID("mine_1");
	ox = (achives.mine_1) ? &tex : &tex2;
	if (ImGui::ImageButton((ImTextureID)ox->ID(), sz, ImVec2(0, 0), ImVec2(offset_x, offset_y)))
	{ }
	if (ImGui::IsItemHovered())
		Tooltip("MINE_1", "Achivement #1");
	ImGui::PopID(); ImGui::SameLine();
	ImGui::PushID("mine_2");
	ox = (achives.mine_2) ? &tex : &tex2;
	if (ImGui::ImageButton((ImTextureID)ox->ID(), sz, ImVec2(offset_x, 0), ImVec2(offset_x * 2, offset_y)))
	{
	}
	if (ImGui::IsItemHovered())
		Tooltip("MINE_2", "Achivement #2");
	ImGui::PopID(); ImGui::SameLine();
	ImGui::PushID("mine_3");
	ox = (achives.mine_3) ? &tex : &tex2;
	if (ImGui::ImageButton((ImTextureID)ox->ID(), sz, ImVec2(offset_x * 2, 0), ImVec2(offset_x * 3, offset_y)))
	{
	}
	if (ImGui::IsItemHovered())
		Tooltip("MINE_3", "Achivement #3");
	ImGui::PopID();
	/////////////////////////////////////////////////////////
	ImGui::Text("Snek");
	ImGui::PushID("snek_1");
	ox = (achives.snek_1) ? &tex : &tex2;
	if (ImGui::ImageButton((ImTextureID)ox->ID(), sz, ImVec2(0, offset_y), ImVec2(offset_x, offset_y * 2)))
	{
	}
	if (ImGui::IsItemHovered())
		Tooltip("SNEK_1", "Achivement #1");
	ImGui::PopID(); ImGui::SameLine();
	ImGui::PushID("snek_2");
	ox = (achives.snek_2) ? &tex : &tex2;
	if (ImGui::ImageButton((ImTextureID)ox->ID(), sz, ImVec2(offset_x, offset_y), ImVec2(offset_x * 2, offset_y * 2)))
	{
	}
	if (ImGui::IsItemHovered())
		Tooltip("SNEK_2", "Achivement #2");
	ImGui::PopID(); ImGui::SameLine();
	ImGui::PushID("snek_3");
	ox = (achives.snek_3) ? &tex : &tex2;
	if (ImGui::ImageButton((ImTextureID)ox->ID(), sz, ImVec2(offset_x * 2, offset_y), ImVec2(offset_x * 3, offset_y * 2)))
	{
	}
	if (ImGui::IsItemHovered())
		Tooltip("SNEK_3", "Achivement #3");
	ImGui::PopID(); 
	/////////////////////////////////////////////////////////
	ImGui::Text("Minesweep");
	ImGui::PushID("run_1");
	ox = (achives.run_1) ? &tex : &tex2;
	if (ImGui::ImageButton((ImTextureID)ox->ID(), sz, ImVec2(0, offset_y * 2), ImVec2(offset_x, offset_y * 3)))
	{
	}
	if (ImGui::IsItemHovered())
		Tooltip("RUN_1", "Achivement #1");
	ImGui::PopID(); ImGui::SameLine();
	ImGui::PushID("run_2");
	ox = (achives.run_2) ? &tex : &tex2;
	if (ImGui::ImageButton((ImTextureID)ox->ID(), sz, ImVec2(offset_x, offset_y * 2), ImVec2(offset_x * 2, offset_y * 3)))
	{
	}
	if (ImGui::IsItemHovered())
		Tooltip("RUN_2", "Achivement #2");
	ImGui::PopID(); ImGui::SameLine();
	ImGui::PushID("run_3");
	ox = (achives.run_3) ? &tex : &tex2;
	if (ImGui::ImageButton((ImTextureID)ox->ID(), sz, ImVec2(offset_x * 2, offset_y * 2), ImVec2(offset_x * 3, offset_y * 3)))
	{
	}
	if (ImGui::IsItemHovered())
		Tooltip("RUN_3", "Achivement #3");
	ImGui::PopID(); ImGui::SameLine();

	ImGui::End();
}

void MinesweepNotes(bool *close);

void MainMenu::MinesweepMenu() {
	static int game_x = 9, game_y = 8, game_m = 9;
	static bool wrap = false, ngame = false, lbrds = false, notes = false;
	static char* aaa = "a"; static int aaa_s = 32;
	ImGui::Begin("Minesweep", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	if (ngame) {
		ImGui::Text(u8"Параметры игры:");
		ImGui::Text(u8"Размер поля: ");
		ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth() * 0.5f);
		ImGui::DragInt("##a", &game_x, 0.1f, 5, 50); ImGui::SameLine();
		ImGui::DragInt("##b", &game_y, 0.1f, 5, 50);
		ImGui::PopItemWidth();
		ImGui::Text(u8"Число мин: ");
		ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth());
		ImGui::DragInt("##c", &game_m, 0.1f, 5, (game_x * game_y) - 1);
		ImGui::Checkbox(u8"Замкнутое поле", &wrap);
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text(u8"Режим замкнутого поля:\nклетки на границе считают клетки на противоположном конце своими соседями");
			ImGui::EndTooltip();
		}

		if (game_m >= (game_x * game_y) - 1) {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), u8"Слишком много мин!");
		}
		else if ((game_x < 5) || (game_y < 5)) {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), u8"Слишком маленькое поле!");
		}
		else {
			if (ImGui::Button(u8"Начать")) {
				auto v = new std::vector<int>();
				v->push_back(game_x);
				v->push_back(game_y);
				v->push_back(game_m);
				v->push_back((bool)wrap);
				//EventQueue::AddEvent(new GameEvent(GAME_START, (void*)v));
				EventQueue::AddEvent(new GameChangeEvent(G_MINESWEEP, (void*)v));
				EventQueue::AddEvent(new AudioEvent_PlaySound("gamestart.wav"));
				ngame = false;
			}
		}
		if (ImGui::Button(u8"Назад")) ngame = false;
	}
	else {
		// all of this will go into a nice floaty at the edge of the screen alongside version name
		ImGui::Text("Welcome to...\n");
		static TextureAGL tex = TextureAGL("textures\\logo_mines.png", false);
		ImGui::Image((ImTextureID)tex.ID(), ImVec2(256, 64));
		ImGui::Separator();
		if (ImGui::Button(u8"Начать игру")) {
			ngame = true;
		}
		if (ImGui::Button(u8"Таблица лидеров"))
			lbrds = true;
		if (ImGui::Button(u8"Помощь"))
			notes = true;
		if (ImGui::Button(u8"Назад в меню")) {
			gmode = 0;
		}
	}
	ImGui::End();
	if (lbrds) scores.Window(&lbrds);
	if (notes) MinesweepNotes(&notes);
}

void MinesweepNotes(bool *close) {
	ImGui::Begin("Notes: Minesweeper", close, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("MINESWEEPER 2\n       Eternal minesweeping");
	ImGui::Text(u8"Управление:\n ЛКМ: открыть поле\n ПКМ: Пометить поле\n\nЦель:\nНайти все мины на игровом поле.\nПо щелчку по полю оно будет открыто. Если на поле находилась мина, то игра закончится;\nесли же мины там нет, то на месте поля будет показано число, означающее количество мин\nв соседних полях.\nДля того, чтобы победить, нужно либо открыть все поля, не содержащие мин, либо пометить\nфлажками (щелчок ПКМ) все поля, в которых мины есть. \nСчетчик сверху окна показывает число оставшихся мин." );
	ImGui::End();
}

void HighScoreTable::Window(bool *flag) {
	ImGui::SetNextWindowSize(ImVec2(200, 230), ImGuiCond_Once);
	ImGui::Begin("Leadership", flag, ImGuiWindowFlags_NoSavedSettings);
	Widget();
	
	ImGui::End();
}

void HighScoreTable::Widget() {
	ImGui::Columns(3, "leaders", true);
	ImGui::NextColumn();
	ImGui::Text("Player"); ImGui::NextColumn();
	ImGui::Text("Score"); ImGui::NextColumn();
	ImGui::Separator();
	int i = 0;
	for (auto z : GetScores()) {
		ImGui::Text("%i", ++i);
		ImGui::NextColumn();
		ImGui::Text(z.player.c_str());
		ImGui::NextColumn();
		ImGui::Text("%i", z.score);
		ImGui::NextColumn();
	}
	ImGui::Separator();
	ImGui::Columns(1);
}

void MainMenu::LeaderBoards(bool* lbrds) {
	
}