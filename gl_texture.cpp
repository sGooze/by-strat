// gl_texture.cpp

#include "mine_gl.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void TextureGL::GenTexture(uint8_t* data) {
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	GLenum scaling = (data == nullptr) ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, scaling);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, scaling);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	if (data != nullptr) glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureAGL::GenTexture(uint8_t* data) {
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	//glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

TextureGL::TextureGL(const std::string& path, bool flip) {
	int chans;
	stbi_set_flip_vertically_on_load(flip);
	uint8_t* data = stbi_load(path.c_str(), &w, &h, &chans, 0);
	if (!data) {
		std::cout << "Unable to load image from file " << path << std::endl;
		stbi_image_free(data);
		id = 0; return;
	}
	GenTexture(data);
	stbi_image_free(data);
}

TextureAGL::TextureAGL(const std::string& path, bool flip) {
	int chans;
	stbi_set_flip_vertically_on_load(flip);
	uint8_t* data = stbi_load(path.c_str(), &w, &h, &chans, 0);
	if (!data) {
		std::cout << "Unable to load transparent image from file " << path << std::endl;
		stbi_image_free(data);
		id = 0; return;
	}
	GenTexture(data);
	stbi_image_free(data);
}


TextureGL::TextureGL(int width, int height) : w(width), h(height) {
	GenTexture(nullptr);
}

TextureGL::~TextureGL() {
	glDeleteTextures(1, &id);
}

CubemapGL::CubemapGL(const std::string& file_prefix, const std::string& file_postfix) {
	static std::string f_pfx[] = { "rt", "lf", "up", "dn", "bk", "ft" };
	int chans, x, y;
	stbi_set_flip_vertically_on_load(false);

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
	for (int i = 0; i < 6; i++) {
		std::string sidename = "textures\\sky\\" + file_prefix + f_pfx[i] + file_postfix;
		//stbi_set_flip_vertically_on_load(((i > 1)&&(i < 3)) ? true : false);
		uint8_t* data = stbi_load(sidename.c_str(), &x, &y, &chans, 0);
		if (!data) {
			std::cout << "Unable to load image from file " << sidename << std::endl;
			stbi_image_free(data);
			/*glDeleteTextures(1, &id);
			id = 0; return;*/
			continue;
		}
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

CubemapGL::~CubemapGL() {
	// TODO: F I X. Thins shouldn't happen with normal resource precaching
	// i hate u and i hate meself even more
	glDeleteTextures(1, &id);
}

FramebufferGL::FramebufferGL(int16_t width, int16_t height) : w(width), h(height), tex(width, height) {
	glGenFramebuffers(1, &fbu);
	glBindFramebuffer(GL_FRAMEBUFFER, fbu);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex.ID(), 0);
	// .. renderbuffer
	glGenRenderbuffers(1, &rbf);
	glBindRenderbuffer(GL_RENDERBUFFER, rbf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbf);
	// .. check for errors
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
