// minefield.cpp

#include "minefield.hpp"

// Cell status: 0..8 - opened, has X neighbors with mines; 9 - opened, has bomb; 10 - closed; 11 - marked
/// when softStep is used, cell stays closed if its neighbor count is not zero
int8_t MineCell::Open(bool softStep) {
	if (open != 10)
		return open;
	if (haveBomb) {
		EventQueue::AddEvent(new GameEvent(GAME_LOST));
		open = 9;
		return open;
	}
	int neighs = 0;
	for (MineCell* cel : neighbors) {
		if (cel == nullptr) continue;
		if (cel->HasBomb()) neighs++;
	}
	if ((!softStep)) {
		open = neighs;
		for (MineCell* cel : neighbors)
		{
			if ((cel != nullptr) && (!cel->HasBomb())) cel->Open(true);
		}
	}
	else if ((neighs == 0)) { 
		open = neighs; 
		for (MineCell* cel : neighbors) 
		{ if ((cel != nullptr) && (!cel->HasBomb())) cel->Open(false); }
	}
	return open;
}

MineField::MineField(const uint16_t sizex, const uint16_t sizey, const uint16_t bombs_all, const bool wrap) 
	: size_x(sizex), size_y(sizey), bombs(bombs_all), bombs_left(bombs_all), field_wrap(wrap) {
	field = new MineCell*[sizex];
	for (int i = 0; i < sizex; i++) field[i] = new MineCell[sizey];
	for (int i = 0; i < sizex; i++) {
		for (int j = 0; j < sizey; j++) {
			MineCell& cel = field[i][j];
			cel.neighbors[0] = Cell(i - 1, j - 1, wrap);
			cel.neighbors[1] = Cell(i    , j - 1, wrap);
			cel.neighbors[2] = Cell(i + 1, j - 1, wrap);
			cel.neighbors[3] = Cell(i - 1, j    , wrap);
			cel.neighbors[4] = Cell(i + 1, j    , wrap);
			cel.neighbors[5] = Cell(i - 1, j + 1, wrap);
			cel.neighbors[6] = Cell(i    , j + 1, wrap);
			cel.neighbors[7] = Cell(i + 1, j + 1, wrap);
		}
	}
	std::random_device seed_provider;
	std::mt19937 rand(seed_provider());
	std::uniform_int_distribution<int> bomb_x(0, sizex - 1);
	std::uniform_int_distribution<int> bomb_y(0, sizey - 1);
	// Randomly select a cell and check it as having a bomb
	for (int i = 0; i < bombs; i++) {
		int bx = bomb_x(rand); int by = bomb_y(rand);
		if (field[bx][by].haveBomb == false)
			field[bx][by].haveBomb = true;
		else i--;
	}
	int xx = std::max(size_x, size_y);
	sz = (xx < 15) ? 30 : (xx < 30) ? 20 : 15;
	time_start = SDL_GetTicks();
}

MineCell* MineField::Cell(int16_t x, int16_t y, bool wrap) {
	if (wrap) {
		uint16_t nx = (x < 0) ? size_x + x : (x >= size_x) ? x - size_x : x;
		uint16_t ny = (y < 0) ? size_y + y : (y >= size_y) ? y - size_y : y;
		return &field[nx][ny];
	}
	return ((x >= 0) && (y >= 0) && (x < size_x) && (y < size_y)) ? &field[x][y] : nullptr;
}

void MineField::OpenAllMines() {
	for (int i = 0; i < size_y; i++) {
		for (int j = 0; j < size_x; j++)
			if ((field[j][i].haveBomb))
				field[j][i].open = 9;
	}
}

uint32_t MineField::GetScore() {
	return bombs * size_x * size_y * ((field_wrap) ? 1.5 : 1) - ((time_stop - time_start)/1000.0);
}

void MineField::CheckMines() {
	int fields_closed = size_x * size_y;
	int bombs_marked = bombs;
	for (int i = 0; i < size_y; i++) {
		for (int j = 0; j < size_x; j++) {
			// Win the game prematurely if all remaining closed fields are either marked or have bombs in them
			if ((field[j][i].open >= 10))
				fields_closed--;
			if ((field[j][i].open == 11) && (field[j][i].HasBomb()))
				bombs_marked--;
		}
	}
	if ((fields_closed == bombs)||(bombs_marked == 0)) {
		time_stop = SDL_GetTicks();
		uint32_t score = GetScore();
		OpenAllMines();
		EventQueue::AddEvent(new GameEvent(GAME_WON, (void*)score));
	}
}

MineField::~MineField() {
	for (int i = 0; i < size_x; i++)
		delete(field[i]);
	delete field;
}