#include "shader.h"

#include "GL/glew.h"

#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "vector"

#include "debug.h"
#include "util.h"

/**
 * --------------------------
 * Shader
 * --------------------------
 */
Shader::Shader(){
	GL_CALL(id = glCreateProgram());
}
Shader::~Shader(){
	GL_CALL(glDeleteProgram(id));
}
void Shader::bind(){
	GL_CALL(glUseProgram(id));
}
void Shader::unbind(){
	GL_CALL(glUseProgram(0));
}
void Shader::printError(char* action){
	GLint infolen;
	glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infolen);
	char* reason = new char[infolen];
	glGetProgramInfoLog(id, infolen, &infolen, reason);
	printf("Shader failed to %s, reason: %s!\n", action, reason);
	delete[] reason;
}
void Shader::attach(GLint s){
	GL_CALL(glAttachShader(id, s));
}
uint8_t Shader::link(){
	GL_CALL(glLinkProgram(id));
	GLint ok;
	glGetProgramiv(id, GL_LINK_STATUS, &ok);
	if(!ok) printError("link");
	return ok != 0;
}
uint8_t Shader::validate(){
	GL_CALL(glValidateProgram(id));
	GLint ok;
	glGetProgramiv(id, GL_VALIDATE_STATUS, &ok);
	if(!ok) printError("validate");
	return ok != 0;
}
GLint Shader::getUniformLocation(char* name){
	for(uint16_t i = 0; i < uniformCache.size(); i++){
		UniformCacheEntry e = uniformCache[i];
		if(strcmp(e.uniformName, name) == 0){
			e.uses++;
			return e.uniformID;
		}
	}
	UniformCacheEntry e;
	e.uniformName = name;
	GL_CALL(e.uniformID = glGetUniformLocation(id, name));
	uniformCache.push_back(e);
	return e.uniformID;
}
uint16_t Shader::cleanupUniformCache(uint16_t minUses){
	uint16_t removed = 0;
	std::vector<UniformCacheEntry> newShaderCache;
	for(uint16_t i = 0; i < uniformCache.size(); i++){
		UniformCacheEntry e = uniformCache[i];
		if(e.uses < minUses) removed++;
		else newShaderCache.push_back(e);
	}
	uniformCache = newShaderCache;
	return removed;
}
static Shader* Shader::loadShaderFromFile(char* fname) {
	FILE* fp = fopen(fname, "r");
	printf("Loading shaders from \"%s\"\n", fname);
	Shader* s = NULL;
	if(fp) {
		ShaderSource src(fp, UnknownShader);
		fclose(fp);
		Shader* s = new Shader();
		GLint cs = src.getShaderFromSource(FragmentShader);
		if(cs) s->attach(cs);
		cs = src.getShaderFromSource(VertexShader);
		if(cs) s->attach(cs);
		cs = src.getShaderFromSource(GeometryShader);
		if(cs) s->attach(cs);
		if(!s->link()) {
			printf("Failed to link %s!\n", fname);
			delete s; return NULL;
		} else if(!s->validate()) {
			printf("Failed to validate %s!\n", fname);
			delete s; return NULL;
		} else *ss = s;
		src.destroy();
		printf("Loaded shaders from \"%s\"\n", fname);
	} else {
		printf("Failed to load shaders from \"%s\"!\n", fname);
		return NULL;
	}
	return s;
}

//setUniform GLfloat
void Shader::setUniform4f(char* name, GLfloat x, GLfloat y, GLfloat z, GLfloat w){
	this->bind();
	GL_CALL(glUniform4f(getUniformLocation(name), x, y, z, w));
}
void Shader::setUniform3f(char* name, GLfloat x, GLfloat y, GLfloat z){
	this->bind();
	GL_CALL(glUniform3f(getUniformLocation(name), x, y, z));
}
void Shader::setUniform2f(char* name, GLfloat x, GLfloat y){
	this->bind();
	GL_CALL(glUniform2f(getUniformLocation(name), x, y));
}
void Shader::setUniform1f(char* name, GLfloat v){
	this->bind();
	GL_CALL(glUniform1f(getUniformLocation(name), v));
}

//setUniform GLdouble
void Shader::setUniform4d(char* name, GLdouble x, GLdouble y, GLdouble z, GLdouble w){
	this->bind();
	GL_CALL(glUniform4d(getUniformLocation(name), x, y, z, w));
}
void Shader::setUniform3d(char* name, GLdouble x, GLdouble y, GLdouble z){
	this->bind();
	GL_CALL(glUniform3d(getUniformLocation(name), x, y, z));
}
void Shader::setUniform2d(char* name, GLdouble x, GLdouble y){
	this->bind();
	GL_CALL(glUniform2d(getUniformLocation(name), x, y));
}
void Shader::setUniform1d(char* name, GLdouble v){
	this->bind();
	GL_CALL(glUniform1d(getUniformLocation(name), v));
}

//setUniform GLint
void Shader::setUniform4i(char* name, GLint x, GLint y, GLint z, GLint w){
	this->bind();
	GL_CALL(glUniform4i(getUniformLocation(name), x, y, z, w));
}
void Shader::setUniform3i(char* name, GLint x, GLint y, GLint z){
	this->bind();
	GL_CALL(glUniform3i(getUniformLocation(name), x, y, z));
}
void Shader::setUniform2i(char* name, GLint x, GLint y){
	this->bind();
	GL_CALL(glUniform2i(getUniformLocation(name), x, y));
}
void Shader::setUniform1i(char* name, GLint v){
	this->bind();
	GL_CALL(glUniform1i(getUniformLocation(name), v));
}

//setUniform GLuint
void Shader::setUniform4ui(char* name, GLuint x, GLuint y, GLuint z, GLuint w){
	this->bind();
	GL_CALL(glUniform4ui(getUniformLocation(name), x, y, z, w));
}
void Shader::setUniform3ui(char* name, GLuint x, GLuint y, GLuint z){
	this->bind();
	GL_CALL(glUniform3ui(getUniformLocation(name), x, y, z));
}
void Shader::setUniform2ui(char* name, GLuint x, GLuint y){
	this->bind();
	GL_CALL(glUniform2ui(getUniformLocation(name), x, y));
}
void Shader::setUniform1ui(char* name, GLuint v){
	this->bind();
	GL_CALL(glUniform1ui(getUniformLocation(name), v));
}

void Shader::setUniformMat4x4f(char* name, GLboolean transpose, GLfloat* dat){
	this->bind();
	GL_CALL(glUniformMatrix4fv(getUniformLocation(name), 1, transpose, dat));
}

/**
 * -------------------------
 * ShaderSource
 * -------------------------
 */
ShaderSource::ShaderSource(FILE* fp, ShaderType defaultType) : destroyed(false){
	sourceSize = fileSize(fp);
	fullSourceFile = new char[sourceSize];
	readFileInto(fp, fullSourceFile);
	parse(defaultType);
}
ShaderSource::ShaderSource(char* contents, uint64_t sourceSize, ShaderType defaultType) :
		sourceSize(sourceSize), destroyed(false){
	fullSourceFile = new char[sourceSize];
	memcpy(fullSourceFile, contents, sourceSize);
	parse(defaultType);
}
ShaderSource::~ShaderSource(){
	destroy();
}
void ShaderSource::printElements(){
	for(uint16_t i = 0; i < elements.size(); i++){
		ShaderSourceElement e = elements[i];
		char* type = ShaderType_tostr(e.type);
		printf("\e[1;33mShaderSourceElement type(%s):\e[0m\n", type);
		printf("\e[33m%.*s\e[0m\n", e.stop - e.start, e.start);
	}
}
GLint ShaderSource::getShaderFromSource(ShaderType type){
	for(uint16_t i = 0; i < elements.size(); i++){
		ShaderSourceElement e = elements[i];
		if(e.type == type) return e.compile();
	}
	return 0;
}
void ShaderSource::destroy(){
	if(destroyed)
		return;
	delete[] fullSourceFile;
	destroyed = true;
}
void ShaderSource::parse(ShaderType defaultType){
	char* startln = fullSourceFile;
	char* endln = fullSourceFile;
	char* endpos = fullSourceFile + sourceSize;
	ShaderType curType = defaultType;
	ShaderSourceElement curSourceElem;
	curSourceElem.type = curType;
	curSourceElem.start = startln;
	for(uint64_t i = 0; i < sourceSize; i++){
		char c = fullSourceFile[i];
		if(c == '\n' || c == '\r'){
			endln = fullSourceFile + (i + 1); //Removes the newline at the start but causes one at the beginning
			uint64_t len = endln - startln;
			len--; //Removes the newline at the end
			if(len > 7 && strncmp("#shader ", startln, 8) == 0){
				char* sident = startln + 8;
				uint32_t slen = len - 8;
				curType = ShaderSourceElement::typeFromString(sident, slen);
				curSourceElem.stop = startln;
				elements.push_back(curSourceElem);
				//printf("%.*s", curSourceElem.length(), curSourceElem.start);
				curSourceElem.type = curType;
				curSourceElem.start = endln;
			}
			startln = endln;
		}
	}
	curSourceElem.stop = endpos;
	elements.push_back(curSourceElem);
	//printf("%.*s", curSourceElem.length(), curSourceElem.start);
}
/**
 * --------------------------
 * ShaderSourceElement
 * --------------------------
 */
ShaderType ShaderSourceElement::typeFromString(char* str, uint32_t len){
	if(len >= 6 && strncmp("vertex", str, len) == 0)
		return VertexShader;
	if(len >= 8 && strncmp("fragment", str, len) == 0)
		return FragmentShader;
	if(len >= 8 && strncmp("geometry", str, len) == 0)
		return GeometryShader;
	return UnknownShader;
}

GLint ShaderSourceElement::compile(){
	GLint gl_type;
	switch(type){
		case VertexShader: gl_type = GL_VERTEX_SHADER; break;
		case FragmentShader: gl_type = GL_FRAGMENT_SHADER; break;
		case GeometryShader: gl_type = GL_GEOMETRY_SHADER; break;
		//default: return NULL;
	}
	GL_CALL(GLint id = glCreateShader(gl_type));
	GLint len = stop - start;
	GL_CALL(glShaderSource(id, 1, &start, &len));
	GL_CALL(glCompileShader(id));
	//Check if it actually worked
	int32_t success;
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);
	if(!success){
		GLsizei logLength = 0;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);
		char infoLog[logLength];
		glGetShaderInfoLog(id, logLength, &logLength, infoLog);
		char* shaderName = "Unknown type";
		if(type == VertexShader)
			shaderName = "Vertex shader";
		if(type == FragmentShader)
			shaderName = "Fragment shader";
		if(type == GeometryShader)
			shaderName = "Geometry shader";
		printf("Failed to compile shader %s:\n%.*s\nReason: %s\n", shaderName, len, start, infoLog);
		return 0;
	}
	return id;
}
/**
 * =================
 *    ShaderType
 * =================
 */
char* ShaderType_tostr(ShaderType st) {
	if(st == VertexShader) return "vertex";
	else if(st == FragmentShader) return "fragment";
	else if(st == GeometryShader) return "geometry";
	return "unknown";
}
