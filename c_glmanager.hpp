// c_glmanager.hpp
//	PURPOSE: Managers for GL-related resources

#pragma once

#include "mine_gl.hpp"

class MeshManager : public Manager<MeshGL> {
public:
	MeshManager() : Manager("models\\") {}
	using Manager::Load;
	void Load(const std::string& label, float* mesh_verticles, uint16_t total_verticles, bool unload_verts = true) {
		if (loaded.count(label) != 0) {
			std::cout << label << ": already precached\n";
			return;
		}
		loaded.emplace(std::piecewise_construct, std::forward_as_tuple(label), std::forward_as_tuple(mesh_verticles, total_verticles, unload_verts));
	}
};

class TextureManager : public Manager<TextureGL> {
public:
	TextureManager() : Manager("textures\\") {}
};