#ifndef GROUND_H
#define GROUND_H

#include <SFML/Graphics.hpp>

class Ground {
public:
    Ground(float x, float y, float width, float height);  
    Ground(float y, float width, float height);
    void draw(sf::RenderWindow& window);                  
    sf::FloatRect getBounds() const;                     

private:
    sf::RectangleShape shape;  
};

#endif // GROUND_H
