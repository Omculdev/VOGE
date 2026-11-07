#version 450

const vec2 positions[3] = {
    vec2(0.0, -0.5),
    vec2(0.0, 0.5),
    vec2(0.5, -0.5)
};
 
const vec4 colors[3] = {
    vec4(0.0353, 1.0, 0.0039, 0.581),
    vec4(0.1922, 0.0157, 0.9804, 0.124),
    vec4(0.9451, 0.0275, 0.1059, 1.0)
};

layout (location = 0) out vec4 outcolor;

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    outcolor = colors[gl_VertexIndex];
}