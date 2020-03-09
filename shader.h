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

struct CompiledShader{
private:
	GLuint id;
public:
	CompiledShader(GLuint id);
	~CompiledShader();
	uint32_t getID(){ return id; }
};

struct ShaderSourceElement{
	char* start;
	char* stop;
	uint32_t id;
	ShaderType type;

	inline static ShaderType typeFromString(char* str) { return typeFromString(str, strlen(str)); };
	static ShaderType typeFromString(char* str, uint32_t len);
	inline uint32_t length(){ return stop - start; }
	CompiledShader compile();
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
	CompiledShader getShaderFromSource(ShaderType type);
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
	void attach(CompiledShader& s);
	void attachAll(uint8_t length, CompiledShader* s);
	inline uint8_t linkAndValidate() { return link() && validate(); };
	uint8_t link();
	uint8_t validate();
	uint16_t cleanupUniformCache(uint16_t minUses);
	template<typename T> void setUniform(char* name, T obj);
	void setUniform2f(char* name, float x, float y);
	void setUniform3f(char* name, float x, float y, float z);
	void setUniform4f(char* name, float x, float y, float z, float w);
	void setUniformMat4x4f(char* name, GLboolean transpose, GLfloat* dat);
};
