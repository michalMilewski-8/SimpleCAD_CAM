#version 400

uniform float x, y;

layout( vertices = 20 ) out;

void main( )
{
	gl_out[ gl_InvocationID ].gl_Position = gl_in[ gl_InvocationID ].gl_Position;
	gl_TessLevelOuter[0] = gl_TessLevelOuter[2] = x;
	gl_TessLevelOuter[1] = gl_TessLevelOuter[3] = y;
	gl_TessLevelInner[0] = x;
	gl_TessLevelInner[1] = y;
}