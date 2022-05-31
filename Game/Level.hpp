#pragma once

#ifndef LEVEL_H

#define LEVEL_H
#include <vector>
#include <string>
#include "GL.hpp"
#include <list>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>
//Level specific gameplay infoo
class Level
{
public:
	Level() {}

	Level(std::string filename);
	~Level() {}

	glm::vec3 curoffset() {
		return posOffsetDelta;
	}

	struct EnemySpawn {
		int ID = 0;
		glm::vec3 worldPos;
		float spawnDist = 0.f;
	};

	bool bossCheck() {
		if (segment == path.size() - 1) return true;
		else return false;
	}

	void update(float elapsed) {
		segmentDelta += elapsed;
		if (segmentDelta >= segmentTime) {
			segment++;
			if (segment >= path.size()) segment = 0;
			segmentDelta = 0.f;
			segmentTime = path[segment].first;
			posOffsetOld = posOffsetNew;
			int segmentNext = segment + 1;
			if (segmentNext >= path.size()) segmentNext = 0;
			posOffsetNew = path[segmentNext].second;
		}
		glm::vec3 newPos = (1.f - segmentDelta / segmentTime) * posOffsetOld + (segmentDelta / segmentTime) * posOffsetNew;
		posOffsetDelta = newPos - lastPos;
		totalOffset += posOffsetDelta;
		speed = posOffsetDelta.z;
		lastPos = newPos;
	}

	void reset() {
		posOffsetOld = glm::vec3(0.f);
		posOffsetNew = glm::vec3(0.f);
		posOffsetDelta = glm::vec3(0.f);
		lastPos = glm::vec3(0.f);
		segment = 0;
		segmentTime = path[segment].first;
		totalOffset = glm::vec3(0.f);
	}

	glm::vec3 posOffsetOld = glm::vec3(0.f); //Where in the scene were we?
	glm::vec3 posOffsetNew = glm::vec3(0.f); //Where in the scene are we?
	glm::vec3 posOffsetDelta = glm::vec3(0.f); //How in the scene have we moved?
	glm::vec3 lastPos = glm::vec3(0.f); //Where in the scene where we last

	int segment = 0;
	float segmentDelta = 0.f;
	float segmentTime = 0.f;
	std::list<EnemySpawn> toSpawn;
	std::vector<std::pair<float, glm::vec3>> path;
	float speed = 1.f;
	glm::vec3 totalOffset = glm::vec3(0.f);

private:

};

#endif // !LEVEL_H