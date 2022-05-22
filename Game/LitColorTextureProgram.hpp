#pragma once

#include "GL.hpp"
#include "Load.hpp"
#include "Scene.hpp"

//Shader program that draws transformed, lit, textured vertices tinted with vertex colors:
struct LitColorTextureProgram {
	LitColorTextureProgram();
	~LitColorTextureProgram();

	GLuint program = 0;

	//Attribute (per-vertex variable) locations:
	GLuint Position_vec4 = -1U;
	GLuint Normal_vec3 = -1U;
	GLuint Color_vec4 = -1U;
	GLuint TexCoord_vec2 = -1U;

	//Uniform (per-invocation variable) locations:
	GLuint OBJECT_TO_CLIP_mat4 = -1U;
	GLuint OBJECT_TO_LIGHT_mat4x3 = -1U;
	GLuint NORMAL_TO_LIGHT_mat3 = -1U;

	//lighting:
	GLuint DO_LIGHT_bool = -1U;
	GLuint LIGHT_TYPE_int_array = -1U;
	GLuint LIGHT_LOCATION_vec3_array = -1U;
	GLuint LIGHT_DIRECTION_vec3_array = -1U;
	GLuint LIGHT_ENERGY_vec3_array = -1U;
	GLuint LIGHT_CUTOFF_float_array = -1U;
	GLuint LIGHT_COUNT_uint = -1U;
	GLuint LIGHT_COUNT_float = -1U;

	//Added material support:
	GLuint MATERIAL_TYPE_int_array = -1U;
	GLuint TEX_ARR_sampler2D_array = -1U;
	GLuint LAYER_COUNT_uint = -1U;
	GLuint viewDir_vec3 = -1U;


	GLuint maxLights = 40U;
	
	//Textures:
	//TEXTURE0 - texture that is accessed by TexCoord
	GLuint TEX = -1U;
};

extern Load< LitColorTextureProgram > lit_color_texture_program;

extern Scene::Drawable::Pipeline lit_color_texture_program_pipeline;
extern Sprite::Pipeline lit_color_texture_program_sprite_pipeline;
