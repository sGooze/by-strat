// sn_main.cpp

#include "sn_main.hpp"

// object_list

bool object_list::Add(object* obj) {
	if (getObject(obj->GetX(), obj->GetY()) != nullptr) {
		return false;
	}
	objlist.push_back(obj);
	return true;
}

COLLISION object_list::getCollision(const int& cx, const int& cy) {
	static SDL_Rect rect = { 0, 0, 1, 1 };
	rect.x = cx; rect.y = cy;
	for (auto it = objlist.begin(); it != objlist.end(); it++) {
		if (SDL_HasIntersection(&rect, &((*it)->GetBBox())))
			return (*it)->onCollide();
	}
	return COLL_NONE;
}

object* object_list::getObject(const int& cx, const int& cy) {
	/*for (auto it = objlist.begin(); it != objlist.end(); it++){
	if (((*it)->GetX() == cx)&&((*it)->GetY() == cy))
	return *it;
	}*/
	static SDL_Rect rect = { 0, 0, 1, 1 };
	rect.x = cx; rect.y = cy;
	for (auto it = objlist.begin(); it != objlist.end(); it++) {
		if (SDL_HasIntersection(&rect, &((*it)->GetBBox())))
			return *it;
	}
	return nullptr;
}

object_list::~object_list() {
	for (auto it = objlist.begin(); it != objlist.end(); it++) {
		delete *it;
	}
}

////******* S N E K E _ B O D Y *******////

sneke::sneke_body::sneke_body(uint16_t length_, int& head_x, int& head_y) : length(length_) {
	wall body_temp(head_x, head_y);
	for (int i = 0; i < length; i++) {
		body.push_back(body_temp);
	}
}

void sneke::sneke_body::Grow() {
	body.push_back(body.back());
	length++;
}

void sneke::sneke_body::Move(uint16_t& head_x, uint16_t& head_y) {
	body.splice(body.begin(), body, --(body.end()));
	body.front().SetXY(head_x, head_y);
}

////******* S N E K E *******////

sneke::sneke(int cx, int cy) : x(cx), y(cy), body(5, cx, cy) {
	// Construct snake body
}

sneke::~sneke() {}

bool sneke::IsCollidingWithBody() {
	for (wall& piece : body.body) {
		if ((x == piece.GetX()) && (y == piece.GetY()))
			return true;
	}
	return false;
}

////******* G A M M I N G *******////

SnekGame::SnekGame() {
	acceptInput = true;
	// resource allocation
	snMesh.Load("snek_scene_wall.obj");
	snMesh.Load("snek_scene_cab.obj");
	snMesh.Load("snek_scene_screen.obj");
	snMesh.Load("pix", pix_verts, 6, false);
	snTex.Load("lightmap_cab.png");
	snTex.Load("lightmap_walls.png");
	
	EventQueue::AddEvent(new AudioEvent_PrecacheMusic("nesong2.ogg"));
	EventQueue::AddEvent(new AudioEvent_PrecacheSound("sken\\collide.wav"));
	EventQueue::AddEvent(new AudioEvent_PrecacheSound("sken\\win.wav"));
	EventQueue::AddEvent(new AudioEvent_PrecacheSound("sken\\lose.wav"));
	EventQueue::AddEvent(new AudioEvent_PrecacheSound("sken\\extralife2.wav"));
	EventQueue::AddEvent(new AudioEvent_PrecacheSound("sken\\eat1.wav"));
	EventQueue::AddEvent(new AudioEvent_PrecacheSound("sken\\eat2.wav"));
	EventQueue::AddEvent(new AudioEvent_PrecacheMusic("bckdrone2.ogg"));
	EventQueue::AddEvent(new AudioEvent_PrecacheMusic("snbckg1.flac"));
	snThinks.Add(new ThinkerDelayedEvent(new AudioEvent_PlayMusic("bckdrone2.ogg"), 1, 1, 1000));
}

SnekGame::~SnekGame() { 
	if (instance != nullptr) delete instance; 
}

void SnGameInstance::SpawnTreat() {
	fruit* candy = new fruit(0, 0);
	bool test;
	do {
		candy->SetXY(coords_x(rand), coords_y(rand));
		test = objects.Add(candy);
	} while (!test);
}

SnGameInstance::SnGameInstance(int size_x, int size_y, uint16_t gspeed, uint16_t glives, bool solid_edges)
	: player(size_x / 2, size_y / 2), x(size_x), y(size_y), coords_x(0, size_x - 1), coords_y(0, size_y - 1), speed(gspeed), lives(glives) {
	std::random_device seed_provider;
	rand.seed(seed_provider());
	if (solid_edges) {
		objects.Add(new wall(0, 0, size_x, 1));
		objects.Add(new wall(0, size_y - 1, size_x, 1));
		objects.Add(new wall(0, 1, 1, size_y-1));
		objects.Add(new wall(size_x - 1, 1, 1, size_y-1));
	}
}

void SnGameInstance::Move(SDL_Scancode key) {
	switch (key) {
	case SDL_SCANCODE_UP:  if ((dir_new == DIR_NONE) && (player.movement_dir != DIR_UP)) dir_new = DIR_DOWN; break;
	case SDL_SCANCODE_LEFT:  if ((dir_new == DIR_NONE) && (player.movement_dir != DIR_RIGHT)) dir_new = DIR_LEFT; break;
	case SDL_SCANCODE_RIGHT: if ((dir_new == DIR_NONE) && (player.movement_dir != DIR_LEFT)) dir_new = DIR_RIGHT; break;
	case SDL_SCANCODE_DOWN:    if ((dir_new == DIR_NONE) && (player.movement_dir != DIR_DOWN)) dir_new = DIR_UP; break;
	}
}

void SnGameInstance::OnCollosion() {
	if (--lives <= 0) {
		// TODO: Slight delay between the impact and respawn
		EventQueue::AddEvent(new SnekEvent(SNEK_LOSE));
		EventQueue::AddEvent(new SnekEvent_Sound("lose.wav"));
	}
	else { 
		EventQueue::AddEvent(new SnekEvent_Sound("collide.wav"));
		player.Respawn(x / 2, y / 2); 
	}
}

void SnGameInstance::Update() {
	// Called once per game update cycle
	static object *obj;
	// Move snek's body
	player.body.Move(player.x, player.y);
	// Move snek's head
	if (dir_new != DIR_NONE)
		player.movement_dir = dir_new;
	switch (player.movement_dir) {
	case DIR_DOWN:  (player.y < y - 1) ? player.y++ : (player.y = 0); break;
	case DIR_LEFT:  (player.x > 0) ? player.x-- : (player.x = x - 1); break;
	case DIR_RIGHT: (player.x < x - 1) ? player.x++ : (player.x = 0); break;
	case DIR_UP:    (player.y > 0) ? player.y-- : (player.y = y - 1); break;
	}
	dir_new = DIR_NONE;
	// First, check for collision with body
	if (player.IsCollidingWithBody()) {
		OnCollosion();
		return;
	}
	obj = objects.getObject(player.x, player.y);
	if (obj != nullptr) {
		switch (obj->onCollide()) {
		case COLL_EAT:
			score += obj->GetValue();
			if (score % 1000 == 0)
			{
				speed += 1;
				EventQueue::AddEvent(new SnekEvent_Sound("extralife2.wav"));
			}
			else if (score % 2500 == 0)
			{
				lives++;
				EventQueue::AddEvent(new SnekEvent_Sound("eat2.wav"));
			}
			else
				EventQueue::AddEvent(new SnekEvent_Sound("eat1.wav"));
			player.body.Grow();
			//obj->SetXY( coords_x(randgen), coords_y(randgen) );

			int i, j;
			do {
				i = coords_x(rand);
				j = coords_y(rand);
			} while (objects.getObject(i, j) != nullptr);
			obj->SetXY(i, j);

			break;
		case COLL_KILL:
			OnCollosion();
			return;
		}
	}
}

