#include "Level.hpp"
#include <string>
#include<vector>
#include <memory>
#include <functional>
#include <iostream>
#include <assert.h>
#include "PlayMode.hpp"
#include <fstream>
#include "data_path.hpp"

#include "GL.hpp"
#include <list>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

Level::Level(std::string fileName) {
	

	std::ifstream fstream;
	std::cout << "starting load of " << fileName << std::endl;
	std::string properPath = data_path(fileName);
	fstream.open(properPath, std::ios::in | std::ios::binary);
	if (fstream.is_open()) {
		std::cout << fileName << " opened succesfully\n";
		path = std::vector<std::pair<float, glm::vec3>>();

		float time;
		std::string idString;
		float x;
		float y;
		float z;
		float dist;
		char nameline[999];
		std::string temp;

		fstream.getline(nameline, 999);
		if ((temp = std::string(nameline)).size() >= 7 && temp.substr(0, 7) == "ENEMIES") {
			int line = 0;
			while (!fstream.eof()) {
				fstream.getline(nameline, 999);
				if ((temp = std::string(nameline)).size() >= 4 && temp.substr(0, 4) == "PATH") {
					line = 0;
					while (!fstream.eof()) {
						fstream.getline(nameline, 999);
						size_t spaceInd = 0;
						while (nameline[spaceInd] != ' ')spaceInd++;

						temp = std::string(nameline);
						time = (float)std::atof(temp.substr(0, spaceInd).c_str());
						temp = temp.substr(spaceInd + 1, temp.size() - spaceInd - 1);

						spaceInd = 0;
						while (temp[spaceInd] != ' ' && spaceInd < temp.size())spaceInd++;
						if (spaceInd >= temp.size()) {
							std::cout << "ERROR: Bad formatting in level file " << fileName << ": line ended early.\n";
							assert(false);
						}
						x = (float)std::atof(temp.substr(0, spaceInd).c_str());
						temp = temp.substr(spaceInd + 1, temp.size() - spaceInd - 1);
						spaceInd = 0;
						while (temp[spaceInd] != ' ' && spaceInd < temp.size())spaceInd++;
						if (spaceInd >= temp.size()) {
							std::cout << "ERROR: Bad formatting in level file " << fileName << ": line ended early.\n";
							assert(false);
						}
						y = (float)std::atof(temp.substr(0, spaceInd).c_str());
						temp = temp.substr(spaceInd + 1, temp.size() - spaceInd - 1);
						z = (float)std::atof(temp.substr(0, temp.size()).c_str());
						path.push_back(std::make_pair(time, glm::vec3(x, y, z)));
					}
				}

				else {
					EnemySpawn newEnemy;
					size_t spaceInd = 0;
					while (nameline[spaceInd] != ' ')spaceInd++;

					temp = std::string(nameline);
					idString = temp.substr(0, spaceInd);
					temp = temp.substr(spaceInd + 1, temp.size() - spaceInd - 1);

					spaceInd = 0;
					while (temp[spaceInd] != ' ' && spaceInd < temp.size())spaceInd++;
					if (spaceInd >= temp.size()) {
						std::cout << "ERROR: Bad formatting in level file " << fileName << ": line ended early.\n";
						assert(false);
					}
					x = (float)std::atof(temp.substr(0, spaceInd).c_str());
					temp = temp.substr(spaceInd + 1, temp.size() - spaceInd - 1);
					spaceInd = 0;
					while (temp[spaceInd] != ' ' && spaceInd < temp.size())spaceInd++;
					if (spaceInd >= temp.size()) {
						std::cout << "ERROR: Bad formatting in level file " << fileName << ": line ended early.\n";
						assert(false);
					}
					y = (float)std::atof(temp.substr(0, spaceInd).c_str());
					temp = temp.substr(spaceInd + 1, temp.size() - spaceInd - 1);
					spaceInd = 0;
					while (temp[spaceInd] != ' ' && spaceInd < temp.size())spaceInd++;
					if (spaceInd >= temp.size()) {
						std::cout << "ERROR: Bad formatting in level file " << fileName << ": line ended early.\n";
						assert(false);
					}
					z = (float)std::atof(temp.substr(0, spaceInd).c_str());
					temp = temp.substr(spaceInd + 1, temp.size() - spaceInd - 1);

					dist = (float)std::atof(temp.substr(0, temp.size()).c_str());

					newEnemy.worldPos.x = x;
					newEnemy.worldPos.y = y;
					newEnemy.worldPos.z = z;
					newEnemy.spawnDist = dist;

					if (idString == "ENEMY_envoPilotMe") {
						newEnemy.ID = PlayMode::Enemy::ENEMY_envoPilotMe;
					}
					else if (idString == "ENEMY_envoPilotRa") {
						newEnemy.ID = PlayMode::Enemy::ENEMY_EnvoPilotRa;
					}
					else if (idString == "ENEMY_grazaJet") {
						newEnemy.ID = PlayMode::Enemy::ENEMY_grazaJet;
					}
					else if (idString == "BOSS_Jebb") {
						newEnemy.ID = PlayMode::Enemy::BOSS_Jebb;
					}
					else if (idString == "BOSS_Gil") {
						newEnemy.ID = PlayMode::Enemy::BOSS_Gil;
					}
					else if (idString == "BOSS_Dark") {
						newEnemy.ID = PlayMode::Enemy::BOSS_Dark;
					}
					else {
						std::cout << "Else\n";
						newEnemy.ID = PlayMode::Enemy::ENEMY_envoPilotMe;
					}
					toSpawn.push_back(newEnemy);
				}
				
			}
		}
		else {

				std::cout << "ERROR: Bad formatting in level file " << fileName << ": ENEMIES Header Incorrect." << std::endl;
				assert(false);
		}
		std::cout << "Level " << fileName << " loaded correctly\n";

	}
	else {
		std::cout << "ERROR: Error loading level " << fileName << std::endl;
		assert(false);
	}
}