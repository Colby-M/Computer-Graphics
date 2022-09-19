#version 330 core
// project 1 Fragment shader 
// Last update September 16, 2022
// 
// This version is used for testing and debugging
// it simply gets the color passed through from the 
// vertex shader.  No lighting calculations are done.
//

uniform vec4 color;

out vec4 FragColor;
void main()
{
		FragColor = color;
}