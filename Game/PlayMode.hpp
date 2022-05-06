#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include "Sprite.hpp"
#define PI_F 3.1415926f


struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----
	
	//camera:
	Scene::Camera* camera = nullptr;

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, space,in,out;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//Sprites:
	float spriteVelocity = 9.0f;

	//Meshes:
	bool bboxIntersect(BBoxStruct object, BBoxStruct stationary); //Intersect bboxes and return true if collision
	
	//Old but useful things related to player
	glm::vec3 get_player_position();
	Scene::Transform* player = nullptr;
	glm::quat player_rotation;
	Scene::Transform playerOrigin;
};

