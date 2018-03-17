// mine_highscores.cpp

#include "mine_highscores.hpp"

void HighScoreTable::Load() {
	std::ifstream filestr(filePath);
	std::string buf;
	while (filestr.good()) {
		std::getline(filestr, buf);
		auto fst = buf.find_first_of('|');
		if ((fst == buf.npos)) {
			continue;
		}
		std::string name = buf.substr(0, fst);
		std::string score = buf.substr(fst + 1);
		Add(name, std::stoi(score));
	}
}

void HighScoreTable::Save() {
	std::ofstream filestr(filePath, std::ofstream::out);
	for (auto xxx : allScores)
		filestr << xxx.player << "|" << xxx.score << std::endl;
	filestr.close();
}

void HighScoreTable::Add(const std::string& player, const uint32_t score) {
	auto it = allScores.begin();
	for (it; it != allScores.end(); it++) {
		if ((*it).score <= score) break;
	}
	allScores.insert(it, HighScore(player, score));
	if (allScores.size() > size_max) allScores.pop_back();
}

