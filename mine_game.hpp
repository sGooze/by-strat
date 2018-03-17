// mine_game.hpp
// PURPOSE: game class

#pragma once

#include "common.hpp"
#include "minefield.hpp"
#include "mine_gui.hpp"
#include "mine_thinker.hpp"

class MineGame {
private:
	MineField* mineField;
	GameField  gameField;
	MainMenu   mainMenu;
	SnekGame*  snekInstance;
	ShootGame* shut;
	CreditsScreen* crdsScr;
	GUI* current;
	// note: why tf we need current: because the engine was initially developed
	// with its own gui system in mind, where only one gui config is rendered
	// on the screen at once, and only one gui recives inputs.
	// since imgui is used as a gui system rn, and it uses its own input
	// processor, the second part is kind of irrelevant

	ThinkerList gThinks;
	SoundManager gSounds;
	MusicManager gMusic;

	bool quit = false;
	bool mouseCap = false;

	void PollAudio();
	void PollEvents();
	void RenderFrame();
	void RenderBackground();

	void StartGame(GameChangeEvent*);
	void EndGame();
public:
	MineGame();
	bool GameFrame();
	bool IsQuit() { return quit; }
};
