#ifndef WEAPON_H
#define WEAPON_H

#include <SFML/Graphics.hpp>
#include "Enemy.h"
#include <vector>

class Weapon
{
public:
	Weapon(sf::Texture& texture, const sf::Vector2f& position);

	void draw(sf::RenderWindow& window);
	void update(const sf::Vector2f& playerPosition, float width, bool facingRight,float deltaTime);
	void checkCollision(std::vector<Enemy>& enemies, float damage, bool facingRight);
	void setDamageMultiplier(float multiplier);

	sf::FloatRect getBounds();

	bool isAttacking;
private:
	sf::Sprite sprite;
	sf::Texture texture;
	sf::Clock frameClock;
	float damageMultiplier = 1.0f;
};

#endif // WEAPON_H
