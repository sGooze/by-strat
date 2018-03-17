// gl_threed.cpp
// for all objects related to 3d model rendering

#include "mine_gl.hpp"

float plane_verts[]{
	1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	-1.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	-1.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
};

GLuint plane_inds[]{
	0, 1, 2,
	3, 4, 5
};

float SkyboxGL::skyboxVerts[] = {
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f,  1.0f
};

glm::vec3 CameraGL::vnul = glm::vec3(0, 0, 0);

ShaderGL* SkyboxGL::skyshade = nullptr;
GLuint SkyboxGL::vbo = 0, SkyboxGL::vao = 0;

float fadeIn = 1.0f;

void MeshGL::GenBuffers() {
	// TODO: Memcopy verticles from original source to allow deletion from the destructor
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*total_verts*8, verticles, GL_STATIC_DRAW);
	// vertex coords as parameter 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coords as parameter 1
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// normals as parameter 2 (unused in most shaders)
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

MeshGL::MeshGL(float* mesh_verticles, uint16_t total_verticles, bool unload_verts)
	: verticles(mesh_verticles), 
	  total_verts(total_verticles), unload(unload_verts){
	GenBuffers();
}

void MeshGL::Render(GLenum mode) {
	glBindVertexArray(vao);
	glDrawArrays(mode, 0, total_verts);
	//glDrawElements(GL_TRIANGLES, sizeof(GLint)*total_inds, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

#define _OBJ_LOADALL -2

/// obj_offset defines, which object to load. Use _OBJ_LOADALL to load all objects' data into one mesh
MeshGL::MeshGL(const std::string& path, int obj_offset) {
	LoadOBJ(path, obj_offset);
	GenBuffers();
}

// my good friend std::vector<string> split
// thx to https://stackoverflow.com/a/236803
template<typename Out>
void split(const std::string& s, char delim, Out result) {
	// TODO: deal with multiple occurences of the same delimiter
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim))
		*(result++) = item;
}

std::vector<std::string> split(const std::string& s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, std::back_inserter(elems));
	return elems;
}

struct face_pt {
	glm::vec3* vert = nullptr;
	glm::vec3* norm = nullptr;
	glm::vec2* tex  = nullptr;
	//face(glm::vec3* v, glm::vec3* n, glm::vec2* t = nullptr) : vert(v), norm(n), tex(t) {}
};


void MeshGL::LoadOBJ(const std::string& path, int obj_offset) {
	static glm::vec2 notex = glm::vec2(0.0f, 0.0f);
	std::ifstream mdlFile;

	mdlFile.open(path);
	if (!mdlFile.good()) {
		std::cout << path << ": unable to open file\n"; return;
	}

	int obj = -1;
	bool skip = true;
	std::string buf;
	std::vector<glm::vec3> verts;  // vertex coordinates
	std::vector<glm::vec2> texts;	// texture coordinates
	std::vector<glm::vec3> norms;  // normals
	std::list<face_pt> faces;	// face descriptions

	while (mdlFile.good()) {
		std::getline(mdlFile, buf);
		std::vector<std::string> tk, kt;
		glm::vec3 temp; face_pt femp;
		switch (buf[0]) {
		case 'o':
			skip = (obj_offset == _OBJ_LOADALL) ? true : (++obj != obj_offset);
			if (!skip) std::cout << buf << std::endl;
			break;
		case 'v':
			if (skip) break;
			tk = split(buf, ' ');
			temp.x = std::stof(tk[1]);
			temp.y = std::stof(tk[2]);
			switch (buf[1]) {
			case ' ':
				temp.z = std::stof(tk[3]);
				verts.push_back(temp);
				break;
			case 't':
				texts.push_back(glm::vec2(temp.x, temp.y));
				break;
			case 'n':
				temp.z = std::stof(tk[3]);
				norms.push_back(temp);
				break;
			}
			break;
		case 'f':
			if (skip) break;
			tk = split(buf, ' ');
			for (int i = 1; i < 4; i++) {
				kt = split(tk[i], '/');
				femp.vert = &verts[std::stoi(kt[0]) - 1];
				femp.norm = &norms[std::stoi(kt[2]) - 1];
				femp.tex = (kt[1].length() > 0) ? &texts[std::stoi(kt[1]) - 1] : &notex;
				faces.push_back(femp);
			}
			break;
		default:
			break;
		}
	}
	// TODO: Replace with a vector?
	int i = 0;
	total_verts = faces.size();
	verticles = new float[total_verts * 8];	// EIGHT vert params for each verticle
	for (face_pt x : faces) {
		verticles[i + 0] = x.vert->x;
		verticles[i + 1] = x.vert->y;
		verticles[i + 2] = x.vert->z;
		verticles[i + 3] = x.tex->x;
		verticles[i + 4] = x.tex->y;
		verticles[i + 5] = x.norm->x;
		verticles[i + 6] = x.norm->y;
		verticles[i + 7] = x.norm->z;
		i += 8;
		/*tvert.push_back(x.vert->x);
		tvert.push_back(x.vert->y);
		tvert.push_back(x.vert->z);
		tvert.push_back(x.tex->x);
		tvert.push_back(x.tex->y);*/
	}
	//verticles = tvert.data();
	//total_verts = tvert.size() / 5;
}

SkyboxGL::SkyboxGL(const std::string& skytex_prefix, const std::string& skytex_postfix)
	: texture(skytex_prefix, skytex_postfix) {}

void SkyboxGL::InitSkybox() {
	skyshade = new ShaderGL("skybox");
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVerts), skyboxVerts, GL_STATIC_DRAW);
	// vertex coords as parameter 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void SkyboxGL::ShutdownSkybox() {
	delete skyshade;
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

void SkyboxGL::Render(glm::mat4& view, glm::mat4& project) {
	//glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);
	skyshade->Use();
	glUniformMatrix4fv(skyshade->Uniform("view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(skyshade->Uniform("project"), 1, GL_FALSE, glm::value_ptr(project));
	texture.Use(0);
	glUniform1i(skyshade->Uniform("skybox"), 0);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
	//glDepthMask(GL_TRUE);
}