#include <SFML/Graphics.hpp>

#include <Windows.h>
#include <iostream>
#include <vector>

#include "PlayerCharacter.h"
#include "Ground.h"
#include "Enemy.h"
class Level
{
public:
	// Constructor
	Level(int level, float width, float height) : levelNumber(level)
	{
		switch (levelNumber)
		{
		case 0:
			spawnPosition = sf::Vector2f(width / 2, height * 7 / 8);
			grounds = std::vector<Ground>{ Ground(height * 7 / 8,width,height) };
			break;
		case 1:
			spawnPosition = sf::Vector2f(0, height / 2);
			grounds = std::vector<Ground>{ Ground(height / 2,width / 2,height),
				Ground(height * 7 / 8, width,height)
			};
			break;
		case 2:
			spawnPosition = sf::Vector2f(0, height / 2);
			grounds = std::vector<Ground>{ Ground(height / 2, width / 3,height),
				Ground(width * 2 / 3,height * 2 / 3, width / 3,height)
			};
			break;
		case 3:
			spawnPosition = sf::Vector2f(0, height * 7 / 8);
			grounds = std::vector<Ground>{ Ground(height * 7 / 8,width / 4,height),
				Ground(width / 3, height * 3/5, width / 4,height),
				Ground(width * 4 / 5, height / 2, width / 5,height) };
			break;
		case 4:
		case 9:
			spawnPosition = sf::Vector2f(0, height * 7 / 8);
			grounds = std::vector<Ground>{ Ground(width/2, height * 3 / 5,width / 2,height),
				Ground(height * 7 / 8, width,height)
			};
			break;
		case 6:
			spawnPosition = sf::Vector2f(0, height * 7 / 8);
			grounds = std::vector<Ground>{ Ground(height * 7 / 8,width,height) };
			break;
		case 7:
			spawnPosition = sf::Vector2f(0, height * 3 / 4);
			grounds = std::vector<Ground>{ Ground(height * 3 / 4,width,height),
			Ground(width / 4,height / 3,width / 2,height / 8) };
			break;
		case 8:
			spawnPosition = sf::Vector2f(0, height / 2);
			grounds = std::vector<Ground>{ Ground(0, height / 2,width / 4,height),
				Ground(height * 7 / 8, width,height)
			};
			break;
		case 5:
		case 10: spawnPosition = sf::Vector2f(0, height / 2);
			grounds = std::vector<Ground>{ Ground(height / 2,width,height)};
			break;
		default:
			spawnPosition = sf::Vector2f(0, height * 7 / 8);
			grounds = std::vector<Ground>{ Ground(height * 7 / 8,width,height) };
			break;
		}
	}

	// Variables
	std::vector<Ground> grounds;
	std::vector<Enemy> enemies;
	sf::Vector2f spawnPosition;
	int levelNumber;

private:
};