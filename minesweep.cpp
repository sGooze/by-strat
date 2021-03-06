// minesweep.cpp : Defines the entry point for the console application.
// also some misc shit 
// TODO: cleanup

#include "common.hpp"
#include "mine_game.hpp"
#include "mine_gl.hpp"

// for timer resolution setup (timeBeginPeriod)
#include <mmsystem.h>

SDL_Window* glWindow = NULL;
SDL_GLContext glContext = NULL;
std::list<Event*> EventQueue::eventList;
MineGame* mineGame = NULL;


void EngineShutdown() {
	if (mineGame != NULL) {
		delete mineGame; 
		mineGame = NULL;
	}
	ImGui_ImplSdlGL3_Shutdown();
	SkyboxGL::ShutdownSkybox();
	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(glWindow);
	Mix_CloseAudio();
	Mix_Quit();
	SDL_Quit();

	timeEndPeriod(1);
}

bool EngineInit() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO) != 0)
		return false;
	std::atexit(EngineShutdown);

	timeBeginPeriod(1);

	// OpenGL init
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	glWindow = SDL_CreateWindow("byZero's fun family game pack vol. 5", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		FIXED_W, FIXED_H, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (glWindow == NULL) return false;
	//SDL_SetWindowBordered(glWindow, SDL_FALSE);
	glContext = SDL_GL_CreateContext(glWindow);
	if (glContext == NULL) return false;
	gladLoadGLLoader(SDL_GL_GetProcAddress);
	std::cout << "Vendor:   " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version:  " << glGetString(GL_VERSION) << std::endl;

	glViewport(0, 0, FIXED_W, FIXED_H);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	SkyboxGL::InitSkybox();

	if (SDL_GL_SetSwapInterval(-1) == -1) {
		std::cout << "Late swap vsync is unsupported, falling back to classic vsync\n";
		SDL_GL_SetSwapInterval(1);
	}

	if (!ImGui_ImplSdlGL3_Init(glWindow)) return false;
	if (Mix_Init(MIX_INIT_OGG | MIX_INIT_FLAC)) {
		printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
		return false;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) { 
		printf("SDL_mixer: mixer opening failed! SDL_mixer Error: %s\n", Mix_GetError()); 
		return false; 
	}
	ImGuiIO& io = ImGui::GetIO();
	//io.Fonts->AddFontDefault();
	io.Fonts->AddFontFromFileTTF("ProggyCleanCyr.ttf", 13, NULL, io.Fonts->GetGlyphRangesCyrillic());
	//io.Fonts->AddFontFromFileTTF("DroidSansMono.ttf", 16, NULL, io.Fonts->GetGlyphRangesCyrillic());
	return true;
}

bool EventQueue::PollEvent(EventType type, Event** returned) {
	static std::list<Event*>::iterator it = eventList.begin();
	for (it; it != eventList.end(); it++) {
		if ((*it)->type == type) {
			*returned = *it;
			it = eventList.erase(it);
			return true;
		}
	}
	it = eventList.begin();
	return false;
}

float frand() {
	static std::random_device seeder;
	static std::mt19937 gen = std::mt19937(seeder());
	static std::uniform_real_distribution<float> dist = std::uniform_real_distribution<float>();
	return dist(gen);
}

int totalEvents = 0;
int destroyedEvents = 0;

int main(int argc, char **argv)
{
	if (!EngineInit()) {
		std::cout << SDL_GetError() << std::endl;
		return -1;
	}
	// to ensure that game object is destroyed before all of the engine libs are shut down
	mineGame = new MineGame();
	while (!mineGame->GameFrame()) {	}
    return 0;
}




