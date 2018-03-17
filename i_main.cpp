// i_main.cpp - r u n - defrag for mentally handicapped
// S C A L E:
//  50 units (player height) = 2 meters (our hero is trofimmo)
//  1 unit = 4cm

#include "mine_gui.hpp"
#include "mine_gl.hpp"
//#include <btBulletDynamicsCommon.h>


/* * * * * * * * * * * * *
* Events
* * * * * * * * * * * * * *
*/

typedef enum {SHUTIN_CLOSE, SHUTIN_SKY, SHUTIN_FINISH, SHUTIN_FAIL, SHUTIN_RESET, SHUTIN_QUICKFADE, SHUTIN_MSG, SHUTIN_CHLEVEL} ShootEventType;

class ShootEvent : public Event {
public:
	ShootEventType purp;
	ShootEvent(ShootEventType purpose) : purp(purpose), Event(EVENT_SHUT) {}
};

class ShootEvent_Finish : public ShootEvent {
public:
	float time;
	std::string nextlevel;
	ShootEvent_Finish(float finTime, const std::string& level) : time(finTime), nextlevel(level), ShootEvent(SHUTIN_FINISH) {}
};

class ShootEvent_Msg : public ShootEvent {
public:
	std::string msg;
	bool nosound;
	ShootEvent_Msg(const std::string& message, bool silent = false) : msg(message), nosound(silent), ShootEvent(SHUTIN_MSG) {}
};

class ShootEvent_ChLevel : public ShootEvent {
public:
	std::string level;
	ShootEvent_ChLevel(const std::string& next_level) : level(next_level), ShootEvent(SHUTIN_CHLEVEL) {}
};

static bool keyStates[SDL_NUM_SCANCODES] = { false };




/* * * * * * * * * * * * *
* Player
* * * * * * * * * * * * * *
*/

static glm::vec3 start;
static glm::vec3 start_def;
static glm::vec3 start_export;

class Player {
private:
	glm::vec3 pos;
	glm::vec3 look;
	glm::vec3 dir;
	glm::vec3 head = glm::vec3(0.0f, 50.0f, 0.0f);
	glm::vec3 acc = glm::vec3(0.0f);
	float pitch = 0.0f, yaw = 0.0f, speed = 0.0f;
	static glm::vec3 up;
public:
	Player(glm::vec3 position) : pos(position), look(glm::vec3(0.0f, 0.0f, -1.0f)), dir(glm::vec3(0.0f, 0.0f, -1.0f)) {}
	void Move(float new_y, float step = 10.0f); // Get the height of the point on which player is standing; if it its lower or higher, then start to apply gravity
	void Rotate(Sint32 xrel, Sint32 yrel);
	glm::mat4 LookAt() { return glm::lookAt(pos + head, pos + head + look, up); }
	void Acc() { ImGui::Text("Acc: %f %f %f\nSpeed: %f", acc.x, acc.y, acc.z, speed); }
	void SetPos(glm::vec3 npos, bool cancelVel = false) { pos = npos; if (cancelVel) acc = glm::vec3(); }
	void SetDir(glm::vec3 nlook) { look = nlook; }
	void SetYaw(float nyaw) { yaw = nyaw; }
	void SetVel(glm::vec3 nvel) { acc = nvel; }
	void Push(glm::vec3 vec) { 
		acc.y = 0;
		acc += vec; 
		pos.y += 10; 
	}
	float GetPitch() { return pitch; }
	float GetSpeed() { return speed; }
	glm::vec3 Pos() { return pos; }
};

static Player camera(glm::vec3(0.0f, 0.0f, 0.0f));

class MessageLog {
private:
	static std::list<std::pair<uint32_t, std::string>> messages;
	static uint16_t max;
	static uint16_t limit;
public:
	static void Add(const std::string& msg) {
		messages.push_back(std::make_pair(SDL_GetTicks(), msg)); 
		if (messages.size() > max)
			messages.pop_front(); 
	}
	static void Think() {
		uint32_t cap = SDL_GetTicks();
		while (true) {
		}
	}
	static void PrintIm() { 
		uint32_t cap = SDL_GetTicks();
		ImVec4 col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		while (!messages.empty() && cap - messages.front().first >= limit)
			messages.pop_front();
		for (auto& x : messages) {
			col.w = (cap - x.first < limit - 1000) ? 1.0f : (1000 - (cap - x.first - (limit - 1000))) / 1000.0f;
			ImGui::TextColored(col, x.second.c_str());
			//ImGui::SameLine(); ImGui::Text("%f", col.w);
		}
	}
};

uint16_t MessageLog::max = 5;
uint16_t MessageLog::limit = 5000;
std::list<std::pair<uint32_t, std::string>> MessageLog::messages = std::list<std::pair<uint32_t, std::string>>();

/* * * * * * * * * * * * *
* Timer
* * * * * * * * * * * * * *
*/

class GlobalGameTimer {
	int32_t time_start;
	int32_t time_stop;
	int32_t time_last;
	int32_t time_stop_def;
	bool active = false;
public:
	bool pause = false;
	GlobalGameTimer(int32_t timestop) : time_stop(timestop), time_stop_def(timestop) {}
	void Start() { time_start = SDL_GetTicks(); active = true; }
	void Stop() { active = false; time_stop = time_stop_def; }
	bool Active() { return active; }
	bool Tick() { 
		if (pause) time_start += SDL_GetTicks() - time_last;
		time_last = SDL_GetTicks();
		if (!active) {
			time_start = time_last;
			// audio que
		}
		else if (time_stop <= time_last - time_start) Stop();
		return (int32_t)(SDL_GetTicks() - time_start) <= time_stop; 
	}
	float GetSecs() { return(time_stop + time_start - time_last) / 1000.0f; }
	void PrintSecs() {
		static ImVec4 norm = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		static ImVec4 dang = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
		ImGui::TextColored((active) ? norm : dang, u8"Время: %.2f", GetSecs());
	}
	void AddTime(int32_t msec) {
		time_stop += msec;
	}
	int32_t GetLength() { return time_stop; }
};
static GlobalGameTimer gtimer = GlobalGameTimer(3000);

typedef enum { PICKUP_REQUIRED } TriggerPickupType;

class PickupCounter {
	static uint16_t picks, left;
public:
	static void Init(uint16_t pickups) { picks = pickups; left = picks; }
	static void Reset() { left = picks; }
	static void Add() { left++; picks++; }
	static uint16_t Left() { return left; }
	static uint16_t OnPickup() { return --left; }
};

uint16_t PickupCounter::picks = 0;
uint16_t PickupCounter::left = 0;

class Model {
protected:
	MeshGL& mdl;
	TextureGL& tex;
	std::string id;
public:
	glm::vec3 pos;
	glm::vec3 angles;

	Model( const std::string& mdl_id, MeshGL& model, TextureGL& texture) : id(mdl_id), mdl(model), tex(texture) {}
	glm::mat4 MtxPos() { 
		glm::mat4 trans;
		trans = glm::translate(trans, pos);
		trans = glm::rotate(trans, angles.x, glm::vec3(1.0, 0.0, 0.0));
		trans = glm::rotate(trans, angles.y, glm::vec3(0.0, 1.0, 0.0));
		return glm::rotate(trans, angles.z, glm::vec3(0.0, 0.0, 1.0));
	}
	void SetPos(glm::vec3 position) { pos = position; }
	void Render() { tex.Use(0); mdl.Render(); }
	std::string ID() { return id; }
};

class ModelAnimated : public Model {
	glm::vec3 frame;
	
public:
	ModelAnimated(const std::string& mdl_id, MeshGL& model, TextureGL& texture, glm::vec3 update_per_frame) : Model(mdl_id, model, texture), frame(update_per_frame) {}
	void Update() { angles += frame; }
};

std::list<Model> allStaticModels;
std::list<ModelAnimated> allRotatingModels;

class Decoration {
	glm::vec3 pos, angle;
	Model& mdl;
public:
	Decoration(Model& model, glm::vec3 position, glm::vec3 angles) : mdl(model), pos(position), angle(angles) {}
	glm::mat4 GetMatrix() {
		glm::mat4 trans;
		trans = glm::translate(trans, pos);
		trans = glm::rotate(trans, angle.x, glm::vec3(1.0, 0.0, 0.0));
		trans = glm::rotate(trans, angle.y, glm::vec3(0.0, 1.0, 0.0));
		return glm::rotate(trans, angle.z, glm::vec3(0.0, 0.0, 1.0));
	}
	void Render() {
		//mdl.pos = pos; mdl.angles = angle;
		mdl.Render();
	}
};

std::list<Decoration> allDecors;

/* * * * * * * * * * * * * 
* Trigger
* * * * * * * * * * * * * *
*/

// An axis-aligned rectangular area, which can detect presence of a player inside of it
class AreaTrig {
protected:
	GLfloat points[32];
	SDL_Rect area;
	MeshGL* mesh3d = nullptr;
	float z;
public:
	friend class ShootGame;
	AreaTrig(int x, int y, int w = 1, int h = 1, float z_ = 3.0f) : z(z_) {
		area.x = x; area.y = y; area.w = w; area.h = h;
	}
	void UpdateMesh() {
		if (mesh3d != nullptr) delete mesh3d;
		points[0] = (float)(area.x);
		points[1] = z;
		points[2] = (float)(area.y);
		points[3] = 0.0f;
		points[4] = 0.0f;
		points[5] = 0.0f;
		points[6] = 1.0f;
		points[7] = 0.0f;
		points[8] = (float)(area.x + area.w);
		points[9] = z;
		points[10] = (float)(area.y);
		points[11] = 0.0f;
		points[12] = 0.0f;
		points[13] = 0.0f;
		points[14] = 1.0f;
		points[15] = 0.0f;
		points[16] = (float)(area.x + area.w);
		points[17] = z;
		points[18] = (float)(area.y + area.h);
		points[19] = 0.0f;
		points[20] = 0.0f;
		points[21] = 0.0f;
		points[22] = 1.0f;
		points[23] = 0.0f;
		points[24] = (float)(area.x);
		points[25] = z;
		points[26] = (float)(area.y + area.h);
		points[27] = 0.0f;
		points[28] = 0.0f;
		points[29] = 0.0f;
		points[30] = 1.0f;
		points[31] = 0.0f;
		mesh3d = new MeshGL(points, 4, false);
	}
	~AreaTrig() {  }
	bool IntersectPoint(int px, int py) {
		static SDL_Point pt; pt.x = px; pt.y = py;
		return (SDL_PointInRect(&pt, &area) == SDL_TRUE);
	}
	bool IntersectPoint(float px, float py) {
		return IntersectPoint((int)px, (int)py);
	}
	void RenderContour() {
		mesh3d->Render(GL_LINE_LOOP);
	};

	float Z() { return points[1]; }
	float IsAbove(const glm::vec3& coords) { return IntersectPoint(coords.x, coords.z) && ((z <= coords.y + 2.5)||((z > coords.y)&&(z <= coords.y + 20))); } /*&& (z <= coords.y + 2.5)*/
	virtual std::string Save() { return ""; }
};


class Platform : public AreaTrig {
private:
	static MeshGL* platmesh;
	TextureGL* tex = nullptr;
public:
	static void AddMesh(MeshGL& m) { platmesh = &m; /*m.Export();*/ }
	Platform(int x, int y, int w = 1, int h = 1, float z = 3.0f, TextureGL* pl_tex = nullptr) : AreaTrig(x, y, w, h, z), tex(pl_tex) {}
	void Move(int x, int y, int w, int h, float z_) {
		area.x = x; area.y = y; area.w = w; area.h = h;
		z = z_;
	}
	void Move(glm::vec3 pos, glm::ivec2 size) {
		area.x = (int)pos.x; area.y = (int)pos.z; area.w = size.x; area.h = size.y;
		z = pos.y;
	}
	glm::mat4 GetMatrix() {
		glm::mat4 model;

		model = glm::translate(model, glm::vec3((float)(area.x), z, (float)(area.y)));
		model = glm::scale(model, glm::vec3(area.w, 1.0f, area.h));
		// TODO: scale
		return model;
	}
	glm::vec2 GetScaleVec() { return glm::vec2(area.w / 50.0f, area.h / 50.0f); }
	void Render(GLenum mode = GL_TRIANGLES) {
		if (tex != nullptr) tex->Use();
		platmesh->Render(mode);
	}
	std::string Save(){
		std::ostringstream temp; char t = '\t';
		temp << "PLAT\t" << area.x << t << area.y << t << area.w << t << area.h << t << z << std::endl;
		return temp.str();
	}
};

// basic class for all triggers - areas that trigger specified actions when player steps inside of them
class Trigger : public AreaTrig {
protected:
	bool triggered = false;
	float height;
public:
	Trigger(int x, int y, int w, int h, float z_, float height_) : AreaTrig(x, y, w, h, z_), height(height_) {}
	bool Inside(glm::vec3& coords) { 
		bool meme = IsAbove(coords) && (coords.y <= height + z); 
		return meme;
	}
	virtual void Check(Player& ply) {}
	void Reset() { triggered = false; }
};

class TriggerPush : public Trigger {
	glm::vec3 force;
public:
	TriggerPush(int x, int y, int w, int h, float z_, float height_, glm::vec3 push_force) : Trigger(x, y, w, h, z_, height_), force(push_force) {}
	void Check(Player& ply) { 
		if (Inside(ply.Pos())){
			if (!triggered) {
				ply.Push(force);
				EventQueue::AddEvent(new AudioEvent_PlaySound("jpad_temp.wav"));
			}
			triggered = true;
		}
		else triggered = false;
	}
	std::string Save() {
		std::ostringstream temp; char t = '\t';
		temp << "TRIG\tPUSH\t" << area.x << t << area.y << t << area.w << t << area.h << t << z << t << height << t << "not" << t << force.x << t << force.y << t << force.z << std::endl;
		return temp.str();
	}
};

class TriggerTeleport : public Trigger {
	glm::vec3 dest;
public:
	TriggerTeleport(int x, int y, int w, int h, float z_, float height_, glm::vec3 destination) : Trigger(x, y, w, h, z_, height_), dest(destination) {}
	void Check(Player& ply) {
		if (Inside(ply.Pos())) {
			if (!triggered)
				ply.SetPos(dest);
			triggered = true;
		}
		else triggered = false;
	}
	std::string Save() {
		std::ostringstream temp; char t = '\t';
		temp << "TRIG\tTELE\t" << area.x << t << area.y << t << area.w << t << area.h << t << z << t << height << t << "not" << t << dest.x << t << dest.y << t << dest.z << std::endl;
		return temp.str();
	}
};

class TriggerStartTimer : public Trigger {
	static std::string trackName;
	static bool trackLoops;
public:
	static void SetTrackName(const std::string& name, bool loop) { trackName = name; trackLoops = loop; EventQueue::AddEvent(new AudioEvent_PrecacheMusic(trackName)); }
	TriggerStartTimer(int x, int y, int w, int h, float z_, float height_) : Trigger(x, y, w, h, z_, height_) {}
	void Check(Player& ply) {
		if (Inside(ply.Pos())) {
			gtimer.Start();
			if (!triggered) {
				EventQueue::AddEvent(new AudioEvent_PlayMusic(trackName, 0, 0, (trackLoops) ? -1 : 0));
				start = start_def = glm::vec3(area.x + (area.w / 2.0f), z, area.y + (area.h / 2.0f));
				triggered = true;
			}
		}
	}
	std::string Save() {
		std::ostringstream temp; char t = '\t';
		temp << "TRIG\tSTART\t" << area.x << t << area.y << t << area.w << t << area.h << t << z << t << height << t << "not" << std::endl;
		return temp.str();
	}
};

std::string TriggerStartTimer::trackName = "trainloop130.flac";
bool		TriggerStartTimer::trackLoops = true;
bool alwaysWin = true;

class TriggerFinish : public Trigger {
	std::string levelname;
public:
	TriggerFinish(int x, int y, int w, int h, float z_, float height_, const std::string& level) : Trigger(x, y, w, h, z_, height_), levelname(level) {}
	void Check(Player& ply) {
		if (Inside(ply.Pos())) {
			if (!triggered) {
				if ((PickupCounter::Left() != 0)) {
					MessageLog::Add(u8"Вы не подобрали все ключи... Попробуйте еще раз!");
					EventQueue::AddEvent(new ShootEvent(SHUTIN_FAIL));
				}
				else if ((gtimer.Active() || alwaysWin)) {
					MessageLog::Add(u8"Финиш!");
					EventQueue::AddEvent(new ShootEvent_Finish(gtimer.GetSecs(), levelname));
					gtimer.Stop();
				}
				else {
					MessageLog::Add(u8"Время вышло... Попробуйте еще раз!");
					EventQueue::AddEvent(new ShootEvent(SHUTIN_FAIL));
				}
				triggered = true;
			}
		}
	}
	std::string Save() {
		std::ostringstream temp; char t = '\t';
		temp << "TRIG\tFIN\t" << area.x << t << area.y << t << area.w << t << area.h << t << z << t << height << t << "not" << t << levelname << t << std::endl;
		return temp.str();
	}
};

class TriggerMsg : public Trigger {
	std::string msg;
	bool repeat;
public:
	TriggerMsg(int x, int y, int w, int h, float z_, float height_, const std::string& message, bool repeatable = false) : Trigger(x, y, w, h, z_, height_), msg(message), repeat(repeatable) {}
	void Check(Player& ply) {
		if (Inside(ply.Pos())) {
			if (!triggered) {
				EventQueue::AddEvent(new ShootEvent_Msg(msg));
				triggered = true;
			}
		}
		else if (repeat) triggered = false;
	}
	std::string Save() {
		std::ostringstream temp; char t = '\t';
		temp << "TRIG\tMSG\t" << area.x << t << area.y << t << area.w << t << area.h << t << z << t << height << t << "not" << t << msg << t << ((repeat) ? "Y" : "N") << std::endl;
		return temp.str();
	}
};

class TriggerCheck : public Trigger {
	glm::vec3 spawn;
	bool ovr; // override default respawn instead of a temporary
public:
	TriggerCheck(int x, int y, int w, int h, float z_, float height_, glm::vec3 spwn) : Trigger(x, y, w, h, z_, height_), spawn(spwn) {}
	void Check(Player& ply) {
		if (Inside(ply.Pos())&&(!triggered)) {
			start = spawn;
			triggered = true;
			EventQueue::AddEvent(new ShootEvent_Msg("Checkpoint!", true));
			EventQueue::AddEvent(new AudioEvent_PlaySound("check.wav"));
		}
	}
	std::string Save() {
		std::ostringstream temp; char t = '\t';
		temp << "TRIG\tCHECK\t" << area.x << t << area.y << t << area.w << t << area.h << t << z << t << height << t << "not" << t << spawn.x << t << spawn.y << t << spawn.z << std::endl;
		return temp.str();
	}
};

class TriggerChLevel : public Trigger {
	std::string next_level;
public:
	TriggerChLevel(int x, int y, int w, int h, float z_, float height_, const std::string& level) : Trigger(x, y, w, h, z_, height_), next_level(level) {}
	void Check(Player& ply) {
		if (Inside(ply.Pos()) && (!triggered)) {
			triggered = true;
			EventQueue::AddEvent(new ShootEvent_ChLevel(next_level));
		}
	}
	std::string Save() {
		std::ostringstream temp; char t = '\t';
		temp << "TRIG\tCHLEV\t" << area.x << t << area.y << t << area.w << t << area.h << t << z << t << height << t << "not" << t << next_level << t << std::endl;
		return temp.str();
	}
};

class TriggerPickup : public Trigger {
	ModelAnimated& model;
	TriggerPickupType type;
public:
	TriggerPickup(int x, int y, int w, int h, float z_, float height_, TriggerPickupType pick_type, ModelAnimated& mdl) : Trigger(x, y, w, h, z_, height_), model(mdl), type(pick_type) {}
	void Check(Player& ply) {
		if (Inside(ply.Pos())) {
			if (!triggered) {
				MessageLog::Add("Picked up an item!");
				switch (type) {
				case PICKUP_REQUIRED:
					EventQueue::AddEvent(new AudioEvent_PlaySound("ar2_pkup.wav", 0));
					//gtimer.AddTime(3000);
					PickupCounter::OnPickup();
					break;
				}
				triggered = true;
			}
		}
	}
	std::string Save() {
		std::ostringstream temp; char t = '\t';
		temp << "TRIG\tITEM\t" << area.x << t << area.y << t << area.w << t << area.h << t << z << t << height << t << "not" << t << type << t << std::endl;
		return temp.str();
	}
	glm::mat4 GetMdlMatrix() {
		glm::vec3 mdl_pos = glm::vec3(area.x + (area.w / 2.0f), z, area.y + (area.h / 2.0f));
		model.SetPos(mdl_pos);
		return model.MtxPos();
	}
	void MdlRender() { if (!triggered) model.Render(); }
};


MeshGL*   Platform::platmesh = nullptr;
glm::vec3 Player::up = glm::vec3(0.0f, 1.0f, 0.0f);



/********************************************************************************************************************************************************/



static std::list<Platform> allPlats;
static std::list<Trigger*> allTriggers;
static std::list<TriggerPickup> allPickups;

static std::list<SkyboxGL> boxes;

/*
 * ShootGame
 *
 */

ShootGame::ShootGame() {
	acceptInput = true;
	shMesh.Load("map.obj");
	shMesh.Load("simpleplat.obj");
	shMesh.Load("floor_1.obj");
	shMesh.Load("frame_b.obj");
	shMesh.Load("testg.obj");
	shMesh.Load("screen_b.obj");
	shMesh.Load("pickup.obj");
	shTex.Load("gd_map.png");
	shTex.Load("cav_uv.png");
	shTex.Load("cfl_uv.png");
	shTex.Load("tgun_view.png");
	shTex.Load("pickup.png");
	boxes.emplace_back("jump", ".png");
	//sky = new SkyboxGL("jump", ".png");
	sky = &boxes.back();
	Platform::AddMesh(shMesh.Get("simpleplat.obj"));
	allRotatingModels.push_back(ModelAnimated("pickup", shMesh.Get("pickup.obj"), shTex.Get("pickup.png"), glm::vec3(0.0f, glm::radians(1.0f), 0.0f)));
	EventQueue::AddEvent(new AudioEvent_PrecacheSound("skary.wav"));
	EventQueue::AddEvent(new AudioEvent_PrecacheSound("info.wav"));
	EventQueue::AddEvent(new AudioEvent_PrecacheSound("jump1.wav"));
	EventQueue::AddEvent(new AudioEvent_PrecacheSound("land1.wav"));
	EventQueue::AddEvent(new AudioEvent_PrecacheSound("fail.wav"));
	EventQueue::AddEvent(new AudioEvent_PrecacheSound("tstep1.wav"));
	EventQueue::AddEvent(new AudioEvent_PrecacheSound("tstep2.wav"));
	EventQueue::AddEvent(new AudioEvent_PrecacheSound("tcrash1.wav"));
	EventQueue::AddEvent(new AudioEvent_PrecacheSound("jpad_temp.wav"));
	EventQueue::AddEvent(new AudioEvent_PrecacheSound("info.wav"));
	EventQueue::AddEvent(new AudioEvent_PrecacheSound("check.wav"));
	EventQueue::AddEvent(new AudioEvent_PrecacheSound("ar2_pkup.wav"));
	EventQueue::AddEvent(new AudioEvent_PrecacheMusic("trainloop130.flac"));
}

ShootGame::~ShootGame() {
#ifdef _DEBUG
	ExportLevel("players\\temp.lvl", 99.0);
#endif
	for (auto& x : allTriggers)
		delete x;
}

void ShootGame::Reset() {
	camera.SetPos(start_def);
	start = start_def;
	camera.SetVel(glm::vec3());
	for (auto& x : allTriggers)
		x->Reset();
	for (auto& x : allPickups)
		x.Reset();
	gtimer.Stop();
	PickupCounter::Reset();
	fadeIn = 20.0f;
	EventQueue::AddEvent(new AudioEvent_StopMusic(500));
	EventQueue::AddEvent(new AudioEvent_PlaySound("tcrash1.wav"));
	EventQueue::AddEvent(new ShootEvent(SHUTIN_QUICKFADE));
}

/*****************************************
* Level loading
*
*/

static bool gameWon = false;

static std::string lastLevel = "players\\level1.lvl";

void ShootGame::ImportObject(const std::string& buf) {
	std::vector<std::string> toks = split(buf, '\t');
	static std::string platTexture = "gd_map.png";
	if (toks.size() == 0) return;
	if ((toks[0] == "PLAYER")) {
		if (toks.size() >= 4) {
			camera.SetPos(glm::vec3(std::stof(toks[1]), std::stof(toks[2]), std::stof(toks[3])));
			start = start_def = start_export = camera.Pos();
		}
		if (toks.size() == 5) {
			//camera.SetDir(glm::vec3(std::stof(toks[4]), std::stof(toks[5]), std::stof(toks[6])));
			camera.SetYaw(std::stof(toks[4]));
		}
	}
	else if (toks[0] == "TIME") {
		if (toks.size() >= 2) gtimer = GlobalGameTimer(std::stoi(toks[1]));
	}
	else if (toks[0] == "TRACK") {
		if (toks.size() >= 3) TriggerStartTimer::SetTrackName(toks[1], (toks[2].at(0) == 'Y'));
	}
	else if (toks[0] == "TEX") {
		if (toks.size() >= 2) {
			platTexture = toks[1];
			shTex.Load(platTexture);
		}
	}
	else if (toks[0] == "SKY") {
		if (toks.size() >= 3) {
			boxes.emplace_back(toks[1], toks[2]);
			sky = &boxes.back();
		}
	}
	else if (toks[0] == "PRECACHE") {
		// resource precaching
		if (toks[1] == "MDL" && toks.size() >= 5) {
			shMesh.Load(toks[3]);
			shTex.Load(toks[4]);
			allStaticModels.push_back(Model(toks[2], shMesh.Get(toks[3]), shTex.Get(toks[4])));
		}
		else if (toks[1] == "TEX" && toks.size() >= 3)
			shTex.Load(toks[1]);
	}
	else if (toks[0] == "DECOR") {
		if (toks.size() >= 8) {
			Model* mdl = nullptr;
			for (auto& x : allStaticModels) {
				if (x.ID() == toks[1])
					mdl = &x;
			}
			if (mdl == nullptr) { std::cout << "Skipping decoration: unknown model " << toks[1] << std::endl; return; }
			allDecors.push_back(Decoration(*mdl, glm::vec3(std::stof(toks[2]), std::stof(toks[3]), std::stof(toks[4])), glm::vec3(std::stof(toks[5]), std::stof(toks[6]), std::stof(toks[7]))));
		}
	}
	else {
		if (toks[0] == "PLAT") {
			if (toks.size() >= 6) {
				allPlats.push_back(Platform(std::stoi(toks[1]), std::stoi(toks[2]), std::stoi(toks[3]), std::stoi(toks[4]), std::stof(toks[5]), &(shTex.Get(platTexture))));
				allPlats.back().UpdateMesh();
			}
		}
		else {
			if ((toks[0] == "TRIG") && (toks.size() >= 9)) {
				/*if (toks[8] == "vis") {
					// create a platform underside the trigger
					allPlats.push_back(Platform(std::stoi(toks[2]), std::stoi(toks[3]), std::stoi(toks[4]), std::stoi(toks[5]), std::stof(toks[6])));
					allPlats.back().UpdateMesh();
				}*/
				if (toks[1] == "PUSH") {
					if (toks.size() >= 12) {
						allTriggers.push_back(new TriggerPush(std::stoi(toks[2]), std::stoi(toks[3]), std::stoi(toks[4]), std::stoi(toks[5]), std::stof(toks[6]), std::stof(toks[7]), glm::vec3(std::stof(toks[9]), std::stof(toks[10]), std::stof(toks[11]))));
					}
				}
				if (toks[1] == "TELE") {
					if (toks.size() >= 12) {
						allTriggers.push_back(new TriggerTeleport(std::stoi(toks[2]), std::stoi(toks[3]), std::stoi(toks[4]), std::stoi(toks[5]), std::stof(toks[6]), std::stof(toks[7]), glm::vec3(std::stof(toks[9]), std::stof(toks[10]), std::stof(toks[11]))));
					}
				}
				if (toks[1] == "START") {
					alwaysWin = false;
					if (toks.size() >= 9) {
						allTriggers.push_back(new TriggerStartTimer(std::stoi(toks[2]), std::stoi(toks[3]), std::stoi(toks[4]), std::stoi(toks[5]), std::stof(toks[6]), std::stof(toks[7])));
					}
				}
				if (toks[1] == "FIN") {
					if (toks.size() >= 9) {
						allTriggers.push_back(new TriggerFinish(std::stoi(toks[2]), std::stoi(toks[3]), std::stoi(toks[4]), std::stoi(toks[5]), std::stof(toks[6]), std::stof(toks[7]),  ((toks.size() > 9) ? toks[9] : "") ));
					}
				}
				if (toks[1] == "MSG") {
					if (toks.size() >= 11) {
						allTriggers.push_back(new TriggerMsg(std::stoi(toks[2]), std::stoi(toks[3]), std::stoi(toks[4]), std::stoi(toks[5]), std::stof(toks[6]), std::stof(toks[7]), toks[9], (toks[10].at(0) == 'Y')));
					}
				}
				if (toks[1] == "CHECK") {
					if (toks.size() >= 12) {
						allTriggers.push_back(new TriggerCheck(std::stoi(toks[2]), std::stoi(toks[3]), std::stoi(toks[4]), std::stoi(toks[5]), std::stof(toks[6]), std::stof(toks[7]), glm::vec3(std::stof(toks[9]), std::stof(toks[10]), std::stof(toks[11]))));
					}
				}
				if (toks[1] == "CHLEV") {
					if (toks.size() >= 10) {
						allTriggers.push_back(new TriggerChLevel(std::stoi(toks[2]), std::stoi(toks[3]), std::stoi(toks[4]), std::stoi(toks[5]), std::stof(toks[6]), std::stof(toks[7]), toks[9]));
					}
				}
				if (toks[1] == "ITEM") {
					if (toks.size() >= 10) {
						switch (std::stoi(toks[9])) {
						case PICKUP_REQUIRED:
							allPickups.push_back(TriggerPickup(std::stoi(toks[2]), std::stoi(toks[3]), std::stoi(toks[4]), std::stoi(toks[5]), std::stof(toks[6]), std::stof(toks[7]), PICKUP_REQUIRED, allRotatingModels.front()));
							break;
						}
						PickupCounter::Add();
					}
				}
			}
		}
	}
}

bool ShootGame::LoadLevel(const std::string& path) {
	static bool loaded = false;	
	static int ver = 2; // Level file version
	std::cout << "Loading level: " << path << std::endl;
	gameWon = false;
	alwaysWin = true; // in case there's no START trigger in the level
	sky = &boxes.front();
	EventQueue::AddEvent(new AudioEvent_StopMusic(0));
	PickupCounter::Init(0);
	if (loaded) {
		allPlats.clear();
		for (auto&x : allTriggers) {
			delete x;
		}
		allTriggers.clear();
		allPickups.clear();
		allDecors.clear();
		allStaticModels.clear();
		//return true;
	}

	//ImportObject("PRECACHE\tMDL\tteleport\ttele.obj\ttele.png\t");
	std::ifstream filestr(path);
	std::string buf;
	if (!filestr.good()) { std::cout << path << ": file missing, empty or corrupted\n"; return false; }
	std::getline(filestr, buf);
	if (std::stoi(buf) != ver) { std::cout << path << ": invalid version; must be " << ver << std::endl; return false; }
	while (filestr.good()) {
		std::getline(filestr, buf);
		ImportObject(buf);
	}

	loaded = true;
	lastLevel = path;
	EventQueue::AddEvent(new ShootEvent(SHUTIN_QUICKFADE));
	return true;
}

static bool passFrame = false, paused = false;

ProcessEventResult ShootGame::ProcessEvent(SDL_Event& event) {
	// TODO: deactivate if inside menu
	if (event.type == SDL_KEYDOWN) {
		/*if (event.key.keysym.scancode == SDL_SCANCODE_F) {
			EventQueue::AddEvent(new GameEvent(GAME_TOGGLE_MOUSE));
			mouseCrutch = !mouseCrutch;
		}*/
		keyStates[event.key.keysym.scancode] = true;
	}
	if (event.type == SDL_KEYUP) {
		if (event.key.keysym.scancode == SDL_SCANCODE_G)
			passFrame = true;
		else if ((event.key.keysym.scancode == SDL_SCANCODE_GRAVE)) {
			paused = !paused;
			EventQueue::AddEvent(new GameEvent(GAME_TOGGLE_MOUSE));
			mouseCrutch = !mouseCrutch;
		}
		else if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {

		}
		else if (event.key.keysym.scancode == SDL_SCANCODE_F5) {
			LoadLevel(lastLevel);
		}
		else if ((event.key.keysym.scancode == SDL_SCANCODE_R)&&!paused) {
			if (keyStates[SDL_SCANCODE_LSHIFT]) Reset();
			else camera.SetPos(start);
		}
		keyStates[event.key.keysym.scancode] = false;
	}
	if (!paused && !inMainMenu && (event.type == SDL_MOUSEBUTTONDOWN) && (event.button.button == SDL_BUTTON_LEFT))
		MessageLog::Add(u8"У вас нет сил нажать на курок...");
	if (mouseCrutch && event.type == SDL_MOUSEMOTION) {
		camera.Rotate(event.motion.xrel, event.motion.yrel);
	}
	return PE_NOP;
}

void ShootGame::InternalEvents() {
	ShootEvent* ev;
	while (EventQueue::PollEvent(EVENT_SHUT, (Event**)&ev)) {
		switch (ev->purp) {
		case SHUTIN_CLOSE:
			//EventQueue::AddEvent(new GameEvent(GAME_STOP));
			mouseCrutch = false;
			EventQueue::AddEvent(new GameEvent(GAME_TOGGLE_MOUSE, (void*)1));
			inMainMenu = true;
			break;
		case SHUTIN_FAIL:
			shThink.Add(new ThinkerSub<float>(fadeIn, 0.05f, 0.0f, 50));
			shThink.Add(new ThinkerDelayedEvent(new ShootEvent(SHUTIN_RESET), 1, 1, 2000));
			EventQueue::AddEvent(new AudioEvent_PlaySound("fail.wav"));
			break;
		case SHUTIN_RESET:
			Reset();
			break;
		case SHUTIN_QUICKFADE:
			fadeIn = 5.0f;
			shThink.Add(new ThinkerSub<float>(fadeIn, 0.1f, 1.0f, 10));
			break;
		case SHUTIN_FINISH:
			if (((ShootEvent_Finish*)ev)->nextlevel.empty()) {
				shThink.Add(new ThinkerSet<bool>(inMainMenu, true, 2000));
				EventQueue::AddEvent(new GameEvent(GAME_TOGGLE_MOUSE, (void*)1));
				mouseCrutch = paused = false;
				if (gameWon) achives.run_3 = true;
				gameWon = true;
			}
			else {
				shThink.Add(new ThinkerDelayedEvent(new ShootEvent_ChLevel(((ShootEvent_Finish*)ev)->nextlevel), 1, 1, 2000));
				MessageLog::Add("Prepare for the next level...");
			}
			
			shThink.Add(new ThinkerSet<float>(fadeIn, 1.0f, 2000));
			
			break;
		case SHUTIN_MSG:
			MessageLog::Add(((ShootEvent_Msg*)ev)->msg);
			if (!(((ShootEvent_Msg*)ev)->nosound)) EventQueue::AddEvent(new AudioEvent_PlaySound("info.wav"));
			break;
		case SHUTIN_CHLEVEL:
			if (!LoadLevel(((ShootEvent_ChLevel*)ev)->level)) {
				inMainMenu = true;
				mouseCrutch = paused = false;
			}
			break;
		}
		delete ev;
	}
}

/*
* Player
*
*/

static void Step() {
	static uint32_t last = 0, cur = 0;
	cur = SDL_GetTicks();
	if (cur - last >= 300) {
		last = cur;
		if (frand() >= 0.5)
			EventQueue::AddEvent(new AudioEvent_PlaySound("tstep2.wav"));
		else
			EventQueue::AddEvent(new AudioEvent_PlaySound("tstep1.wav"));
		//EventQueue::AddEvent(new AudioEvent_PlaySound((frand() >= 0.5) ? "tstep1.wav" : "tstep2.wav"));
	}
}

void Player::Rotate(Sint32 xrel, Sint32 yrel) {
	static float sense = 0.1f;
	yaw += xrel * sense;
	pitch -= yrel * sense;
	if (pitch >= 89.99f) pitch = 89.99f;
	if (pitch <= -89.99f) pitch = -89.99f;
}

bool flyin = false;

void Player::Move(float new_y, float step) {
	static float g = 10.0f / 60.0f;	// acceleration per frame
	float cameraSpeed; 
	//static float horizontalSlow = 0.6f; // step = 1
	static float horizontalSlow = 0.96f;  // step = 10  

	look.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	look.y = sin(glm::radians(pitch));
	look.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	look = glm::normalize(look);

	bool fall = (pos.y != new_y);
	if (((pos.y - new_y <= 2.5f)|| ((pos.y < new_y)&&(pos.y - new_y >= -20.0f))) && (acc.y <= 0.0f) && (new_y > 0.0f))
	//if ((std::abs(pos.y - new_y) <= 2.5f)&&(acc.y <= 0.0f))
	{
		pos.y = new_y; 
		fall = false;
		if (acc.y < -7.0f) EventQueue::AddEvent(new AudioEvent_PlaySound("land1.wav"));
		acc.y = 0.0f;
	}

	static unsigned int fr = 0; // number of on-the-ground frames 
	unsigned int fr_tr = 1 * step; // number of on-the-ground frames that must pass before acceleration starts to slow down
	if (!fall) {
		// if we're not falling, then we can freely move and our ground speed quickly halts
		cameraSpeed = (keyStates[SDL_SCANCODE_LSHIFT]) ? 0.05f : 0.23f;
		//horizontalSlow = 0.7f; // step;
		if (keyStates[SDL_SCANCODE_SPACE]) {
			if (keyStates[SDL_SCANCODE_LCTRL])
				pos.y -= 21.0f;
			else {
				//acc.y = 2.0f; // head = 10.0
				acc.y = 2.5f;   // head = 50.0
				fr = 0;
				EventQueue::AddEvent(new AudioEvent_PlaySound("jump1.wav"));
			}
		}

		fr++;
		dir = look; dir.y = 0.0f; dir = glm::normalize(dir);
	}
	else {
		// if we're falling, then our downward acceleration constatntly increases, and our horizontal impulse is preserved
		cameraSpeed = 0.005f;
		acc.y -= g / step;
		fr = 0;
	}
	if (keyStates[SDL_SCANCODE_W] && !fall)
		acc += cameraSpeed * dir;
	if (keyStates[SDL_SCANCODE_S])
		acc -= cameraSpeed * dir;
	if (keyStates[SDL_SCANCODE_A])
		acc -= glm::normalize(glm::cross(dir, up)) * cameraSpeed;
	if (keyStates[SDL_SCANCODE_D])
		acc += glm::normalize(glm::cross(dir, up)) * cameraSpeed;
	
	pos += (acc * (1 / step ));
	if (acc.y <= -100.0f) acc.y = -100.0f;
	speed = glm::length(acc);
	if ((speed > 1.5f) && (!fall)) Step();
	if (fr >= fr_tr) {
		acc.x *= horizontalSlow; acc.z *= horizontalSlow; fr = fr_tr;
	}
	if (pos.y <= 0) SetPos(start, true);
}

/* * * * * * * * * * * * * * *
* Level editor
** * * * * * * * * * * * * * *
*/

static Platform temp = Platform(0, 0);
static std::forward_list<std::pair<bool, bool>> added;	// history of added plats & trigs;
static bool showPlayerInfo = false;

void Tooltip(const std::string& text) {
	ImGui::BeginTooltip();
	ImGui::PushTextWrapPos(450.0f);
	ImGui::TextUnformatted(text.c_str());
	ImGui::PopTextWrapPos();
	ImGui::EndTooltip();
}

void ShootGame::EditorMenu() {
	static bool addTrig = false, console = false;
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New level")) LoadLevel("players\\_blank");
			if (ImGui::MenuItem("Load level")) EventQueue::AddEvent(new ShootEvent(SHUTIN_CLOSE));
			ImGui::Separator();
			if (ImGui::MenuItem("Quit")) EventQueue::AddEvent(new GameEvent(GAME_STOP));
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit")) {
			if (ImGui::MenuItem("Player info", "", &showPlayerInfo));
			if (ImGui::MenuItem("Console", "", &console));
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	// Toolbar
	{
		static TextureGL& tex = TextureGL("textures\\editor-panel.png", false);
		ImGuiIO& io = ImGui::GetIO();
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 52, 20));
		ImGui::SetNextWindowSize(ImVec2(52, io.DisplaySize.y - 20));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f));
		ImGui::Begin("##tools", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar);
		
		ImVec2 sz = ImVec2(32, 32);

		ImGui::PushID("undo");
		if (ImGui::ImageButton((ImTextureID)tex.ID(), sz, ImVec2(), ImVec2(1, 0.25f), 2) && !added.empty()) {
			if (added.front().first) {
				allTriggers.pop_back();
			}
			if (added.front().second) {
				allPlats.pop_back();
			}
			MessageLog::Add("Undone!");
			added.pop_front();
		}
		ImGui::PopID();
		if (ImGui::IsItemHovered())
			Tooltip("Undo");

		ImGui::PushID("add");  
		if (ImGui::ImageButton((ImTextureID)tex.ID(), sz, ImVec2(0, 0.25f), ImVec2(1, 0.5f), 2)) addTrig = !addTrig; 
		ImGui::PopID();
		if (ImGui::IsItemHovered())
			Tooltip("Add trigger");
		ImGui::Separator();

		ImGui::PushID("move");
		if (ImGui::ImageButton((ImTextureID)tex.ID(), sz, ImVec2(0, 0.5f), ImVec2(1, 0.75f), 2)) ImGui::OpenPopup("pos");
		if (ImGui::BeginPopup("pos")) {
			auto x = camera.Pos();
			ImGui::Text("Position:");
			ImGui::DragFloat3("Start", glm::value_ptr(x), 0.5f, -10000, 10000);
			camera.SetPos(x, true);
			ImGui::EndPopup();
		}
		ImGui::PopID();
		if (ImGui::IsItemHovered())
			Tooltip("Move player to");

		ImGui::Separator();
		//ImGui::PushID("save");
		if (ImGui::ImageButton((ImTextureID)tex.ID(), sz, ImVec2(0, 0.75f), ImVec2(1, 1), 2)) ImGui::OpenPopup("savefile");
		//ImGui::PopID();
		if (ImGui::IsItemHovered())
			Tooltip("Save as");

		static float time = 20.0;
		static char name[128] = "players\\user.lvl";
		if (ImGui::BeginPopup("savefile")) {
			ImGui::Text("Save as:");
			ImGui::InputText("##edit", name, IM_ARRAYSIZE(name));
			ImGui::DragFloat3("Start", glm::value_ptr(start_export), 0.5f, -10000, 10000);
			ImGui::DragFloat("Time", &time, 0.5f, 0.0f, 10000.0f, "%.2f sec");
			if (ImGui::Button("OK")) {
				ImGui::CloseCurrentPopup();
				ExportLevel(name, time);
			}
			ImGui::EndPopup();
		}

		ImGui::End();
		ImGui::PopStyleColor();
	}
	if (addTrig) Editor();
	if (console) {
		ImGui::Begin("Console##executestr", &console, ImGuiWindowFlags_NoSavedSettings);
		static char inp[512];
		ImGui::InputText("", inp, IM_ARRAYSIZE(inp), ImGuiInputTextFlags_AllowTabInput);
		ImGui::SameLine();
		if (ImGui::Button("Go")) ImportObject(std::string(inp));
		ImGui::End();
	}
}

void ShootGame::Editor() {
	//static int x = 0, y = 0, w = 100, h = 100; static float z = 0;
	static bool isTrigger = false, isVisible = true;
	static glm::vec3 pos = glm::vec3(0.0f, 250.0f, 0.0f);
	static glm::ivec2 size = glm::ivec2(100, 100);
	static std::string saveStatus;
	
	ImGui::SetNextWindowPosCenter(ImGuiCond_Appearing);
	ImGui::Begin("Add trigger", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::DragFloat3("pos", glm::value_ptr(pos), 0.5f, -10000, 10000);
	ImGui::DragInt2("size", glm::value_ptr(size), 0.5f, -10000, 10000);
	temp.Move(pos, size);

	static std::ostringstream obj;
	static char type[32];
	static char parameters[512];
	ImGui::Checkbox("Is a trigger", &isTrigger);
	if (isTrigger) {
		ImGui::SameLine();
		ImGui::Checkbox("Is visible", &isVisible);
		ImGui::InputText("Type", type, IM_ARRAYSIZE(type), ImGuiInputTextFlags_CharsNoBlank);
		ImGui::Text("Parameters");
		ImGui::InputText("##params", parameters, IM_ARRAYSIZE(parameters), ImGuiInputTextFlags_AllowTabInput);
	}

	//static int added = 0; // so that we couldn't undo things we didn't add
		if (ImGui::Button("Add trigger")) {
		if (isTrigger) {
			obj = std::ostringstream();
			// TODO: SET TRIGGER height PARAM
			obj << "TRIG\t" << type << '\t' << temp.area.x << '\t' << temp.area.y << '\t' << temp.area.w << '\t' << temp.area.h << '\t' << temp.z << '\t' << 10.0 << '\t' << "not" << '\t' << parameters << std::endl;
			ImportObject(obj.str());
		}
		if ((!isTrigger) || (isVisible)) {
			allPlats.push_back(Platform((int)pos.x, (int)pos.z, size.x, size.y, pos.y));
			allPlats.back().UpdateMesh();
		}
		MessageLog::Add("Added!");
		added.push_front(std::pair<bool, bool>(isTrigger, (!isTrigger) || (isVisible)));
	}
	/*
	ImGui::Separator();
	// TODO: list of favorite places?
	if (ImGui::Button("Teleport"))
		camera.SetPos(glm::vec3(pos.x, pos.y + 0.1f, pos.z));
	// TODO: flyin: g = 0; fall = false;
	//ImGui::Checkbox("Free flight", &flyin);
	//if (ImGui::Button("Quit")) EventQueue::AddEvent(new ShootEvent(SHUTIN_CLOSE));
	*/
	ImGui::End();
}

bool ShootGame::ExportLevel(const char* path, float time) {
	std::ofstream out(path, std::ofstream::out); char t = '\t';
	out << "2\n";	// version name!
	out << "PLAYER\t" << start_export.x << t << start_export.y << t << start_export.z << std::endl;
	out << "TIME\t" << (uint32_t)(time * 1000) << std::endl;
	for (auto& x : allPlats)
		out << x.Save();
	out << std::endl;
	for (auto& x : allTriggers)
		out << x->Save();
	out << std::endl;
	for (auto& x : allPickups)
		out << x.Save();
	out.close();
	lastLevel = std::string(path);
	return true;
}

/*
* Render
*
*/

void ShootGame::DrawMenu() {
	static bool lvlerr = false, help = false, showLoad = achives.run_3;
	static char lvname[128] = "players\\level1.lvl";
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Once, ImVec2(0.5f, 0.5f));
	ImGui::Begin("ShootMenu", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar);
	ImGui::Text(u8"Нажмите кнопку для начала игры:");
	

	if (showLoad) {
		ImGui::InputText("##lvname", lvname, IM_ARRAYSIZE(lvname));
		if (ImGui::Button("Load level")) {
			help = false;
			if (LoadLevel(std::string(lvname))) {
				EventQueue::AddEvent(new GameEvent(GAME_TOGGLE_MOUSE, (void*)2)); mouseCrutch = true; inMainMenu = false; paused = false;
				added.clear();
				//lastLevel = std::string(lvname);
			}
			else lvlerr = true;
		}
	} else if (ImGui::Button(u8"Начать игру")) {
		help = false;
		if (LoadLevel(lastLevel)) {
			EventQueue::AddEvent(new GameEvent(GAME_TOGGLE_MOUSE, (void*)2)); mouseCrutch = true; inMainMenu = false; paused = false;
			added.clear();
			//lastLevel = std::string(lvname);
		}
		else lvlerr = true;
	}
	if (ImGui::Button(u8"Справка")) help = !help;
	if (ImGui::Button(u8"Выход"))
		EventQueue::AddEvent(new GameEvent(GAME_STOP));
	ImGui::End();
	if (lvlerr) {
		ImGui::OpenPopup("Error!##lvlerr"); lvlerr = false;
	}
	if (gameWon) {
		static bool clikkk = false;
		ImGui::SetNextWindowPosCenter();
		ImGui::Begin("tempbanner", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
		
		if (clikkk)
			ImGui::Text(u8"I don't know where I am...\n I don't know how long have I been here...\n  I don't know if I'll ever get out of here...\n\n         All I know is that i have to\n\n      ########    #       #   #       #\n      #       #   #       #   ##      #\n      #       #   #       #   # #     #\n      ########    #       #   #  ##   #\n      ###         #       #   #    #  #\n      #  ####      #      #   #     # #\n      #      #      #######   #      ##\n	  \n                a byzero game\n			coming          soon\n\n\n");
		else {
			ImGui::Text(u8"Поздравлем, вы прошли демо-версию!");
			ImGui::Text(u8"Лидеры:");
			shScores.Widget();
			ImGui::Text(u8"Нажмите OK чтобы продолжить.");
			if (ImGui::Button("OK")) { 
				clikkk = true; 
				EventQueue::AddEvent(new AudioEvent_StopMusic(15));
				EventQueue::AddEvent(new AudioEvent_PlaySound("skary.wav"));
			}
		}
		
		ImGui::End();
	}
	if (help) {
		ImGui::SetNextWindowPosCenter();
		ImGui::Begin("Help", &help, ImGuiWindowFlags_NoSavedSettings);
		ImGui::Text(u8"Управление:\n WASD ········ бег\n Shift+WASD ·· ходьба\n Space ······· прыжок\n Ctrl+Space ·· спырыгнуть вниз\n R ··········· назад к чекпойнту\n Shift+R ····· перезапуск\n \nЦель:\n Добраться до финиша до того, как закончится отмеряемое\n таймером время.\n ");
		ImGui::End();
	}
	if (ImGui::BeginPopupModal("Error!##lvlerr", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Unable to load level: an error has occured");
		ImGui::Text("Level name: %s", lvname);
		if (ImGui::Button("OK", ImVec2(120, 0)))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

void ShootGame::Render() {
	static ShaderGL bckSh = ShaderGL("default");
	static ShaderGL chrSh = ShaderGL("chrome");
	static MeshGL& gun = shMesh.Get("testg.obj");
	static MeshGL& flr = shMesh.Get("floor_1.obj");
	static MeshGL& pickup = shMesh.Get("pickup.obj");
	static TextureGL& fieldt = shTex.Get("gd_map.png");
	static TextureGL& viewt = shTex.Get("tgun_view.png");
	static TextureGL& cfluv = shTex.Get("cfl_uv.png");
	static TextureGL& picktex = shTex.Get("pickup.png");
	static TextureAGL frame = TextureAGL("textures\\frame.png");
	static TextureAGL framen = TextureAGL("textures\\frame_none.png");
	//static ModelAnimated testanim = ModelAnimated("test", pickup, picktex, glm::vec3(0.0f, glm::radians(1.0f), 0.0f));

	InternalEvents();
	
	buf->Use();

	if (inMainMenu) {
		DrawMenu();
		glClearColor(0x00, 0x00, 0x00, 0xff);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		buf->Default();
		return;
	}

	shThink.Think();
	gtimer.pause = paused;
	gtimer.Tick();

	glClearColor(0xFF, 0xF0, 0xE9, 0xff);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float newY = 0.0f;
	ImGui::Begin("debugPos", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);

	static int steps_per_frame = 10; 
	static float sf = (float)steps_per_frame;
	glm::vec3 coords;
	for (int i = 0; i < steps_per_frame; i++) {
		coords = camera.Pos();
		for (auto& trig : allPlats) {
			if (trig.IsAbove(coords) && (trig.Z() > newY)) {
				newY = trig.Z();
			}
		}

		if (!paused) 
			camera.Move(newY, sf); 
		if (paused && passFrame) {
			camera.Move(newY, sf);
			passFrame = false;
		}
		for (auto& trig : allTriggers) {
			trig->Check(camera);
		}
		for (auto& pick : allPickups)
			pick.Check(camera);
	}

	if (showPlayerInfo) {
		ImGui::Text("pos %f %f %f\nnewY %f", coords.x, coords.y, coords.z, newY);
		camera.Acc();
		ImGui::Separator();
	}
	if (!alwaysWin) gtimer.PrintSecs();
	ImGui::Text(u8"Ключей осталось: %i", PickupCounter::Left());
	MessageLog::PrintIm();
	//ImGui::Text("Fade %f", fadeIn);
	ImGui::End();

	//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	//if (!mouseCrutch) ImGui::ShowMetricsWindow();

	glm::mat4 model, modelOld;
	glm::mat4 view = camera.LookAt();
	static glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)FIXED_W / (float)FIXED_H, 3.1f, 5000.0f);
	static glm::mat4 viewmdproj = glm::perspective(glm::radians(90.0f), (float)FIXED_W / (float)FIXED_H, 1.1f, 5000.0f);

	chrSh.Use();
	cfluv.Use();
	glUniformMatrix4fv(chrSh.Uniform("model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(chrSh.Uniform("view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(chrSh.Uniform("projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniform1i(chrSh.Uniform("FrameTex"), 1);
	for (auto& trig : allPlats) {
		glUniform2fv(chrSh.Uniform("tex_scale"), 1, glm::value_ptr(trig.GetScaleVec()));
		fieldt.Use();
		frame.Use(1);
		model = trig.GetMatrix();
		glUniformMatrix4fv(chrSh.Uniform("model"), 1, GL_FALSE, glm::value_ptr(model));
		trig.Render();
		cfluv.Use();
		glUniformMatrix4fv(chrSh.Uniform("model"), 1, GL_FALSE, glm::value_ptr(modelOld));
		trig.RenderContour();
	}
	glUniform2fv(chrSh.Uniform("tex_scale"), 1, glm::value_ptr(glm::vec2(1.0f)));
	framen.Use(1);
	for (auto& dec : allDecors) {
		glUniformMatrix4fv(chrSh.Uniform("model"), 1, GL_FALSE, glm::value_ptr(dec.GetMatrix()));
		dec.Render();
	}

	bckSh.Use();
	for (auto& x : allRotatingModels)
		x.Update();
	//glUniformMatrix4fv(bckSh.Uniform("model"), 1, GL_FALSE, glm::value_ptr(testanim.MtxPos()));
	glUniformMatrix4fv(bckSh.Uniform("view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(bckSh.Uniform("projection"), 1, GL_FALSE, glm::value_ptr(projection));
	for (auto& pick : allPickups) {
		glUniformMatrix4fv(bckSh.Uniform("model"), 1, GL_FALSE, glm::value_ptr(pick.GetMdlMatrix()));
		pick.MdlRender();
	}


	if (paused) {
		EditorMenu();
		cfluv.Use();
		model = temp.GetMatrix();
		glUniformMatrix4fv(bckSh.Uniform("model"), 1, GL_FALSE, glm::value_ptr(model));
		temp.Render(GL_LINE_LOOP);
	}
	/* plat render was here */

	float speed = camera.GetSpeed();
	static float f = 0; if (speed > 0.0001f) f +=  0.005 * speed; else f = 0;
	float shift_x = (std::sinf(f) * 0.5) * speed / 4.0f;
	glUniformMatrix4fv(bckSh.Uniform("model"), 1, GL_FALSE, glm::value_ptr( glm::translate(modelOld, glm::vec3(shift_x, camera.GetPitch() / 90.0, 0.0f))));
	glUniformMatrix4fv(bckSh.Uniform("view"), 1, GL_FALSE, glm::value_ptr(modelOld));
	glUniformMatrix4fv(bckSh.Uniform("projection"), 1, GL_FALSE, glm::value_ptr(viewmdproj));
	viewt.Use();
	gun.Render();
	sky->Render(glm::mat4(glm::mat3(view)), projection);

	buf->Default();
}