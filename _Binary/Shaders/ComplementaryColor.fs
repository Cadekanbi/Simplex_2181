#version 330

in vec3 Color;

uniform bool SetComplementary = false;
uniform vec3 BasicColor = vec3(-1,-1,-1);

out vec4 Fragment;

void main()
{	
	vec3 val = Color;
	if(BasicColor.r != -1.0 && BasicColor.g != -1.0 && BasicColor.b != -1.0) {
		val = BasicColor;
	}
	
	if(SetComplementary == true) {
		val = vec3(1.0, 1.0, 1.0) - val;
	}
		
	Fragment = vec4(val,1);
	
	return;
}