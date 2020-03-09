#shader vertex
#version 330 core

layout (location = 0) in vec4 v;
out vec2 texc;

uniform mat4 proj;

void main() {
	gl_Position = proj * vec4(v.xy, 0.0, 1.0);
	texc = v.zw;
}

#shader fragment
#version 330 core

in vec2 texc;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main() {
	vec4 s = vec4(1, 1, 1, texture(text, texc).r);
	color = vec4(1, 1, 1, 1); //vec4(textColor, 1) * s;
}
