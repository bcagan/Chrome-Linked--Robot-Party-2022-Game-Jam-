#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include "Sprite.hpp"
#include "Level.hpp"
#define PI_F 3.1415926f

#ifndef PLAYMODE_H

#define PLAYMODE_H

#define ENEMY_MELEE_TIME 60



//Global screen width and height
inline int screenW = 0;
inline int screenH = 0;
extern SDL_Window* window;

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
	} left, right, down, up, space,in,out, arrowleft,arrowright,arrowup,arrowdown, rightfire, leftfire, melee;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;
	 
	Level level;

	void offsetObjects();
		
	//Sprites:
	float spriteVelocityPlayer = 5.0f;
	float spriteVelocityReticle = 13.5;
	glm::vec2 cursorPos; //0,0 top left

		//Projectiles
		void createProjectileMech(int type, glm::vec3 motionVec, glm::vec3 playerPos);
		void updateProjectiles(float elapsed);
		int projectileHit(Sprite toHit);
		
		float projSpeed = 40.f;
		float enemProjSpeed = 15.f;
		struct Projectile
		{

			int meleeTimer = 0;
			int meleeTime = ENEMY_MELEE_TIME;
			glm::vec3 motionVector;
			Sprite projSprite;
			int type = PROJ_SlowPlayer;
			//Bombs are "mines", so dont explode unless impacted

			enum { PROJ_Melee, PROJ_MeleeEnemy, PROJ_SlowPlayer, PROJ_SlowPlayerReflect, PROJ_RapidPlayer, PROJ_SlowEnemy, PROJ_RapidEnemy, PROJ_Missile, PROJ_Bomb };
		};
		float projCutoff = 100.f; 
		int projInter(Projectile& proj, Sprite hitSprite, float spriteHeight, float spriteWidth, glm::vec3 spritePos);

		//Proj damage
		float rapidPlayerDam = 1.5f;
		float slowPlayerDam = 8.f;
		float meleePlayerDam = 15.f;

		Projectile genericProjectile1;
		std::list<Projectile> projectiles;

		int rapidFiredCooldown = 8;
		int leftFiredCooldown = rapidFiredCooldown;
		int slowFiredCooldown = 30;
		int rightFiredCooldown = slowFiredCooldown;
		int fireCooldown = 30;
		float projZDelta =1.5f;
		float reflectZDelta = 1.5f;

		//Enemies
		struct Enemy {
			float health = 10.f;
			bool alive = true;
			int enemyCooldown = 100; //Optional timer
			int enemyTimer = 0;

			int shotCooldown = 0;
			int shotTimer = 0;
			int projType = Projectile::PROJ_RapidEnemy;

			//Firing can happen if cooldowns says so, or, if ai says so. Ai can be decided in update, and can take all play mode info
			glm::vec2 shotOffset; //Where shots fire from

			//Enemy visualization
			Sprite sprite;
			std::vector<std::pair<int, glm::vec2>> path; //Int is amount of frames along segment, vec is pos at the start of that segment

			//Enemy type AI
			//Ai examples: Fire at player but still use cooldown, fire when player is line of sight of enemy, but not quite there. Change path if player is near them, use other shot ai, etc.
			//AI depends on enemy's type;
			int type = ENEMY_EnvoPilotRa;

			//Enemy instance specific
			glm::vec3 initPos;
			int segment;
			int framesInSegment;

			bool inDodge = false;
			bool posDodge = false;
			int dodgeCount = 30;
			int dodgeTime = 0;

			bool inMelee = false;
			int meleeTimer = 0;
			int toPlayerTime = 90;
			int attackTime = 60;
			int returnTime = 30;
			glm::vec3 attackPos = glm::vec3(0.f);
			glm::vec3 startPos = glm::vec3(0.f);
			glm::vec3 targetPos = glm::vec3(0.f);
			

			enum{ENEMY_envoPilotMe, ENEMY_EnvoPilotRa, ENEMY_grazaJet, BOSS_Jebb, BOSS_Gil, BOSS_Dark};



			//For death sequence only, aka if(dead == false)
			//Death animation is included in sprite
			int framesToDeath = 240;
			//Remove sprite when == 0;
			//Keep in place? Probably best by default
			//Maybe fade out in last frames to solve
		};
		std::list<Enemy> enemies; //All SPAWNED enemies
		void updateAllEnemies();
		//Makes most sense to hardcode and build enemies by hand, since there won't be many.

		void addEnemyCheck(int ID, glm::vec3 worldPos, float spawnDist); //Enemy type, Starting pos (WORLD),  spawn distance
		struct EnemySpawn {
			int ID = Enemy::ENEMY_grazaJet;
			glm::vec3 worldPos;
			float spawnDist = 0.f;
		};
		std::list<EnemySpawn> toSpawn;
		std::list<EnemySpawn>::iterator curSpawnCheck;
		void spawnEnemies();
		float spawnCutoff = 60.f;

		Enemy grazaloidJet;
		Enemy envocronMeleePilot;
		Enemy envocronRangedPilot;
		Enemy bossJebb;
		Enemy bossGil;
		Enemy bossDark;

	//Meshes:
	bool bboxIntersect(BBoxStruct object, BBoxStruct stationary); //Intersect bboxes and return true if collision
	
	//Useful things related to player
	glm::vec3 get_player_position();
	Scene::Transform* player = nullptr;
	glm::quat player_rotation;
	Scene::Transform playerOrigin;

	//All in camera space
	float maxHeight = 1.0f;
	float minHeight = -1.0f;
	float maxWidth = 1.0f;
	float minWidth = -1.0f;
	float playerCamHeight = 0.f;
	float playerCamDepth = 1.f;
	
	struct Mech{

		Sprite* playerSprite;
		Sprite* reticle;
		Sprite* controlReticle;

		float health = 200.f;
		int meleeHitInvince = 45;
		int meleeHitInvinceTimer = 0;

		int meleeTime = 30;
		int meleeTimer = 0;

		std::list<Projectile>::iterator reflect;
	} mech; 


	struct StateMachine {
		int currentState = STATE_horiSteady;
		int nextState = STATE_horiSteady;
		int transitionState = STATE_horiSteady;

		int framesTo = 0;
		int framesDuring = 0;

		bool inTransition = false;


		enum {
			//The following are for horizontal movement and for animation
			STATE_horiSteady,
			STATE_left,
			STATE_right,
			STATE_leftToRight,
			STATE_rightToLeft,
			STATE_leftToSteady,
			STATE_rightToSteady,

			STATE_up,
			STATE_down,
			STATE_vertSteady,
			STATE_upToDown,
			STATE_downToUp,
			STATE_upToSteady,
			STATE_downToSteady
		};
		void transitionMachine(int from, int to);
		void updateStateMachine();
	};
	bool win = false;

	StateMachine animation;
	StateMachine horiMovement;
	StateMachine vertMovement;
	void printMotionState();
};



#endif // !PLAYMODE_H