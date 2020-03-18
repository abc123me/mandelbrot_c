#shader vertex
#version 330 core

layout (location = 0) in vec4 pos;
out vec2 tcord;

uniform float textScale;
uniform mat4 proj;

void main() {
	gl_Position = vec4(pos.xy * textScale, 0.0, 1.0);
	gl_Position *= proj;
	tcord = pos.zw;
}

#shader fragment
#version 330 core

in vec2 tcord;
out vec4 col;

uniform vec4 textForeColor;
uniform vec4 textBackColor;
uniform sampler2D samp;
uniform float threshold;

vec4 lerp4(float v, vec4 min, vec4 max) {
	vec4 o = max - min;
	o *= v;
	return o + min;
}
void main() {
	float s = texture(samp, tcord).r;
	if(threshold > 0) {
		if(s > threshold) s = 1;
		else s = 0;
	}
	col = lerp4(s, textBackColor, textForeColor);
}
