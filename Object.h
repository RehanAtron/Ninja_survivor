#ifndef OBJECT_H
#define OBJECT_H

#include <SFML/Graphics.hpp>
#include <vector>
#include "Item.cpp"
#include "PlayerCharacter.h"

class Object
{
public:
    // Constructor
    Object(const sf::Vector2f& position, const sf::Texture& textureFile, bool awarding);

    // Member functions
    void draw(sf::RenderWindow& window);
    void interact(Player& player,std::vector<Item>& items, sf::RenderWindow& window, int& currency);

    sf::FloatRect getBounds();
    bool isInteracted();  

private:
    // Member variables
    sf::Sprite sprite;
    sf::Texture texture;
    bool chest;
    bool interacted;
    std::vector<Item> storedItems;
};

#endif // !OBJECT_H
