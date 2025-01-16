#include "Ground.h"
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
using namespace sf;

Ground::Ground(float x, float y, float width, float height) {
    shape.setSize(sf::Vector2f(width, height));
    shape.setPosition(x, y);
    shape.setFillColor(sf::Color(21,21,28));  
}
Ground::Ground(float y, float width, float height) {
    shape.setSize(sf::Vector2f(width, height));
    shape.setPosition(0, y);
    shape.setFillColor(sf::Color(21, 21, 28));
}

void Ground::draw(sf::RenderWindow& window) {
    window.draw(shape);
}

sf::FloatRect Ground::getBounds() const {
    return shape.getGlobalBounds();
}
