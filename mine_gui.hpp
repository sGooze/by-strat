// main_gui.hpp
// PURPOSE: gui classes

#pragma once

#include "common.hpp"
#include "mine_highscores.hpp"

// to be used to switch GUIs with a button press
typedef enum{PE_NOP, PE_SWITCH, PE_EXIT, PE_NEWGAME, PE_ENDGAME} ProcessEventResult;

class TrophyController {
public:
	bool mine_1, mine_2, mine_3;
	bool snek_1, snek_2, snek_3;
	bool run_1, run_2, run_3;
	void LoadData() {
		std::ifstream filestr("players\\test.lvl");
		std::string buf;
		std::getline(filestr, buf);
		if (!filestr.good()) {
			return;
		}
		filestr.close();
		mine_1 = (buf[0] == 'a');
		mine_2 = (buf[1] == 'a');
		mine_3 = (buf[2] == 'a');
		snek_1 = (buf[3] == 'a');
		snek_2 = (buf[4] == 'a');
		snek_3 = (buf[5] == 'a');
		run_1 = (buf[6] == 'a');
		run_2 = (buf[7] == 'a');
		run_3 = (buf[8] == 'a');
	}
	void SaveData() {
		std::ofstream filestr("players\\test.lvl", std::ofstream::out);
		filestr << ((mine_1) ? 'a' : 'b') << ((mine_2) ? 'a' : 'b') << ((mine_3) ? 'a' : 'b') << ((snek_1) ? 'a' : 'b') << ((snek_2) ? 'a' : 'b') << ((snek_3) ? 'a' : 'b') << ((run_1) ? 'a' : 'b') << ((run_2) ? 'a' : 'b') << ((run_3) ? 'a' : 'b') << std::endl;
		filestr.close();
	}
	~TrophyController() {
		SaveData();
	}
};

extern TrophyController achives;

class MineGame;

class GUI {
protected:
	friend class MineGame;
	bool acceptInput = false;
public:
	GUI() {};
	virtual bool IsVisible() = 0;
	virtual ProcessEventResult ProcessEvent(SDL_Event& event) = 0;
	virtual void Render() = 0;
	virtual ~GUI() {};
};

class MineField;
class SkyboxGL;

class GameField : public GUI {
private:
	//MineField* field;	// should be private
	void RenderBackground();
public:
	GameField();

	SkyboxGL* mineSky = nullptr;
	MineField* field = nullptr;	// field controlled by the gui
	//float fadeIn = 0;
	bool game_was_over = false; 
	bool game_was_won = false;
	bool IsVisible() { return !(field == nullptr); }
	ProcessEventResult ProcessEvent(SDL_Event& event);
	void Render();
};

class MainMenu : public GUI {
private:
	bool visible = true;
	bool gameAboutToStart = false;
	void LeaderBoards(bool*);
	void MinesweepMenu();
public:
	MainMenu() { acceptInput = false; }
	HighScoreTable scores = HighScoreTable("players\\scores.txt");
	//char player_name[64] = "BigBoy";
	bool IsVisible() { return visible; }
	bool SetVisible(bool new_vis) { visible = new_vis; }
	ProcessEventResult ProcessEvent(SDL_Event& event);
	void Render();
};

class SnGameInstance;

#include "mine_thinker.hpp"
#include "c_glmanager.hpp"
#define SCRW 512
#define SCRH 618
class SnekGame : public GUI {
private:
	SnGameInstance *instance = nullptr;
	FramebufferGL bufScreen = FramebufferGL(SCRW, SCRH);
	ThinkerList snThinks;
	MeshManager snMesh;
	TextureManager snTex;

	HighScoreTable snekSkor = HighScoreTable("players\\neks.txt");

	void RenderBackground();
	void RenderGame();
	void PlaySound(const std::string&);
public:
	SnekGame();

	char *pname;
	bool IsVisible() { return true; }
	ProcessEventResult ProcessEvent(SDL_Event& event);
	void SnekEvents();
	void Render();
	~SnekGame();
};

class ShootGame : public GUI {
private:
	MeshManager shMesh;
	TextureManager shTex;
	SkyboxGL* sky = nullptr;
	ThinkerList shThink;
	HighScoreTable shScores = HighScoreTable("players\\runnin.txt");

	bool paused = false;
	bool inMainMenu = true;
	void DrawMenu();
	void EditorMenu();
	void Editor();
	void Reset();
	void InternalEvents();
	bool ExportLevel(const char* path, float time);
	void ImportObject(const std::string& str);
public:
	bool mouseCrutch = false;
	ShootGame();
	bool LoadLevel(const std::string& path);
	bool IsVisible() { return true; }
	ProcessEventResult ProcessEvent(SDL_Event& event);
	void Render();
	~ShootGame();
};

class CreditsScreen : public GUI {
private:
	MeshManager shMesh;
	TextureManager shTex;
	ShaderGL* shader = nullptr;
	std::vector<std::string> pages;
	void Menu();
public:
	CreditsScreen(ShaderGL* shad_bckg);
	bool IsVisible() { return true; }
	ProcessEventResult ProcessEvent(SDL_Event& event) { return PE_NOP; };
	void Render();
};