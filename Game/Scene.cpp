#include "Scene.hpp"

#include "gl_errors.hpp"
#include "read_write_chunk.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <array>
#include "PlayMode.hpp"

//-------------------------


struct Vertex {
	glm::vec4 Position;
	glm::vec3 Normal;
	glm::vec4 Color;
	glm::vec2 TexCoord;
};

glm::mat4x3 Scene::Transform::make_local_to_parent() const {
	//compute:
	//   translate   *   rotate    *   scale
	// [ 1 0 0 p.x ]   [       0 ]   [ s.x 0 0 0 ]
	// [ 0 1 0 p.y ] * [ rot   0 ] * [ 0 s.y 0 0 ]
	// [ 0 0 1 p.z ]   [       0 ]   [ 0 0 s.z 0 ]
	//                 [ 0 0 0 1 ]   [ 0 0   0 1 ]

	glm::mat3 rot = glm::mat3_cast(rotation);
	return glm::mat4x3(
		rot[0] * scale.x, //scaling the columns here means that scale happens before rotation
		rot[1] * scale.y,
		rot[2] * scale.z,
		position
	);
}

glm::mat4x3 Scene::Transform::make_parent_to_local() const {
	//compute:
	//   1/scale       *    rot^-1   *  translate^-1
	// [ 1/s.x 0 0 0 ]   [       0 ]   [ 0 0 0 -p.x ]
	// [ 0 1/s.y 0 0 ] * [rot^-1 0 ] * [ 0 0 0 -p.y ]
	// [ 0 0 1/s.z 0 ]   [       0 ]   [ 0 0 0 -p.z ]
	//                   [ 0 0 0 1 ]   [ 0 0 0  1   ]

	glm::vec3 inv_scale;
	//taking some care so that we don't end up with NaN's , just a degenerate matrix, if scale is zero:
	inv_scale.x = (scale.x == 0.0f ? 0.0f : 1.0f / scale.x);
	inv_scale.y = (scale.y == 0.0f ? 0.0f : 1.0f / scale.y);
	inv_scale.z = (scale.z == 0.0f ? 0.0f : 1.0f / scale.z);

	//compute inverse of rotation:
	glm::mat3 inv_rot = glm::mat3_cast(glm::inverse(rotation));

	//scale the rows of rot:
	inv_rot[0] *= inv_scale;
	inv_rot[1] *= inv_scale;
	inv_rot[2] *= inv_scale;

	return glm::mat4x3(
		inv_rot[0],
		inv_rot[1],
		inv_rot[2],
		inv_rot * -position
	);
}

glm::mat4x3 Scene::Transform::make_local_to_world() const {
	if (!parent) {
		return make_local_to_parent();
	} else {
		return parent->make_local_to_world() * glm::mat4(make_local_to_parent()); //note: glm::mat4(glm::mat4x3) pads with a (0,0,0,1) row
	}
}
glm::mat4x3 Scene::Transform::make_world_to_local() const {
	if (!parent) {
		return make_parent_to_local();
	} else {
		return make_parent_to_local() * glm::mat4(parent->make_world_to_local()); //note: glm::mat4(glm::mat4x3) pads with a (0,0,0,1) row
	}
}

//-------------------------

glm::mat4 Scene::Camera::make_projection() const {
	return glm::infinitePerspective( fovy, aspect, near );
}

//-------------------------


void Scene::draw(Camera const& camera) const {
	assert(camera.transform);
	glm::mat4 world_to_clip = camera.make_projection() * glm::mat4(camera.transform->make_world_to_local());
	glm::mat4x3 world_to_light = glm::mat4x3(1.0f);
	draw(world_to_clip, world_to_light);
}

void Scene::draw(glm::mat4 const& world_to_clip, glm::mat4x3 const& world_to_light) const {

	//Iterate through all drawables, sending each one to OpenGL:
	for (auto const& drawable : drawables) {
		//Reference to drawable's pipeline for convenience:
		Scene::Drawable::Pipeline const& pipeline = drawable.pipeline;

		//skip any drawables without a shader program set:
		if (pipeline.program == 0) continue;
		//skip any drawables that don't reference any vertex array:
		if (pipeline.vao == 0) continue;
		//skip any drawables that don't contain any vertices:
		if (pipeline.count == 0) continue;

		//Skip any drawable indicated to not be drawn
		if (!drawable.transform->doDraw) continue;

		//Set shader program:
		glUseProgram(pipeline.program);
		GL_ERRORS();
		//Set attribute sources:
		glBindVertexArray(pipeline.vao);
		GL_ERRORS();

		//Configure program uniforms:

		//the object-to-world matrix is used in all three of these uniforms:
		assert(drawable.transform); //drawables *must* have a transform
		glm::mat4x3 object_to_world = drawable.transform->make_local_to_world();

		//OBJECT_TO_CLIP takes vertices from object space to clip space:
		if (pipeline.OBJECT_TO_CLIP_mat4 != -1U) {
			glm::mat4 object_to_clip = world_to_clip * glm::mat4(object_to_world);
			glUniformMatrix4fv(pipeline.OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(object_to_clip));
		}
		GL_ERRORS();

		//the object-to-light matrix is used in the next two uniforms:
		glm::mat4x3 object_to_light = world_to_light * glm::mat4(object_to_world);

		//OBJECT_TO_CLIP takes vertices from object space to light space:
		if (pipeline.OBJECT_TO_LIGHT_mat4x3 != -1U) {
			glUniformMatrix4x3fv(pipeline.OBJECT_TO_LIGHT_mat4x3, 1, GL_FALSE, glm::value_ptr(object_to_light));
		}
		GL_ERRORS();

		//NORMAL_TO_CLIP takes normals from object space to light space:
		if (pipeline.NORMAL_TO_LIGHT_mat3 != -1U) {
			glm::mat3 normal_to_light = glm::inverse(glm::transpose(glm::mat3(object_to_light)));
			glUniformMatrix3fv(pipeline.NORMAL_TO_LIGHT_mat3, 1, GL_FALSE, glm::value_ptr(normal_to_light));
		}
		GL_ERRORS();

		//Set layer num to 0
		if (pipeline.LAYER_COUNT_uint != -1U) {
			unsigned int layerVal = 0;
			glUniform1ui(pipeline.LAYER_COUNT_uint, layerVal);
		}
		GL_ERRORS();

		//Set layer 1 material to 1
		if (pipeline.MATERIAL_TYPE_int_array != -1U) {
			int mat = 1;
			glUniform1iv(pipeline.MATERIAL_TYPE_int_array, 1, &mat);
		}
		GL_ERRORS();

		//Add view vector
		if (pipeline.viewDir_vec3 != -1U) {
			glm::vec3 dir = glm::normalize(cameras.front().transform->rotation * glm::vec3(0, 0, -1.f));
			glUniform3fv(pipeline.viewDir_vec3, 1, glm::value_ptr(dir));
		}

		//Add light bool
		if (pipeline.DO_LIGHT_bool != -1U) {
			glUniform1ui(pipeline.DO_LIGHT_bool, (uint32_t)true);
		}


		//Not text
		if (pipeline.TEXT_BOOL != -1U) {
			glUniform1ui(pipeline.TEXT_BOOL, (uint32_t)false);
		}
		GL_ERRORS();
		if (pipeline.TEXT_BOOL2 != -1U) {
			glUniform1ui(pipeline.TEXT_BOOL2, (uint32_t)false);
		}
		GL_ERRORS();

		//set any requested custom uniforms:
		if (pipeline.set_uniforms) pipeline.set_uniforms();

		std::vector<int> tempTexLocation;
		tempTexLocation = std::vector<int>(Drawable::Pipeline::TextureCount) ;
		//set up textures:
		for (uint32_t i = 0; i < Drawable::Pipeline::TextureCount; ++i) {
			if (pipeline.textures[i].texture != 0) {
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(pipeline.textures[i].target, pipeline.textures[i].texture);
				tempTexLocation[i] = i;
			}
		}
		GL_ERRORS();
		glUniform1iv(glGetUniformLocation(pipeline.program, "TEX_ARR"), Drawable::Pipeline::TextureCount, tempTexLocation.data());


		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(pipeline.textures[1].target, pipeline.textures[1].texture);
		//glUniform1i(glGetUniformLocation(pipeline.program, "TEX"), 1);

		GL_ERRORS();
		//draw the object:
		glDrawArrays(pipeline.type, pipeline.start, pipeline.count);

		GL_ERRORS();
		//un-bind textures:
		for (uint32_t i = 0; i < Drawable::Pipeline::TextureCount; ++i) {
			if (pipeline.textures[i].texture != 0) {
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(pipeline.textures[i].target, 0);
			}
		}
		GL_ERRORS();
		glActiveTexture(GL_TEXTURE0);
		GL_ERRORS();

	}

	glUseProgram(0);
	glBindVertexArray(0);

	GL_ERRORS();
}


void Scene::spriteDraw(Camera const& camera, bool proj, bool play, bool box) {
	assert(camera.transform);
	glm::mat4 world_to_clip = camera.make_projection() * glm::mat4(camera.transform->make_world_to_local());
	glm::mat4x3 world_to_light = glm::mat4x3(1.0f);
	spriteDraw(world_to_clip, world_to_light, proj, play, box);
}

void Scene::spriteDraw(glm::mat4 const& world_to_clip, glm::mat4x3 const& world_to_light, bool proj, bool play, bool box) {


	//Create sprite draw program
	GLuint vao = 0;
	GLuint vbo = 0;
	GLuint program = spriteProgram; //shader program; passed to glUseProgram
	
	GL_ERRORS();
	glEnable(GL_BLEND);
	GL_ERRORS();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Iterate through all sprites uodate their animations and draw the current frame
	for (std::list<Sprite>::iterator spriteIterator = sprites.begin(); spriteIterator != sprites.end(); spriteIterator++) {
		Sprite sprite = *spriteIterator;
		//printf("Entered sprite draw\n");
		if (sprite.name.substr(0, 10) == std::string("projectile") && !proj) continue;
		if (sprite.name.substr(0, 10) != std::string("projectile") && proj) continue;
		if (sprite.name.substr(0, 6) == std::string("player") && !play) continue;
		if (sprite.name.substr(0, 6) != std::string("player") && play) continue;
		if (sprite.name.substr(0, 7) == std::string("textbox") && !box) continue;
		if (sprite.name.substr(0, 7) != std::string("textbox") && box) continue;
		if (!sprite.doDraw) continue;

		//Reference to sprite's pipeline for convenience:
		spriteIterator->pipeline.updateAnimation();
		Sprite::Pipeline pipeline = sprite.pipeline;


		//Create quad at pos of sprite width and height
		float width = (float)sprite.width/SPRITE_SCALE*sprite.size.x;
		float height = (float)sprite.height/SPRITE_SCALE * sprite.size.y;
		std::array<Vertex, 6> vertices;
		glm::vec4 quadColor = glm::vec4(1.0f);
		if (pipeline.hitTimer > 0) {
			float percentage = (float)pipeline.hitTimer / (float)pipeline.hitTime;
			if(!play) quadColor = glm::vec4(1.0f, percentage * 0.5f + 0.5f, percentage * 0.5f + 0.5f, 1.0f);
			else quadColor = glm::vec4(percentage * 0.5f + 0.5f, 1.0f, percentage * 0.5f + 0.5f, 1.0f);
		}

		vertices[0].Position = glm::vec4(0.f, 0.f, height, 1.0f);
		vertices[1].Position = glm::vec4(0.f, 0.f, 0.f, 1.0f);
		vertices[2].Position = glm::vec4(width, 0.f, 0.f, 1.0f);
		vertices[3].Position = glm::vec4(0.f, 0.f, height, 1.0f);
		vertices[4].Position = glm::vec4(width, 0.f, 0.f, 1.0f);
		vertices[5].Position = glm::vec4(width, 0.f, height, 1.0f);
		for (size_t c = 0; c < 6; c++) {
			vertices[c].Normal = glm::vec3(0.0f, 0.0f, 1.0f);
			vertices[c].Color = quadColor;
		}
		vertices[0].TexCoord = glm::vec2(0.0f, 0.0f);
		vertices[1].TexCoord = glm::vec2(0.0f, 1.0f);
		vertices[2].TexCoord = glm::vec2(1.0f, 1.0f);
		vertices[3].TexCoord = glm::vec2(0.0f, 0.0f);
		vertices[4].TexCoord = glm::vec2(1.0f, 1.0f);
		vertices[5].TexCoord = glm::vec2(1.0f, 0.0f);

		glUseProgram(program);


		GL_ERRORS();
		glGenVertexArrays(1, &vao);
		GL_ERRORS();
		glBindVertexArray(vao);
		GL_ERRORS();
		glGenBuffers(1, &vbo); //Creating 1 generic buffer
		GL_ERRORS();
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		GL_ERRORS();
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(Vertex), (void*)vertices.data(), GL_DYNAMIC_DRAW);
		GL_ERRORS();
		GLint location = glGetAttribLocation(program, "Position");
		GL_ERRORS();
		glEnableVertexAttribArray(location);
		GL_ERRORS();
		glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		GL_ERRORS();
		location = glGetAttribLocation(program, "Normal");
		GL_ERRORS();
		glEnableVertexAttribArray(location);
		GL_ERRORS(); //ERRORS FROM 295 to 309
		glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(4 * sizeof(float)));
		GL_ERRORS();
		location = glGetAttribLocation(program, "Color");
		GL_ERRORS();
		glEnableVertexAttribArray(location);
		GL_ERRORS();
		glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(7 * sizeof(float)));
		GL_ERRORS();
		location = glGetAttribLocation(program, "TexCoord");
		GL_ERRORS();
		glEnableVertexAttribArray(location);
		GL_ERRORS();
		glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(11 * sizeof(float)));
		GL_ERRORS();
		
		Scene::Camera camera = cameras.front();
		assert(camera.transform);
		Scene::Transform newTransform;
		//newTransform.parent = camera.transform;
		newTransform.position = sprite.pos;
		newTransform.rotation = camera.transform->rotation;
		newTransform.rotation = glm::normalize(
			newTransform.rotation
			* glm::angleAxis(-PI_F/2.f, glm::vec3(1.0f, 0.0f, 0.0f)));
		//Set attribute sources:
		glBindVertexArray(vao);
		//Configure program uniforms:
		
		//the object-to-world matrix is used in all three of these uniforms:
		glm::mat4x3 object_to_world = newTransform.make_local_to_world();


		glUseProgram(program);

		//OBJECT_TO_CLIP takes vertices from object space to clip space:
		if (pipeline.OBJECT_TO_CLIP_mat4 != -1U) {
			glm::mat4 object_to_clip = world_to_clip * glm::mat4(object_to_world);
			glUniformMatrix4fv(pipeline.OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(object_to_clip));
		}

		GL_ERRORS();
		//the object-to-light matrix is used in the next two uniforms:
		glm::mat4x3 object_to_light = world_to_light * glm::mat4(object_to_world);

		//OBJECT_TO_CLIP takes vertices from object space to light space:
		if (pipeline.OBJECT_TO_LIGHT_mat4x3 != -1U) {
			glUniformMatrix4x3fv(pipeline.OBJECT_TO_LIGHT_mat4x3, 1, GL_FALSE, glm::value_ptr(object_to_light));
		}

		GL_ERRORS();
		//NORMAL_TO_CLIP takes normals from object space to light space:
		if (pipeline.NORMAL_TO_LIGHT_mat3 != -1U) {
			glm::mat3 normal_to_light = glm::inverse(glm::transpose(glm::mat3(object_to_light)));
			glUniformMatrix3fv(pipeline.NORMAL_TO_LIGHT_mat3, 1, GL_FALSE, glm::value_ptr(normal_to_light));
		}

		GL_ERRORS();

		//Set layer num
		size_t layerCount = (*sprite.pipeline.animations)[sprite.pipeline.currentAnimation].numLayers;
		//size_t layerCount = 1;
		if (pipeline.LAYER_COUNT_uint != -1U) {
			glUniform1ui(pipeline.LAYER_COUNT_uint, (uint32_t) layerCount);
		}
		GL_ERRORS();

		//Load mats array
		if (pipeline.MATERIAL_TYPE_int_array != -1U) {

			std::vector<int> mats = (*sprite.pipeline.animations)[sprite.pipeline.currentAnimation].getMaterials();
			glUniform1iv(pipeline.MATERIAL_TYPE_int_array, (uint32_t) layerCount, mats.data());
		}
		GL_ERRORS();

		//Add view vector
		if (pipeline.viewDir_vec3 != -1U) {
			glm::vec3 dir = glm::normalize(cameras.front().transform->rotation * glm::vec3(0, 0, -1.f));
			glUniform3fv(pipeline.viewDir_vec3, 1, glm::value_ptr(dir));
		}

		//Add light bool
		if (pipeline.DO_LIGHT_bool != -1U) {
			glUniform1ui(pipeline.DO_LIGHT_bool,(uint32_t) pipeline.doLight);
		}


		//Not text
		if (pipeline.TEXT_BOOL != -1U) {
			glUniform1ui(pipeline.TEXT_BOOL, (uint32_t)false);
		}
		GL_ERRORS();
		if (pipeline.TEXT_BOOL2 != -1U) {
			glUniform1ui(pipeline.TEXT_BOOL2, (uint32_t)false);
		}
		GL_ERRORS();

		std::vector<int> tempTexLocation;
		tempTexLocation = std::vector<int>(layerCount);
		//set up textures:
		for (uint32_t i = 0; i <layerCount; ++i) {
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture((*pipeline.animations)[pipeline.currentAnimation].layers[i].textures[pipeline.currentFrame].target, (*pipeline.animations)[pipeline.currentAnimation].layers[i].textures[pipeline.currentFrame].texture);
				tempTexLocation[i] = i;
		}
		GL_ERRORS();
		glUniform1iv(glGetUniformLocation(program, "TEX_ARR"), (uint32_t)layerCount, tempTexLocation.data());

		GL_ERRORS();

		if(pipeline.isGui)
			glDisable(GL_DEPTH_TEST);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glEnable(GL_DEPTH_TEST);

		GL_ERRORS();
		
		
		//un-bind textures:
		for (uint32_t i = 0; i < layerCount; ++i) {
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture((*pipeline.animations)[pipeline.currentAnimation].layers[i].textures[pipeline.currentFrame].target, 0);
		}
		glActiveTexture(GL_TEXTURE0);
		
		glUseProgram(0);
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		
	}

	GL_ERRORS();
}


void Scene::load(std::string const &filename,
	std::function< void(Scene &, Transform *, std::string const &) > const &on_drawable) {

	std::ifstream file(filename, std::ios::binary);

	std::vector< char > names;
	read_chunk(file, "str0", &names);

	struct HierarchyEntry {
		uint32_t parent;
		uint32_t name_begin;
		uint32_t name_end;
		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 scale;
	};
	static_assert(sizeof(HierarchyEntry) == 4 + 4 + 4 + 4*3 + 4*4 + 4*3, "HierarchyEntry is packed.");
	std::vector< HierarchyEntry > hierarchy;
	read_chunk(file, "xfh0", &hierarchy);

	struct MeshEntry {
		uint32_t transform;
		uint32_t name_begin;
		uint32_t name_end;
	};
	static_assert(sizeof(MeshEntry) == 4 + 4 + 4, "MeshEntry is packed.");
	std::vector< MeshEntry > meshes;
	read_chunk(file, "msh0", &meshes);

	struct CameraEntry {
		uint32_t transform;
		char type[4]; //"pers" or "orth"
		float data; //fov in degrees for 'pers', scale for 'orth'
		float clip_near, clip_far;
	};
	static_assert(sizeof(CameraEntry) == 4 + 4 + 4 + 4 + 4, "CameraEntry is packed.");
	std::vector< CameraEntry > cameras;
	read_chunk(file, "cam0", &cameras);

	struct LightEntry {
		uint32_t transform;
		char type;
		glm::u8vec3 color;
		float energy;
		float distance;
		float fov;
	};
	static_assert(sizeof(LightEntry) == 4 + 1 + 3 + 4 + 4 + 4, "LightEntry is packed.");
	std::vector< LightEntry > lights;
	read_chunk(file, "lmp0", &lights);


	//--------------------------------
	//Now that file is loaded, create transforms for hierarchy entries:

	std::vector< Transform * > hierarchy_transforms;
	hierarchy_transforms.reserve(hierarchy.size());

	for (auto const &h : hierarchy) {
		transforms.emplace_back();
		Transform *t = &transforms.back();
		if (h.parent != -1U) {
			if (h.parent >= hierarchy_transforms.size()) {
				throw std::runtime_error("scene file '" + filename + "' did not contain transforms in topological-sort order.");
			}
			t->parent = hierarchy_transforms[h.parent];
		}

		if (h.name_begin <= h.name_end && h.name_end <= names.size()) {
			t->name = std::string(names.begin() + h.name_begin, names.begin() + h.name_end);
		} else {
				throw std::runtime_error("scene file '" + filename + "' contains hierarchy entry with invalid name indices");
		}

		t->position = h.position;
		t->rotation = h.rotation;
		t->scale = h.scale;

		hierarchy_transforms.emplace_back(t);
	}
	assert(hierarchy_transforms.size() == hierarchy.size());

	for (auto const &m : meshes) {
		if (m.transform >= hierarchy_transforms.size()) {
			throw std::runtime_error("scene file '" + filename + "' contains mesh entry with invalid transform index (" + std::to_string(m.transform) + ")");
		}
		if (!(m.name_begin <= m.name_end && m.name_end <= names.size())) {
			throw std::runtime_error("scene file '" + filename + "' contains mesh entry with invalid name indices");
		}
		std::string name = std::string(names.begin() + m.name_begin, names.begin() + m.name_end);

		if (on_drawable) {
			on_drawable(*this, hierarchy_transforms[m.transform], name);
		}

	}

	for (auto const &c : cameras) {
		if (c.transform >= hierarchy_transforms.size()) {
			throw std::runtime_error("scene file '" + filename + "' contains camera entry with invalid transform index (" + std::to_string(c.transform) + ")");
		}
		if (std::string(c.type, 4) != "pers") {
			std::cout << "Ignoring non-perspective camera (" + std::string(c.type, 4) + ") stored in file." << std::endl;
			continue;
		}
		this->cameras.emplace_back(hierarchy_transforms[c.transform]);
		Camera *camera = &this->cameras.back();
		camera->fovy = c.data / 180.0f * 3.1415926f; //FOV is stored in degrees; convert to radians.
		camera->near = c.clip_near;
		//N.b. far plane is ignored because cameras use infinite perspective matrices.
	}

	for (auto const &l : lights) {
		if (l.transform >= hierarchy_transforms.size()) {
			throw std::runtime_error("scene file '" + filename + "' contains lamp entry with invalid transform index (" + std::to_string(l.transform) + ")");
		}
		if (l.type == 'p') {
			//good
		} else if (l.type == 'h') {
			//fine
		} else if (l.type == 's') {
			//okay
		} else if (l.type == 'd') {
			//sure
		} else {
			std::cout << "Ignoring unrecognized lamp type (" + std::string(&l.type, 1) + ") stored in file." << std::endl;
			continue;
		}
		this->lights.emplace_back(hierarchy_transforms[l.transform]);
		Light *light = &this->lights.back();
		light->type = static_cast<Light::Type>(l.type);
		light->energy = glm::vec3(l.color) / 255.0f * l.energy;
		light->spot_fov = l.fov / 180.0f * 3.1415926f; //FOV is stored in degrees; convert to radians.
	}

	//load any extra that a subclass wants:
	load_extra(file, names, hierarchy_transforms);

	if (file.peek() != EOF) {
		std::cerr << "WARNING: trailing data in scene file '" << filename << "'" << std::endl;
	}



}

//-------------------------

Scene::Scene(std::string const &filename, std::function< void(Scene &, Transform *, std::string const &) > const &on_drawable) {
	load(filename, on_drawable);
}

Scene::Scene(Scene const &other) {
	set(other);
}

Scene &Scene::operator=(Scene const &other) {
	set(other);
	return *this;
}

void Scene::set(Scene const &other, std::unordered_map< Transform const *, Transform * > *transform_map_) {

	std::unordered_map< Transform const *, Transform * > t2t_temp;
	std::unordered_map< Transform const *, Transform * > &transform_to_transform = *(transform_map_ ? transform_map_ : &t2t_temp);

	transform_to_transform.clear();

	//null transform maps to itself:
	transform_to_transform.insert(std::make_pair(nullptr, nullptr));

	//Copy transforms and store mapping:
	transforms.clear();
	for (auto const &t : other.transforms) {
		transforms.emplace_back();
		transforms.back().name = t.name;
		transforms.back().position = t.position;
		transforms.back().rotation = t.rotation;
		transforms.back().scale = t.scale;
		transforms.back().parent = t.parent; //will update later

		//store mapping between transforms old and new:
		auto ret = transform_to_transform.insert(std::make_pair(&t, &transforms.back()));
		assert(ret.second);
	}

	//update transform parents:
	for (auto &t : transforms) {
		t.parent = transform_to_transform.at(t.parent);
	}

	//copy other's drawables, updating transform pointers:
	drawables = other.drawables;
	for (auto &d : drawables) {
		d.transform = transform_to_transform.at(d.transform);
	}

	//copy other's cameras, updating transform pointers:
	cameras = other.cameras;
	for (auto &c : cameras) {
		c.transform = transform_to_transform.at(c.transform);
	}

	//copy other's lights, updating transform pointers:
	lights = other.lights;
	for (auto &l : lights) {
		l.transform = transform_to_transform.at(l.transform);
	}
}
