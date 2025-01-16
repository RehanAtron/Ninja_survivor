#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>

#include "PlayerCharacter.h"
#include "Ground.h"
#include "Enemy.h"
#include "Item.cpp"

const float SCREEN_WIDTH = 1280;
const float SCREEN_HEIGHT = 720;

Player::Player(const sf::Vector2f& position, const sf::Texture& textureFile, sf::Texture& weaponTexture, const float& moveSpeed)
    :texture(textureFile), gravity(10), OnGround(false), velocity(0, 0), collisionTimer(0), Hit(false), facingRight(true), weapon(weaponTexture, position) {

    sprite.setTexture(texture);
    sprite.setPosition(position);
    sprite.setOrigin(sprite.getGlobalBounds().width / 2, sprite.getGlobalBounds().height / 2);
    speed = moveSpeed;
    health = 10;


    // Store base stats
    baseHealth = 10;
    damage = 1;

    if (!font.loadFromFile("Textures/font.ttf")) { 
        std::cout << "Error loading font" << std::endl;
    }

    statsText.setFont(font);
    statsText.setCharacterSize(20);
    statsText.setFillColor(sf::Color::Red);
    statsText.setPosition(10, SCREEN_HEIGHT/8);
}

void Player::update(float deltaTime, std::vector<Ground>& grounds) {
    if (!health) return;

    /*float previousVelocityY = velocity.y;*/
    sf::Vector2f newScale = baseScale;

    // Apply gravity if not on the ground
    if (!OnGround)
        velocity.y += gravity * deltaTime;

    // Collision detection with the ground
    OnGround = false;  // Reset on ground status
    for (auto& ground : grounds)
    {
        if (sprite.getGlobalBounds().intersects(ground.getBounds()))
        {
            sf::FloatRect groundBounds = ground.getBounds();
            sf::FloatRect spriteBounds = sprite.getGlobalBounds();

            // Adjust sprite bounds based on origin offset
            sf::Vector2f originOffset(sprite.getOrigin().x, sprite.getOrigin().y);
            spriteBounds.left -= originOffset.x;
            spriteBounds.top -= originOffset.y;

            float spriteBottom = spriteBounds.top + spriteBounds.height;
            float groundTop = groundBounds.top;

            float spriteRight = spriteBounds.left + spriteBounds.width;
            float groundLeft = groundBounds.left;

            float spriteLeft = spriteBounds.left;
            float groundRight = groundBounds.left + groundBounds.width;

            // Determine the amount of overlap on each side
            float overlapTop = spriteBottom - groundTop;
            float overlapBottom = groundBounds.top + groundBounds.height - spriteBounds.top;
            float overlapLeft = spriteRight - groundLeft;
            float overlapRight = groundRight - spriteLeft;

            // Find the smallest overlap to determine collision direction
            float minOverlap = std::min({ overlapTop, overlapBottom, overlapLeft, overlapRight });

            if (minOverlap == overlapTop && velocity.y > 0) {
                // Vertical collision: landing on top of the ground
                sprite.setPosition(sprite.getPosition().x, groundTop - (spriteBounds.height - originOffset.y));
                velocity.y = 0;
                OnGround = true;
            }
            else if (minOverlap == overlapLeft && velocity.x > 0) {
                // Horizontal collision: hitting the left side of the ground
                sprite.setPosition(groundLeft - (spriteBounds.width - originOffset.x), sprite.getPosition().y);
                velocity.x = 0; // Stop horizontal movement
            }
            else if (minOverlap == overlapRight && velocity.x < 0) {
                // Horizontal collision: hitting the right side of the ground
                sprite.setPosition(groundRight + originOffset.x, sprite.getPosition().y);
                velocity.x = 0; // Stop horizontal movement
            }
        }
    }


    // Handle movement and velocity
    handleInput(deltaTime);


    // Hurt animation
    if (Hit) {
        hurtPulseTimer += deltaTime;

        // Calculate fade factor using a sinusoidal wave
        float fadeFactor = 0.5f + 0.5f * sin(hurtPulseTimer * 10.0f); // Range: [0, 1]

        // Interpolate between white and red for fading effect
        sf::Color interpolatedColor = sf::Color(
            static_cast<sf::Uint8>(255),  // Red remains constant
            static_cast<sf::Uint8>(255 * (1.0f - fadeFactor)), // Green fades
            static_cast<sf::Uint8>(255 * (1.0f - fadeFactor)), // Blue fades
            255  // Alpha is fully opaque
        );
        sprite.setColor(interpolatedColor);

        // Calculate scale pulsing effect
        float pulseScale = 1.0f + 0.2f * sin(hurtPulseTimer * 10.0f); // Scale oscillates
        newScale *= pulseScale; // Apply pulse effect
        sprite.setScale(newScale);

        if (hurtPulseTimer >= hurtPulseDuration) {
            hurtPulseTimer = 0.0f;
            Hit = false;
            sprite.setColor(sf::Color::White); // Reset to default color
        }
    }

    // Sprite flipping
    if (facingRight) {
        sprite.setScale(newScale.x, newScale.y);
    }
    else {
        sprite.setScale(-newScale.x, newScale.y);
    }
    // Handle weapon 
    weapon.update(sprite.getPosition(), sprite.getGlobalBounds().width/2, facingRight,deltaTime);
}


void Player::draw(sf::RenderWindow& window) {
    // Stat display
    std::string stats = "Damage: " + std::to_string(int(damage)) + "\n";

    std::ostringstream stream;
    stream << std::fixed << std::setprecision(1) << calculateTotalDamageMultiplier(); // Format to one decimal place
    stats += "Damage Multiplier: " + stream.str() + "x";

    statsText.setString(stats);

    window.draw(statsText);
    weapon.draw(window);
    window.draw(sprite);
}

void Player::handleInput(float deltaTime) {
    if (!canDash) {
        dashCooldownTimer += deltaTime;
        if (dashCooldownTimer >= dashCooldown) {
            canDash = true;
            dashCooldownTimer = 0.0f;
        }
    }

    if (isDashing) {
        // Continue the dash
        dashTimer += deltaTime;
        // Calculate normalized time (0.0 to 1.0)
        float t = dashTimer / dashTime;
        if (t >= 1.0f) {
            // Dash complete
            isDashing = false;
            dashTimer = 0.0f;
            velocity = { 0, 0 }; // Reset velocity after dash
        }
        else {
            // Move the sprite by the dash distance over time
            sprite.move(dashDirection * (dashDistance / dashTime) * deltaTime);
        }
    }

    // Normal movement logic
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) && canDash && !isDashing) {
        // Start dashing
        isDashing = true;
        canDash = false;
        dashTimer = 0.0f;
        dashDirection = { 0.0f, 0.0f }; // Reset dash direction

        // Determine horizontal dash direction
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            dashDirection.x = -1; // Dash left
            facingRight = false;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || 
            sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            dashDirection.x = 1;  // Dash right
            facingRight = true;
        }
        else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::W) &&
            !sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            // Only use facing direction if no vertical input
            dashDirection.x = facingRight ? 1 : -1;
        }

        // Determine vertical dash direction
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            dashDirection.y = -1; // Dash up
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || 
            sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            dashDirection.y = 1;  // Dash down
        }

        // Normalize the diagonal dash vector to maintain consistent dash speed
        if (dashDirection.x != 0.0f && dashDirection.y != 0.0f) {
            float length = std::sqrt(dashDirection.x * dashDirection.x +
                dashDirection.y * dashDirection.y);
            dashDirection.x /= length;
            dashDirection.y /= length;
        }

        return; // Skip normal movement input during dash
    }

    // Horizontal movement
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || 
        sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        velocity.x = -1;  // Move left
        facingRight = false;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        velocity.x = 1;  // Move right
        facingRight = true;
    }
    else {
        velocity.x = 0;
    }

    // Vertical movement
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && OnGround) {
        velocity.y = -3;
    }

    // Apply movement
    sprite.move(velocity * speed * deltaTime);

}

void Player::handleCollision(std::vector<Enemy>& enemies, float deltaTime) {
    if (!health) return;
    weapon.checkCollision(enemies, damage,facingRight);
    if (knockbackActive) {
        // Continue semicircular knockback motion
        knockbackTimer += deltaTime;

        // Compute normalized time in [0, 1]
        float t = knockbackTimer / knockbackDuration;

        if (t >= 1.0f) {
            // Knockback complete
            knockbackActive = false;
            sprite.setColor(sf::Color::White); // Reset color after hit
            return;
        }

        // Semicircular trajectory
        float x = knockbackDirection.x * knockbackDistance * t; // Horizontal movement
        float y = -4 * knockbackArcHeight * t * (1 - t);        // Vertical arc

        // Apply knockback motion
        sprite.setPosition(knockbackStartPosition + sf::Vector2f(x, y));
        return; // Exit the function to continue knockback
    }

    // Check for collisions
    for (auto& enemy : enemies) {
        if (sprite.getGlobalBounds().intersects(enemy.getBounds())) {
            if (Hit) return; // Skip if already hit
            if (enemy.hit()) return; // Skip if enemy was attacked
            Hit = true;
            health--;
        }
    }

}
void Player::recalculateStats() {

    // Calculate flat bonuses
    int additionalDamage = 0;
    for (const auto& item : items) {
        additionalDamage += item.getDamage();
    }

    // Apply multipliers and bonuses
    if (health < 10)
        health += items[items.size() - 1].getHealth();
    damage = 1 + additionalDamage;

    // Update weapon damage multiplier
    float damageMultiplier = calculateTotalDamageMultiplier();
    weapon.setDamageMultiplier(damageMultiplier);

}

float Player::calculateTotalDamageMultiplier() const {
    float multiplier = 1.0f;
    for (const auto& item : items) {
        multiplier *= item.getDamageMultiplier();
    }
    return multiplier;
}


sf::FloatRect Player::getBounds(){
    return sprite.getGlobalBounds();
}

void Player::SetPosition(sf::Vector2f& position) {
    sprite.setPosition(position);
}
void Player::SetHealth(float health) {
    this->health = health;
}
void Player::ChangeHealth(float health)
{
    this->health += health;
}

sf::Vector2f Player::position() {
    return sprite.getPosition();
}

float Player::getHealth() {
    return health;
}

float Player::getDamage() {
    return damage;
}

Weapon& Player::getWeapon() {
    return weapon;
}