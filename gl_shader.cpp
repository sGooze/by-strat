// gl_shader.cpp

#include "mine_gl.hpp"

ShaderGL::ShaderGL(const std::string& name) :
	vtxPath("shaders\\" + name + ".vert"), fragPath("shaders\\" + name + ".frag") {
	Reload();
}

ShaderGL::ShaderGL(std::string& path_vtx, std::string& path_frag) 
	: vtxPath(path_vtx), fragPath(path_frag) {
	Reload();
}

std::string ShaderGL::LoadCodeFromFile(std::string& path) {
	std::ifstream codeFile;
	std::stringstream codeStream;

	codeFile.exceptions(std::ifstream::badbit);
	try
	{
		// Open files
		codeFile.open(path);
		// Read file's buffer contents into streams
		codeStream << codeFile.rdbuf();
		// close file handlers
		codeFile.close();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR: Unable to read file \"" << path << "\"\n";
	}
	// Convert stream into string
	return codeStream.str();
}

void ShaderGL::Reload() {
	std::string vertCode = LoadCodeFromFile(vtxPath);
	std::string fragCode = LoadCodeFromFile(fragPath);
	if ((vertCode.length() > 0) && (fragCode.length() > 0))
		if (Rebuild(vertCode.c_str(), fragCode.c_str()))
			return;
	std::cout << "Unable to rebuild shader program\n";
}

bool ShaderGL::Rebuild(const char* vtx, const char* frag) {
	int compSuccess;
	char compLog[512];
	int vtxShader, fragShader;

	vtxShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vtxShader, 1, &vtx, NULL);
	glCompileShader(vtxShader);
	glGetShaderiv(vtxShader, GL_COMPILE_STATUS, &compSuccess);
	if (!compSuccess) {
		glGetShaderInfoLog(vtxShader, 512, NULL, compLog);
		std::cout << "VERTEX SHADER COMPILATION FAILED:\n" << compLog << std::endl;
		return false;
	}

	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &frag, NULL);
	glCompileShader(fragShader);
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compSuccess);
	if (!compSuccess) {
		glGetShaderInfoLog(fragShader, 512, NULL, compLog);
		std::cout << "FRAGMENT SHADER COMPILATION FAILED:\n" << compLog << std::endl;
		return false;
	}

	GLuint shdProgram = glCreateProgram();
	glAttachShader(shdProgram, vtxShader);
	glAttachShader(shdProgram, fragShader);
	glLinkProgram(shdProgram);
	glGetProgramiv(shdProgram, GL_LINK_STATUS, &compSuccess);
	if (!compSuccess) {
		glGetProgramInfoLog(shdProgram, 512, NULL, compLog);
		std::cout << "PROGRAM LINKING FAILED:\n" << compLog << std::endl;
		return false;
	}
	glDeleteShader(vtxShader); glDeleteShader(fragShader);
	if (program != -1) glDeleteProgram(program);
	program = shdProgram;

	return true;
}

void ShaderGL::SetNewPaths(const std::string& path_vtx, const std::string& path_frag) {
	vtxPath = (path_vtx.length() > 0) ? path_vtx : vtxPath;
	fragPath = (path_frag.length() > 0) ? path_frag : fragPath;
	Reload();
}

ShaderGL::~ShaderGL() {
	if (program != -1) glDeleteProgram(program);
}