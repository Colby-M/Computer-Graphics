#version 330 core
//
// Project 1 Vertex shader.  
// Passes vertex information through without changing it.
//  Being used for debugging purposes.
// 
uniform mat4 modelingMatrix;
uniform mat4 viewingMatrix;
uniform mat4 projectionMatrix;
uniform vec4 color;

in vec4 vPosition;

void main()
{
    gl_Position = projectionMatrix * viewingMatrix * modelingMatrix * vPosition;
}
