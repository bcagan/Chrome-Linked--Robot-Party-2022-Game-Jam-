#pragma once

/*
 * A scene manages a hierarchical arrangement of transformations (via "Transform").
 *
 * Each transformation may have associated:
 *  - Drawing data (via "Drawable")
 *  - Camera information (via "Camera")
 *  - Light information (via "Light")
 *
 */

#include "GL.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <list>
#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>

#include "Sprite.hpp"


struct BBoxStruct {
	glm::vec3 min = glm::vec3(std::numeric_limits< float >::infinity());
	glm::vec3 max = glm::vec3(-std::numeric_limits< float >::infinity());
};

struct Scene {


	struct Transform {
		//Transform names are useful for debugging and looking up locations in a loaded scene:
		std::string name;

		//The core function of a transform is to store a transformation in the world:
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); //n.b. wxyz init order
		glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

		//The transform above may be relative to some parent transform:
		Transform *parent = nullptr;

		std::pair<GLuint, GLuint> texInf;
		bool useTexInf = false;

		//It is often convenient to construct matrices representing this transformation:
		// ..relative to its parent:
		glm::mat4x3 make_local_to_parent() const;
		glm::mat4x3 make_parent_to_local() const;
		// ..relative to the world:
		glm::mat4x3 make_local_to_world() const;
		glm::mat4x3 make_world_to_local() const;

		//since hierarchy is tracked through pointers, copy-constructing a transform  is not advised:
		Transform(Transform const &) = delete;
		//if we delete some constructors, we need to let the compiler know that the default constructor is still okay:
		Transform() = default;

		BBoxStruct bbox;
		bool doDraw = true;
	};

	struct Drawable {
		//a 'Drawable' attaches attribute data to a transform:
		Drawable(Transform *transform_) : transform(transform_) { assert(transform); }
		Transform * transform;

		//Contains all the data needed to run the OpenGL pipeline:
		struct Pipeline {
			//Purely storage to pass bbox info the transform
			glm::vec3 min = glm::vec3(0.0f);
			glm::vec3 max = glm::vec3(0.0f);

			GLuint program = 0; //shader program; passed to glUseProgram

			//attributes:
			GLuint vao = 0; //attrib->buffer mapping; passed to glBindVertexArray

			GLenum type = GL_TRIANGLES; //what sort of primitive to draw; passed to glDrawArrays
			GLuint start = 0; //first vertex to draw; passed to glDrawArrays
			GLuint count = 0; //number of vertices to draw; passed to glDrawArrays

			//uniforms:
			GLuint OBJECT_TO_CLIP_mat4 = -1U; //uniform location for object to clip space matrix
			GLuint OBJECT_TO_LIGHT_mat4x3 = -1U; //uniform location for object to light space (== world space) matrix
			GLuint NORMAL_TO_LIGHT_mat3 = -1U; //uniform location for normal to light space (== world space) matrix
			
			//Added Material Support:
			GLuint MATERIAL_TYPE_int_array = -1U;
			GLuint TEX_ARR_sampler2D_array = -1U;
			GLuint LAYER_COUNT_uint = -1U;
			GLuint viewDir_vec3 = -1U;

			std::function< void() > set_uniforms; //(optional) function to set any other useful uniforms

			//Lighting
			GLuint DO_LIGHT_bool = -1U;
			GLuint LIGHT_COUNT_uint = -1U;
			GLuint LIGHT_COUNT_float = -1U;


			GLuint LIGHT_TYPE_int_array = -1U;
			GLuint LIGHT_LOCATION_vec3_array = -1U;
			GLuint LIGHT_DIRECTION_vec3_array = -1U;
			GLuint LIGHT_ENERGY_vec3_array = -1U;
			GLuint LIGHT_CUTOFF_float_array = -1U;
			//text
			GLuint TEXT_BOOL = -1U;
			GLuint TEXT_BOOL2 = -1U;
			GLuint TEXT_COLOR = -1U;

			//texture objects to bind for the first TextureCount textures:
			enum : uint32_t { TextureCount = 4 };
			struct TextureInfo {
				GLuint texture = 0;
				GLenum target = GL_TEXTURE_2D;
			} textures[TextureCount];
		} pipeline;
	};

	struct QuadPipeline {

		GLuint program = 0; //shader program; passed to glUseProgram
		//texture objects to bind for the first TextureCount textures:
		GLuint texture = 0;
		GLuint TEX = -1U;
		GLenum target = GL_TEXTURE_2D;

		GLuint position = -1U;
		GLuint texcoord = -1U;
	} quadPipeline;

	struct Camera {
		//a 'Camera' attaches camera data to a transform:
		Camera(Transform *transform_) : transform(transform_) { assert(transform); }
		Transform * transform;
		//NOTE: cameras are directed along their -z axis

		//perspective camera parameters:
		float fovy = glm::radians(60.0f); //vertical fov (in radians)
		float aspect = 1.0f; //x / y
		float near = 0.01f; //near plane
		//computed from the above:
		glm::mat4 make_projection() const;
	};

	struct Light {
		//a 'Light' attaches light data to a transform:
		Light(Transform *transform_) : transform(transform_) { assert(transform); }
		Transform * transform;
		//NOTE: directional, spot, and hemisphere lights are directed along their -z axis

		enum Type : char {
			Point = 'p',
			Hemisphere = 'h',
			Spot = 's',
			Directional = 'd'
		} type = Point;

		//light energy convolved with our conventional tristimulus spectra:
		//  (i.e., "red, gree, blue" light color)
		glm::vec3 energy = glm::vec3(1.0f);

		//Spotlight specific:
		float spot_fov = glm::radians(45.0f); //spot cone fov (in radians)
	};

	//Scenes, of course, may have many of the above objects:
	std::list< Transform > transforms;
	std::list< Drawable > drawables;
	std::list< Sprite > sprites;
	std::list< Camera > cameras;
	std::list< Light > lights;
	GLuint spriteProgram = 0;


	//Scenes in this game also have a fancy library of sprites so I only load animations once. 
	std::unordered_map<std::string, Sprite> spriteLib;

	//Draw a fullscreen textured quad
	void drawQuad(glm::vec2 drawPos = glm::vec2(-1.f), glm::vec2 size = glm::vec2(2.f)) const;

	//The "draw" function provides a convenient way to pass all the things in a scene to OpenGL:
	void draw(Camera const& camera) const;

	//All sprite drawining is handled here, including updating animations, quad creation, translation, and drawing
	void spriteDraw(Camera const& camera, bool proj = false, bool play = false, bool box = false);
	void cloudDraw(Camera const& camera) ;

	//..sometimes, you want to draw with a custom projection matrix and/or light space:
	void draw(glm::mat4 const& world_to_clip, glm::mat4x3 const& world_to_light = glm::mat4x3(1.0f)) const;
	void cloudDraw(glm::mat4 const& world_to_clip, glm::mat4x3 const& world_to_light = glm::mat4x3(1.0f)) ;
	void spriteDraw(glm::mat4 const& world_to_clip, glm::mat4x3 const& world_to_light = glm::mat4x3(1.0f), bool proj = false, bool play = false, bool box = false);


	//add transforms/objects/cameras from a scene file to this scene:
	// the 'on_drawable' callback gives your code a chance to look up mesh data and make Drawables:
	// throws on file format errors
	void load(std::string const &filename,
		std::function< void(Scene &, Transform *, std::string const &) > const &on_drawable = nullptr
	);

	//this function is called to read extra chunks from the scene file after the main chunks are read:
	// this is useful if you, e.g., subclassing scene to represent a game level/area
	virtual void load_extra(std::istream &from, std::vector< char > const &str0, std::vector< Transform * > const &xfh0) { }

	//empty scene:
	Scene() = default;

	//load a scene:
	Scene(std::string const &filename, std::function< void(Scene &, Transform *, std::string const &) > const &on_drawable);

	//copy a scene (with proper pointer fixup):
	Scene(Scene const &); //...as a constructor
	Scene &operator=(Scene const &); //...as scene = scene
	//... as a set() function that optionally returns the transform->transform mapping:
	void set(Scene const &, std::unordered_map< Transform const *, Transform * > *transform_map = nullptr);
};
