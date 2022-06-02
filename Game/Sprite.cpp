#include "Sprite.hpp"
#include <iostream>
#include <assert.h>
#include <string>
#include <fstream>
#include "data_path.hpp"

void Sprite::addAnimation(std::string fileName, bool verbosity) {
	std::ifstream fstream;
	if (verbosity)  std::cout << "starting load of " << fileName << std::endl;
	std::string properPath = data_path(fileName);
	fstream.open(properPath, std::ios::in | std::ios::binary);
	if (fstream.is_open()) {
		if (verbosity)  	std::cout << fileName << " opened succesfully\n";
		int nameSize;
		std::string name;
		int width, height;
		int numLayers, numFrames, frameLength;
		char intline[32];
		char nameline[999];
		numFrames = 0;

		//Animation info
		fstream.getline(intline, 32);
		nameSize = std::stoi(std::string(intline));
		if (verbosity) std::cout << "nameSize " << nameSize << std::endl;
		fstream.getline(nameline, 999);
		name = std::string(nameline).substr(0,nameSize);
		if (verbosity) std::cout << "name " << name << std::endl;
		fstream.getline(intline, 32);
		width = std::stoi(std::string(intline));
		if (verbosity) std::cout << "width " << width << std::endl;
		fstream.getline(intline, 32);
		height = std::stoi(std::string(intline));
		if (verbosity) std::cout << "height " << height << std::endl;
		fstream.getline(intline, 32);
		numLayers = std::stoi(std::string(intline));
		if (verbosity) std::cout << "# layers " << numLayers << std::endl;
		fstream.getline(intline, 32);
		numFrames = std::atoi(std::string(intline).c_str());
		if (verbosity) std::cout << "# frames " << numFrames << std::endl;
		fstream.getline(intline, 32);
		frameLength = std::stoi(std::string(intline));
		if (verbosity) std::cout << "frame length " << frameLength << std::endl;
		if (fileName == "/Sources/Animations/ANIMATE_prologue.txt") numFrames = 8; //IDK WTF is happening here but it keeps turning what is cleary in the file into 64?

		SpriteAnimation animation(numLayers, numFrames);
		animation.name = name;
		animation.frameTime = frameLength;

		if (verbosity)  std::cout << "Meta data gained succesfully\n";

		//Layers
		for (int l = 0; l < numLayers; l++) { 
			int material;
			fstream.getline(intline, 32);
			material = std::stoi(std::string(intline));
			animation.layers[l].material = material;


			for (int t = 0; t < numFrames; t++) {
				int texSize;
				fstream.getline(intline, 32);
				texSize = std::stoi(std::string(intline));
				char* data = (char*)malloc(sizeof(char)*texSize);
				fstream.read(data, texSize);
				fstream.ignore(texSize, '\n');
				//Create opengl texture
				glGenTextures(1, &(animation.layers[l].textures[t].texture)); 
				glBindTexture(GL_TEXTURE_2D, (animation.layers[l].textures[t].texture));
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				//Clamp to edge makes it clear if we are out of bounds

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				//As we want the tex to be a clearly defined sprite, use nearest neighbor
				if (data) {
					//Creating a texture: (using data created above)
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
					glGenerateMipmap(GL_TEXTURE_2D);
				}
				else {
					std::cout << "Failed to load texture" << std::endl;
					assert(false);
				}
				if (verbosity) 	std::cout << "layer " << l << " frame " << t << " texture created\n";
			}
		}
		if (verbosity)  std::cout << "All data in " <<name << " read\n";
		fstream.close(); //Close file before continuing
		pipeline.animations->insert_or_assign(name.c_str(), animation);
		pipeline.numAnimations++; //returns true if new addition to library

		if (verbosity) std::cout << fileName << " loaded succesfully\n";
	}
	else {
		std::cout << "ERROR: Error adding annimation " << fileName << "." << std::endl;
		assert(false);
	}
}