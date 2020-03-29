/*
    ______                __        __                       __         
   / ____/________ ______/ /_____ _/ /  ________  ____  ____/ /__  _____
  / /_  / ___/ __ `/ ___/ __/ __ `/ /  / ___/ _ \/ __ \/ __  / _ \/ ___/
 / __/ / /  / /_/ / /__/ /_/ /_/ / /  / /  /  __/ / / / /_/ /  __/ /    
/_/   /_/   \__,_/\___/\__/\__,_/_/  /_/   \___/_/ /_/\__,_/\___/_/     
========================================================================
 - Utilized by main_gui.cpp::core_render_buf
 - Takes iteration in as vertex data
 - Takes uniform data in for gradient coloring
 - GL_BLEND needs to be OFF for this to work!
*/
#shader vertex
#version 330 core

layout (location = 0) in {
	uint iter;
	vec2 cmplx_pos;
};

struct grad_t {
	vec3 color;
	uint iter;
};
uniform grad_t grad[16];
uniform uint grad_len;
uniform mat4 proj;

vec4 lerp4(float v, vec4 min, vec4 max) {
	vec4 o = max - min;
	o *= v;
	return o + min;
}
void main() {
	vec3 gcol = grad[0].col;
	gl_FragColor = gcol;
	gl_Position = 
	gl_Position *= proj;
}


#shader fragment
#version 330 core

void main() {
	
}
