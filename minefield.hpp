// minefield.hpp - minefield class

#pragma once
#include "common.hpp"

class MineField;

class MineCell
{
private:
	bool haveBomb = false;
	int8_t open = 10;
	MineCell* neighbors[8] = {nullptr};
public:
	friend class MineField;
	MineCell() {};
	MineCell(bool bomb) : haveBomb(bomb) {};
	bool HasBomb() { return haveBomb; }
	int8_t Mark() { if (open == 10) open = 11; else if (open == 11) open = 10; return open; }
	int8_t Open(bool softStep = false);
	int8_t IsOpen() { return open; }
	std::string ToString() {
		switch (open) {
		case 0: return ""; break;	// Empty
		case 9: return "B"; break;	// Has a bomb
		case 10: return "?"; break;	// Unopened
		case 11: return "!"; break;	// Marked
		default: return std::to_string(open);
		}
	}
};

class MineField {
private:
	MineCell** field;			// initialized as a two-dimensional array
	uint16_t size_x, size_y;
	uint16_t bombs, bombs_left;
	bool field_wrap;
	uint32_t time_start;
	uint32_t time_stop = 0;
	int sz;				// button size for GUI
public:
	MineField(const uint16_t sizex, const uint16_t sizey, const uint16_t bombs_all = 10, const bool wrap = false);
	~MineField();
	MineCell* Cell(int16_t x, int16_t y, bool wrap = false);
	void OpenCell(int16_t coord_x, int16_t coord_y) { field[coord_x][coord_y].Open(); }
	void GUIWindow();
	// parameters
	uint16_t GetSizeX() { return size_x; }
	uint16_t GetSizeY() { return size_y; }
	uint16_t GetBombs() { return bombs; }
	float GetTime() { return (time_stop - time_start) / 1000.0; }
	uint32_t GetScore();
	bool GetFieldWrap() { return field_wrap; }
	void OpenAllMines();
	void CheckMines();
	std::vector<int>* GetParametersArgs() {
		auto v = new std::vector<int>();
		v->push_back(size_x);
		v->push_back(size_y);
		v->push_back(bombs);
		v->push_back((int)field_wrap);
		return v;
	}
};
