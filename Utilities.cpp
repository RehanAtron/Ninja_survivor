#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <Windows.h>

const float FadeSpeed = 10;

static void FadeIn(sf::Sprite& sprite, sf::RenderWindow& window, float deltatime)
{

    sf::Color currentColor = sprite.getColor();
    if (currentColor.a < 255) currentColor.a = 255;
    sprite.setColor(currentColor);
    window.draw(sprite);
}

static void FadeOut(sf::Sprite& sprite, sf::RenderWindow& window, float deltatime)
{

    sf::Color currentColor = sprite.getColor();
    if (currentColor.a > 0)
    {
        currentColor.a -= FadeSpeed * deltatime;
        if (currentColor.a < 0) currentColor.a = 0;
    }
    sprite.setColor(currentColor);
    window.draw(sprite);
}