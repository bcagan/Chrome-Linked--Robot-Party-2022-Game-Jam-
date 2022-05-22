#pragma once

#ifndef SPRITE_H
#define SPRITE_H


/*
A sprite is defined in this game as follow:

	A 2D object of a set width and height scaled by a universal factor, that is rendered in 3D space, along the camera's rotation, as follows:
	Sprites have a sett exture (which can be gained through a sprite animation) whose resolution matches the sprites width and height, without
	mip-mapping. This sprite exists in the same 3D space as the game, enabling it to interact with lighting, have material properties, and have
	world space interactions. 

	Lighting is unique for sprites, each texel is lit by the average of all rendered pixels for said texel, or, the lighting for a given texel
	is averaged across the entire texel, and then each intersecting pixel is rendered with this same color value.
*/


//Sprites should have their initial placement, and name (whcih defines its type) in world space, but will not be rendered as a scene mesh. Scene meshes
//Will follow normal rendering rules.

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

#define SPRITE_SCALE 32.f 


class Sprite
{
public:

	Sprite()
	{
	}

	~Sprite()
	{
	}

	void addAnimation(std::string fileName);

	std::string name = "";


	//Animations
		//Sprite textures work different than mesh textures
		struct SpriteTextureInfo {
			GLuint texture = 0;
			GLenum target = GL_TEXTURE_2D;
		};
		struct SpriteLayer {
			int material = 0; //default - no material
			std::vector<SpriteTextureInfo> textures;
		};
		struct SpriteAnimation {
			SpriteAnimation() {}; //Dont use
			SpriteAnimation(int numLayersX, int framesX) {  //Init sprite vector sizes before setting sprite data
				numLayers = numLayersX;
				frames = framesX;
				layers = std::vector<SpriteLayer>(numLayers);
				for (int l = 0; l < numLayers; l++) {
					layers[l].textures = std::vector<SpriteTextureInfo>(frames);
				}
			}

			std::vector<SpriteLayer> layers; //Size =  numLayers
			size_t frames = 1;//> 0, size of textures
			int frameTime = 1;//Number of fps until frame should be updated
			size_t numLayers; //Default 0 - albedo 1 - material

			std::vector<int> getMaterials() {
				std::vector<int> mats = std::vector<int>(numLayers);
				for (int l = 0; l < numLayers; l++) {
					mats[l] = layers[l].material;
				}
				return mats;
			}//Fills a numLayers size vector with currentAnimation[layer].material

			std::string name;
		};

	glm::vec3 pos; //World space position
	int width = 8;
	int height = 8;
	glm::vec2 size = glm::vec2(1.f); //Size modifier

	struct Pipeline {
		//Purely storage to pass bbox info the transform
		glm::vec3 min = glm::vec3(0.0f);
		glm::vec3 max = glm::vec3(0.0f);

		bool isGui = false;

		//uniforms:
		GLuint OBJECT_TO_CLIP_mat4 = -1U; //uniform location for object to clip space matrix
		GLuint OBJECT_TO_LIGHT_mat4x3 = -1U; //uniform location for object to light space (== world space) matrix
		GLuint NORMAL_TO_LIGHT_mat3 = -1U; //uniform location for normal to light space (== world space) matrix

		//Lighting
		GLuint DO_LIGHT_bool = -1U;
		GLuint LIGHT_COUNT_uint = -1U;
		GLuint LIGHT_COUNT_float = -1U;
		bool doLight = true;

		GLuint LIGHT_TYPE_int_array = -1U;
		GLuint LIGHT_LOCATION_vec3_array = -1U;
		GLuint LIGHT_DIRECTION_vec3_array = -1U;
		GLuint LIGHT_ENERGY_vec3_array = -1U;
		GLuint LIGHT_CUTOFF_float_array = -1U;

		//Added Material Support:
		GLuint MATERIAL_TYPE_int_array = -1U;
		GLuint TEX_ARR_sampler2D_array = -1U;
		GLuint LAYER_COUNT_uint = -1U;
		GLuint viewDir_vec3 = -1U;

		std::function< void() > set_uniforms; //(optional) function to set any other useful uniforms

		size_t numAnimations = 0;
		std::string currentAnimation = "default";
		size_t currentFrame = 0;
		size_t currentFrameTime = 0;
		std::unordered_map<std::string, SpriteAnimation>* animations;  //Pointer so multiple sprites don't allocate the same animation too often
		SpriteAnimation defaultAnimation; 
		std::vector<SpriteTextureInfo> currentTextures; //layer size vector

		void setAnimation(std::string whichAnimation) {
			currentAnimation = whichAnimation;
			if (animations->find(whichAnimation) == animations->end()) {
				std::cout << ("ERROR: setAnimation: Cannot find animation ") << whichAnimation << std::endl;
				assert(false);
			}
			SpriteAnimation currentRef = (*animations)[whichAnimation];
			currentTextures = std::vector<SpriteTextureInfo>(currentRef.numLayers);
			currentFrame = 0;
			currentFrameTime = 0;
			updateAnimation();
		}

		void updateAnimation() {
			SpriteAnimation currentRef = (*animations)[currentAnimation];
			currentFrameTime++;
			if (currentFrameTime == currentRef.frameTime) {
				currentFrameTime = 0;
				currentFrame++;
				if (currentFrame == currentRef.frames) currentFrame = 0;
				for (int l = 0; l < currentRef.numLayers; l++) {
					currentTextures[l] = currentRef.layers[l].textures[currentFrame];
				}
			}
		}

	} pipeline;

private:

};

#endif // !SPRITE_H