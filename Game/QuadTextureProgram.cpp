#include "QuadTextureProgram.hpp"

#include "gl_compile_program.hpp"
#include "gl_errors.hpp"
#include <vector>
#include "load_save_png.hpp"
#include "Scene.hpp"
#include <assert.h>


Scene::QuadPipeline quad_texture_program_pipeline; 

Load< QuadTextureProgram > quad_texture_program(LoadTagEarly, []() -> QuadTextureProgram const* {
	QuadTextureProgram* ret = new QuadTextureProgram();

	//----- build the pipeline template -----
	quad_texture_program_pipeline.program = ret->program;
	quad_texture_program_pipeline.TEX = ret->TEX;
	quad_texture_program_pipeline.position = ret->Position_vec4;
	quad_texture_program_pipeline.texcoord = ret->TexCoord_vec2;

	assert(quad_texture_program_pipeline.TEX != -1U);


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

	quad_texture_program_pipeline.texture = tex;
	quad_texture_program_pipeline.target = GL_TEXTURE_2D;

	return ret;
});

QuadTextureProgram::QuadTextureProgram() {
	//Compile vertex and fragment shaders using the convenient 'gl_compile_program' helper function:
	program = gl_compile_program(
		//vertex shader:
		"#version 330\n"
		"in vec4 quadPosition;\n"
		"in vec3 quadNormal;\n"
		"in vec4 quadColor;\n"
		"in vec2 quadTexCoord;\n"
		"out vec2 quadtexCoord;\n"
		"void main() {\n"
		"	gl_Position = quadPosition;\n"
		"	quadtexCoord = quadTexCoord;\n"
		"}\n"
		,
		//fragment shader:
		"#version 330\n"
		"uniform sampler2D quadTEX;\n"
		"in vec2 quadtexCoord;\n"
		"out vec4 fragColor;\n"
		"void main() {\n"
		"	fragColor = texture(quadTEX,quadtexCoord);\n"
		"}\n"
	);
	//As you can see above, adjacent strings in C/C++ are concatenated.
	// this is very useful for writing long shader programs inline.


	//look up the locations of vertex attributes:
	Position_vec4 = glGetAttribLocation(program, "quadPosition");
	GL_ERRORS();
	TexCoord_vec2 = glGetAttribLocation(program, "quadTexCoord");;
	GL_ERRORS();
	TEX = glGetUniformLocation(program, "quadTEX");
	GL_ERRORS();
	if (Position_vec4 == -1U) {
		std::cout << "ERROR: QuadTextureProgram()\n";
		assert(false);
	}
	if (TexCoord_vec2 == -1U) {
		std::cout << "ERROR:TEXCOORD QuadTextureProgram()\n";
		assert(false);
	}

	if (TEX == -1U) {
		std::cout << "ERROR:TEX QuadTextureProgram()\n";
		assert(false);
	}


	glUseProgram(program); //bind program -- glUniform* calls refer to this program now
	GL_ERRORS();

	glUseProgram(0); //unbind program -- glUniform* calls refer to ??? now
}

QuadTextureProgram::~QuadTextureProgram() {
	glDeleteProgram(program);
	program = 0;
}

