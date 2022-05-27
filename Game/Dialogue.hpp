#pragma once
#ifndef DIALOGUE_H

#define DIALOGUE_H

#include "GL.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <list>
#include <memory>
#include <functional>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <assert.h>
#include <string>
#include "Sprite.hpp"


class Dialogue
{
public:


	Dialogue::Dialogue() {
		data = std::vector<Line>();
		portraits = std::vector<Sprite>();
		textColor = glm::vec3(1.f);
	}
	Dialogue::Dialogue(std::string filename) {
		data = std::vector<Line>();
		portraits = std::vector<Sprite>();
		textColor = glm::vec3(1.f);
		load(filename);
	}

	void load(std::string filename);


	struct Line {
		int portrait;
		int background;
		std::string text;
	};
	int curLine = 0;
	glm::vec3 textColor;

	Line currentLine() {
		return data[curLine];
	}

	std::pair<GLuint, GLenum> currentPortrait() {
		Sprite curSprite = portraits[data[curLine].portrait];
		Sprite::SpriteAnimation animation = curSprite.pipeline.defaultAnimation;
		Sprite::SpriteTextureInfo texInfo = animation.layers[0].textures[0];
		return std::make_pair(texInfo.texture, texInfo.target);
	}

	std::pair<GLuint, GLenum> currentBG() {
		Sprite::SpriteAnimation animation = backgrounds.pipeline.defaultAnimation;
		Sprite::SpriteTextureInfo texInfo = animation.layers[0].textures[data[curLine].background];
		return std::make_pair(texInfo.texture, texInfo.target);
	}

	std::string currentTex() {
		return data[curLine].text;
	}

	void lastLine() {
		curLine--;
		if (curLine < 0) curLine = (int)data.size() - 1;
	}

	void nextLine() {
		curLine++;
		if (curLine >= data.size()) curLine = 0;
	}

	bool atStart() { return curLine == 0; }

	bool atEnd() { return curLine == data.size() - 1; }

	Dialogue::~Dialogue()
	{
	}
	

private:
	std::vector<Sprite> portraits;
	Sprite backgrounds;
	std::vector<Line> data;

};

#endif // !DIALOGUE_H
