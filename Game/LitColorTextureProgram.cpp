#include "LitColorTextureProgram.hpp"

#include "gl_compile_program.hpp"
#include "gl_errors.hpp"
#include <vector>
#include "load_save_png.hpp"

Scene::Drawable::Pipeline lit_color_texture_program_pipeline;
Sprite::Pipeline lit_color_texture_program_sprite_pipeline;

Load< LitColorTextureProgram > lit_color_texture_program(LoadTagEarly, []() -> LitColorTextureProgram const* {
	LitColorTextureProgram* ret = new LitColorTextureProgram();

	//----- build the pipeline template -----
	lit_color_texture_program_pipeline.program = ret->program;

	lit_color_texture_program_pipeline.OBJECT_TO_CLIP_mat4 = ret->OBJECT_TO_CLIP_mat4;
	lit_color_texture_program_pipeline.OBJECT_TO_LIGHT_mat4x3 = ret->OBJECT_TO_LIGHT_mat4x3;
	lit_color_texture_program_pipeline.NORMAL_TO_LIGHT_mat3 = ret->NORMAL_TO_LIGHT_mat3;

	lit_color_texture_program_sprite_pipeline.OBJECT_TO_CLIP_mat4 = ret->OBJECT_TO_CLIP_mat4;
	lit_color_texture_program_sprite_pipeline.OBJECT_TO_LIGHT_mat4x3 = ret->OBJECT_TO_LIGHT_mat4x3;
	lit_color_texture_program_sprite_pipeline.NORMAL_TO_LIGHT_mat3 = ret->NORMAL_TO_LIGHT_mat3;


	lit_color_texture_program_pipeline.LIGHT_COUNT_uint = ret->LIGHT_COUNT_uint;
	lit_color_texture_program_pipeline.LIGHT_COUNT_float = ret->LIGHT_COUNT_float;
	lit_color_texture_program_pipeline.LIGHT_TYPE_int_array = ret->LIGHT_TYPE_int_array;
	lit_color_texture_program_pipeline.LIGHT_LOCATION_vec3_array = ret->LIGHT_LOCATION_vec3_array;
	lit_color_texture_program_pipeline.LIGHT_DIRECTION_vec3_array = ret->LIGHT_DIRECTION_vec3_array;
	lit_color_texture_program_pipeline.LIGHT_ENERGY_vec3_array = ret->LIGHT_ENERGY_vec3_array;
	lit_color_texture_program_pipeline.LIGHT_CUTOFF_float_array = ret->LIGHT_CUTOFF_float_array;
	lit_color_texture_program_pipeline.MATERIAL_TYPE_int_array = ret->MATERIAL_TYPE_int_array;
	lit_color_texture_program_pipeline.TEX_ARR_sampler2D_array = ret->TEX_ARR_sampler2D_array;
	lit_color_texture_program_pipeline.LAYER_COUNT_uint = ret->LAYER_COUNT_uint;
	lit_color_texture_program_pipeline.viewDir_vec3 = ret->viewDir_vec3;
	lit_color_texture_program_pipeline.DO_LIGHT_bool = ret->DO_LIGHT_bool;
	lit_color_texture_program_pipeline.TEXT_BOOL = ret->TEXT_BOOL;
	lit_color_texture_program_pipeline.TEXT_BOOL2 = ret->TEXT_BOOL2;
	lit_color_texture_program_pipeline.TEXT_COLOR = ret->TEXT_COLOR;
	lit_color_texture_program_sprite_pipeline.LIGHT_COUNT_uint = ret->LIGHT_COUNT_uint;
	lit_color_texture_program_sprite_pipeline.LIGHT_COUNT_float = ret->LIGHT_COUNT_float;
	lit_color_texture_program_sprite_pipeline.LIGHT_TYPE_int_array = ret->LIGHT_TYPE_int_array;
	lit_color_texture_program_sprite_pipeline.LIGHT_LOCATION_vec3_array = ret->LIGHT_LOCATION_vec3_array;
	lit_color_texture_program_sprite_pipeline.LIGHT_DIRECTION_vec3_array = ret->LIGHT_DIRECTION_vec3_array;
	lit_color_texture_program_sprite_pipeline.LIGHT_ENERGY_vec3_array = ret->LIGHT_ENERGY_vec3_array;
	lit_color_texture_program_sprite_pipeline.LIGHT_CUTOFF_float_array = ret->LIGHT_CUTOFF_float_array;
	lit_color_texture_program_sprite_pipeline.MATERIAL_TYPE_int_array = ret->MATERIAL_TYPE_int_array;
	lit_color_texture_program_sprite_pipeline.TEX_ARR_sampler2D_array = ret->TEX_ARR_sampler2D_array;
	lit_color_texture_program_sprite_pipeline.LAYER_COUNT_uint = ret->LAYER_COUNT_uint;
	lit_color_texture_program_sprite_pipeline.viewDir_vec3 = ret->viewDir_vec3;
	lit_color_texture_program_sprite_pipeline.DO_LIGHT_bool = ret->DO_LIGHT_bool;
	lit_color_texture_program_sprite_pipeline.TEXT_BOOL = ret->TEXT_BOOL;
	lit_color_texture_program_sprite_pipeline.TEXT_BOOL2 = ret->TEXT_BOOL2;
	lit_color_texture_program_sprite_pipeline.TEXT_COLOR = ret->TEXT_COLOR;


	//make a 1-pixel white texture to bind by default:
	GLuint tex;
	glGenTextures(1, &tex);

	glBindTexture(GL_TEXTURE_2D, tex);
	std::vector< glm::u8vec4 > tex_data(1, glm::u8vec4(0xff));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);




	/*std::string filename = std::string("sources/stary.png");
	GLuint tex1;
	glGenTextures(1, &tex1);
	std::vector<glm::u8vec4> data;
	int width = 192;
	int height = 192;
	glm::uvec2 size = glm::uvec2(width,height);
	load_png(filename, &size, &data, UpperLeftOrigin);
	if (data.size() > 0)
	{
		glBindTexture(GL_TEXTURE_2D, tex1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 1);
	}
	else  throw std::runtime_error("Error loading texture");*/

	lit_color_texture_program_pipeline.textures[0].texture = tex;
	lit_color_texture_program_pipeline.textures[0].target = GL_TEXTURE_2D;/*
	lit_color_texture_program_pipeline.textures[1].texture = tex1;
	lit_color_texture_program_pipeline.textures[1].target = GL_TEXTURE_2D;*/

	return ret;
});

LitColorTextureProgram::LitColorTextureProgram() {
	//Compile vertex and fragment shaders using the convenient 'gl_compile_program' helper function:
	program = gl_compile_program(
		//vertex shader:
		"#version 330\n"
		"uniform mat4 OBJECT_TO_CLIP;\n"
		"uniform mat4x3 OBJECT_TO_LIGHT;\n"
		"uniform mat3 NORMAL_TO_LIGHT;\n"
		"in vec4 Position;\n"
		"in vec3 Normal;\n"
		"in vec4 Color;\n"
		"in vec2 TexCoord;\n"
		"out vec3 position;\n"
		"out vec3 normal;\n"
		"out vec4 color;\n"
		"out vec2 texCoord;\n"
		"uniform uint TEXT_BOOL;\n"
		"void main() {\n"
		"	if(TEXT_BOOL == 0u){\n"
		"		gl_Position = OBJECT_TO_CLIP * Position;\n"
		"		position = OBJECT_TO_LIGHT * Position;\n"
		"		normal = NORMAL_TO_LIGHT * Normal;\n"
		"	}\n"
		"	else{\n"
		"		gl_Position = Position;\n"
		"		position =  vec3(Position.x,Position.y,Position.z);\n"
		"		normal = Normal;\n"
		"	}\n"
		"	color = Color;\n"
		"	texCoord = TexCoord;\n"
		"}\n"
		,
		//fragment shader:
		"#version 330\n"
		"uniform uint LAYER_COUNT;\n"//ADDED
		"uniform int MATERIAL_TYPE[" + std::to_string(4) + "];\n"//ADDED
		"uniform sampler2D TEX_ARR[" + std::to_string(4) + "];\n" //ADDED
		"uniform uint LIGHT_COUNT;\n"
		"uniform float LIGHT_COUNT_F;\n"
		"uniform int LIGHT_TYPE[" + std::to_string(maxLights) + "];\n"
		"uniform vec3 LIGHT_LOCATION[" + std::to_string(maxLights) + "];\n"
		"uniform vec3 LIGHT_DIRECTION[" + std::to_string(maxLights) + "];\n"
		"uniform vec3 LIGHT_ENERGY[" + std::to_string(maxLights) + "];\n"
		"uniform float LIGHT_CUTOFF[" + std::to_string(maxLights) + "];\n"
		"uniform vec3 viewDir;\n"//ADDED
		"uniform uint DO_LIGHT;\n"//ADDED
		"uniform vec3 TEXT_COLOR;\n"
		"uniform uint TEXT_BOOL2;\n"
		"in vec3 position;\n"
		"in vec3 normal;\n"
		"in vec4 color;\n"
		"in vec2 texCoord;\n"
		"out vec4 fragColor;\n"
		"void main() {\n"
		"	if(TEXT_BOOL2 == 1u){\n"
		"       vec4 inText = vec4(1.0,1.0,1.0, texture(TEX_ARR[0],texCoord).r);\n"
		"       fragColor = vec4(TEXT_COLOR,1.0) * inText;\n"
		"	}\n"
		"	else{\n"
		"	vec3 n = normalize(normal);\n"
		"   vec3 total = vec3(0.0f);\n"
		"	vec3 lightColor = vec3(0.0f);\n"
		"	vec4 albedo = vec4(1.f);\n"
		"	vec3 specular = vec3(0.f);\n"
		"	vec3 emissive = vec3(0.f);\n"
		"	float shininess = 0.f;\n"
		"	float emissiveFactor = 0.f;\n"
		"	for(uint layer = 0u; layer < LAYER_COUNT; ++layer){ \n"
		"		if(MATERIAL_TYPE[layer] == 0){\n"//Albedo
		"			albedo = color*texture(TEX_ARR[layer],texCoord);\n"
		"		}\n"
		"		if(MATERIAL_TYPE[layer] == 1){\n"//Specular - Type 1
		"			shininess = 12.f;\n"
		"			specular =  color.rgb*texture(TEX_ARR[layer],texCoord).rgb;\n"
		"		}\n"
		"		if(MATERIAL_TYPE[layer] == 2){\n"//Emissive - Type 1
		"			emissiveFactor = 1.f;\n"
		"			emissive =  color.rgb*texture(TEX_ARR[layer],texCoord).rgb;\n"
		"		}\n"
		"	}\n"
		"	if(LAYER_COUNT == 0u){\n" //Account for vertex color only meshes
		"		albedo = color;\n"
		"	}\n"
		"	if(DO_LIGHT > 0u){\n"
		"	for(uint light = 0u; light < LIGHT_COUNT; ++light){ \n"
		"		int lightType = LIGHT_TYPE[light];\n"
		"		vec3 lightLocation = LIGHT_LOCATION[light];\n"
		"		vec3 lightDirection = LIGHT_DIRECTION[light];\n"
		"		vec3 lightEnergy = LIGHT_ENERGY[light];\n"
		"		float lightCutoff = LIGHT_CUTOFF[light];\n"
		"		if (lightType == 0) { //point light \n"
		"			vec3 l = (lightLocation - position);\n"
		"			float dis2 = dot(l,l);\n"
		"			l = normalize(l);\n"
		"			float nl = max(0.0, dot(n, l)) / max(1.0, dis2);\n"
		"			if(dis2 > 100.0f) nl = 0.0f;\n"
		"			vec3 reflectDir = reflect(-l,n);"
		"			vec3 diffuse = albedo.rgb * nl * lightEnergy;\n"
		"			float specularFactor = pow(max(dot(viewDir,reflectDir),0.0),shininess);\n"
		"			if(shininess == 0.0) specularFactor = 0.f;\n"
		"			vec3 specularL = specular.rgb * nl  * specularFactor * lightEnergy;\n"
		"			vec3 emissiveL = emissive.rgb * emissiveFactor;\n"
		"			total += diffuse + specularL + emissiveL;\n"
		"		} else if (lightType == 1) { //hemi light \n" //Hemi light ignores materials
		"			total += albedo.rgb*(dot(n,-lightDirection) * 0.5 + 0.5) * lightEnergy;\n"
		"		} else if (lightType == 2) { //spot light \n"
		"			vec3 l = (lightLocation - position);\n"
		"			float dis2 = dot(l,l);\n"
		"			l = normalize(l);\n"
		"			float nl = max(0.0, dot(n, l)) / max(1.0, dis2);\n"
		"			float c = dot(l,-lightDirection);\n"
		"			nl *= smoothstep(lightCutoff,mix(lightCutoff,1.0,0.1), c);\n"
		"			if(dis2 > 200.0f) nl = 0.0f;\n"
		"			total += nl * lightEnergy;\n"
		"		} else { //(lightType == 3) //directional light \n"
		"			vec3 l = (lightLocation - position);\n"
		"			float dis2 = dot(l,l);\n"
		//"			if(dis2 <= 200.0f){\n"
		"				vec3 lightDir = normalize(-lightDirection);\n"
		"				vec3 reflectDir = reflect(-lightDir,n);\n"
		"				vec3 diffuse = albedo.rgb * max(dot(n,lightDir),0.0) * lightEnergy;\n"
		"				float specularFactor = pow(max(dot(viewDir,reflectDir),0.0),shininess);\n"
		"				if(shininess == 0.0) specularFactor = 0.f;\n"
		"				vec3 specularL = specular.rgb * specularFactor * lightEnergy;\n"
		"				vec3 emissiveL = emissive.rgb * emissiveFactor;\n"
		"				total += diffuse + specularL + emissiveL;\n"
	//	"			}\n"

		"		}\n"
		"	}\n"
		"	vec4 texColor = vec4(total, albedo.a);\n"
//		"	vec4 texColor = vec4(specular.rgb,albedo.a);\n"
		"	fragColor = texColor;\n"
		"	if(position.z < -15.0){\n"
		"		float alpha = (1.0 / (-position.z - 14.0));\n"
		"		fragColor = vec4(1 - alpha)*vec4(0.8,0.8,0.8,1.0) + vec4(alpha)*fragColor;\n"
		"	}\n"
		"	}\n"
		"	else fragColor = albedo;\n"
		"}\n"
		"}\n"
	);
	//As you can see above, adjacent strings in C/C++ are concatenated.
	// this is very useful for writing long shader programs inline.

	//look up the locations of vertex attributes:osition_vec4 = glGetAttribLocation(program, "Position");
	Normal_vec3 = glGetAttribLocation(program, "Normal");
	Color_vec4 = glGetAttribLocation(program, "Color");
	TexCoord_vec2 = glGetAttribLocation(program, "TexCoord");

	//look up the locations of uniforms:
	OBJECT_TO_CLIP_mat4 = glGetUniformLocation(program, "OBJECT_TO_CLIP");
	OBJECT_TO_LIGHT_mat4x3 = glGetUniformLocation(program, "OBJECT_TO_LIGHT");
	NORMAL_TO_LIGHT_mat3 = glGetUniformLocation(program, "NORMAL_TO_LIGHT");

	DO_LIGHT_bool = glGetUniformLocation(program, "DO_LIGHT");
	LIGHT_COUNT_uint = glGetUniformLocation(program, "LIGHT_COUNT");
	LIGHT_COUNT_float = glGetUniformLocation(program, "LIGHT_COUNT_F");

	LIGHT_TYPE_int_array = glGetUniformLocation(program, "LIGHT_TYPE");
	LIGHT_LOCATION_vec3_array = glGetUniformLocation(program, "LIGHT_LOCATION");
	LIGHT_DIRECTION_vec3_array = glGetUniformLocation(program, "LIGHT_DIRECTION");
	LIGHT_ENERGY_vec3_array = glGetUniformLocation(program, "LIGHT_ENERGY");
	LIGHT_CUTOFF_float_array = glGetUniformLocation(program, "LIGHT_CUTOFF");

	MATERIAL_TYPE_int_array = glGetUniformLocation(program, "MATERIAL_TYPE");
	TEX_ARR_sampler2D_array = glGetUniformLocation(program, "TEX_ARR");
	LAYER_COUNT_uint = glGetUniformLocation(program, "LAYER_COUNT");
	viewDir_vec3 = glGetUniformLocation(program, "viewDir");

	TEXT_BOOL = glGetUniformLocation(program, "TEXT_BOOL");
	TEXT_BOOL2 = glGetUniformLocation(program, "TEXT_BOOL2");
	TEXT_COLOR = glGetUniformLocation(program, "TEXT_COLOR");



	glUseProgram(program); //bind program -- glUniform* calls refer to this program now

	int texArr[4];
	for (int i = 0; i < 4; i++) {
		texArr[i] = i;
	}

	glUniform1iv(TEX_ARR_sampler2D_array, 4, texArr); //set TEX_ARR[i] to TEXTURE_i

	glUseProgram(0); //unbind program -- glUniform* calls refer to ??? now
}

LitColorTextureProgram::~LitColorTextureProgram() {
	glDeleteProgram(program);
	program = 0;
}

