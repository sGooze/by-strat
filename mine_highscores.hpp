// mine_highscores.hpp
//	PURPOSE: high-score table class, with import/export

#pragma once

#include "common.hpp"

struct HighScore {
	std::string player;
	uint32_t score;
	HighScore(const std::string& pl, uint32_t sc) : player(pl), score(sc) {}
};

class HighScoreTable {
private:
	std::string filePath;
	std::list<HighScore> allScores;
	uint8_t size_max = 10;
public:
	HighScoreTable(const std::string& path) : filePath(path) { Load(); };
	void Load();
	void Save();
	const std::list<HighScore> GetScores() { return allScores; }
	void Add(const std::string& player, const uint32_t score);
	~HighScoreTable() { Save(); };
	void Window(bool *flag);
	void Widget();
};