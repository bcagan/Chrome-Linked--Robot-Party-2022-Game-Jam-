#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "Scene.hpp"
#include "Sprite.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <string>

#define AVOID_RECOLLIDE_OFFSET 0.05f
#define DEATH_LAYER -10.0f

GLuint test_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > test_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("testScene.pnct"));
	test_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > test_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("testScene.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = test_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = test_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;
		drawable.pipeline.min = mesh.min;
		drawable.pipeline.max = mesh.max;

	});
});

/*Load< Sound::Sample > mainMusic(LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample(data_path("A-Stellar-Jaunt.wav"));
});*/

PlayMode::PlayMode() : scene(*test_scene) {
	auto getBBox = [this](Scene scene, std::string name) { //Load bboxes into transformations for easier access 
		BBoxStruct newBBox;
		newBBox.min = glm::vec3(0.0f);
		newBBox.max = glm::vec3(0.0f);
		for (auto whichDrawable : scene.drawables) {
			if (whichDrawable.transform->name == name) {
				newBBox.min = whichDrawable.pipeline.min;
				newBBox.max = whichDrawable.pipeline.max;
			}
		}
		return newBBox;
	};


	//Scene sprite params need to be set before spirtes can be added
	scene.spriteProgram = lit_color_texture_program->program;
	scene.spriteLib = std::unordered_map<std::string, Sprite>();

	{//Create sprites (will be done in dedicated functions in final game)
		Sprite test;
		test.pipeline = lit_color_texture_program_sprite_pipeline;
		test.pipeline.animations = new std::unordered_map<std::string, Sprite::SpriteAnimation>();
		test.addAnimation("/Sources/Animations/ANIMATE_fastTest.txt"); //Fast
		test.addAnimation("/Sources/Animations/ANIMATE_slowTest.txt"); //Slow
		test.pipeline.setAnimation("fastTest");
		test.pipeline.defaultAnimation = (*test.pipeline.animations)["fastTest"];
		test.width = 16; test.height = 16;
		scene.spriteLib["test"] = test; 

	}

	for (auto &transform : scene.transforms) {
		std::string transformStr = std::string(transform.name);
		if (transformStr.size() >= 9 && transformStr.substr(0, 6) == std::string("sprite")) { //If a sprite, set to not be drawn, and create a corresponding sprite in scene
			transform.doDraw = false; //Don't draw a sprite indicator
			glm::vec3 pos = transform.position;
			size_t numIndicator = 9;
			for (size_t ind = 8; ind < transformStr.size(); ind++) { //Disregard final number of indicator
				if (transformStr[ind] == '_') {
					numIndicator = ind;
					break;
				}
			}
			std::string spriteType = transformStr.substr(7, numIndicator - 7); //Get type of sprite and get generic version from library
			Sprite newSprite = scene.spriteLib[spriteType];

			newSprite.pos = pos; //Create instance of spriteType at pos, and put in sprite list to be drawn/manipulated
			scene.sprites.push_back(newSprite);
		}
	}/*

	for (size_t c = 0; c < numPlatforms; c++) {
		std::string errorStr = std::string("Platform").append(std::to_string(c)).append(std::string(" not found."));
		if (platformArray[c] == nullptr) throw std::runtime_error(errorStr.c_str());
	}
	if (player == nullptr) throw std::runtime_error("Platform not found.");*/


	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	//start music loop playing:
	// (note: position will be over-ridden in update())
	//bg_loop = Sound::loop_3D(*mainMusic, 1.0f, get_player_position(), 10.0f);
}

PlayMode::~PlayMode() {
}


bool PlayMode::bboxIntersect(BBoxStruct object, BBoxStruct stationary) { //Checks that intersection occurs on all 3 axises
	BBoxStruct smallerX = object; //Checks if smaller objects is within bounds of larger object
	BBoxStruct largerX = stationary;
	if (object.max.x - object.min.x > stationary.max.x - stationary.min.x) {
		smallerX = stationary;
		largerX = object;
	}
	bool xBool = (smallerX.min.x >= largerX.min.x && smallerX.min.x <= largerX.max.x
		|| smallerX.max.x <= largerX.max.x && smallerX.max.x >= largerX.min.x); 
	BBoxStruct smallerY = object; //Does the same for y and z as x
	BBoxStruct largerY = stationary;
	if (object.max.y - object.min.y > stationary.max.y - stationary.min.y) {
		smallerY = stationary;
		largerY = object;
	}
	bool yBool = (smallerY.min.y >= largerY.min.y && smallerY.min.y <= largerY.max.y
		|| smallerY.max.y <= largerY.max.y && smallerY.max.y >= largerY.min.y);
	BBoxStruct smallerZ = object;
	BBoxStruct largerZ = stationary;
	if (object.max.z - object.min.z > stationary.max.z - stationary.min.z) {
		smallerZ = stationary;
		largerZ = object;
	}
	bool zBool = (smallerZ.min.z >= largerZ.min.z && smallerZ.min.z <= largerZ.max.z
		|| smallerZ.max.z <= largerZ.max.z && smallerZ.max.z >= largerZ.min.z);
	return xBool && yBool && zBool; //Only true if all 3 axises have an intersection
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_q) {
			in.downs += 1;
			in.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_e) {
			out.downs += 1;
			out.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_n) {
			scene.sprites.front().pipeline.setAnimation("fastTest");
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_m) {
			scene.sprites.front().pipeline.setAnimation("slowTest");
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.downs += 1;
			space.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_q) {
			in.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_e) {
			out.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = false;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			return true;
		}
	}

	return false;
}




void PlayMode::update(float elapsed) {

	


	/*{ //update listener to camera position:
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		glm::vec3 at = frame[3];
		Sound::listener.set_position_right(at, right, 1.0f / 60.0f);
	}

	{//update music position
		bg_loop->set_position(get_player_position(),bg_loop->pan.ramp);
	}*/


	//Update test sprite pos
	//Horizontal
	glm::vec3 motionVec = glm::vec3(0.f);
	float updateDist = spriteVelocity * elapsed;
	if (left.pressed && !right.pressed) {
		motionVec.x -= updateDist;
	}
	else if (right.pressed && !left.pressed) {
		motionVec.x += updateDist;
	}
	//Vertical
	if (down.pressed && !up.pressed) {
		motionVec.z -= updateDist;
	}
	else if (up.pressed && !down.pressed) {
		motionVec.z += updateDist;
	}
	//Depth
	if (in.pressed && !out.pressed) {
		motionVec.y -= updateDist;
	}
	else if (out.pressed && !in.pressed) {
		motionVec.y += updateDist;
	}

	Scene::Transform newTransform;
	newTransform.rotation = scene.cameras.front().transform->rotation;
	newTransform.rotation = glm::normalize(
		newTransform.rotation
		* glm::angleAxis(-PI_F / 2.f, glm::vec3(1.0f, 0.0f, 0.0f)));
	scene.sprites.front().pos += newTransform.rotation * motionVec;

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	glUseProgram(lit_color_texture_program->program); uint32_t lightCount = std::min<uint32_t>((uint32_t)scene.lights.size(), lit_color_texture_program->maxLights);

	std::vector<int32_t> light_type;
	light_type.reserve(lightCount);
	std::vector<glm::vec3> light_location;
	light_location.reserve(lightCount);
	std::vector<glm::vec3> light_direction;
	light_direction.reserve(lightCount);
	std::vector<glm::vec3> light_energy;
	light_energy.reserve(lightCount);
	std::vector<float> light_cutoff;
	light_cutoff.reserve(lightCount);
	GL_ERRORS();

	for (auto const& light : scene.lights)
	{
		glm::mat4 light_to_world = light.transform->make_local_to_world();
		//set up lighting information for this light:
		light_location.emplace_back(glm::vec3(light_to_world[3]));
		light_direction.emplace_back(glm::vec3(-light_to_world[2]));
		light_energy.emplace_back(light.energy);

		if (light.type == Scene::Light::Point)
		{
			light_type.emplace_back(0);
			light_cutoff.emplace_back(1.0f);
		}
		else if (light.type == Scene::Light::Hemisphere)
		{
			light_type.emplace_back(1);
			light_cutoff.emplace_back(1.0f);
		}
		else if (light.type == Scene::Light::Spot)
		{
			light_type.emplace_back(2);
			light_cutoff.emplace_back(std::cos(0.5f * light.spot_fov));
		}
		else if (light.type == Scene::Light::Directional)
		{
			light_type.emplace_back(3);
			light_cutoff.emplace_back(1.0f);
		}

		//skip remaining lights if maximum light count reached:
		if (light_type.size() == lightCount)
			break;
	}

	glUniform1ui(lit_color_texture_program->LIGHT_COUNT_uint, lightCount);
	GL_ERRORS();

	GL_ERRORS();
	glUniform1f(lit_color_texture_program->LIGHT_COUNT_float, (float)lightCount);

	GL_ERRORS();

	GL_ERRORS();
	glUniform1iv(lit_color_texture_program->LIGHT_TYPE_int_array, lightCount, light_type.data());
	glUniform3fv(lit_color_texture_program->LIGHT_LOCATION_vec3_array, lightCount, glm::value_ptr(light_location[0]));
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3_array, lightCount, glm::value_ptr(light_direction[0]));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3_array, lightCount, glm::value_ptr(light_energy[0]));
	glUniform1fv(lit_color_texture_program->LIGHT_CUTOFF_float_array, lightCount, light_cutoff.data());


	GL_ERRORS();
	glClearColor(.54f, 0.796f, 0.89f, 1.f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);
	GL_ERRORS();
	scene.spriteDraw(*camera);
	GL_ERRORS();

	
	GL_ERRORS();
}

glm::vec3 PlayMode::get_player_position() {
	//the vertex position here was read from the model in blender:
	return player->make_local_to_world() * glm::vec4(0.f, 0.f, 0.0f, 1.0f);
}
