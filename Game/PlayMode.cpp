#include "PlayMode.hpp"

#include "QuadTextureProgram.hpp"
#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "Scene.hpp"
#include "Sprite.hpp"
#include "Level.hpp"
#include "Dialogue.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <string>
#include "Text.hpp"
#include <array>
#include "load_save_png.hpp"

#define AVOID_RECOLLIDE_OFFSET 0.05f
#define DEATH_LAYER -10.0f

GLuint test_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > test_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("level1Scene.pnct"));
	test_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > test_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("level1Scene.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
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



	//Enemies
	toSpawn = std::list<EnemySpawn>();
	curSpawnCheck = toSpawn.begin();


	//Load the level
	level = Level("/Sources/Levels/LEVEL_1.txt"); //Level 1
	//Copy level data: (Memory inefficient, but I cant figure out another way to do this without pissing off the compiler, or messing with other code)
	//For now its fine, its not like Im loading that many enemies at a time
	for (auto& enem : level.toSpawn) {
		addEnemyCheck(enem.ID, enem.worldPos, enem.spawnDist);
	}
	

	//Scene sprite params need to be set before spirtes can be added
	scene.spriteProgram = lit_color_texture_program->program;
	scene.spriteLib = std::unordered_map<std::string, Sprite>();

	{//Create sprites (will be done in dedicated functions in final game)

		Sprite grazaloid;
		grazaloid.pipeline = lit_color_texture_program_sprite_pipeline;
		grazaloid.pipeline.animations = new std::unordered_map<std::string, Sprite::SpriteAnimation>();
		grazaloid.addAnimation("/Sources/Animations/ANIMATE_DevilBotFlyingIdle.txt");
		grazaloid.pipeline.setAnimation("DevilBotFlyingIdle");
		grazaloid.pipeline.defaultAnimation = (*grazaloid.pipeline.animations)["DevilBotFlyingIdle"];
		grazaloid.width = 128; grazaloid.height = 128;
		grazaloid.size = glm::vec2(.25f);
		scene.spriteLib["grazaloid"] = grazaloid;

		Sprite meleebot;
		meleebot.pipeline = lit_color_texture_program_sprite_pipeline;
		meleebot.pipeline.animations = new std::unordered_map<std::string, Sprite::SpriteAnimation>();
		meleebot.addAnimation("/Sources/Animations/ANIMATE_SpikebotFlyingIdle.txt");
		meleebot.pipeline.setAnimation("SpikebotFlyingIdle");
		meleebot.pipeline.defaultAnimation = (*meleebot.pipeline.animations)["SpikebotFlyingIdle"];
		meleebot.addAnimation("/Sources/Animations/ANIMATE_SpikebotMeleeAttack.txt");
		meleebot.width = 128; meleebot.height = 128;
		meleebot.size = glm::vec2(.25f);
		scene.spriteLib["meleebot"] = meleebot;

		Sprite rangebot;
		rangebot.pipeline = lit_color_texture_program_sprite_pipeline;
		rangebot.pipeline.animations = new std::unordered_map<std::string, Sprite::SpriteAnimation>();
		rangebot.addAnimation("/Sources/Animations/ANIMATE_NosepassFlyingIdle.txt");
		rangebot.addAnimation("/Sources/Animations/ANIMATE_NosepassMouthCannon.txt");
		rangebot.pipeline.setAnimation("NosepassMouthCannon");
		rangebot.pipeline.defaultAnimation = (*rangebot.pipeline.animations)["NosepassMouthCannon"];
		rangebot.width = 128; rangebot.height = 128;
		rangebot.size = glm::vec2(.25f);
		scene.spriteLib["rangebot"] = rangebot;

		Sprite reticle;
		reticle.pipeline = lit_color_texture_program_sprite_pipeline;
		reticle.pipeline.animations = new std::unordered_map<std::string, Sprite::SpriteAnimation>();
		reticle.addAnimation("/Sources/Animations/ANIMATE_reticle.txt");
		reticle.pipeline.setAnimation("reticle");
		reticle.pipeline.defaultAnimation = (*reticle.pipeline.animations)["reticle"];
		reticle.width = 16; reticle.height = 16;
		reticle.size = glm::vec2(0.5f);
		scene.spriteLib["reticle"] = reticle;

		Sprite ship;
		ship.pipeline = lit_color_texture_program_sprite_pipeline;
		ship.pipeline.animations = new std::unordered_map<std::string, Sprite::SpriteAnimation>();
		ship.addAnimation("/Sources/Animations/ANIMATE_ShipFlyingIdle.txt");
		ship.addAnimation("/Sources/Animations/ANIMATE_ShipFlyingIdleW.txt");
		ship.addAnimation("/Sources/Animations/ANIMATE_ShipFlyingIdleE.txt");
		ship.pipeline.setAnimation("ShipFlyingIdle");
		ship.pipeline.defaultAnimation = (*ship.pipeline.animations)["ShipFlyingIdle"];
		ship.width = 64; ship.height = 64;
		ship.size = glm::vec2(0.5f/16.f);
		scene.spriteLib["ship"] = ship;

		Sprite projectile;
		projectile.pipeline = lit_color_texture_program_sprite_pipeline;
		projectile.pipeline.animations = new std::unordered_map<std::string, Sprite::SpriteAnimation>();
		projectile.addAnimation("/Sources/Animations/ANIMATE_projectileEnSlow.txt");
		projectile.pipeline.setAnimation("projectile1");
		projectile.pipeline.defaultAnimation = (*projectile.pipeline.animations)["projectile1"];
		projectile.width = 16; projectile.height = 16;
		projectile.size = glm::vec2(0.5f);
		projectile.addAnimation("/Sources/Animations/ANIMATE_projectileEnRapid.txt");
		projectile.addAnimation("/Sources/Animations/ANIMATE_projectilePlayerSlow.txt");
		projectile.addAnimation("/Sources/Animations/ANIMATE_projectilePlayerRapid.txt");
		projectile.addAnimation("/Sources/Animations/ANIMATE_projectileDark.txt");
		scene.spriteLib["projectile1"] = projectile;

		Sprite bossSpriteJebb;
		bossSpriteJebb.pipeline = lit_color_texture_program_sprite_pipeline;
		bossSpriteJebb.pipeline.animations = new std::unordered_map<std::string, Sprite::SpriteAnimation>();
		bossSpriteJebb.addAnimation("/Sources/Animations/ANIMATE_GammaBotFlyingIdle.txt");
		bossSpriteJebb.addAnimation("/Sources/Animations/ANIMATE_GammaBotDoubleCannons.txt");
		bossSpriteJebb.pipeline.setAnimation("GammaBotFlyingIdle");
		bossSpriteJebb.pipeline.defaultAnimation = (*bossSpriteJebb.pipeline.animations)["GammaBotFlyingIdle"];
		bossSpriteJebb.width = 256; bossSpriteJebb.height = 256;
		bossSpriteJebb.size = glm::vec2(0.75f);
		scene.spriteLib["Jebb"] = bossSpriteJebb;

		Sprite bossSpriteDark;
		bossSpriteDark.pipeline = lit_color_texture_program_sprite_pipeline;
		bossSpriteDark.pipeline.animations = new std::unordered_map<std::string, Sprite::SpriteAnimation>();
		bossSpriteDark.addAnimation("/Sources/Animations/ANIMATE_DSDBotFlyingIdle.txt");
		bossSpriteDark.addAnimation("/Sources/Animations/ANIMATE_DSDBotDoubleCannons.txt");
		bossSpriteDark.pipeline.setAnimation("DSDBotFlyingIdle");
		bossSpriteDark.pipeline.defaultAnimation = (*bossSpriteDark.pipeline.animations)["DSDBotFlyingIdle"];
		bossSpriteDark.width = 256; bossSpriteDark.height = 256;
		bossSpriteDark.size = glm::vec2(0.75f);
		scene.spriteLib["Dark"] = bossSpriteDark;

		Sprite zivMech;
		zivMech.pipeline = lit_color_texture_program_sprite_pipeline;
		zivMech.pipeline.animations = new std::unordered_map<std::string, Sprite::SpriteAnimation>();
		zivMech.addAnimation("/Sources/Animations/ANIMATE_ZivMechFlying.txt");
		zivMech.addAnimation("/Sources/Animations/ANIMATE_ZivMechFlyingLeft.txt");
		zivMech.addAnimation("/Sources/Animations/ANIMATE_ZivMechFlyingRight.txt");
		zivMech.addAnimation("/Sources/Animations/ANIMATE_ZivMechAttack.txt");
		zivMech.pipeline.setAnimation("ZivMechFlying");
		zivMech.pipeline.defaultAnimation = (*zivMech.pipeline.animations)["ZivMechFlying"];
		zivMech.width = 128; zivMech.height = 128;
		zivMech.size = glm::vec2(1.f); 
		zivMech.hitBoxOff = glm::vec2(0.15 * zivMech.width * zivMech.size.x / SPRITE_SCALE, 0.15 * zivMech.width * zivMech.size.y / SPRITE_SCALE);
		zivMech.hitBoxSize = glm::vec2(0.7f);
		scene.spriteLib["zivMech"] = zivMech;

		Sprite textboxSprite;
		textboxSprite.pipeline = lit_color_texture_program_sprite_pipeline;
		textboxSprite.pipeline.animations = new std::unordered_map<std::string, Sprite::SpriteAnimation>();
		textboxSprite.addAnimation("/Sources/Animations/ANIMATE_textbox.txt");
		textboxSprite.pipeline.setAnimation("textbox");
		textboxSprite.pipeline.defaultAnimation = (*zivMech.pipeline.animations)["textbox"];
		textboxSprite.width = 3840; textboxSprite.height = 2160;
		scene.spriteLib["textbox"] = textboxSprite;

		Sprite cloud1;
		cloud1.pipeline = lit_color_texture_program_sprite_pipeline;
		cloud1.pipeline.animations = new std::unordered_map < std::string, Sprite::SpriteAnimation > ();
		cloud1.addAnimation("/Sources/Animations/ANIMATE_cloud1.txt");
		cloud1.pipeline.setAnimation("cloud1");
		cloud1.pipeline.defaultAnimation = (*cloud1.pipeline.animations)["cloud1"];
		cloud1.width = 128; cloud1.height = 64;
		cloud1.size = glm::vec2(0.85f);
		scene.spriteLib["cloud1"] = cloud1;

		Sprite BGSprite;
		BGSprite.pipeline = lit_color_texture_program_sprite_pipeline;
		BGSprite.pipeline.animations = new std::unordered_map < std::string, Sprite::SpriteAnimation >();
		BGSprite.addAnimation("/Sources/Animations/ANIMATE_bg.txt");
		BGSprite.pipeline.setAnimation("bg");
		BGSprite.pipeline.defaultAnimation = (*BGSprite.pipeline.animations)["bg"];
		BGSprite.width = 3840; BGSprite.height = 2160;
		bgint = BGSprite.pipeline.defaultAnimation.layers[0].textures[0].texture;
	}

	//Load textures for buildings and other meshes if needed
	{
		std::string filename =  data_path(std::string("sources/building1color.png"));
		GLuint tex1;
		glGenTextures(1, &tex1);
		std::vector<glm::u8vec4> data;
		int width = 256;
		int height = 256;
		glm::uvec2 size = glm::uvec2(width, height);
		load_png(filename, &size, &data, LowerLeftOrigin);
		if (data.size() > 0)
		{
			glBindTexture(GL_TEXTURE_2D, tex1);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
			glBindTexture(GL_TEXTURE_2D, 1);
		}
		else  throw std::runtime_error("Error loading texture"); 
		filename = data_path(std::string("sources/building1color.png"));
		GLuint tex2;
		glGenTextures(1, &tex2);
		load_png(filename, &size, &data, LowerLeftOrigin);
		if (data.size() > 0)
		{
			glBindTexture(GL_TEXTURE_2D, tex2);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
			glBindTexture(GL_TEXTURE_2D, 1);
		}
		else  throw std::runtime_error("Error loading texture");
		buildTexts = std::vector<std::pair<GLuint, GLuint>>(2);
		buildTexts[0] = std::make_pair(tex1, tex2); 
		filename = data_path(std::string("sources/building2color.png"));
		GLuint tex3;
		glGenTextures(1, &tex3);
		load_png(filename, &size, &data, LowerLeftOrigin);
		if (data.size() > 0)
		{
			glBindTexture(GL_TEXTURE_2D, tex3);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
			glBindTexture(GL_TEXTURE_2D, 1);
		}
		else  throw std::runtime_error("Error loading texture"); filename = data_path(std::string("sources/building2specular.png"));
		GLuint tex4;
		glGenTextures(1, &tex4);
		load_png(filename, &size, &data, LowerLeftOrigin);
		if (data.size() > 0)
		{
			glBindTexture(GL_TEXTURE_2D, tex4);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
			glBindTexture(GL_TEXTURE_2D, 1);
		}
		else  throw std::runtime_error("Error loading texture");
		buildTexts[1] = std::make_pair(tex3, tex4);

	}


	glm::vec3 initPos = glm::vec3(0.f);

	for (auto& transform : scene.transforms) {
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

			if (spriteType == "zivMech") {
				newSprite.pos = pos; //Create instance of spriteType at pos, and put in sprite list to be drawn/manipulated
				initPos = pos ;
				newSprite.size = glm::vec2(0.4f);
				newSprite.name = "player";
				scene.sprites.push_back(newSprite);
			}
			else if (spriteType == "reticle") {
				newSprite.pos = initPos + glm::vec3(0.f, 0.f, 0.2f);
				newSprite.name = "reticle";
				scene.sprites.push_back(newSprite);
			}

		}
		else if ((transformStr.substr(0, 8) == std::string("Building")) && (transformStr.size() > 8)) {
			int intEndInd = 8;
			while (transformStr[intEndInd] != 'T')intEndInd++;
			int buildingType = std::atoi(transformStr.substr(8, intEndInd - 8).c_str());
			assert(buildingType < buildTexts.size());
			transform.texInf = buildTexts[buildingType];
			transform.useTexInf = true;
		}
	}
	Sprite newProjectile = scene.spriteLib["projectile1"];
	newProjectile.name = "projectileGeneric1";
	newProjectile.pos = initPos + glm::vec3(0.f, 0.f, -1.f);
	newProjectile.pipeline.doLight = false;
	newProjectile.pipeline.isGui = true;
	genericProjectile1.projSprite = newProjectile;
	/*
	for (size_t c = 0; c < numPlatforms; c++) {
		std::string errorStr = std::string("Platform").append(std::to_string(c)).append(std::string(" not found."));
		if (platformArray[c] == nullptr) throw std::runtime_error(errorStr.c_str());
	}
	if (player == nullptr) throw std::runtime_error("Platform not found.");*/


	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();


	//Text
	text = Text(std::string("Play-Regular.ttf"), 128);
	Sprite textboxSprite = scene.spriteLib["textbox"];
	textboxSprite.name = std::string("textbox");
	textboxSprite.pipeline.isGui = true;
	textboxSprite.pipeline.doLight = false;
	textboxSprite.size = glm::vec2(1/160.f);
	textboxSprite.pos = camera->transform->make_local_to_world() * glm::vec4(-0.4f,-0.25f,-1.f,1.f);
	scene.sprites.push_back(textboxSprite);
	prologue = Dialogue("Sources/Dialogues/DIALOGUE_prologue.txt");
	preboss = Dialogue("Sources/Dialogues/DIALOGUE_preboss.txt");
	postboss = Dialogue("Sources/Dialogues/DIALOGUE_postboss.txt");
	predark = Dialogue("Sources/Dialogues/DIALOGUE_predark.txt");
	postdark = Dialogue("Sources/Dialogues/DIALOGUE_postdark.txt");
	tutorial = Dialogue("Sources/Dialogues/DIALOGUE_tutorial.txt");
	

	//Had I done my due dilligence, and wrote a quick quad rendering shader, this would have been far easier. The texture is 4k. But instead I decided
	//To use my 3D shader, and ended up having to fudge the positions to get them to look right. Whoops, lessons learned :/ Sometimes more work is actually less work and better work.

	//Init game data
	vertMovement.currentState = vertMovement.STATE_vertSteady;
	for (auto& sprite : scene.sprites) {
		if (sprite.name == std::string("player")) {
			mech.playerSprite = &sprite;
		}
		else if (sprite.name == std::string("reticle")) {
			mech.reticle = &sprite;
			mech.reticle->pipeline.doLight = false;
			mech.reticle->pipeline.isGui = true;
		}
		else if (sprite.name == std::string("textbox")) {
			textbox = &sprite;
		}
	}

	Sprite reticleControl = scene.spriteLib["ship"];
	reticleControl.name = "reticleControl";
	reticleControl.pos = initPos + glm::vec3(1.f, 0.f, 0.2f);
	scene.sprites.push_back(reticleControl);
	mech.controlReticle = &(scene.sprites.back());
	mech.controlReticle->pipeline.doLight = false;
	mech.controlReticle->pipeline.isGui = true;
	mech.controlReticle->size = glm::vec2(0.5f / 3.f);



	//Add sun and other lights
	Scene::Transform* lightTransform = new Scene::Transform();
	lightTransform->rotation = glm::normalize(glm::angleAxis(-PI_F / 2.f, glm::vec3(0., 0.0f, -1.0f)));
	Scene::Light newLight(lightTransform);
	newLight.type = newLight.Hemisphere;
	newLight.energy = glm::vec3(0.4f);
	scene.lights.push_back(newLight);
	Scene::Transform* lightTransform1 = new Scene::Transform();
	lightTransform1->rotation = glm::normalize(glm::angleAxis(PI_F / 2.f, glm::vec3(1.f, 0.f, 0.f)))
		* glm::normalize(glm::angleAxis(0.3f, glm::vec3(0.f, 1.f, 0.f)));
	Scene::Light newLight1(lightTransform1);
	newLight1.type = newLight1.Directional;
	newLight1.energy = glm::vec3(0.55f, 0.55f, 0.43f);
	scene.lights.push_back(newLight1);
	Scene::Transform* lightTransform2 = new Scene::Transform();
	Scene::Light newLight2(lightTransform2);
	newLight2.type = newLight2.Directional;
	newLight2.energy = glm::vec3(0.2f, 0.2f, 0.17f);
	scene.lights.push_back(newLight2);




	/*Enemy grazaloidJet;
	Enemy envocronMeleePilot;
	Enemy envocronRangedPilot;
	Enemy bossJebb;
	Enemy bossGil;
	Enemy bossDark;*/

	//Define enemies
	grazaloidJet.health = 15.f;
	grazaloidJet.shotCooldown = 10;
	grazaloidJet.projType = Projectile::PROJ_RapidEnemy;
	//TEMP SPRITE:
	grazaloidJet.sprite = scene.spriteLib["grazaloid"];
	grazaloidJet.sprite.name = "enemyGrazaloid";
	grazaloidJet.type = Enemy::ENEMY_grazaJet;
	grazaloidJet.path = std::vector<std::pair<int, glm::vec2>>();
	grazaloidJet.path.push_back(std::make_pair(30, glm::vec2(0.f)));
	grazaloidJet.path.push_back(std::make_pair(30, glm::vec2(0.f)));
	float xOffsetGraz = 2.f;
	grazaloidJet.path.push_back(std::make_pair(30, glm::vec2(xOffsetGraz, 0.f)));
	grazaloidJet.path.push_back(std::make_pair(40, glm::vec2(xOffsetGraz, 0.f)));
	grazaloidJet.path.push_back(std::make_pair(30, glm::vec2(-xOffsetGraz, 0.f)));
	grazaloidJet.path.push_back(std::make_pair(30, glm::vec2(-xOffsetGraz, 0.f)));
	grazaloidJet.shotOffset = glm::vec2(0.f);

	//Hit box offset by 0.1 sprite size on either side to make projectile aiming more forgiving
	grazaloidJet.sprite.hitBoxOff = glm::vec2(-0.1 * grazaloidJet.sprite.width * grazaloidJet.sprite.size.x / SPRITE_SCALE, -0.1 * grazaloidJet.sprite.width * grazaloidJet.sprite.size.y / SPRITE_SCALE);
	grazaloidJet.sprite.hitBoxSize = glm::vec2(1.2f);

	//Both envocron pilots
	envocronMeleePilot.path = std::vector<std::pair<int, glm::vec2>>();
	envocronMeleePilot.path.push_back(std::make_pair(1, glm::vec2(0.0f)));
	envocronMeleePilot.sprite.hitBoxOff = glm::vec2(-0.1 * envocronMeleePilot.sprite.width * envocronMeleePilot.sprite.size.x / SPRITE_SCALE, -0.1 * envocronMeleePilot.sprite.width * envocronMeleePilot.sprite.size.y / SPRITE_SCALE);

	//Hit box offset by 0.1 sprite size on either side 
	envocronMeleePilot.sprite.hitBoxSize = glm::vec2(1.2f);
	envocronMeleePilot.shotOffset = glm::vec2(0.f);
	//For both, enemy cooldown is despawn timer

	//Ranged only
	envocronRangedPilot = envocronMeleePilot;
	envocronRangedPilot.sprite = scene.spriteLib["rangebot"];
	envocronRangedPilot.health = 4.f * grazaloidJet.health;
	envocronRangedPilot.shotCooldown = 40;
	envocronRangedPilot.enemyCooldown = 1200;
	envocronRangedPilot.projType = Projectile::PROJ_SlowEnemy;
	envocronRangedPilot.type = envocronRangedPilot.ENEMY_EnvoPilotRa;
	envocronRangedPilot.sprite.name = "enemyEnvocronRa";

	//Melee only
	envocronMeleePilot.health = 10.f * grazaloidJet.health;
	envocronMeleePilot.sprite = scene.spriteLib["meleebot"];
	envocronMeleePilot.shotCooldown = 200;
	envocronMeleePilot.enemyCooldown = 3000;
	envocronMeleePilot.toPlayerTime = 20;
	envocronMeleePilot.attackTime = ENEMY_MELEE_TIME;
	envocronMeleePilot.returnTime = 30;
	envocronMeleePilot.projType = Projectile::PROJ_MeleeEnemy;
	envocronMeleePilot.type = Enemy::ENEMY_envoPilotMe;
	envocronMeleePilot.sprite.name = "enemyEnvocronMe";



	//Bosses

	bossJebb.health = JEBB_HEALTH;
	bossJebb.shotCooldown = 10;
	bossJebb.enemyCooldown = 600;
	bossJebb.projType = Projectile::PROJ_RapidEnemy;
	//TEMP SPRITE:
	bossJebb.sprite = scene.spriteLib["Jebb"];
	bossJebb.initPos = bossJebb.initPos + glm::vec3(0.15f, 0.f, 0.f);
	bossJebb.sprite.name = "enemyJebb";
	bossJebb.type = Enemy::BOSS_Jebb;
	bossJebb.path = std::vector<std::pair<int, glm::vec2>>();
	bossJebb.path.push_back(std::make_pair(30, glm::vec2(0.f, 2.5f)));
	bossJebb.path.push_back(std::make_pair(30, glm::vec2(0.f, 2.5f)));
	float xOffsetJebb = 7.0f;
	bossJebb.path.push_back(std::make_pair(30, glm::vec2(xOffsetJebb, 2.5f)));
	bossJebb.path.push_back(std::make_pair(30, glm::vec2(xOffsetJebb, 2.5f)));
	bossJebb.path.push_back(std::make_pair(40, glm::vec2(xOffsetJebb, 2.5f + xOffsetJebb / 2.f)));
	bossJebb.path.push_back(std::make_pair(30, glm::vec2(-xOffsetJebb, 2.5f + xOffsetJebb / 2.f)));
	bossJebb.path.push_back(std::make_pair(30, glm::vec2(-xOffsetJebb, 2.5f)));
	bossJebb.path.push_back(std::make_pair(30, glm::vec2(-xOffsetJebb, 2.5f)));
	bossJebb.shotOffset = glm::vec2(0.f);

	bossDark.health = DARK_HEALTH;
	bossDark.initPos += glm::vec3(0.15f, 0.f, 0.f);
	bossDark.shotCooldown = 10;
	bossDark.enemyCooldown = 500;
	bossDark.projType = Projectile::PROJ_RapidEnemy;
	//TEMP SPRITE:
	bossDark.sprite = scene.spriteLib["Dark"];
	bossDark.sprite.name = "enemyDark";
	bossDark.type = Enemy::BOSS_Dark;
	bossDark.path = std::vector<std::pair<int, glm::vec2>>();
	bossDark.path.push_back(std::make_pair(20, glm::vec2(2.5f)));
	bossDark.path.push_back(std::make_pair(20, glm::vec2(2.5f)));
	float xOffsetDark = 8.5f;
	bossDark.path.push_back(std::make_pair(20, glm::vec2(xOffsetDark, 2.5f)));
	bossDark.path.push_back(std::make_pair(20, glm::vec2(xOffsetDark, 2.5f)));
	bossDark.path.push_back(std::make_pair(30, glm::vec2(xOffsetDark, 2.5+ xOffsetDark / 2.f)));
	bossDark.path.push_back(std::make_pair(20, glm::vec2(-xOffsetDark, 2.5 + xOffsetDark / 2.f)));
	bossDark.path.push_back(std::make_pair(20, glm::vec2(-xOffsetDark, 2.5f)));
	bossDark.path.push_back(std::make_pair(20, glm::vec2(-xOffsetDark, 2.5f)));
	bossDark.shotOffset = glm::vec2(0.f);

	
	//All other information defined in updateAllEnemies
	//All spawning to be handled in draw

	//Clouds
	cloudLib = std::vector<Sprite>();
	clouds = std::list<Sprite>();

	//Add all cloud sprites to lib vector
	Sprite newCloud = scene.spriteLib["cloud1"];
	newCloud.name = "cloud1";
	cloudLib.push_back(newCloud);

	//Initalize clouds
	std::array<std::array<bool,C_MAX_PER_LINE>,C_NUM_LINES> cloudMatrix;
	std::array<int,C_NUM_LINES> matrixFill;
	std::array<std::vector<int>, C_NUM_LINES> cloudMatrixInd;
	for (int c = 0; c < C_NUM_LINES; c++) {
		matrixFill[c] = 0;
		for (int cc = 0; cc < C_MAX_PER_LINE; cc++) {
			cloudMatrix[c][cc] = false;
		}
		cloudMatrixInd[c] = std::vector<int>();
	}
	while (curClouds < maxClouds) {
		curClouds++;
		int lineIndex = rand() % C_NUM_LINES;
		while (matrixFill[lineIndex] + 1 >= C_MAX_PER_LINE) {
			lineIndex++;
			if (lineIndex >= C_NUM_LINES) lineIndex = 0;
		}
		matrixFill[lineIndex]++;
		int cloudIndex = rand() % C_MAX_PER_LINE;
		while (cloudMatrix[lineIndex][cloudIndex]) {
			cloudIndex++;
			if (cloudIndex >= maxCloudsPerLine) cloudIndex = 0;
		}
		cloudMatrix[lineIndex][cloudIndex] = true;
		cloudMatrixInd[lineIndex].push_back(cloudIndex);

		
	}

	for (int i = (int) cloudMatrixInd.size() - 1; i >= 0; i--) {
		for (int jl = 0; jl < (int)cloudMatrixInd[i].size(); jl++) {
			int j = cloudMatrixInd[i][jl];
				float xPercentage = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				float zPercentage = (float)i / float(C_NUM_LINES);
				glm::vec3 inCamPos = glm::vec3((maxCloudX - minCloudX) * xPercentage + minCloudX, 0.f, (maxCloudZ - minCloudZ) * zPercentage + minCloudZ + xPercentage*2.f) ;
				Sprite newCloud = cloudLib[rand() % cloudLib.size()];
				newCloud.pos = camera->transform->make_local_to_world() * glm::vec4(inCamPos, 1.f);
				newCloud.pos.z = -2.5f;
				clouds.push_back(newCloud);
		}
	}


	//start music loop playing:
	// (note: position will be over-ridden in update())
	//bg_loop = Sound::loop_3D(*mainMusic, 1.0f, get_player_position(), 10.0f);
}



PlayMode::~PlayMode() {
}

//Enemies
//Makes most sense to hardcode and build enemies by hand, since there won't be many.
void PlayMode::updateAllEnemies() {
	std::vector< std::list<Enemy>::iterator> toDespawn;
	for (auto& iter = enemies.begin(); iter != enemies.end(); iter++) {


		if (iter->alive == false)
			if (iter->framesToDeath <= 0) {
				if (iter->type == iter->BOSS_Jebb) bossJebbDefeated = true;
				else if (iter->type == iter->BOSS_Dark) bossDarkDefeated = true;
				toDespawn.push_back(iter);
			}
			else {
				iter->framesToDeath--;
			}
		else
		{
			if (iter->sprite.pipeline.hitTimer > 0) iter->sprite.pipeline.hitTimer--;
			//Health drop
			std::vector<int> projHitResVec = projectileHit(iter->sprite);
			bool thereWasMelee = false;
			for (int ind = 0; ind < projHitResVec.size(); ind++) {
				int projHitRes = projHitResVec[ind];
				if (projHitRes == Projectile::PROJ_Melee) thereWasMelee = true;
				if ((!iter->inMelee || (iter->inMelee && ((iter->meleeTimer > iter->toPlayerTime + iter->attackTime)))) //Melee enemy has invulnerability to proj unless returning 
					&& projHitRes == Projectile::PROJ_RapidPlayer && iter->sprite.pipeline.hitTimer < 18) {
					iter->health -= rapidPlayerDam;
					iter->sprite.pipeline.hitTimer = iter->sprite.pipeline.hitTime;
				}
				else if ((!iter->inMelee || (iter->inMelee && (iter->meleeTimer < iter->toPlayerTime || iter->meleeTimer > iter->toPlayerTime + iter->attackTime)))
					&& projHitRes == Projectile::PROJ_SlowPlayer && iter->sprite.pipeline.hitTimer == 0) {
					iter->health -= slowPlayerDam;
					iter->sprite.pipeline.hitTimer = iter->sprite.pipeline.hitTime;
				}
				else if ((!iter->inMelee || (iter->inMelee && (iter->meleeTimer < iter->toPlayerTime || iter->meleeTimer > iter->toPlayerTime + iter->attackTime)))
					&& projHitRes == Projectile::PROJ_SlowPlayerReflect && iter->sprite.pipeline.hitTimer == 0) {
					iter->health -= 2.f * slowPlayerDam;
					iter->sprite.pipeline.hitTimer = iter->sprite.pipeline.hitTime;
				}
				else if (projHitRes == Projectile::PROJ_Melee && iter->sprite.pipeline.hitTimer == 0) { //Melee enemy can be hit by melee attacks always
					iter->health -= meleePlayerDam;
					iter->sprite.pipeline.hitTimer = iter->sprite.pipeline.hitTime;
				}
			}
			if (iter->health <= 0.f) {
				iter->alive = false;
				iter->sprite.pipeline.allRed = true;
			}
			else {//AI
				//Movement:
				int nextSegment = iter->segment;
				//Update path
				if (!iter->inDodge && !iter->inMelee) {
					iter->framesInSegment--;
					if (iter->framesInSegment == 0) {
						iter->segment++;
						if (iter->segment == iter->path.size()) iter->segment = 0;
						iter->framesInSegment = iter->path[iter->segment].first;
					}
					nextSegment = iter->segment + 1;
					if (nextSegment >= iter->path.size()) nextSegment = 0;
				}
				//Interpolate between segment starting positions based on percentage of current segement traveled
				float offset = (float)iter->framesInSegment / (float)iter->path[iter->segment].first;
				glm::vec2 posOffset = glm::vec2(offset) * iter->path[iter->segment].second + glm::vec2(1.f - offset) * iter->path[nextSegment].second;
				glm::vec4 cameraPosModified = glm::vec4(camera->transform->make_world_to_local() * glm::vec4(iter->initPos, 1.f) + glm::vec3(posOffset, 0.f), 1.f);
				if(!iter->inMelee)iter->sprite.pos = camera->transform->make_local_to_world() * cameraPosModified;
				//Note pos is always offset along with the camera movement, so always based on initPos, which is built when adding to spawned list
				
				//Atacking:
				switch (iter->type)
				{
				case(Enemy::ENEMY_grazaJet):
					iter->shotTimer++;
					if (iter->shotTimer == iter->shotCooldown) { //Create rapid projectile in direction of movement
						Projectile newProj = genericProjectile1;
						newProj.projSprite.pipeline.setAnimation("projectileEnRapid");
						newProj.type = Projectile::PROJ_RapidEnemy;
						newProj.projSprite.pos = iter->sprite.pos;
						newProj.motionVector = normalize(camera->transform->make_local_to_world()*(enemProjSpeed*glm::vec4(0.f,0.f,1.0f,-1.f)));
						projectiles.push_back(newProj);
						iter->shotTimer = 0;
					}
					//"Dodge"
					glm::vec3 cameraPos = camera->transform->make_world_to_local()*glm::vec4(iter->sprite.pos,1.f);
					glm::vec3 cameraMech = camera->transform->make_world_to_local() * glm::vec4(mech.playerSprite->pos, 1.f);
					if (iter->dodgeTime == 0 && abs(cameraPos.x / abs(cameraPos.z) - cameraMech.x / abs(cameraMech.z)) <= tan(camera->fovy) * camera->aspect * 0.3f
						&& abs(cameraPos.y / abs(cameraPos.z) - cameraMech.y / abs(cameraMech.z)) <= tan(camera->fovy)  * 0.1f) {
						iter->inDodge = true;
						if (cameraPos.x > 0) {
							iter->posDodge = true;
						}
						else {
							iter->posDodge = false;
						}
					}
					if(iter->inDodge && iter->dodgeTime < iter->dodgeCount){ //If projected x position of player and enemy "collide" (within a range )
						glm::vec3 difVec = glm::vec3(0.f);
						if (iter->posDodge) { //Move towards the center of the screen on the x axis by 5%
							cameraPos.x -= tan(camera->fovy) * camera->aspect * 0.1f * abs(cameraPos.z) / (float)iter->dodgeCount;
							difVec.x -= tan(camera->fovy) * camera->aspect * 0.1f * abs(cameraPos.z) / (float)iter->dodgeCount;
						}
						else{
							difVec.x += tan(camera->fovy) * camera->aspect * 0.1f * abs(cameraPos.z) / (float)iter->dodgeCount;
						}
						iter->dodgeTime++;
						iter->initPos += camera->transform->rotation * difVec;
					}
					else if (iter->dodgeTime >= iter->dodgeCount) { //Reset doge variables
						iter->dodgeTime = 0;
						iter->inDodge = false;
					}
					break;
				case(Enemy::ENEMY_envoPilotMe):
					if (!iter->inMelee) {
						iter->shotTimer++;
					}
					iter->enemyTimer++;

					if (iter->enemyTimer > iter->enemyCooldown) { //Enemy will exit stage after cooldown # of frames
						if (iter->enemyTimer > iter->enemyCooldown + 30) {
							toDespawn.push_back(iter);
						}
						else {
							iter->initPos += glm::vec3(0.f, 0.f, 0.1f);
						}
					}
					else if (thereWasMelee && iter->shotTimer > iter->shotCooldown && mech.meleeTimer > 0 && iter->meleeTimer < iter->toPlayerTime + iter->attackTime) { //Any melee attack
						if(iter->meleeTimer == 0) iter->startPos = iter->sprite.pos; //From the player should cancel a melee enemy
						iter->meleeTimer = iter->toPlayerTime + iter->attackTime;
						iter->inMelee = true;
						iter->attackPos = iter->sprite.pos;
						if(iter->sprite.pipeline.currentAnimation != "SpikebotFlyingIdle") iter->sprite.pipeline.setAnimation("SpikebotFlyingIdle");
					}
					else if(iter->shotTimer > iter->shotCooldown){ //If the melee enemy is actively attacking
						if (iter->meleeTimer == 0) { //Initialize attack

							iter->sprite.pipeline.setAnimation("SpikebotMeleeAttack");
							iter->startPos = iter->sprite.pos;
							iter->inMelee = true;
							iter->targetPos = offsetPlayerPos;// +glm::vec3(0.f, 0.5f, 0.0f);
						}
						if (iter->meleeTimer < iter->toPlayerTime) { //First phase. Move towards players init position
							iter->sprite.pos = (iter->targetPos - iter->startPos) * ((float)iter->meleeTimer / iter->toPlayerTime) + iter->startPos;
						}
						else if (iter->meleeTimer == iter->toPlayerTime) { //Once the player is reached, set up attack pos and create attack proj
							iter->attackPos = iter->sprite.pos;
							Projectile newProj = genericProjectile1;
							newProj.type = Projectile::PROJ_MeleeEnemy;
							newProj.projSprite.pos = iter->sprite.pos;
							newProj.motionVector = glm::vec3(0.f);
							newProj.projSprite.doDraw = false;
							newProj.meleeTimer = 0;
							projectiles.push_back(newProj);
						}
						else if (iter->meleeTimer < iter->toPlayerTime + iter->attackTime) { 
							iter->sprite.pos = iter->attackPos;
						}
						else { //Once attack finishes, return to initial positon
							float soFar = (float)(iter->meleeTimer - iter->toPlayerTime - iter->attackTime);
							float percentageTraveled = soFar / (float)(iter->returnTime);
							iter->sprite.pos = (iter->startPos - iter->attackPos) * percentageTraveled + iter->attackPos;
						}
						iter->meleeTimer++;
						if (iter->meleeTimer == iter->returnTime + iter->toPlayerTime + iter->attackTime) { //Reset variables at end of attack
							iter->shotTimer = 0;
							iter->meleeTimer = 0;
							iter->inMelee = false;
							if (iter->sprite.pipeline.currentAnimation != "SpikebotFlyingIdle") iter->sprite.pipeline.setAnimation("SpikebotFlyingIdle");
						}
					}
					break;
				case(Enemy::ENEMY_EnvoPilotRa):
					iter->enemyTimer++;
					if (iter->enemyTimer > iter->enemyCooldown) { //Enemy will exit stage after cooldown # of frames
						if (iter->enemyTimer > iter->enemyCooldown + 30) {
							toDespawn.push_back(iter);
						}
						else {
							iter->initPos += glm::vec3(0.f, 0.f, 0.1f);
						}
					}
					else {
						iter->shotTimer++;
						if (iter->shotTimer == iter->shotCooldown) { //Create slow projectile that targets player
							Projectile newProj = genericProjectile1;
							newProj.type = Projectile::PROJ_SlowEnemy;
							newProj.projSprite.pos = iter->sprite.pos + camera->transform->rotation*glm::vec3(glm::vec2(iter->sprite.size.x *iter->sprite.width /SPRITE_SCALE,
								iter->sprite.size.y * iter->sprite.height / SPRITE_SCALE)/2.f,0.f) ;
							newProj.motionVector = normalize(enemProjSpeed*normalize(offsetPlayerPos + glm::vec3(0.4f) - newProj.projSprite.pos));
							projectiles.push_back(newProj);
							iter->shotTimer = 0;
						}

					}
					break;
				case(Enemy::BOSS_Jebb):
					iter->shotTimer++;
					iter->enemyTimer++;
					if (iter->shotTimer == iter->shotCooldown) { //Create 2 rapid projectiles in direction of player
						Projectile newProj = genericProjectile1;
						newProj.projSprite.pipeline.setAnimation("projectileEnRapid");
						newProj.type = Projectile::PROJ_RapidEnemy;
						newProj.projSprite.pos = iter->sprite.pos;
						newProj.motionVector = normalize(enemProjSpeed * normalize(mech.playerSprite->pos + glm::vec3(0.4f) - iter->sprite.pos));
						projectiles.push_back(newProj);
						newProj.projSprite.pos = iter->sprite.pos + camera->transform->rotation * glm::vec3(iter->sprite.width * iter->sprite.size.x / SPRITE_SCALE, 0.f, 0.f);
						newProj.motionVector = normalize(enemProjSpeed * normalize(mech.playerSprite->pos + glm::vec3(0.4f) - newProj.projSprite.pos));
						projectiles.push_back(newProj);
						iter->shotTimer = 0;
					}
					if (iter->enemyTimer == iter->enemyCooldown) { //randomly create satillite enemy
						iter->enemyTimer = 0;
						float randomEnemy = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
						if (randomEnemy < 0.3f) {
							Enemy newEnemy = grazaloidJet;
							newEnemy.initPos = mech.playerSprite->pos + camera->transform->rotation * glm::vec3(-2.f, 0.f, -20.f);
							newEnemy.segment = 0;
							newEnemy.framesInSegment = newEnemy.path[0].first;
							enemies.push_back(newEnemy);
						}
						else if (randomEnemy < 0.6f) {
							Enemy newEnemy = envocronMeleePilot;
							newEnemy.initPos = mech.playerSprite->pos + camera->transform->rotation * glm::vec3(2.f, 0.f, -20.f);
							newEnemy.segment = 0;
							newEnemy.framesInSegment = newEnemy.path[0].first;
							enemies.push_back(newEnemy);

						}
						else {
							Enemy newEnemy = envocronRangedPilot;
							newEnemy.initPos = mech.playerSprite->pos + camera->transform->rotation * glm::vec3(2.f, 1.f, -20.f);
							newEnemy.segment = 0;
							newEnemy.framesInSegment = newEnemy.path[0].first;
							enemies.push_back(newEnemy);
						}
					}
					break;
				case(Enemy::BOSS_Dark):
					iter->shotTimer++;
					iter->enemyTimer++;
					if (iter->shotTimer == iter->shotCooldown) { //Create 2 rapid projectiles in direction of player
						Projectile newProj = genericProjectile1;
						newProj.projSprite.pipeline.setAnimation("projectileDark");
						newProj.type = Projectile::PROJ_RapidEnemy;
						newProj.projSprite.pos = iter->sprite.pos;
						newProj.motionVector = normalize(enemProjSpeed * normalize(mech.playerSprite->pos + glm::vec3(0.4f) - iter->sprite.pos));
						projectiles.push_back(newProj);
						newProj.projSprite.pos = iter->sprite.pos + camera->transform->rotation * glm::vec3(iter->sprite.width * iter->sprite.size.x / SPRITE_SCALE, 0.f, 0.f);
						newProj.motionVector = normalize(enemProjSpeed * normalize(mech.playerSprite->pos + glm::vec3(0.4f) - newProj.projSprite.pos));
						projectiles.push_back(newProj);
						iter->shotTimer = 0;
					}
					if (iter->enemyTimer == iter->enemyCooldown) { //randomly create satillite enemy
						iter->enemyTimer = 0;
						float randomEnemy = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
						if (randomEnemy < 0.3f) {
							Enemy newEnemy = grazaloidJet;
							newEnemy.initPos = mech.playerSprite->pos + camera->transform->rotation * glm::vec3(-2.f, 0.f, -20.f);
							newEnemy.segment = 0;
							newEnemy.framesInSegment = newEnemy.path[0].first;
							enemies.push_back(newEnemy);
						}
						else if (randomEnemy < 0.6f) {
							Enemy newEnemy = envocronMeleePilot;
							newEnemy.initPos = mech.playerSprite->pos + camera->transform->rotation * glm::vec3(2.f, 0.f, -20.f);
							newEnemy.segment = 0;
							newEnemy.framesInSegment = newEnemy.path[0].first;
							enemies.push_back(newEnemy);

						}
						else {
							Enemy newEnemy = envocronRangedPilot;
							newEnemy.initPos = mech.playerSprite->pos + camera->transform->rotation * glm::vec3(2.f, 1.f, -20.f);
							newEnemy.segment = 0;
							newEnemy.framesInSegment = newEnemy.path[0].first;
							enemies.push_back(newEnemy);
						}
					}
					break;
				default:
					break;
				}

			}
		}
	}
	for (int ind = 0; ind < toDespawn.size(); ind++) {
		enemies.erase(toDespawn[ind]);
	}
}

void PlayMode::addEnemyCheck(int ID, glm::vec3 worldPos, float spawnDist) { //Adds enemy to generic toSpawn list
	bool isEmpty = false;
	if (toSpawn.size() == 0) isEmpty = true;
	EnemySpawn newSpawn;
	newSpawn.ID = ID;
	newSpawn.worldPos = worldPos;
	newSpawn.spawnDist = spawnDist;
	toSpawn.push_back(newSpawn);
	if (isEmpty) curSpawnCheck = toSpawn.begin();
}

void PlayMode::spawnEnemies() { //Spawn all enemies that are within spawnDist distance from player at current frame
	bool stillSpawn = true;
	while (stillSpawn && curSpawnCheck != toSpawn.end()) {
		EnemySpawn curCheck = *curSpawnCheck;
		Enemy newEnemy;
		if (distance(curCheck.worldPos, mech.playerSprite->pos) <= curCheck.spawnDist) {
			switch (curCheck.ID)
			{
			case(Enemy::ENEMY_grazaJet):
				newEnemy = grazaloidJet;
				newEnemy.initPos = (curCheck.worldPos);
				newEnemy.segment = 0;
				newEnemy.framesInSegment = newEnemy.path[0].first;
				enemies.push_back(newEnemy);
				curSpawnCheck++;
				break;
			case(Enemy::ENEMY_envoPilotMe):
				newEnemy = envocronMeleePilot;
				newEnemy.initPos = (curCheck.worldPos);
				newEnemy.segment = 0;
				newEnemy.framesInSegment = newEnemy.path[0].first;
				enemies.push_back(newEnemy);
				curSpawnCheck++;
				break;
			case(Enemy::ENEMY_EnvoPilotRa):
				newEnemy = envocronRangedPilot;
				newEnemy.initPos = (curCheck.worldPos);
				newEnemy.segment = 0;
				newEnemy.framesInSegment = newEnemy.path[0].first;
				enemies.push_back(newEnemy);
				curSpawnCheck++;
				break;
			default:
				stillSpawn = false;
				break;
			}
		}
		else stillSpawn = false;
	}
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
			if (!pauseMode && !isdialogueDisplay) {
				SDL_SetRelativeMouseMode(SDL_FALSE);
				pauseMode = true;
			}
			else if (pauseMode && !isdialogueDisplay) {
				pauseMode = false;
				SDL_SetRelativeMouseMode(SDL_TRUE);
			}
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			if (isdialogueDisplay && !left.pressed && !curDisplay->atStart()) curDisplay->lastLine();
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			if (isdialogueDisplay && !right.pressed && curDisplay->atEnd()) {
				dialogueEnd = true;
			}
			if (isdialogueDisplay && !right.pressed) curDisplay->nextLine();
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
		else if (evt.key.keysym.sym == SDLK_y) {
			if (fullscreen) {
				fullscreen = false;
				SDL_SetWindowFullscreen(window, 0);
			}
			else {
				fullscreen = true;
				SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
			}
			return true;
		}
		
		else if (evt.key.keysym.sym == SDLK_RSHIFT) {
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_LSHIFT) {
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_LEFT) {
			arrowleft.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_RIGHT) {
			arrowright.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_UP) {
			arrowup.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_DOWN) {
			arrowdown.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.downs += 1;
			space.pressed = true;
			leftfire.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_t) {
			if (!T.pressed && isdialogueDisplay) toggleTextBox();
			T.pressed = true;
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
		else if (evt.key.keysym.sym == SDLK_i) {
			Mode::current = nullptr;
			in.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_e) {
			out.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = false;
			leftfire.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_LSHIFT) {
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_LEFT) {
			arrowleft.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_RIGHT) {
			arrowright.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_UP) {
			arrowup.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_DOWN) {
			arrowdown.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_t) {
			T.pressed = false;
			return true;
		}
	}
	else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (evt.button.button == SDL_BUTTON_LEFT) rightfire.pressed = true;
		else if (evt.button.button == SDL_BUTTON_RIGHT) {
			melee.pressed = true;
		}
		else if (evt.button.button == SDL_BUTTON_RIGHT) melee.pressed = true;
		return true;
	}
	else if (evt.type == SDL_MOUSEBUTTONUP) {
		if (evt.button.button == SDL_BUTTON_LEFT) rightfire.pressed = false;
		else if (evt.button.button == SDL_BUTTON_RIGHT) {
			melee.pressed = false;
			melee.unpressed = true;
		}
		return true;
	}
	else if (evt.type == SDL_MOUSEMOTION)
	{
		if (SDL_GetRelativeMouseMode() == SDL_TRUE)
		{
			SDL_WarpMouseInWindow(window, window_size.x / 2, window_size.y / 2); //Allows mouse to not get caught on window edge
		}
		return true;
	}

	return false;
}


//Projectiles

void PlayMode::createProjectileMech(int type, glm::vec3 motionVec, glm::vec3 playerPos) {
	Projectile newProj = genericProjectile1;
	if (type == Projectile::PROJ_RapidPlayer) {
		newProj.projSprite.pipeline.setAnimation(std::string("projectilePlayerRapid"));
	}
	else if(type == Projectile::PROJ_SlowPlayer){
		newProj.projSprite.pipeline.setAnimation("projectilePlayerSlow");
	}
	else {
		newProj.projSprite.doDraw = false;
	}
	newProj.type = type;
	if (type == Projectile::PROJ_Melee) {
		newProj.projSprite.pos = playerPos;
		newProj.motionVector = motionVec;
		newProj.projSprite.name = "projectileMelee";
		mech.reflect = projectiles.end();
	}
	else {
		newProj.projSprite.pos = playerPos + normalize(motionVec) * 0.01f;
		newProj.motionVector = normalize(motionVec);
	}
	projectiles.push_back(newProj);
}

void PlayMode::updateProjectiles(float elapsed) {
	if (pauseMode && !isdialogueDisplay) return;
	std::vector< std::list<Projectile>::iterator> delIterators = std::vector<std::list<Projectile>::iterator>();
	for (std::list<Projectile>::iterator it = projectiles.begin(); it != projectiles.end(); it++) {
		Projectile proj = *it;
		it->projSprite.pos += proj.motionVector * elapsed * projSpeed;
		Scene::Camera cam = scene.cameras.front();
		float projCameraDepth = (cam.transform->make_world_to_local() * glm::vec4(proj.projSprite.pos, 1.f)).z;
		if (projCameraDepth < -projCutoff || projCameraDepth > 1.f) {
			delIterators.push_back(it);
		}
		else if (it->type == it->PROJ_MeleeEnemy) {
			it->meleeTimer++;
			if (it->meleeTimer > it->meleeTime) delIterators.push_back(it);
		}
		else if (it->type == it->PROJ_Melee) {
			if (mech.meleeTimer == 0) delIterators.push_back(it);
			else projectileHit(it->projSprite);
		}
		if (mech.meleeTimer != 0 && mech.reflect != projectiles.end()) {
			mech.reflect->projSprite.pipeline.currentAnimation = "projectilePlayerSlow";
			mech.reflectDef = true;
			mech.reflect->type = Projectile::PROJ_SlowPlayerReflect;
			mech.reflect->motionVector.y = -mech.reflect->motionVector.y;
			mech.reflect = projectiles.end();
		}
	}
	for (int itInd = 0; itInd < delIterators.size(); itInd++) {
		projectiles.erase(delIterators[itInd]);
	}

}

inline bool vecComp(float spritePos, float projPos, float spriteWidth, float projWidth) {
	return abs(spritePos - projPos) <= spriteWidth || abs(spritePos - projPos) <= projWidth;
};

int PlayMode::projInter(Projectile& proj, Sprite hitSprite, float spriteHeight, float spriteWidth, glm::vec3 spritePos){
	
	glm::vec3 projPos = scene.cameras.front().transform->make_world_to_local() * glm::vec4(proj.projSprite.pos, 1.f) + glm::vec3(proj.projSprite.hitBoxOff,0.f);
	

	float projWidth = proj.projSprite.width * proj.projSprite.size.x / SPRITE_SCALE / -projPos.z * proj.projSprite.hitBoxOff.x;
	float projHeight = proj.projSprite.height * proj.projSprite.size.y / SPRITE_SCALE / -projPos.z * proj.projSprite.hitBoxOff.y;
	if (abs(projPos.z - spritePos.z) < projZDelta || (hitSprite.name == "projectileMelee" && abs( spritePos.z - projPos.z) < reflectZDelta && spritePos.z >= projPos.z)) {

		if (vecComp(spritePos.x, projPos.x, spriteWidth, projWidth) && vecComp(spritePos.y, projPos.y, spriteHeight, projHeight)) {
			return proj.type;
		}
		else if (hitSprite.name == "projectileMelee" && vecComp(spritePos.x, projPos.x, 2.f*spriteWidth, 2.f * projWidth) && vecComp(spritePos.y, projPos.y, 2.f * spriteHeight, 2.f * projHeight)) {
			return proj.type;
		}
	}
	return -1;
};


std::vector<int> PlayMode::projectileHit(Sprite hitSprite) {

	
	glm::vec3 spritePos = scene.cameras.front().transform->make_world_to_local() * glm::vec4(hitSprite.pos,1.f) + glm::vec3(hitSprite.hitBoxOff, 0.f);
	float spriteWidth = hitSprite.width * hitSprite.size.x / SPRITE_SCALE * hitSprite.hitBoxSize.x;
	float spriteHeight = hitSprite.height * hitSprite.size.y / SPRITE_SCALE * hitSprite.hitBoxSize.y;
	std::vector<int> ret = std::vector<int>();
	for (auto& projIt = projectiles.begin(); projIt != projectiles.end(); projIt++) {
		if (hitSprite.name != "projectileMelee") {
			if (hitSprite.name != "player" && ret.size() > 1) return ret;
			else if (hitSprite.name == "player" && ret.size() > 4) return ret;
		}
		Projectile& proj = *projIt;
		if((proj.type == Projectile::PROJ_RapidPlayer || proj.type == Projectile::PROJ_SlowPlayer || proj.type == Projectile::PROJ_SlowPlayerReflect || proj.type == Projectile::PROJ_Melee)
			&& ( hitSprite.name.size() > 5 && hitSprite.name.substr(0,5) == std::string("enemy"))) {
			int temp = projInter(proj, hitSprite, spriteHeight, spriteWidth, spritePos);
			if (temp >= 0) ret.push_back( temp);
		}
		else if ((proj.type == Projectile::PROJ_RapidEnemy || proj.type ==
			Projectile::PROJ_SlowEnemy || proj.type == Projectile::PROJ_MeleeEnemy) && (hitSprite.name == "player")) {
			int temp = projInter(proj, hitSprite, spriteHeight, spriteWidth, spritePos);
			if (temp >= 0) ret.push_back( temp);
		}
		else if (( proj.type == Projectile::PROJ_SlowEnemy) && (hitSprite.name == "projectileMelee")) {
			int temp = projInter(proj, hitSprite, spriteHeight, spriteWidth, spritePos);
			if (temp >= 0) {
				mech.reflect = projIt;
				ret.push_back( temp);
			}
		}
	}
	return ret;
}

//State machines:
/*

//The following are for horizontal movement and for animation
				STATE_horiSteady,
				STATE_left,
				STATE_right,
				STATE_leftToRight,
				STATE_rightToLeft,

				STATE_up,
				STATE_down,
				STATE_vertSteady
*/
void PlayMode::StateMachine::transitionMachine( int from, int to) {
	if (from == STATE_horiSteady && to == STATE_left) {
		inTransition = false;
		currentState = STATE_left;

	}
	else if (from == STATE_horiSteady && to == STATE_right) {
		inTransition = false;
		currentState = STATE_right;
	}
	else if (from == STATE_left && to == STATE_right) {
		if (!inTransition || transitionState != STATE_leftToRight) {
			inTransition = true;
			currentState = STATE_left;
			nextState = STATE_right;
			transitionState = STATE_leftToRight;

			framesDuring = 0;
			framesTo = 12;
		}
	}
	else if (from == STATE_left && to == STATE_horiSteady) {
		if (!inTransition || transitionState != STATE_leftToSteady) {
			inTransition = true;
			currentState = STATE_left;
			nextState = STATE_horiSteady;
			transitionState = STATE_leftToSteady;

			framesDuring = 0;
			framesTo = 3;
		}
	}
	else if (from == STATE_right && to == STATE_left) {
		if (!inTransition || transitionState != STATE_rightToLeft) {
			inTransition = true;
			currentState = STATE_right;
			nextState = STATE_left;
			transitionState = STATE_rightToLeft;

			framesDuring = 0;
			framesTo = 12;
		}
		
	}
	else if (from == STATE_right && to == STATE_horiSteady) {
		if (!inTransition || transitionState != STATE_rightToSteady) {
			inTransition = true;
			currentState = STATE_right;
			nextState = STATE_horiSteady;
			transitionState = STATE_rightToSteady;

			framesDuring = 0;
			framesTo = 3;
		}

	}
	else if (from == STATE_up && to == STATE_vertSteady) {
		inTransition = false;
		currentState = STATE_vertSteady;
	}
	else if (from == STATE_down && to == STATE_vertSteady) {
		inTransition = false;
		currentState = STATE_vertSteady;
	}
	else if (from == STATE_vertSteady && to == STATE_up) {
		inTransition = false;
		currentState = STATE_up;
	}
	else if (from == STATE_vertSteady && to == STATE_down) {
		inTransition = false;
		currentState = STATE_down;
	}
	//Of note. Even if in transition, if currentState is either, and to is steady state, will still work


}

void PlayMode::StateMachine::updateStateMachine() {
	if (inTransition == true) {
		framesDuring++;
		if (framesDuring == framesTo) {
			inTransition = false;
			currentState = nextState;
			framesDuring = 0;
		}
	}
}

void PlayMode::printMotionState() {

	std::cout << "STATES: \n";
	std::cout << "Verti: ";
	if (vertMovement.inTransition) {
		std::cout << "transition\n";
		std::cout << "Steady\n";

	}
	else {
		switch (vertMovement.currentState)
		{
		case(vertMovement.STATE_up):
			std::cout << "Up\n";
			break;
		case(vertMovement.STATE_down):
			std::cout << "Down\n";
			break;
		default:
			std::cout << "Steady\n";
		}
	}
	std::cout << "Hori: ";
	if (horiMovement.inTransition) {
		std::cout << "transition\n";
		switch (horiMovement.transitionState)
		{
		case(horiMovement.STATE_leftToSteady):
			std::cout << "Left to steady\n";
			break;
		case(horiMovement.STATE_rightToSteady):
			std::cout << "Right to steady\n";
			break;
		case(horiMovement.STATE_leftToRight):
			std::cout << "Left to right\n";
			break;
		case(horiMovement.STATE_rightToLeft):
			std::cout << "Right to left\n";
			break;
		default:
			std::cout << "Steady\n";
		}
	}
	else {
		switch (horiMovement.currentState)
		{
		case(horiMovement.STATE_left):
			std::cout << "Left\n";
			break;
		case(horiMovement.STATE_right):
			std::cout << "Right\n";
			break;
		default:
			std::cout << "Steady\n";
		}
	}

}

void PlayMode::offsetObjects() {
	glm::vec3 posOffsetDelta = level.curoffset();
	mech.playerSprite->pos += level.posOffsetDelta;
	mech.reticle->pos += level.posOffsetDelta;
	mech.controlReticle->pos += level.posOffsetDelta;
	textbox->pos += level.posOffsetDelta;
	camera->transform->position += level.posOffsetDelta;
	for (auto& iter : enemies) {
		if(iter.alive) iter.initPos += level.posOffsetDelta;
		if (iter.alive && iter.inMelee) {
			iter.attackPos += level.posOffsetDelta;
			iter.startPos += level.posOffsetDelta;
			iter.targetPos += level.posOffsetDelta;
		}
	}
	for (auto& iter : projectiles) {
		iter.projSprite.pos += level.posOffsetDelta;
	}
}

void PlayMode::updateClouds() {



	
	std::vector<std::list<Sprite>::iterator> despawnClouds = std::vector<std::list<Sprite>::iterator>();
	glm::vec3 posOffsetDelta = level.curoffset();
	for (std::list<Sprite>::iterator iter = clouds.begin(); iter != clouds.end(); iter++) {
		if (!pauseMode &&  !level.bossCheck()) iter->pos += level.posOffsetDelta;
		glm::vec3 offsetCamera = camera->transform->make_world_to_local() * glm::vec4(iter->pos, 1.f) + glm::vec3(0.f, 0.f, cloudSpeed);
		iter->pos = camera->transform->make_local_to_world() * glm::vec4(offsetCamera, 1.0f);
	}



	bool limitReached = false;
	std::list<Sprite>::iterator iter = clouds.begin();
	while (!limitReached) {
		glm::vec3 offsetCamera = camera->transform->make_world_to_local() * glm::vec4(iter->pos, 1.f);
		if (offsetCamera.z > maxCloudZ) {
			float xPercentage = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			float x = xPercentage * (maxCloudX - minCloudX) + minCloudX;
			glm::vec3 inCamPos = glm::vec3(x, 0.f, minCloudZ + 2*xPercentage);
			iter->pos = camera->transform->make_local_to_world() * glm::vec4(inCamPos, 1.f );
			iter->pos.z = -2.5;
			Sprite newCloud = *iter;
			std::list<Sprite>::iterator delIter = iter;
			iter++;
			if (iter == clouds.end()) limitReached = true;
			clouds.erase(delIter);
			clouds.push_back(newCloud);
		}
		else limitReached = true;
		if(!limitReached){
			if (iter == clouds.end()) limitReached = true;
		}
	}
}

void PlayMode::updateMechAnimation() {
	std::string nextAnimation;
	switch (horiMovement.currentState)
	{
	case(StateMachine::STATE_left):
		nextAnimation = "ZivMechFlyingLeft";
		break;
	case(StateMachine::STATE_right):
		nextAnimation = "ZivMechFlyingRight";
		break;
	default:
		nextAnimation = "ZivMechFlying";
		break;
	}
	if (mech.meleeTimer > 0) nextAnimation = "ZivMechAttack";
	else if (horiMovement.inTransition) nextAnimation = "ZivMechFlying";
	if (nextAnimation != mech.playerSprite->pipeline.currentAnimation) mech.playerSprite->pipeline.setAnimation(nextAnimation);
}

void PlayMode::displayDialogue() {
	isdialogueDisplay = true;
	textBoxDisplay = true;
	while (!curDisplay->atStart()) curDisplay->lastLine();
	pauseMode = true;
	dialogueStart = true;
}

void PlayMode::updatestage(float elapsed){
	switch (currentstage)
	{
	case(gameplaystage::gs_init):
		SDL_SetRelativeMouseMode(SDL_TRUE);
		if (!dialogueStart) {
			curDisplay = &tutorial;
			currentstage = gameplaystage::gs_tutorial;
			displayDialogue();
		}
		break;
	case(gameplaystage::gs_tutorial):
		if (dialogueEnd) {
			dialogueEnd = false;
			curDisplay = &prologue;
			currentstage = gameplaystage::gs_prologue;
			displayDialogue();
		}
		break;
	case(gameplaystage::gs_prologue):
		if (dialogueEnd) {
			dialogueEnd = false;
			dialogueStart = false;
			pauseMode = false;
			isdialogueDisplay = false;
			continueGame = true;
			currentstage = gameplaystage::gs_level;
		}
		break;
	case(gameplaystage::gs_level):
		if (!dialogueStart && level.bossCheck()) {
			curDisplay = &preboss;
			currentstage = gameplaystage::gs_preboss;
			displayDialogue();
		}
		break;
	case(gameplaystage::gs_preboss):
		if (dialogueEnd && !bossJebbSpawned) {
			dialogueEnd = false;
			dialogueStart = false;
			pauseMode = false;
			isdialogueDisplay = false;
			continueGame = true;
			currentstage = gameplaystage::gs_boss;
			bossJebbSpawned = true;
			Enemy newEnemy = bossJebb;
			newEnemy.initPos = mech.playerSprite->pos + camera->transform->rotation * glm::vec3(-bossJebb.sprite.width * bossJebb.sprite.size.x / 2.f / SPRITE_SCALE, -bossJebb.sprite.height * bossJebb.sprite.size.y / 2.f / SPRITE_SCALE, -50.f);
			newEnemy.segment = 0;
			newEnemy.framesInSegment = newEnemy.path[0].first;
			enemies.push_back(newEnemy);
			bossJebbActive = &enemies.back();
		}
		break;
	case(gameplaystage::gs_boss):
		if (bossJebbDefeated) {
			if (bosstime < highscore) {
				currentstage = gameplaystage::gs_predark;
				if (!dialogueStart) {
					curDisplay = &predark;
					displayDialogue();
				}
			}
			else {
				if (!dialogueStart) {
					curDisplay = &postboss;
					currentstage = gameplaystage::gs_postboss;
					displayDialogue();
				}
			}
		}
		else bosstime += elapsed;
		break;
	case(gameplaystage::gs_postboss):
		if (dialogueEnd) {
			dialogueEnd = false;
			dialogueStart = false;
			win = true;
			isdialogueDisplay = false;
		}
		break;

	case(gameplaystage::gs_predark):
		if (dialogueEnd && !bossDarkSpawned) {
			dialogueEnd = false;
			dialogueStart = false;
			pauseMode = false;
			isdialogueDisplay = false;
			continueGame = true;
			currentstage = gameplaystage::gs_dark;
			bossDarkSpawned = true;
			Enemy newEnemy = bossDark;
			mech.health = mech.maxHealth;
			newEnemy.initPos = mech.playerSprite->pos + camera->transform->rotation * glm::vec3(-bossDark.sprite.width * bossDark.sprite.size.x / 2.f / SPRITE_SCALE, -bossDark.sprite.height * bossDark.sprite.size.y / 2.f / SPRITE_SCALE, -50.f);
			newEnemy.segment = 0;
			newEnemy.framesInSegment = newEnemy.path[0].first;
			enemies.push_back(newEnemy);
			bossDarkActive = &enemies.back();
		}
		break;
	case(gameplaystage::gs_dark):
		if (bossDarkDefeated) {
			
			if (!dialogueStart) {
				curDisplay = &postdark;
				currentstage = gameplaystage::gs_postdark;
				displayDialogue();
			}
		}
		else bosstime += elapsed;
		break;
	case(gameplaystage::gs_postdark):
		if (dialogueEnd) {
			dialogueEnd = false;
			dialogueStart = false;
			win = true;
			isdialogueDisplay = false;
		}
		break;
	default:
		break;
	}
}

void PlayMode::resetAll() {
	resetGame();
	currentstage = gameplaystage::gs_init;
}

void PlayMode::resetGame() {
	curSpawnCheck = toSpawn.begin();
	std::list<Enemy>::iterator iter = enemies.begin();
	while (iter != enemies.end()) {
		std::list<Enemy>::iterator despawn = iter;
		iter++;
		enemies.erase(despawn);
	}

	std::list<Projectile>::iterator proj = projectiles.begin();
	while (proj != projectiles.end()) {
		std::list<Projectile>::iterator despawn = proj;
		proj++;
		projectiles.erase(despawn);
	}
	mech.health = mech.maxHealth;
	bosstime = 0.f;
	currentstage = gs_level;
	win = false;
	bossJebbSpawned = false;
	dialogueEnd = false;
	dialogueStart = false;
	pauseMode = false;
	isdialogueDisplay = false;
	continueGame = true;
	currentstage = gameplaystage::gs_level;
	bossDarkDefeated = false;
	bossJebbDefeated = false;
	bossDarkSpawned = false;
	bossDark.alive = true;
	bossDark.health = DARK_HEALTH;
	bossJebb.alive = true;
	bossJebb.health = JEBB_HEALTH;
	bossJebb.framesToDeath = 30;
	bossDark.framesToDeath = 30;

	mech.playerSprite->pos -= level.totalOffset;
	mech.reticle->pos -= level.totalOffset;
	mech.controlReticle->pos -= level.totalOffset;
	textbox->pos -= level.totalOffset;
	camera->transform->position -= level.totalOffset;
	
	for (auto cloud = clouds.begin(); cloud != clouds.end(); cloud++) {
		cloud->pos -= level.totalOffset;
	}

	level.reset();

}

void PlayMode::update(float elapsed) {
	if (!win && mech.health > 0.f) {

		updatestage(elapsed);

		//Get mouse info
		int x, y;
		SDL_GetMouseState(&x, &y);
		cursorPos = glm::vec2((float)x / (float)screenW, (float)y / (float)screenH);

		//Update level
		if (!level.bossCheck() && !pauseMode)level.update(elapsed);
		if(!level.bossCheck() && !pauseMode) offsetObjects();
		updateClouds();

		if (level.bossCheck() && !dialogueStart) continueGame = false;

		/*{ //update listener to camera position:
			glm::mat4x3 frame = camera->transform->make_local_to_parent();
			glm::vec3 right = frame[0];
			glm::vec3 at = frame[3];
			Sound::listener.set_position_right(at, right, 1.0f / 60.0f);
		}

		{//update music position
			bg_loop->set_position(get_player_position(),bg_loop->pan.ramp);
		}*/


		//Update transition machine
		if(!pauseMode) horiMovement.updateStateMachine();
		updateMechAnimation();
		//Horizontal
		glm::vec3 motionVec = glm::vec3(0.f);
		glm::vec3 reticleMotionVec = glm::vec3(0.f);
		float updateDistPlayer = spriteVelocityPlayer * elapsed;
		float updateDistReticle = spriteVelocityReticle * elapsed;
		//Horizontal
		if (!pauseMode) {
			if (left.pressed && !right.pressed) {
				horiMovement.transitionMachine(horiMovement.currentState, StateMachine::STATE_left);
			}
			else if (right.pressed && !left.pressed) {
				horiMovement.transitionMachine(horiMovement.currentState, StateMachine::STATE_right);
			}
			else {
				horiMovement.transitionMachine(horiMovement.currentState, StateMachine::STATE_horiSteady);
			}
			//Vertical
			if (down.pressed && !up.pressed) {
				vertMovement.transitionMachine(vertMovement.currentState, StateMachine::STATE_down);
				//motionVec.z -= updateDist;
			}
			else if (up.pressed && !down.pressed) {
				//motionVec.z += updateDist;
				vertMovement.transitionMachine(vertMovement.currentState, StateMachine::STATE_up);
			}
			else {
				vertMovement.transitionMachine(vertMovement.currentState, StateMachine::STATE_vertSteady);
			}
		}

		//Update mech sprite pos
		if (horiMovement.currentState == horiMovement.STATE_left) {
			if (!horiMovement.inTransition)
				motionVec.x -= updateDistPlayer;
			else if (horiMovement.currentState == horiMovement.STATE_leftToRight) {
				motionVec.x -= updateDistPlayer * (cos(PI_F * (float)horiMovement.framesDuring / (float)horiMovement.framesTo));
			}
		}
		else if (horiMovement.currentState == horiMovement.STATE_right) {
			if (!horiMovement.inTransition)
				motionVec.x += updateDistPlayer;
			else if (horiMovement.currentState == horiMovement.STATE_rightToLeft) {
				motionVec.x += updateDistPlayer * (cos(PI_F * (float)horiMovement.framesDuring / (float)horiMovement.framesTo));
			}
		}
		if (vertMovement.currentState == vertMovement.STATE_down) {
			motionVec.z -= updateDistPlayer;
		}
		else if (vertMovement.currentState == vertMovement.STATE_up) {
			motionVec.z += updateDistPlayer;
		}

		Scene::Transform newTransform;
		newTransform.rotation = scene.cameras.front().transform->rotation;
		newTransform.rotation = glm::normalize(
			newTransform.rotation
			* glm::angleAxis(-PI_F / 2.f, glm::vec3(1.0f, 0.0f, 0.0f)));
		if (!pauseMode) mech.playerSprite->pos += newTransform.rotation * motionVec;




		//Player out of bounds
		glm::vec3 playerCamPos = (camera->transform->make_world_to_local() * glm::vec4(mech.playerSprite->pos, 1.0f));
		playerCamDepth = -playerCamPos.z;
		maxHeight = tan(camera->fovy) * playerCamDepth / 2.f;
		minHeight = -maxHeight;
		maxHeight += 0.4f;
		maxWidth = camera->aspect * maxHeight;
		minWidth = camera->aspect * minHeight;
		playerCamHeight = playerCamPos.y;
		float playerCamWidth = playerCamPos.x;

		//Cursor pos update
		float cursorTop = tan(camera->fovy) * (1.0f) / 2.f;
		float cursorBottom = -cursorTop;
		float cursorRight = camera->aspect * cursorBottom;
		float cursorLeft = -cursorRight;
		glm::vec3 cursorXYZ = glm::vec3((cursorRight - cursorLeft) * cursorPos.x + cursorLeft, (cursorTop - cursorBottom) * cursorPos.y +
			cursorBottom, 1.f) * -(playerCamDepth + 2.f);
		glm::vec3 oldReticlePos = mech.controlReticle->pos;
		if (!pauseMode) {
			mech.controlReticle->pos = scene.cameras.front().transform->make_local_to_world() * glm::vec4(cursorXYZ, 1.f);
			glm::vec3 posDif = mech.controlReticle->pos - oldReticlePos;
			if (posDif.x < 0 && posDif.x < -0.15f) {
				if (mech.controlReticle->pipeline.currentAnimation != "ShipFlyingIdleE")
					mech.controlReticle->pipeline.setAnimation("ShipFlyingIdleE");
			}
			else if (posDif.x > 0 && posDif.x > 0.15f) {
				if (mech.controlReticle->pipeline.currentAnimation != "ShipFlyingIdleW")
					mech.controlReticle->pipeline.setAnimation("ShipFlyingIdleW");
			}
			else {
				if (mech.controlReticle->pipeline.currentAnimation != "ShipFlyingIdle")
					mech.controlReticle->pipeline.setAnimation("ShipFlyingIdle");

			}
		}
		float depthDif = (playerCamDepth + 2.f) / (playerCamDepth * 2.f);
		if (!pauseMode)	mech.controlReticle->size = glm::vec2(depthDif/3.f);

		if (!pauseMode) {
			if (playerCamHeight + mech.playerSprite->size.y * mech.playerSprite->height / SPRITE_SCALE > maxHeight) {
				playerCamHeight = maxHeight - mech.playerSprite->size.y * mech.playerSprite->height / SPRITE_SCALE;
				playerCamPos.y = playerCamHeight;
				vertMovement.transitionMachine(vertMovement.currentState, vertMovement.STATE_vertSteady);
			}
			else if (playerCamHeight < minHeight) {
				playerCamHeight = minHeight;
				playerCamPos.y = playerCamHeight;
				vertMovement.transitionMachine(vertMovement.currentState, vertMovement.STATE_vertSteady);
			}

			if (playerCamWidth + mech.playerSprite->size.x * mech.playerSprite->width / SPRITE_SCALE > maxWidth) {
				playerCamWidth = maxWidth - mech.playerSprite->size.x * mech.playerSprite->width / SPRITE_SCALE;
				playerCamPos.x = playerCamWidth;
				horiMovement.transitionMachine(horiMovement.currentState, StateMachine::STATE_horiSteady);
			}
			else if (playerCamWidth < minWidth) {
				playerCamWidth = minWidth;
				playerCamPos.x = playerCamWidth;
				horiMovement.transitionMachine(horiMovement.currentState, StateMachine::STATE_horiSteady);
			}
			mech.playerSprite->pos = camera->transform->make_local_to_world() * glm::vec4(playerCamPos, 1.0f);
		}

		//Control reticle out of bounds
		glm::vec3 reticleCamPos = (camera->transform->make_world_to_local() * glm::vec4(mech.controlReticle->pos, 1.0f));
		float reticleCamHeight = reticleCamPos.y;
		float reticleCamWidth = reticleCamPos.x;
		if (!pauseMode) {
			if (reticleCamHeight + mech.controlReticle->size.y * mech.controlReticle->height / SPRITE_SCALE > maxHeight) {
				reticleCamHeight = maxHeight - mech.controlReticle->size.y * mech.controlReticle->height / SPRITE_SCALE;
				reticleCamPos.y = reticleCamHeight;
			}
			else if (reticleCamHeight < minHeight) {
				reticleCamHeight = minHeight;
				reticleCamPos.y = reticleCamHeight;
			}

			if (reticleCamWidth + mech.controlReticle->size.x * mech.controlReticle->width / SPRITE_SCALE > maxWidth) {
				reticleCamWidth = maxWidth - mech.controlReticle->size.x * mech.controlReticle->width / SPRITE_SCALE;
				reticleCamPos.x = reticleCamWidth;
				horiMovement.transitionMachine(horiMovement.currentState, StateMachine::STATE_horiSteady);
			}
			else if (reticleCamWidth < minWidth) {
				reticleCamWidth = minWidth;
				reticleCamPos.x = reticleCamWidth;
				horiMovement.transitionMachine(horiMovement.currentState, StateMachine::STATE_horiSteady);
			}
		}

		float offsetReticleHeight = reticleCamHeight + mech.reticle->height * mech.reticle->size.y / SPRITE_SCALE / 2.f / -(playerCamDepth + 2.f);
		float offsetReticleWidth = reticleCamWidth + mech.reticle->width * mech.reticle->size.x / SPRITE_SCALE / 2.f / -(playerCamDepth + 2);
		glm::vec3 offsetReticleCamPos = glm::vec3(offsetReticleWidth, offsetReticleHeight, reticleCamPos.z);


		if (!pauseMode && playerCamHeight + mech.playerSprite->size.y * mech.playerSprite->height / SPRITE_SCALE < 0.85f * maxHeight) mech.reticle->pos = mech.playerSprite->pos + newTransform.rotation * glm::vec3((float)mech.playerSprite->width / SPRITE_SCALE / 8.f, 0.f, 1.13f);
		else if (!pauseMode) mech.reticle->pos = mech.playerSprite->pos + newTransform.rotation * glm::vec3((float)mech.playerSprite->width / SPRITE_SCALE / 8.f, 0.001f, 0.0f);

		offsetPlayerPos = mech.playerSprite->pos + newTransform.rotation* glm::vec3(
			mech.playerSprite->size * glm::vec2(mech.playerSprite->width, mech.playerSprite->height) / SPRITE_SCALE / 4.f, 0.f);


		if (!pauseMode) {
			if (leftFiredCooldown > 0) leftFiredCooldown--;
			if (rightFiredCooldown > 0) rightFiredCooldown--;

			//Projectiles and melee
				//Melee:
			if (mech.meleeTimer > 0) mech.meleeTimer--;
			else if (melee.pressed && mech.meleeTimer == 0 && melee.unpressed) {
				melee.unpressed = false;
				mech.meleeTimer = mech.meleeTime;
				createProjectileMech(Projectile::PROJ_Melee, glm::vec3(0.f), offsetPlayerPos);
			}
			//Projectiles:
			if (leftFiredCooldown == 0 && leftfire.pressed && mech.meleeTimer == 0) {
				glm::vec3 playerProjMotion = glm::normalize(newTransform.rotation * glm::vec3(motionVec.x, motionVec.y + 5.f, motionVec.z));
				createProjectileMech(Projectile::PROJ_RapidPlayer, playerProjMotion, mech.reticle->pos);
				leftFiredCooldown = rapidFiredCooldown;
			}
			if (rightFiredCooldown == 0 && rightfire.pressed) {
				glm::vec3 controlProjMotion = glm::normalize(newTransform.rotation * glm::vec3(0.f, 1.f, 0.f));
				createProjectileMech(Projectile::PROJ_SlowPlayer, controlProjMotion, camera->transform->make_local_to_world() * glm::vec4(cursorXYZ, 1.0f));
				rightFiredCooldown = slowFiredCooldown;
			}
		}
		if(!isdialogueDisplay) updateProjectiles(elapsed);


		//Update player health
		if (mech.playerSprite->pipeline.hitTimer > 0 && !pauseMode)mech.playerSprite->pipeline.hitTimer--;
		std::vector<int> hitResVec = projectileHit(*(mech.playerSprite));
		if (hitResVec.size() > 0 && mech.playerSprite->pipeline.hitTimer == 0 && !pauseMode) {
			mech.playerSprite->pipeline.hitTimer = mech.playerSprite->pipeline.hitTime;
			for (int ind = 0; ind < hitResVec.size(); ind++) {
				int hitRes = hitResVec[ind];
				switch (hitRes) {
				case(Projectile::PROJ_RapidEnemy):
					mech.health -= 0.3f;
					break;
				case(Projectile::PROJ_SlowEnemy):
					if (!mech.reflectDef) mech.health -= 2.f;
					break;
				case(Projectile::PROJ_Bomb):
					mech.health -= 5.0f;
					break;
				case(Projectile::PROJ_MeleeEnemy):
					if (mech.meleeHitInvinceTimer == 0) {
						mech.health -= 5.0f;
						mech.meleeHitInvinceTimer = mech.meleeHitInvince;
					}
					break;
				default:
					break;
				}
			}
		}
		if (mech.reflectDef)mech.reflectDef = false;
		if (mech.meleeHitInvinceTimer > 0 && !pauseMode) mech.meleeHitInvinceTimer--;
		//If still alive, spawn enemies
		if (!pauseMode)updateAllEnemies();
		if (!pauseMode)spawnEnemies();
		for (auto& enem : enemies) {
			glm::vec3 initPos = camera->transform->make_world_to_local() * glm::vec4(enem.sprite.pos, 1.f);
		}

		//reset button press counters:
		left.downs = 0;
		right.downs = 0;
		up.downs = 0;
		down.downs = 0;
	}
	else if (win) {
		//std::cout << "Win!\n";
		//Trigger credits	
		resetAll();
	}
	else {
		resetGame();
	}
}

void PlayMode::drawTextbox(std::string textStr, GLuint bgTex, GLuint portTex) {
	quad_texture_program_pipeline.texture = bgTex;
	scene.drawQuad();
	if (textBoxDisplay && (currentstage != gs_tutorial)) {
		scene.spriteDraw(*camera, false, false, true);
		text.textColor = glm::vec3(1.f);
		text.displayText(textStr, 0, glm::vec2(-0.55f, -.6f), glm::vec2(1.1f, 0.4f), 0.0016f);
		quad_texture_program_pipeline.texture = portTex;
		scene.drawQuad(glm::vec2(-0.8775f,-.93f),glm::vec2(0.25f,0.27f*16.f/10.f));
	}
}

// comparison sprites by y (backwards)
inline bool compare_sprites(const PlayMode::Enemy& first, const PlayMode::Enemy& second)
{

	return first.sprite.pos.y > second.sprite.pos.y;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {


	//Add any temp sprites to sprite list
	//Projectiles
	for (auto& proj : projectiles) {
		proj.projSprite.pipeline.updateAnimation();
		scene.sprites.push_back(proj.projSprite);
	}
	enemies.sort(compare_sprites);
	for (auto& enemy : enemies) {
		enemy.sprite.pipeline.updateAnimation();
		scene.sprites.push_back(enemy.sprite);
	}
	int cloudNum = 0;
	for (auto& cloud = clouds.end(); cloud != clouds.begin(); cloud--) {
		if (cloud == clouds.end()) cloud--;
		cloud->pipeline.updateAnimation();
		scene.sprites.push_back(*cloud);
	}


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
			glm::vec3 dirDir = glm::vec3(-light_to_world[2]);
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


	quad_texture_program_pipeline.texture = bgint;
	scene.drawQuad(glm::vec2(-1.f), glm::vec2(2.f));

	if (currentstage == gs_tutorial) {
		mech.controlReticle->doDraw = false;
		mech.reticle->doDraw = false;
	}
	else {
		mech.reticle->doDraw = true;
		mech.controlReticle->doDraw = true;
	}
	if(currentstage != gs_tutorial) scene.draw(*camera);
	GL_ERRORS();
	if (currentstage != gs_tutorial)scene.spriteDraw(*camera, false,false,false);
	scene.cloudDraw(*camera);
	if (currentstage != gs_tutorial)scene.spriteDraw(*camera, true, false, false);
	if (currentstage != gs_tutorial)scene.spriteDraw(*camera, false, false, false,true);
	scene.spriteDraw(*camera, false, true, false);
	GL_ERRORS();

	
	GL_ERRORS();

	//Delete all temporary sprites
	//projectiles

	std::vector<std::list<Sprite>::iterator> deleteVec = std::vector<std::list<Sprite>::iterator>();
	for (auto spriteIt = scene.sprites.begin(); spriteIt != scene.sprites.end();  spriteIt++) {
		if (spriteIt->name.size() > 10 && spriteIt->name.substr(0, 10) == "projectile") {
			deleteVec.push_back((spriteIt));
		}
		if (spriteIt->name.size() > 5 && spriteIt->name.substr(0, 5) == "enemy") {
			deleteVec.push_back(spriteIt);
		}
		if (spriteIt->name.size() >= 5 && spriteIt->name.substr(0, 5) == "cloud") {
			deleteVec.push_back(spriteIt);
		}
	}
	for (int deleteInd = 0; deleteInd < deleteVec.size(); deleteInd++) {
		scene.sprites.erase(deleteVec[deleteInd]);
	}
	deleteVec.clear();

	if(isdialogueDisplay) drawTextbox(curDisplay->currentTex(), curDisplay->currentBG().first, curDisplay->currentPortrait().first);
	

	float healthPercentage = (float)((int)(mech.health / mech.maxHealth * 1000.f)) / 10.f;
	if (healthPercentage < 1 / 3.f * 100.f) text.textColor = glm::vec3(1.f, 0.3f, 0.3f);
	else text.textColor = glm::vec3(0.3f, 1.f, 0.6f);
	std::string healthString;
	if(healthPercentage == 100.f) healthString = std::string("Health: ").append(std::to_string(healthPercentage).substr(0,3)).append(" %");
	else healthString = std::string("Health: ").append(std::to_string(healthPercentage).substr(0, 4)).append("%");
	if (!isdialogueDisplay && !pauseMode)text.displayText(healthString, 0, glm::vec2(-.95f, 0.9f), glm::vec2(1.f, 0.2f), 0.0014f);
	else if (!isdialogueDisplay) text.displayText("Paused", 0, glm::vec2(-.95f, 0.9f), glm::vec2(1.f, 0.2f), 0.0014f);
	
	
	if (currentstage == gs_boss) {
		float bosspercentage = (float)((int)(bossJebbActive->health / JEBB_HEALTH * 1000.f)) / 10.f;
		text.textColor = glm::vec3(1.f, 0.3f, 0.3f);
		std::string bosshealthString;
		if (bosspercentage == 100.f) bosshealthString = std::string("Enemy: ").append(std::to_string(bosspercentage).substr(0, 3)).append(" %");
		else bosshealthString = std::string("Enemy: ").append(std::to_string(bosspercentage).substr(0, 4)).append("%");
		if (!isdialogueDisplay && !pauseMode) text.displayText(bosshealthString, 0, glm::vec2(0.5f, 0.9f), glm::vec2(1.f, 0.2f), 0.0014f);
		
	}
	if (currentstage == gs_dark) {
		float bosspercentage = (float)((int)(bossDarkActive->health / DARK_HEALTH * 1000.f)) / 10.f;
		text.textColor = glm::vec3(1.f, 0.3f, 0.3f);
		std::string bosshealthString;
		if (bosspercentage == 100.f) bosshealthString = std::string("Enemy: ").append(std::to_string(bosspercentage).substr(0, 3)).append(" %");
		else bosshealthString = std::string("Enemy: ").append(std::to_string(bosspercentage).substr(0, 4)).append("%");
		if (!isdialogueDisplay && !pauseMode) text.displayText(bosshealthString, 0, glm::vec2(0.5f, 0.9f), glm::vec2(1.f, 0.2f), 0.0014f);

	}
	
	std::string yourTime = std::to_string((float)((int)(bosstime * 1000.f)) / 1000.f);
	std::string highTime = std::to_string((float)((int)(highscore * 1000.f)) / 1000.f);

	text.textColor = glm::vec3(0.3f, 1.f, 0.6f);
	if (!isdialogueDisplay && bossJebbSpawned && currentstage == gs_boss)text.displayText(yourTime, 0, glm::vec2(-.95f, -0.9f), glm::vec2(1.f, 0.2f), 0.0014f);
	if (!isdialogueDisplay && bossJebbSpawned && currentstage == gs_boss)text.displayText(highTime, 0, glm::vec2(.75, -0.9f), glm::vec2(1.f, 0.2f), 0.0014f);
}

glm::vec3 PlayMode::get_player_position() {
	//the vertex position here was read from the model in blender:
	return player->make_local_to_world() * glm::vec4(0.f, 0.f, 0.0f, 1.0f);
}
