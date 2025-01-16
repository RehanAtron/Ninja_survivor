#include "Enemy.h"
#include <SFML/Graphics.hpp>
#include <iostream>

const float SCREEN_WIDTH = 1280;
const float SCREEN_HEIGHT = 720;
const float DETECTION_RANGE = 300.0f; // Range at which enemy detects target
const float GROUND_CHECK_DISTANCE = 50.0f; // Distance to check for ground ahead
const float WALL_CHECK_DISTANCE = 20.0f; // Distance to check for walls ahead

const float CHARGE_SPEED_MULTIPLIER = 4.0f; // Enemy moves faster during charge
const float CHARGE_DURATION = 1.0f; // How long the charge lasts
const float CHARGE_COOLDOWN = 2.0f; // Time between charges
const float CHARGE_TELEGRAPH_DURATION = 1.0f; // How long to pause before charging

// Add new constants for hit animation
const float HIT_FLASH_DURATION = 0.2f;
const float HIT_ROTATION_SPEED = 720.0f; // Degrees per second
const float HIT_BOUNCE_HEIGHT = 50.0f;
const float HIT_BOUNCE_DURATION = 0.3f;
const float WALL_BUFFER = 50.0f; // Minimum distance from walls

// Health bar constants
const float HEALTH_BAR_WIDTH = 50.0f;
const float HEALTH_BAR_HEIGHT = 5.0f;
const float HEALTH_BAR_OFFSET = 10.0f;

sf::Vector2f normalize(const sf::Vector2f& vector) {
    float length = std::sqrt(vector.x * vector.x + vector.y * vector.y);
    if (length != 0) {
        return sf::Vector2f(vector.x / length, vector.y / length);
    }
    return vector; // Return the original vector if its length is zero
}


Enemy::Enemy(sf::Vector2f spawnPosition, sf::Texture& texture, float speed, float health, bool flying, bool canCharge)
    : gravity(5), OnGround(false), velocity(0, 0), speed(speed), texture(texture),
    health(health), maxHealth(health), following(false), facingRight(false), isFlying(flying),
    canCharge(canCharge), isCharging(false), chargeTimer(0.0f), chargeCooldown(0.0f),
    isTelegraphing(false), telegraphTimer(0.0f), hitFlashTimer(0.0f), hitRotation(0.0f),
    hitBounceTimer(0.0f), originalY(spawnPosition.y)
{
    sprite.setTexture(texture);
    // Set origin to center
    sprite.setOrigin(sprite.getGlobalBounds().width / 2.f, sprite.getGlobalBounds().height / 2.f);
    sprite.setPosition(spawnPosition);

    // Initialize health bars
    healthBarBackground.setSize(sf::Vector2f(HEALTH_BAR_WIDTH, HEALTH_BAR_HEIGHT));
    healthBarBackground.setFillColor(sf::Color(100, 100, 100));
    healthBarBackground.setOrigin(HEALTH_BAR_WIDTH / 2.f, 0); // Center horizontally

    healthBarFill.setSize(sf::Vector2f(HEALTH_BAR_WIDTH, HEALTH_BAR_HEIGHT));
    healthBarFill.setFillColor(sf::Color::Red);
    healthBarFill.setOrigin(HEALTH_BAR_WIDTH / 2.f, 0); // Center horizontally
}

void Enemy::draw(sf::RenderWindow& window) {

    // Hit flash
    if (hitFlashTimer > 0) {
        float flashIntensity = (hitFlashTimer / HIT_FLASH_DURATION);
        sprite.setColor(sf::Color(255,
            255 - (155 * flashIntensity),
            255 - (155 * flashIntensity),
            255));
    }
    else if (!isDeathAnimating) {
        sprite.setColor(isTelegraphing ? sf::Color(255, 200, 200) : sf::Color::White);
    }

    // Apply hit rotation
    if ((knockbackActive && isFlying) || isDeathAnimating) {
        sprite.setRotation(hitRotation);
    }
    else {
        sprite.setRotation(0);
    }


    // Simply flip the sprite scale for direction
    sprite.setScale(facingRight ? -1.0f : 1.0f, 1.0f);

    // Set telegraph color
    if (isTelegraphing) {
        sprite.setColor(sf::Color(255, 200, 200));
    }
    else {
        sprite.setColor(sf::Color::White);
    }

    window.draw(sprite);

    // Update health bar position (centered above sprite)
    sf::Vector2f healthBarPos = sprite.getPosition();
    healthBarPos.y -= sprite.getGlobalBounds().height / 2 + HEALTH_BAR_OFFSET;

    float healthPercent = health / maxHealth;
    healthBarFill.setSize(sf::Vector2f(HEALTH_BAR_WIDTH * healthPercent, HEALTH_BAR_HEIGHT));

    healthBarBackground.setPosition(healthBarPos);
    healthBarFill.setPosition(healthBarPos);

    window.draw(healthBarBackground);
    window.draw(healthBarFill);
}

void Enemy::update(float deltaTime, std::vector<Ground>& grounds, int& currency) {
    // Update hit flash timer
    if (hitFlashTimer > 0) {
        hitFlashTimer -= deltaTime;
    }

    if (damageCooldownTimer > 0.0f) {
        damageCooldownTimer -= deltaTime;
    }

    if (knockbackActive) {
        knockbackTimer += deltaTime;
        float t = knockbackTimer / knockbackDuration;

            if (t >= 1.0f) {
                knockbackActive = false;
                knockbackTimer = 0.0f;
                hitRotation = 0.0f;
                sprite.setRotation(0.0f);
            }
            else {
                // Apply rotation during knockback
                hitRotation += HIT_ROTATION_SPEED * deltaTime;

                // Calculate new position with bounce effect
                float bounceHeight = HIT_BOUNCE_HEIGHT * std::sin(t * 3.1415);
                sf::Vector2f newPos = knockbackStartPosition + knockbackDirection * knockbackDistance * t;
                newPos.y -= bounceHeight;

                // Check if new position would be inside any ground
                bool positionValid = true;
                for (const auto& ground : grounds) {
                    if (ground.getBounds().contains(newPos)) {
                        positionValid = false;
                        break;
                    }
                }

                // Check screen bounds with buffer
                sf::Vector2f halfSize(sprite.getGlobalBounds().width / 2, sprite.getGlobalBounds().height / 2);
                if (newPos.x < WALL_BUFFER + halfSize.x ||
                    newPos.x > SCREEN_WIDTH - WALL_BUFFER - halfSize.x) {
                    positionValid = false;
                }

                // Only update position if valid
                if (positionValid) {
                    sprite.setPosition(newPos);
                }
                else {
                    // End knockback early if position is invalid
                    knockbackActive = false;
                    knockbackTimer = 0.0f;
                    hitRotation = 0.0f;
                    sprite.setRotation(0.0f);
                }
                return;
            }
    }

    if (chargeCooldown > 0.0f) {
        chargeCooldown -= deltaTime;
    }

    if (isFlying) {
        hoverTime += deltaTime * 2.0f;
        hoverOffset = std::sin(hoverTime);

        float distanceToTarget = std::sqrt(
            std::pow(targetPosition.x - sprite.getPosition().x, 2) +
            std::pow(targetPosition.y - sprite.getPosition().y, 2)
        );

        following = distanceToTarget < DETECTION_RANGE * 10;
        sf::Vector2f direction;
        if (following)
            direction = normalize(targetPosition - sprite.getPosition());
        else
            direction = sf::Vector2f{ 1,1 };
        sprite.setPosition(
            sprite.getPosition().x + direction.x * speed * deltaTime,
            sprite.getPosition().y + direction.y * speed * deltaTime + hoverOffset
        );
    }
    else {
        float distanceToTarget = std::sqrt(
            std::pow(targetPosition.x - sprite.getPosition().x, 2) +
            std::pow(targetPosition.y - sprite.getPosition().y, 2)
        );

        following = distanceToTarget < DETECTION_RANGE;

        if (isTelegraphing) {
            velocity.x = 0;
            facingRight = targetPosition.x > sprite.getPosition().x;

            telegraphTimer += deltaTime;
            if (telegraphTimer >= CHARGE_TELEGRAPH_DURATION) {
                isTelegraphing = false;
                isCharging = true;
                chargeTimer = 0.0f;
            }
        }
        else if (isCharging) {
            // Ground check during charging
            sf::Vector2f groundCheckPos = sprite.getPosition();
            groundCheckPos.x += (facingRight ? GROUND_CHECK_DISTANCE : -GROUND_CHECK_DISTANCE);
            groundCheckPos.y += sprite.getGlobalBounds().height / 2 + 5.0f;

            bool groundAhead = false;
            for (const auto& ground : grounds) {
                if (ground.getBounds().contains(groundCheckPos)) {
                    groundAhead = true;
                    break;
                }
            }

            if (!groundAhead) {
                // Stop charging if no ground ahead
                isCharging = false;
                chargeTimer = 0.0f;
                chargeCooldown = CHARGE_COOLDOWN;
                velocity.x = 0;
            }
            else {
                // Continue charging
                chargeTimer += deltaTime;
                if (chargeTimer >= CHARGE_DURATION) {
                    isCharging = false;
                    chargeTimer = 0.0f;
                    chargeCooldown = CHARGE_COOLDOWN;
                }
                velocity.x = (facingRight ? 1 : -1) * speed * CHARGE_SPEED_MULTIPLIER;
            }
        }
        else if (following) {
            float direction = targetPosition.x - sprite.getPosition().x > 0 ? 1 : -1;
            velocity.x = direction * speed;
            facingRight = direction > 0;

            if (canCharge && distanceToTarget < DETECTION_RANGE && chargeCooldown <= 0.0f) {
                isTelegraphing = true;
                telegraphTimer = 0.0f;
                velocity.x = 0;
            }
        }
        else {
            velocity.x = (facingRight ? 1 : -1) * speed;

            // Ground and wall checks need to account for centered origin
            sf::Vector2f originOffset = sprite.getOrigin();
            sf::Vector2f spriteBounds = sf::Vector2f(sprite.getGlobalBounds().width, sprite.getGlobalBounds().height);

            sf::Vector2f groundCheckPos = sprite.getPosition();
            groundCheckPos.x += (facingRight ? GROUND_CHECK_DISTANCE : -GROUND_CHECK_DISTANCE);
            groundCheckPos.y += spriteBounds.y / 2 + 5.0f;

            sf::Vector2f wallCheckPos = sprite.getPosition();
            wallCheckPos.x += (facingRight ? WALL_CHECK_DISTANCE : -WALL_CHECK_DISTANCE);

            bool groundAhead = false;
            bool wallAhead = false;

            for (const auto& ground : grounds) {
                if (ground.getBounds().contains(groundCheckPos)) {
                    groundAhead = true;
                }
                if (ground.getBounds().contains(wallCheckPos)) {
                    wallAhead = true;
                }
            }

            if (!groundAhead || wallAhead ||
                (facingRight && sprite.getPosition().x >= SCREEN_WIDTH - spriteBounds.x / 2) ||
                (!facingRight && sprite.getPosition().x <= spriteBounds.x / 2)) {
                facingRight = !facingRight;
                velocity.x = -velocity.x;
                if (isCharging || isTelegraphing) {
                    isCharging = false;
                    isTelegraphing = false;
                    chargeTimer = 0.0f;
                    chargeCooldown = CHARGE_COOLDOWN;
                }
            }
        }

        if (!OnGround)
            velocity.y += gravity;

        OnGround = false;
        for (auto& ground : grounds) {
            if (sprite.getGlobalBounds().intersects(ground.getBounds())) {
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

                float overlapTop = spriteBottom - groundTop;
                float overlapBottom = groundBounds.top + groundBounds.height - spriteBounds.top;
                float overlapLeft = spriteRight - groundLeft;
                float overlapRight = groundRight - spriteLeft;

                float minOverlap = std::min({ overlapTop, overlapBottom, overlapLeft, overlapRight });

                if (minOverlap == overlapTop && velocity.y > 0) {
                    sprite.setPosition(sprite.getPosition().x,
                        groundTop - (spriteBounds.height - originOffset.y));
                    velocity.y = 0;
                    OnGround = true;
                }
                else if (minOverlap == overlapLeft && velocity.x > 0) {
                    sprite.setPosition(groundLeft - (spriteBounds.width - originOffset.x),
                        sprite.getPosition().y);
                    velocity.x = 0;
                    if (isCharging || isTelegraphing) {
                        isCharging = false;
                        isTelegraphing = false;
                        chargeTimer = 0.0f;
                        chargeCooldown = CHARGE_COOLDOWN;
                    }
                }
                else if (minOverlap == overlapRight && velocity.x < 0) {
                    sprite.setPosition(groundRight + originOffset.x,
                        sprite.getPosition().y);
                    velocity.x = 0;
                    if (isCharging || isTelegraphing) {
                        isCharging = false;
                        isTelegraphing = false;
                        chargeTimer = 0.0f;
                        chargeCooldown = CHARGE_COOLDOWN;
                    }
                }
            }
        }
    }

    sprite.move(velocity * deltaTime);

    if (health <= 0) {
        alive = false;
        srand(time(0));
        currency += std::rand() % 11 + 20;
    }

    // Screen bounds checking with centered origin
    sf::Vector2f halfSize(sprite.getGlobalBounds().width / 2, sprite.getGlobalBounds().height / 2);
    if (sprite.getPosition().x < halfSize.x)
        sprite.setPosition(halfSize.x, sprite.getPosition().y);
    else if (sprite.getPosition().x > SCREEN_WIDTH - halfSize.x)
        sprite.setPosition(SCREEN_WIDTH - halfSize.x, sprite.getPosition().y);
    if (sprite.getPosition().y > SCREEN_HEIGHT)
        health = 0;
}

void Enemy::takeDamage(float damage, const sf::Vector2f& hitDirection, float knockbackDistance) {
    if(knockbackActive || damageCooldownTimer > 0) return;

    if (isCharging || isTelegraphing) {
        health -= damage;
        damageCooldownTimer = 0.2f;
        hitFlashTimer = HIT_FLASH_DURATION;
        return;
    }

    health -= damage;
    if (health < 0) health = 0;
    knockbackActive = true;
    knockbackStartPosition = sprite.getPosition();
    knockbackDirection = normalize(hitDirection); // Make sure to normalize the hit direction
    this->knockbackDistance = knockbackDistance;
    knockbackTimer = 0.0f;
    knockbackDuration = 0.3f; // Increased duration for better effect
    damageCooldownTimer = 0.2f;
    hitFlashTimer = HIT_FLASH_DURATION;
    hitRotation = 0.0f; // Reset rotation
    hitBounceTimer = 0.0f; // Reset bounce timer
}

void Enemy::setTarget(const sf::Vector2f& target) {
    targetPosition = target;
}

bool Enemy::isAlive() {
    return alive;
}

bool Enemy::hit() {
    return knockbackActive;
}

float Enemy::getHealth() {
    return health;
}

sf::FloatRect Enemy::getBounds() {
    return sprite.getGlobalBounds();
}

sf::Vector2f Enemy::position() {
    return sprite.getPosition();
}

