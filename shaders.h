#ifndef SHADERS_H
#define SHADERS_H

#pragma once

// Vertex Shader Source
const char* vertex_shader = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

)";

// Fragment Shader Source
const char* fragment_shader = R"(
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 1.0, 1.0, 1.0); // Solid white
}

)";

#endif