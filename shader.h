#pragma once

#include "GL/glew.h"

#include "stdint.h"
#include "stdio.h"
#include "string.h"

#include "vector"

enum ShaderType{
	UnknownShader, VertexShader, FragmentShader, GeometryShader
};
char* ShaderType_tostr(ShaderType st);

struct ShaderSourceElement{
	char* start;
	char* stop;
	uint32_t id;
	ShaderType type;

	inline static ShaderType typeFromString(char* str) { return typeFromString(str, strlen(str)); };
	static ShaderType typeFromString(char* str, uint32_t len);
	inline uint32_t length(){ return stop - start; }
	GLint compile();
};
struct UniformCacheEntry{
	char* uniformName;
	GLint uniformID;
	uint32_t uses;
};

class ShaderSource{
private:
	char* fullSourceFile;
	uint64_t sourceSize;
	uint8_t destroyed;
	std::vector<ShaderSourceElement> elements;

	void parse(ShaderType defaultType);
public:
	ShaderSource(FILE* dat, ShaderType defaultType);
	ShaderSource(char* fullSource, uint64_t contentLength, ShaderType defaultType);
	~ShaderSource();
	void printElements();
	void destroy();
	GLint getShaderFromSource(ShaderType type);
};
class Shader{
private:
	uint32_t id;
	std::vector<UniformCacheEntry> uniformCache;
	void printError(char* action);
	GLint getUniformLocation(char* name);
public:
	Shader();
	~Shader();
	void bind();
	void unbind();
	void attach(GLint id);
	inline uint8_t linkAndValidate() { return link() && validate(); };
	uint8_t link();
	uint8_t validate();
	uint16_t cleanupUniformCache(uint16_t minUses);
	
	void setUniform1f(char* name, GLfloat x);
	void setUniform2f(char* name, GLfloat x, GLfloat y);
	void setUniform3f(char* name, GLfloat x, GLfloat y, GLfloat z);
	void setUniform4f(char* name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	void setUniform1d(char* name, GLdouble x);
	void setUniform2d(char* name, GLdouble x, GLdouble y);
	void setUniform3d(char* name, GLdouble x, GLdouble y, GLdouble z);
	void setUniform4d(char* name, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	void setUniform1i(char* name, GLint x);
	void setUniform2i(char* name, GLint x, GLint y);
	void setUniform3i(char* name, GLint x, GLint y, GLint z);
	void setUniform4i(char* name, GLint x, GLint y, GLint z, GLint w);
	void setUniform1ui(char* name, GLuint x);
	void setUniform2ui(char* name, GLuint x, GLuint y);
	void setUniform3ui(char* name, GLuint x, GLuint y, GLuint z);
	void setUniform4ui(char* name, GLuint x, GLuint y, GLuint z, GLuint w);
	
	void setUniformMat4x4f(char* name, GLboolean transpose, GLfloat* dat);
};
