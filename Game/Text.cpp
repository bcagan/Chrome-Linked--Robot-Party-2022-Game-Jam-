#include <glm/gtc/type_ptr.hpp>
#include "Text.hpp"

#include <random>
#include<vector>
#include <iostream>
#include<fstream>
#include <stdlib.h>
#include <assert.h>
#include <array>
#include <algorithm>

#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <hb.h>
#include <hb-ft.h>
#include "Scene.hpp"



unsigned int Text::getTexture(unsigned int codepoint, bool* success) {
	for (auto whichTexture : foundGlyph) {
		if (whichTexture.codepoint == codepoint) {
			*success = true;
			return whichTexture.texture;
		}
	}
	*success = false;
	return 0;
}

void Text::createBuf(std::string text) {
	//https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c was
	//references for this code
	hb_buffer = hb_buffer_create();
	hb_buffer_add_utf8(hb_buffer, text.c_str(), (int)text.length(), 0, -1);
	hb_buffer_guess_segment_properties(hb_buffer);
	assert(hb_buffer != NULL);

	/* Shape it! */
	hb_shape(hb_font, hb_buffer, NULL, 0);

	/* Get glyph information and positions out of the buffer. */
	size_t len = hb_buffer_get_length(hb_buffer);
	hb_glyph_info_t* info = hb_buffer_get_glyph_infos(hb_buffer, NULL);
	hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(hb_buffer, NULL);


	//Load buffer into glyph vector
	curLine.clear();
	curLine.reserve(len);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	for (size_t c = 0; c < len; c++) {
		Glyph newGlyph;
		curLine.push_back(newGlyph);
		bool success = false;
		if (FT_Load_Glyph(ft_face, info[c].codepoint, FT_LOAD_RENDER)) {
			std::runtime_error("Glyph was unable to load");
		}
		unsigned int texId = getTexture(info[c].codepoint, &success);
		if (!success) {
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				ft_face->glyph->bitmap.width,
				ft_face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				ft_face->glyph->bitmap.buffer
			);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			curLine[c].textureID = texture;
			curLine[c].size = glm::ivec2(ft_face->glyph->bitmap.width, ft_face->glyph->bitmap.rows);
			TexInfo newInfo;
			newInfo.codepoint = info[c].codepoint;
			newInfo.texture = texture;
			foundGlyph.push_back(newInfo);
		}
		else {
			curLine[c].textureID = texId;
			curLine[c].size = glm::ivec2(ft_face->glyph->bitmap.width, ft_face->glyph->bitmap.rows);
		}
		assert(curLine[c].textureID != 0);
		curLine[c].bearing = glm::ivec2(ft_face->glyph->bitmap_left, ft_face->glyph->bitmap_top);
		curLine[c].advance = (unsigned int)(pos[c].x_advance / 64.f);
	}
}



void Text::displayText(std::string inText, size_t level) { //Also uses https://learnopengl.com/In-Practice/Text-Rendering

	//Create glyphs
	createBuf(inText);


	//Set variables
	float x = 0.0f;
	float z = 0.0f;
	float scale = 0.015f;
	glm::vec3 color = glm::vec3(1.f, 1.0f, 1.0f) * textColor;
	
	GLuint VAO, VBO;
	glGenBuffers(1, &VBO);


	struct Vertex {
		glm::vec4 Position;
		glm::vec3 Normal;
		glm::vec4 Color;
		glm::vec2 TexCoord;
	};
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);
	GLint location = glGetAttribLocation(lit_color_texture_program->program, "Position");
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	location = glGetAttribLocation(lit_color_texture_program->program, "Normal");
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(4 * sizeof(float)));
	location = glGetAttribLocation(lit_color_texture_program->program, "Color");
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(7 * sizeof(float)));
	location = glGetAttribLocation(lit_color_texture_program->program, "TexCoord");
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(11 * sizeof(float)));
	GL_ERRORS();

	//Should be generealized to its own function later

	assert(!curLine.empty());
	for (size_t g = 0; g < curLine.size(); g++) {
		Glyph glyph = curLine[g];

		//Create plane mesh
		float xPos = x + glyph.bearing.x * scale;
		float zPos = z - (glyph.size.y - glyph.bearing.y) * scale;
		float width = glyph.size.x * scale;
		float height = glyph.size.y * scale;
		std::array<Vertex, 6> vertices;

		vertices[0].Position = glm::vec4(xPos, -1.f, zPos + height, 1.0f);
		vertices[1].Position = glm::vec4(xPos, -1.f, zPos, 1.0f);
		vertices[2].Position = glm::vec4(xPos + width,-1.f, zPos, 1.0f);
		vertices[3].Position = glm::vec4(xPos, 1.f, zPos + height, 1.0f);
		vertices[4].Position = glm::vec4(xPos + width, -1.f, zPos, 1.0f);
		vertices[5].Position = glm::vec4(xPos + width, -1.f, zPos + height, 1.0f);

		for (size_t c = 0; c < 6; c++) {
			vertices[c].Normal = glm::vec3(0.0f, 0.0f, 1.0f);
			vertices[c].Color = glm::vec4(0, 1.0f, 1.0f, 1.0f);
		}
		vertices[0].TexCoord = glm::vec2(0.0f, 0.0f);
		vertices[1].TexCoord = glm::vec2(0.0f, 1.0f);
		vertices[2].TexCoord = glm::vec2(1.0f, 1.0f);
		vertices[3].TexCoord = glm::vec2(0.0f, 0.0f);
		vertices[4].TexCoord = glm::vec2(1.0f, 1.0f);
		vertices[5].TexCoord = glm::vec2(1.0f, 0.0f);

		glUseProgram(lit_color_texture_program->program);



		//upload vertices to vertex_buffer:
		glUniform1ui(glGetUniformLocation(lit_color_texture_program->program, "TEXT_BOOL"), (uint32_t)true);
		GL_ERRORS();
		glUniform1ui(glGetUniformLocation(lit_color_texture_program->program, "TEXT_BOOL2"), (uint32_t)true);
		GL_ERRORS();
		glUniform3fv(glGetUniformLocation(lit_color_texture_program->program, "TEXT_COLOR"), 1, glm::value_ptr(color));
		GL_ERRORS();


		std::vector<int> tempTexLocation;
		tempTexLocation = std::vector<int>(1);
		glActiveTexture(GL_TEXTURE0);
		GL_ERRORS();
		glBindTexture(GL_TEXTURE_2D, glyph.textureID);
		GL_ERRORS();
		tempTexLocation[0] = 0;
		GL_ERRORS();
		glUniform1iv(glGetUniformLocation(lit_color_texture_program->program, "TEX_ARR"), 1, tempTexLocation.data());

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data()); //upload vertices array

		glDrawArrays(GL_TRIANGLES, 0, 6);
		x += (glyph.advance) * scale;

	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glUseProgram(0);

}