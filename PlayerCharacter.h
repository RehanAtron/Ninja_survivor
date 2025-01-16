#ifndef PLAYER_H
#define PLAYER_H

#include <SFML/Graphics.hpp>
#include "Ground.h"
#include "Enemy.h"
#include "Weapon.h"
#include "Item.cpp"

class Player {
public:
    // Constructor
    Player(const sf::Vector2f& position, const sf::Texture& textureFile, sf::Texture& weaponTexture, const float& speed);

    // Member functions
    void update(float deltaTime, std::vector<Ground>& grounds);
    void draw(sf::RenderWindow& window);
    void handleInput(float deltaTime);
    void handleCollision(std::vector<Enemy>& enemies,float deltaTime);
    void SetPosition(sf::Vector2f& position);
    void SetHealth(float health);
    void ChangeHealth(float health);

    sf::Vector2f position();
    sf::FloatRect getBounds();
    Weapon& getWeapon();
    float getHealth();
    float getDamage();

    void recalculateStats();
    float calculateTotalDamageMultiplier() const;

    bool facingRight;
    std::vector<Item> items;
private:
    // Member variables
    sf::Sprite sprite;
    sf::Texture texture;
    sf::Vector2f velocity;
    float collisionTimer;
    float speed;
    float gravity;
    float baseSpeed;
    float baseHealth;
    bool OnGround;
    bool Hit;
    float health;
    float damage;

    Weapon weapon;

    // Knockback variables
    bool knockbackActive = false;      // Is the player currently in knockback?
    float knockbackTimer = 0.0f;       // Time elapsed during knockback
    float knockbackDuration = 0.5f;    // Duration of the knockback effect
    float knockbackDistance = 200.0f;  // Maximum horizontal distance to cover
    float knockbackArcHeight = 100.0f; // Maximum height of the arc
    sf::Vector2f knockbackStartPosition; // Starting position for the arc
    sf::Vector2f knockbackDirection;  // Direction of the knockback

    // Dash variables
    bool isDashing = false;         
    float dashDistance = 200.f;     
    float dashTime = 0.1f;          // Total time for dash
    float dashTimer = 0.0f;         // Timer to track dash progress
    sf::Vector2f dashDirection;     

    bool canDash = true;            
    float dashCooldown = 0.3f;      // Time between dashes 
    float dashCooldownTimer = 0.0f; // Tracks time since the last dash

    // Stats    
    sf::Text statsText;
    sf::Font font;

    // Animation parameters
    const float jumpSquashFactor = 0.8f;    // Vertical squash when jumping
    const float jumpStretchFactor = 1.2f;    // Horizontal stretch when jumping
    const float fallStretchFactor = 1.3f;    // Vertical stretch when falling
    const float fallSquashFactor = 0.9f;     // Horizontal squash when falling
    const float hurtPulseDuration = 1.0f;    // Duration of hurt animation
    float hurtPulseTimer = 0.0f;            // Timer for hurt animation
    sf::Vector2f baseScale{ 1.0f, 1.0f };     // Store the base scale for animations


};

#endif  PLAYER_H



