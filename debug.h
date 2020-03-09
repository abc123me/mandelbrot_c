#ifndef _DEBUG_H
#define _DEBUG_H

#include "GL/glew.h"
#include "stdio.h"

namespace Debug {
	void glClearErrors();
	uint32_t glErrorCallback(char* file, uint32_t line, char* cmd);
	char* errorCodeToString(GLint code, char* buf);
};

#define GL_CALL_CHECK(cmd, on_err) Debug::glClearErrors(); cmd; if(Debug::glErrorCallback(__FILE__, __LINE__, #cmd)) on_err;

#ifdef DEBUG_MODE
#define GL_CALL(cmd) Debug::glClearErrors(); cmd; Debug::glErrorCallback(__FILE__, __LINE__, #cmd);
#define GL_CALL_CHECK_DBG(cmd, on_err) GL_CALL_CHECK(cmd, on_err)
#else
#define GL_CALL(cmd) cmd;
#define GL_CALL_CHECK_DBG(cmd, on_err) cmd;
#endif

#endif
