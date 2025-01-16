#include <iostream>

#include <SFML/Graphics.hpp>

#include "Weapon.h"
#include "Enemy.h"

Weapon::Weapon(sf::Texture& texture, const sf::Vector2f& position) : texture(texture) {
    sprite.setTexture(this->texture);
    sprite.setPosition(position);
    isAttacking = false;
}

void Weapon::draw(sf::RenderWindow& window){
    window.draw(sprite);
}

void Weapon::update(const sf::Vector2f& playerPosition, float width, bool facingRight, float deltaTime) {
    static float swingAngle = 0.0f;
    static const float SWING_SPEED = 540.0f;  // Degrees per second
    static const float START_ANGLE = 0.0f;
    static const float END_ANGLE = 90.0f;
    static const float COOLDOWN_DURATION = 0.5f;
    static float cooldownTimer = 0.0f;
    static bool animationInProgress = false;

    // Update cooldown timer
    if (cooldownTimer > 0) {
        cooldownTimer -= deltaTime;
    }

    // Detect mouse click
    bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::J) || sf::Keyboard::isKeyPressed(sf::Keyboard::X);

    // Handle new or held click
    if (mousePressed && cooldownTimer <= 0.0f) {
        animationInProgress = true;
        isAttacking = true;
        if (!animationInProgress) {
            swingAngle = START_ANGLE;  // Reset to start angle for a new attack
        }
    }

    // Update animation if in progress
    if (animationInProgress) {
        swingAngle += SWING_SPEED * deltaTime;

        // Check if the swing is complete
        if (swingAngle >= END_ANGLE) {
            swingAngle = START_ANGLE;  // Reset to 0 degrees for the next swing
            cooldownTimer = COOLDOWN_DURATION;

            // Stop animation if the mouse is not held
            if (!mousePressed) {
                animationInProgress = false;
                isAttacking = false;
            }
        }
    }

    // Set origin to the bottom center of the sprite
    sf::FloatRect spriteBounds = sprite.getLocalBounds();
    sprite.setOrigin(spriteBounds.width / 2.0f, spriteBounds.height);

    // Update rotation
    sprite.setRotation(facingRight ? swingAngle : -swingAngle);

    // Update weapon position to align with the player's position
    sprite.setPosition(playerPosition.x, playerPosition.y);
}

void Weapon::setDamageMultiplier(float multiplier) { 
    damageMultiplier = multiplier; 
}

void Weapon::checkCollision(std::vector<Enemy>& enemies, float damage, bool facingRight) {

    for (auto& enemy : enemies) {
        if (sprite.getGlobalBounds().intersects(enemy.getBounds()) && isAttacking) {
            sf::Vector2f hitDirection = facingRight ? sf::Vector2f(1.0f, 0.0f) : sf::Vector2f(-1.0f, 0.0f);

            // Apply damage and knockback
            enemy.takeDamage(damage * damageMultiplier, hitDirection, 50.0f);
        }
    }
}
sf::FloatRect Weapon::getBounds() {
    return sprite.getGlobalBounds();
}