/*
 * Debugging.cpp
 *
 *  Created on: Nov 8, 2018
 *      Author: jeremiah
 */

#include "debug.h"

void Debug::glClearErrors(){
	while(glGetError() != GL_NO_ERROR);
};
uint32_t Debug::glErrorCallback(char* file, uint32_t line, char* cmd){
	char buf[128];
	uint32_t cnt = 0;
	while(true){
		GLint err = glGetError();
		if(err == GLEW_NO_ERROR)
			break;
		printf("OpenGL error %s, at %s:%i - for %s\n", Debug::errorCodeToString(err,  buf), file, line, cmd);
		cnt++;
	}
	return cnt;
};
char* Debug::errorCodeToString(GLint c, char* buf){
	if(c == GL_NO_ERROR)
		return "no error";
	if(c == GL_INVALID_OPERATION)
		return "invalid operation";
	if(c == GL_INVALID_INDEX)
		return "invalid index";
	if(c == GL_INVALID_VALUE)
		return "invalid value";
	if(c == GL_INVALID_ENUM)
		return "invalid enum";
	sprintf(buf, "code %i\x00", c);
	return buf;
}

