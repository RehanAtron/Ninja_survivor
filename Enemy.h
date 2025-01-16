#ifndef ENEMY_H
#define ENEMY_H

#include <SFML/Graphics.hpp>
#include "Ground.h"

// Base Enemy class
class Enemy {
public:
    Enemy(sf::Vector2f spawnPosition, sf::Texture& texture, float speed, float health, bool flying, bool charging);

    void update(float deltaTime, std::vector<Ground>& grounds, int& currency);
    void draw(sf::RenderWindow& window);
    void takeDamage(float damage, const sf::Vector2f& hitDirection, float knockbackDistance);
    void setTarget(const sf::Vector2f& target);

    sf::FloatRect getBounds();
    sf::Vector2f position();
    float getHealth();
    bool hit();
    bool isAlive();

private:
    sf::Sprite sprite;
    sf::Texture texture;
    sf::Vector2f velocity;
    float gravity;
    float speed;
    float damageCooldownTimer = 0;
    bool OnGround;
    bool alive = true;
    bool facingRight;
    float health;
    float maxHealth; // Store initial health for health bar calculations
    sf::RectangleShape healthBarBackground;
    sf::RectangleShape healthBarFill;

    // Flying-specific variables
    bool isFlying;
    float hoverTime = 0.0f;
    float hoverOffset = 0.0f;

    // Charging variables
    bool isTelegraphing;     
    float telegraphTimer;
    bool isCharging;
    bool canCharge;
    float chargeTimer;
    float chargeCooldown;

    // Knockback variables
    bool knockbackActive = false;
    float knockbackDistance = 0;
    float knockbackTimer = 0;
    float knockbackDuration = 0;
    sf::Vector2f knockbackStartPosition;
    sf::Vector2f knockbackDirection;
    float hitFlashTimer; 
    float hitRotation;
    float hitBounceTimer;
    float originalY;

    // Pathfinding
    bool following;
    sf::Vector2f targetPosition;

    // Death animation
    bool isDeathAnimating = false;
    float deathRotation = 0.0f;
    float deathTimer = 0.0f;
    float DEATH_ANIMATION_DURATION = 0.5f;
    float DEATH_ROTATION_SPEED = 720.0f; // Degrees per second
    float DEATH_FALL_SPEED = 500.0f;

};

#endif //ENEMY_H