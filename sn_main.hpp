// sn_main.hpp
//	main header for snke game

#pragma once

#include "common.hpp"
#include "mine_gui.hpp"

enum COLLISION { COLL_NONE, COLL_KILL, COLL_EAT };
enum DIRECTION { DIR_UP = 0, DIR_RIGHT = 1, DIR_DOWN = 2, DIR_LEFT = 3, DIR_NONE = -1 };

// TODO: VIRTUAL DESTRUCTORS FOR E V E R Y TH IN G (events and thinkers); 
// needed to properly destroy objects through base pointers

class object {
protected:
	//uint16_t x, y;
	SDL_Rect bbox;
	SDL_Color color = { 0xFF, 0xFF, 0xFF, 0xFF };
	SDL_Color color2 = { 0xFF, 0xFF, 0xFF, 0xFF };
	//ANIMATION_STYLE anim_style = ANIM_NONE;
	uint8_t anim_time = 1;
public:
	virtual COLLISION onCollide() = 0;
	virtual uint16_t GetValue() { return 0; }
	object(int cx, int cy) : bbox{ cx, cy, 1, 1 } {};
	virtual ~object() {};
	uint16_t GetX() { return bbox.x; }
	uint16_t GetY() { return bbox.y; }
	float fGetX() { return (float)bbox.x; }
	float fGetY() { return (float)bbox.y; }
	void SetXY(int cx, int cy) { bbox.x = cx; bbox.y = cy; }
	void SetXY(uint16_t& cx, uint16_t& cy) { bbox.x = cx; bbox.y = cy; }
	SDL_Rect& GetBBox() { return bbox; }
	SDL_Color& GetColor() { return color; }
};

class wall : public object {
public:
	wall(int cx, int cy, int cwidth = 1, int cheight = 1) : object(cx, cy) { bbox.w = cwidth; bbox.h = cheight; };
	virtual COLLISION onCollide() { return COLL_KILL; }
};

class fruit : public object {
	uint16_t value;
public:
	fruit(int cx, int cy, uint16_t val_points = 100) : object(cx, cy), value(val_points) {};
	COLLISION onCollide() { return COLL_EAT; }
	uint16_t GetValue() { return value; }
};


class object_list {
private:
	friend class SnGameInstance;
	std::list<object*> objlist;
public:
	object_list() {};
	~object_list();
	COLLISION getCollision(const int& cx, const int& cy); // Gets object collision result; may remove it
	object* getObject(const int& cx, const int& cy);
	//std::list<object&> getObjectsByType()
	bool Add(object* obj);                      // Finds object in the list, removes it and returns TRUE
};

class field;

class sneke {
private:
	friend class SnGameInstance;
	uint16_t x, y;
	DIRECTION movement_dir = DIR_LEFT;
	//std::list<DIRECTION> movement_dirs;

	class sneke_body {
	public:
		std::list<wall> body;
		uint16_t last_piece, length;

		sneke_body(uint16_t length_, int& head_x, int& head_y);
		void Grow();
		void Move(uint16_t& head_x, uint16_t& head_y);
		void Coil(int cx, int cy) { for (auto& a : body) a.SetXY(cx, cy); }
		//void PushMoveDir(DIRECTION new_direction);          // Adds new movement direction to movement_dirs list
		//DIRECTION PopMoveDir();                             // Returns first movement direction from the queue. Returns DIR_NONE if queue is empty

		uint16_t GetLength() { return length; }
	};

	sneke_body body;
public:
	sneke(int cx, int cy);
	~sneke();
	
	bool IsCollidingWithBody();
	void Respawn(int cx, int cy) { x = cx; y = cy; body.Coil(cx, cy); }
	uint16_t GetX() { return x; }
	uint16_t GetY() { return y; }
	uint16_t GetLength() { return body.length; }
	std::list<wall>& GetBody() { return body.body; };
};

// Single game instance: reused only when game is replayed
// TODO: Starting the game; main loop; render to the framebuffer; render framebuffer to the plane
class SnGameInstance {
private:
	friend class SnekGame;
	uint16_t x, y;
	uint16_t speed, lives;
	sneke player;
	DIRECTION dir_new = DIR_NONE;
	object_list objects;
	uint32_t score = 0;


	std::mt19937 rand;
	std::list<object*> GetObjectList() { return objects.objlist; }
	std::uniform_int_distribution<int> coords_x;
	std::uniform_int_distribution<int> coords_y;
public:
	SnGameInstance(int size_x, int size_y, uint16_t gspeed = 2, uint16_t glives = 3, bool solid_edges = false);
	void OnCollosion();
	void Move(SDL_Scancode btn);
	void Update();
	void SpawnTreat();
};

typedef enum {SNEK_START, SNEK_LOSE, SNEK_SOUND, SNEK_SCORE, SNEK_EXIT} SnekEventType;

class SnekEvent : public Event {
public:
	const SnekEventType purpose;
	SnekEvent(SnekEventType eventPurpose) : Event(EVENT_SENK), purpose(eventPurpose){}
};

class SnekEvent_StartGame : public SnekEvent {
public:
	uint16_t x, y, speed, live; bool wall;
	SnekEvent_StartGame(uint16_t size_x, uint16_t size_y, uint16_t game_speed, uint16_t lives, bool solid) 
		: SnekEvent(SNEK_START), x(size_x), y(size_y), speed(game_speed), live(lives), wall(solid) {}
};

class SnekEvent_Sound : public SnekEvent {
public:
	std::string name;
	SnekEvent_Sound(const std::string& filename) : SnekEvent(SNEK_SOUND), name("sken\\" + filename) {}
};

extern float pix_verts[];