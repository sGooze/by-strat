// mine_gl.hpp
// PURPOSE: stuff used for graphics rendering

#pragma once

#include "common.hpp"
#include <glad\glad.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "imgui\imgui.h"
#include "imgui_impl_sdl_gl3.h"

extern SDL_Window* glWindow;
extern SDL_GLContext glContext;

class ShaderGL {
private:
	GLint program = -1;
	std::string vtxPath;
	std::string fragPath;

	bool Rebuild(const char* vtx, const char* frag);
	std::string LoadCodeFromFile(std::string& path);
public:
	ShaderGL() {};
	ShaderGL(const std::string& name);
	ShaderGL(std::string& path_vtx, std::string& path_frag);
	void SetNewPaths(const std::string& path_vtx, const std::string& path_frag);
	void Reload();
	void Use() { glUseProgram(program); }
	GLint Uniform(const char* name) { return glGetUniformLocation(program, name); }
	GLint Uniform(const std::string& name) { return glGetUniformLocation(program, name.c_str()); }
	~ShaderGL();
};

// postponed loading could be implemented for postponed precaching...
class TextureGL {
protected:
	GLuint id = 0;
	int w, h;
	void GenTexture(uint8_t* data);
	TextureGL() {};
public:
	TextureGL(const std::string& file_path, bool flip = true);
	TextureGL(int width, int height);
	GLuint ID() { return id; }
	~TextureGL();
	void Use(GLuint tunit = 0){
		glActiveTexture(GL_TEXTURE0 + tunit);
		glBindTexture(GL_TEXTURE_2D, id); 
		glActiveTexture(GL_TEXTURE0);
	}
};

// Transparent texture: used for various frames around platforms
class TextureAGL : public TextureGL {
	void GenTexture(uint8_t* data);
public:
	TextureAGL(const std::string& file_path, bool flip = true);
};

class CubemapGL {
private:
	GLuint id = 0;
public:
	CubemapGL() {}
	CubemapGL(const std::string& file_prefix, const std::string& file_postfix);
	~CubemapGL();
	void Use(GLuint tunit = 0){
		glActiveTexture(GL_TEXTURE0 + tunit);
		glBindTexture(GL_TEXTURE_CUBE_MAP, id);
		glActiveTexture(GL_TEXTURE0);
	}
};

class FramebufferGL {
private:
	GLuint fbu, rbf;
	TextureGL tex;	// make a pointer to allow external texture binding?
	int16_t w, h;
public:
	FramebufferGL(int16_t width, int16_t height);
	void Use() { glViewport(0, 0, w, h); glBindFramebuffer(GL_FRAMEBUFFER, fbu); }
	void Default() { glViewport(0, 0, FIXED_W, FIXED_H); glBindFramebuffer(GL_FRAMEBUFFER, 0); }
	TextureGL& GetTexture() { return tex; }
	~FramebufferGL() { glDeleteBuffers(1, &rbf); glDeleteBuffers(1, &fbu); Default(); }
};

class CameraGL {
private:
	static glm::vec3 vnul;
	glm::vec3 pos;
	glm::vec3 targ;
	glm::vec3 up;
public:
	CameraGL(glm::vec3 position = vnul, glm::vec3 target = vnul, glm::vec3 upward = glm::vec3(0.0f, 1.0f, 0.0f)) {};
	glm::mat4 LookAt() { return glm::lookAt(pos, targ, up); }
};

class camPos {
	float p[6];
public:
	camPos(float cx, float cy, float cz, float ca, float cb, float cc) : p{ cx, cy, cz, ca, cb, cc } {}
	float& x() { return p[0]; }
	float& y() { return p[1]; }
	float& z() { return p[2]; }
	float& a() { return p[3]; }
	float& b() { return p[4]; }
	float& c() { return p[5]; }
	glm::mat4 lookAt() { return glm::lookAt(glm::vec3(p[0], p[1], p[2]), glm::vec3(p[3], p[4], p[5]), glm::vec3(0.0, 1.0, 0.0)); }
};


/* Standalone skybox class, which draws a skycube with a specified texture */
class SkyboxGL {
private:
	CubemapGL texture;
	//static ShaderGL* skyshade;
	static float skyboxVerts[];
	static GLuint vbo, vao;
public:
	static ShaderGL* skyshade;
	static void InitSkybox();
	static void ShutdownSkybox();
	SkyboxGL() {}
	SkyboxGL(const std::string& skytex_prefix, const std::string& skytex_postfix);
	void Render(glm::mat4& view, glm::mat4& project);
};

class MeshGL {
private:
	GLuint vbo, ebo, vao;
	float* verticles = NULL;
	std::vector<float> tvert;
	uint16_t total_verts;
	bool unload;
	void GenBuffers();
public:
	MeshGL() {}
	MeshGL(float* mesh_verticles, uint16_t total_verticles, bool unload_verts = true);
	MeshGL(const std::string& filePath, int obj_offset = 0);
	void LoadOBJ(const std::string& path, int obj_offset = 0);
	void Render(GLenum mode = GL_TRIANGLES);
	void Export() {
		std::ofstream out = std::ofstream("model2.txt");
		out << "float plat[] = {\n\t";
		for (int i = 0; i < total_verts * 8; i++) {
			out << verticles[i] << ", ";
			//if ((i > 0) && (i % 5 == 0)) out << "\n\t";
		}
		out << "\n\t};\n";
		out.close();
	}
	GLuint VAO() { return vao; }
	~MeshGL() {
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ebo);
		glDeleteVertexArrays(1, &vao);
		if (unload) delete[] verticles;
	}
};

float plane_verts[];
GLuint plane_inds[];
extern float fadeIn;
extern MeshGL* plMesh;
extern FramebufferGL *buf;