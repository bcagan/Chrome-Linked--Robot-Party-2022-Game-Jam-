#pragma once

#ifndef TEXTF_H
#define TEXTF_H

#include <glm/gtc/type_ptr.hpp>

#include <random>
#include<vector>
#include <iostream>
#include<fstream>
#include <stdlib.h>
#include <assert.h>
#include <array>
#include <algorithm>

#include <stdlib.h>
#include <stdio.h>
#include <hb.h>
#include <hb-ft.h>
#include "data_path.hpp"

#define FONT_SIZE 36 //HarfBuzz implementation based on 
//https://github.com/harfbuzz/harfbuzz-tutorial/blob/master/hello-harfbuzz-freetype.c

struct Glyph {
	unsigned int textureID = 0;
	glm::uvec2   size = glm::uvec2(0, 0);       // (width, height) of glyph
	glm::ivec2   bearing = glm::ivec2(0, 0);    // baseline to (left,top) offset
	unsigned int advance = 0;    // Offset to advance to next glyph
}; //Glyphs will be mapped to the above after being created in the face
//From their, the buffer of glyphs will be turne dinto a buffer of above references
//Textuere will be generated one at a time

//https://learnopengl.com/In-Practice/Text-Rendering was used to build
//the above data type

struct TexInfo {
	unsigned int texture = 0;
	unsigned int codepoint = 0; //Should turn into map
};


class Text
{
public:

	Text::Text() {

		std::string fontString = std::string("BalooDa2-Medium.ttf");
		std::string properPath = data_path(fontString);
		hb_font = NULL;
		hb_buffer = NULL;
		FT_Error ft_error;

		if ((ft_error = FT_Init_FreeType(&ft_library)))
			abort();
		if ((ft_error = FT_New_Face(ft_library, properPath.c_str(), 0, &ft_face))) {
			std::cout << "ERROR: Cannot open " << fontString  << std::endl;
			abort();
		}
		if ((ft_error = FT_Set_Char_Size(ft_face, FONT_SIZE * 64, FONT_SIZE * 64, 0, 0)))
			abort();


		/* Create hb-ft font. */
		hb_font = hb_ft_font_create(ft_face, NULL);
		foundGlyph = std::vector<TexInfo>();

	}


	Text::Text(std::string fontString) {
		std::string properPath = data_path(fontString);
		hb_font = NULL;
		hb_font = NULL;
		hb_buffer = NULL;
		FT_Error ft_error;

		if ((ft_error = FT_Init_FreeType(&ft_library)))
			abort();
		if ((ft_error = FT_New_Face(ft_library, properPath.c_str(), 0, &ft_face)))
			abort();
		if ((ft_error = FT_Set_Char_Size(ft_face, FONT_SIZE * 64, FONT_SIZE * 64, 0, 0)))
			abort();


		/* Create hb-ft font. */
		hb_font = hb_ft_font_create(ft_face, NULL);
		foundGlyph = std::vector<TexInfo>();

	}
	Text::~Text()
	{
	}

	//Text info
	std::string currentText;
	int currentSpeaker = 0;
	FT_Face ft_face = nullptr;
	hb_font_t* hb_font = nullptr;
	std::vector<Glyph> curLine;
	std::vector<TexInfo> foundGlyph;
	unsigned int getTexture(unsigned int codepoint, bool* success);
	void createBuf(std::string text);
	void setFont(std::string fontfile);
	FT_Library ft_library = nullptr;
	hb_buffer_t* hb_buffer = nullptr;
	void displayText(std::string inText, size_t level);
	
	glm::vec3 textColor = glm::vec3(1.0f);

private:

};


#endif // !TEXTF_H

