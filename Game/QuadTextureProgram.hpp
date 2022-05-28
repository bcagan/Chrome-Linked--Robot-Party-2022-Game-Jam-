#pragma once

#include "GL.hpp"
#include "Load.hpp"
#include "Scene.hpp"

//Shader program that draws transformed, lit, textured vertices tinted with vertex colors:
struct QuadTextureProgram {
	QuadTextureProgram();
	~QuadTextureProgram();

	GLuint program = 0;

	//Attribute (per-vertex variable) locations:
	GLuint Position_vec4 = -1U;
	GLuint TexCoord_vec2 = -1U;
	
	//Textures:
	//TEXTURE0 - texture that is accessed by TexCoord
	GLuint TEX = -1U;
};

extern Load< QuadTextureProgram > quad_texture_program;
extern Scene::QuadPipeline quad_texture_program_pipeline; 

