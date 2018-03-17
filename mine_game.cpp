// mine_game.cpp

#include "mine_game.hpp"
#include "mine_gl.hpp"

// 3D background resources

TrophyController achives = TrophyController();

FramebufferGL* buf = nullptr;
MeshGL* plMesh = nullptr;

static MeshGL* sceneMesh = nullptr;
static ShaderGL bckSh;
static CameraGL sceneCam = CameraGL(glm::vec3(4.8, -4.8, 2.6), glm::vec3(0.68, -1.12, 1.5));
static TextureGL* sceneTex = nullptr;
static MeshGL* credMesh = nullptr;
static TextureGL* credTex = nullptr;

static void DrawLogo();

MineGame::MineGame() {
	buf = new FramebufferGL(FIXED_W, FIXED_H);
	plMesh = new MeshGL(plane_verts, 6, false);

	bckSh.SetNewPaths("shaders\\default.vert", "shaders\\default.frag");
	sceneMesh = new MeshGL("models\\scenetest.obj");
	sceneTex = new TextureGL("textures\\scenetest.png");
	/*sceneMesh = new MeshGL("models\\templo.obj");
	sceneTex = new TextureGL("textures\\lm.png");*/
	
	gSounds.Load("gamestart.wav");
	gSounds.Load("bomb_succ.wav");
	gSounds.Load("bomb_fail.wav");
	gSounds.Load("bomb_mark.wav");
	gSounds.Load("bomb_unmark.wav");
	gSounds.Load("gamestart.wav");
	gMusic.Load("tensepph2.flac");
	gMusic.Load("tempwin1.ogg");
	gMusic.Load("menu1_fade.flac");
	current = &mainMenu;
	gMusic.Volume(100);

	achives.LoadData();

	// intro
#ifndef _DEBUG
	int track = (int)(frand() * 10.0f) + 1;
	std::string trackname = "intro\\intro" + std::to_string(track) + ".wav";
	gMusic.Load(trackname);
	DrawLogo();
	gMusic.Play(trackname, 0, 0, 1);
	int delay = (track == 4) ? 10000 : (track == 9) ? 6000 : (track == 10) ? 5000 : 4000;
	SDL_Delay(delay);

	fadeIn = 0.0f;
	gThinks.Add(new ThinkerAdd<float>(fadeIn, 0.005f, 1.0f, 20));
	gMusic.Play("menu1_fade.flac");
#endif
	//EventQueue::AddEvent(new GameChangeEvent(G_SHUT));
}

static void DrawLogo() {
	ShaderGL effectSh("postproc");
	TextureGL logo = TextureGL("textures\\logo.png");
	// 0.78125f 0.5859375f 0.21875f
	float logo_verts[]{
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f,  0.0f, 0.0f, 0.21875f, 0.5859375f,0.0f, 1.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.21875f, 0.0f,0.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,0.0f, 1.0f, 0.0f,
		1.0f,  0.0f, 0.0f, 1.0f, 0.5859375f,0.0f, 1.0f, 0.0f,
		0.0f,  0.0f, 0.0f, 0.21875f, 0.5859375f,0.0f, 1.0f, 0.0f,
	};
	auto logoMesh = MeshGL(logo_verts, 6, false);
	glClearColor(0x00, 0x00, 0x00, 0xff);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	effectSh.Use();
	logo.Use();
	glUniform1i(effectSh.Uniform("screenTexture"), 0);
	glUniform1f(effectSh.Uniform("fade"), fadeIn);
	glDisable(GL_DEPTH_TEST);
	logoMesh.Render();
	glEnable(GL_DEPTH_TEST);
	SDL_GL_SwapWindow(glWindow);
}

static float x = 6.210f, y = 1.280f, z = 6.540f, a = 0.0f, b = 0.0f, c = 0.0f;

static float randFloatForCam() {
	static std::mt19937 rand(SDL_GetTicks());
	static auto dist = std::uniform_real_distribution<float>(-10.0f, 10.0f);
	return dist(rand);
}

static void DebugWin() {
	ImGui::Begin("Place camera", nullptr, ImGuiWindowFlags_NoSavedSettings);
	ImGui::DragFloat("x", &x, 0.01f, -10.0f, 10.0f);
	ImGui::DragFloat("y", &y, 0.01f, -10.0f, 10.0f);
	ImGui::DragFloat("z", &z, 0.01f, -10.0f, 10.0f);
	ImGui::DragFloat("a", &a, 0.01f, -10.0f, 10.0f);
	ImGui::DragFloat("b", &b, 0.01f, -10.0f, 10.0f);
	ImGui::DragFloat("c", &c, 0.01f, -10.0f, 10.0f);
	ImGui::End();
}

#define SKY_COUNT 3

void MineGame::StartGame(GameChangeEvent* evg) {
	// called when a new game is started
	std::vector<int>* v;
	int sky;
	fadeIn = 1.0f;
	switch (evg->label) {
	case G_MINESWEEP:
		v = (std::vector<int>*)evg->arguments;
		mineField = new MineField((*v)[0], (*v)[1], (*v)[2], (*v)[3] == 1);
		delete v;
		sky = (int)(frand() * 10.0f) % SKY_COUNT;
		std::cout << "SKY: " << sky << std::endl;
		gameField.mineSky = new SkyboxGL("sky" + std::to_string(sky), ".png");
		gameField.field = mineField;
		fadeIn = 0.0f;
		gThinks.Add(new ThinkerAdd<float>(fadeIn, 0.01f, 1.0f, 20));
		gMusic.Play("tensepph2.flac", 1500, 1000);
		current = &gameField;
		break;
	case G_SNEKE:
		gMusic.FadeOut(1000);
		if (snekInstance == nullptr)
			snekInstance = new SnekGame();
		current = snekInstance;
		break;
	case G_SHUT:
		if (shut == nullptr)
			shut = new ShootGame();
		current = shut;
		gMusic.FadeOut(1000);
		//SDL_SetRelativeMouseMode(SDL_TRUE);
		//mouseCap = true;
		break;
	case G_CREDS:
		if (crdsScr == nullptr) {
			crdsScr = new CreditsScreen(&bckSh);
			gMusic.Load("prph_saw.flac");
		}
		gMusic.Play("prph_saw.flac", 0, 0, 1);
		current = crdsScr;
		break;
	case G_NONE:
		/*if ((current != nullptr) && (current != &mainMenu))
			delete current;*/
		current = &mainMenu;
	}
	std::cout << "Game started!";
}

void MineGame::EndGame() {
	if (current == &gameField) {
		delete mineField;
		gameField.field = nullptr;
	}
	//else if ((current != &mainMenu)&&(current != snekInstance)) delete current;
	gSounds.Halt();
	gMusic.Play("menu1_fade.flac", 4000, 2000, -1, 10.0);
	current = &mainMenu;
	fadeIn = 1.0f;
	x = randFloatForCam();
	y = (randFloatForCam() > 0.0f) ? 4.0f : 1.280f;
	z = randFloatForCam();
	mouseCap = false; SDL_SetRelativeMouseMode(SDL_FALSE);
	std::cout << "Game ended\n";
}

/* PollEvents
/	- polls all events from SDL pipeline and processes them
*/
void MineGame::PollEvents() {
	static SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (!mouseCap) ImGui_ImplSdlGL3_ProcessEvent(&event);
		if (current->acceptInput)
			current->ProcessEvent(event);
		switch (event.type) {
		case SDL_KEYUP:
			if (event.key.keysym.scancode == SDL_SCANCODE_F10)
				quit = true; 
			break;
		case SDL_QUIT:
			quit = true; break;
		}
	}
	GameEvent *evg;
	while (EventQueue::PollEvent(EVENT_GAME, (Event**)&evg)) {
		switch (evg->purpose) {
		case GAME_START:
			StartGame((GameChangeEvent*)evg);
			break;
		case GAME_STOP:
			EndGame();
			break;
		case GAME_LOST:
			gMusic.FadeOut(500);
			gThinks.Add(new ThinkerSub<float>(fadeIn, 0.015f, 0.0f, 20));
			mineField->OpenAllMines();
			gameField.game_was_over = true;
			gameField.game_was_won = false;
			break;
		case GAME_WON:
			gMusic.Play("tempwin1.ogg", 0, 0, 1);
			gThinks.Add(new ThinkerAdd<float>(fadeIn, 0.3f, 20.0f, 20));
			// TODO: Custom player name
			mainMenu.scores.Add(player_name, (int32_t)(evg->arguments));
			mineField->OpenAllMines();
			gameField.game_was_over = gameField.game_was_won = true;
			break;
		case GAME_TOGGLE_MOUSE:
			mouseCap = (evg->arguments == nullptr) ? !mouseCap : (bool)((uint16_t)evg->arguments - 1);
			SDL_SetRelativeMouseMode((SDL_bool)mouseCap);
			break;
		case GAME_EXIT:
			quit = true; break;
		}
		delete evg;
	}
}

static double frameStart, frameLength = 1000.0 / FPS_MAX;
static int16_t sleep;

bool MineGame::GameFrame() {
	frameStart = SDL_GetTicks();
	static double frameLast = 0;

	PollEvents();
	PollAudio();
	gThinks.Think();
	gMusic.Think();

	// rendering: render gamefield if game is active
	if (frameStart - frameLast >= frameLength) {
		RenderFrame();
		frameLast = frameStart;
	}
	//sleep = frameStart + frameLength - SDL_GetTicks();
	//if (sleep > 0) SDL_Delay(sleep);
	//if (sleep > 0) SDL_Delay(10);
	//SDL_Delay(100);
	return quit;
}


void MineGame::RenderBackground() {
	/*if (y >= 4.0f) {
		float radius = 5.0f;
		x = sin(SDL_GetTicks() / 4000.0) * radius;
		z = cos(SDL_GetTicks() / 4000.0) * radius;
	}*/

	buf->Use();

	glClearColor(0xb2, 0xb2, 0xb2, 0xff);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	static glm::mat4 model;
	glm::mat4 view = glm::lookAt(glm::vec3(x, y, z), glm::vec3(a, b, c), glm::vec3(0.0, 1.0, 0.0));
	static glm::mat4 projection = glm::perspective(glm::radians(50.0f), (float)FIXED_W / (float)FIXED_H, 0.1f, 100.0f);

	bckSh.Use();
	// send matrices
	glUniformMatrix4fv(bckSh.Uniform("model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(bckSh.Uniform("view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(bckSh.Uniform("projection"), 1, GL_FALSE, glm::value_ptr(projection));

	sceneTex->Use();
	glUniform1i(bckSh.Uniform("PlaneTex"), 0);
	sceneMesh->Render();

	buf->Default();
}

void MineGame::RenderFrame() {
	static ShaderGL effectSh("postproc");
	glClearColor(0x00, 0x00, 0x00, 0xff);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ImGui_ImplSdlGL3_NewFrame(glWindow);
	
	if (current == &mainMenu) RenderBackground();
	current->Render();
	// Apply postprocessing
	// TODO: Make postprocess shader overridable, with automatical reset after returning to menu
	effectSh.Use();
	buf->GetTexture().Use();
	glUniform1i(effectSh.Uniform("screenTexture"), 0);
	glUniform1f(effectSh.Uniform("fade"), fadeIn);
	glDisable(GL_DEPTH_TEST);
	plMesh->Render();
	glEnable(GL_DEPTH_TEST);
	
	//DebugWin();

	ImGui::Render();
	SDL_GL_SwapWindow(glWindow);
}