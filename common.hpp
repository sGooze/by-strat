#pragma once

#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  


//#include <vld.h>

#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <unordered_map>
#include <tuple>
#include <list>
#include <forward_list>

#include <fstream>
#include <sstream>

#include <SDL.h>
#include <SDL_mixer.h>

#include "mine_events.hpp"
#include "mine_audio.hpp"

#define FPS_MAX 60.0
#define FIXED_W 800
#define FIXED_H 600

float frand();
std::vector<std::string> split(const std::string& s, char delim);

char player_name[];