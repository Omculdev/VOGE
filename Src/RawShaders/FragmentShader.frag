#version 450

layout (location = 0) in vec4 outcolor;

layout (location = 0) out vec4 finalcolor;

void main() {
    finalcolor = outcolor;
}