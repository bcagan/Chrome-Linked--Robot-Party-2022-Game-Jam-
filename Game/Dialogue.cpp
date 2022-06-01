#include "Dialogue.hpp"
#include "GL.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "LitColorTextureProgram.hpp"

#include <list>
#include <memory>
#include <functional>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <assert.h>
#include <string>
#include "Sprite.hpp"
#include <fstream>
#include "data_path.hpp"


void Dialogue::load(std::string filename) {
	std::ifstream fstream;
	std::string properPath = data_path(filename);
	fstream.open(properPath, std::ios::in | std::ios::binary);
	char bufLine[999];
	if (fstream.is_open()) {
		fstream.getline(bufLine, 999);
		while (!fstream.eof()) {
			if (std::string(bufLine).substr(0, 2) == "BG") {
				fstream.getline(bufLine, 999);
				std::string bg = std::string("/Sources/Animations/ANIMATE_") + std::string(bufLine);
				bg = bg.substr(0, bg.size() - 1);
				std::string bgName = std::string( bufLine).substr(0, std::string(bufLine).size() - 5);
				Sprite sprite;
				sprite.pipeline = lit_color_texture_program_sprite_pipeline;
				sprite.pipeline.animations = new std::unordered_map<std::string, Sprite::SpriteAnimation>();
				sprite.addAnimation(bg);
				if (bgName[bgName.size() - 1] == '.') bgName = bgName.substr(0, bgName.size() - 1);
				sprite.pipeline.setAnimation(bgName);
				sprite.pipeline.defaultAnimation = (*sprite.pipeline.animations)[bgName];
				sprite.width = 3840; sprite.height = 2160;
				sprite.pipeline.isGui = true;
				sprite.pipeline.doLight = false;
				backgrounds = (sprite);
				fstream.getline(bufLine, 999);
				if (std::string(bufLine).substr(0, 3) != "EOB") {
					std::cout << "ERROR: More than one BG line\n";
					fstream.close();
					assert(false);
				}

			}
			else if (std::string(bufLine).substr(0, 4) == "PORT") {

				fstream.getline(bufLine, 999);
				while (!fstream.eof() && std::string(bufLine).substr(0, 3) != "EOB") {
					std::string curPortrait = std::string("/Sources/Animations/ANIMATE_") + std::string(bufLine);
					curPortrait = curPortrait.substr(0, curPortrait.size() - 1);
					std::string portraitName = std::string(bufLine).substr(0, std::string(bufLine).size() - 5);
					Sprite sprite;
					sprite.pipeline = lit_color_texture_program_sprite_pipeline;
					sprite.pipeline.animations = new std::unordered_map<std::string, Sprite::SpriteAnimation>();
					sprite.addAnimation(curPortrait);
					sprite.pipeline.setAnimation(portraitName);
					sprite.pipeline.defaultAnimation = (*sprite.pipeline.animations)[portraitName];
					sprite.width = 256; sprite.height = 256;
					sprite.pipeline.isGui = true;
					sprite.pipeline.doLight = false;
					portraits.push_back(sprite);
					fstream.getline(bufLine, 999);
				}
				if (fstream.eof()) {
					std::cout << "ERROR: EOF encountered before EOB (PORT) \n";
					fstream.close();
					assert(false);
				}

			}
			else if (std::string(bufLine).substr(0, 4) == "DIAL") {

				fstream.getline(bufLine, 999);
				while (!fstream.eof() && std::string(bufLine).substr(0, 3) != "EOB") {
					std::string bufString(bufLine);
					Line line;
					int space1 = 0;
					int space2 = 0;
					while (space1 < bufString.size() && bufString[space1] != ' ') space1++;
					if (space1 >= bufString.size()) {
						std::cout << "ERROR: Bad formatting in dialogue\n";
						fstream.close();
						assert(false);
					}
					space2 = space1 + 1;
					while (space2 < bufString.size() && bufString[space2] != ' ') space2++;
					if (space2 >= bufString.size()) {
						std::cout << "ERROR: Bad formatting in dialogue\n";
						fstream.close();
						assert(false);
					}
					line.portrait = std::atoi(bufString.substr(0, space1).c_str());
					if (line.portrait >= portraits.size()) {
						std::cout << "ERROR: Portrait " << line.portrait << " doesn't exist\n";
						fstream.close();
						assert(false);
					}
					line.background = std::atoi(bufString.substr(space1 + 1, space2 - space1 - 1).c_str());
					if (line.background >= backgrounds.pipeline.defaultAnimation.frames) {
						std::cout << "ERROR: Background " << line.background << " doesn't exist\n";
						fstream.close();
						assert(false);
					}
					line.text = bufString.substr(space2 + 1, bufString.size() - space2 - 1);
					data.push_back(line);
					fstream.getline(bufLine, 999);
				}
				if (fstream.eof()) {
					std::cout << "ERROR: EOF encountered before EOB (DIAL)\n";
					fstream.close();
					assert(false);
				}


			}
			else {
				std::cout << "File " << filename << " formatting error : Missing Header.Did EOB occur too early ? \n";
				fstream.close();
				assert(false);
			}
			fstream.getline(bufLine, 999);
		}
	}
	else {
		std::cout << "Unable to load dialogue file " << filename << std::endl;
		assert(false);
	}
	fstream.close();

}