#ifndef ITEM_H
#define ITEM_H

#include <string>

class Item
{
public:
    // Default constructor
    Item()
        : name("Unknown"), damage(0), health(0),
        damageMultiplier(1.0f), price(0) {}

    // Parameterized constructor
    Item(const std::string& name, int damage, int health,
        float damageMultiplier, int price)
        : name(name), damage(damage), health(health),
        damageMultiplier(damageMultiplier), price(price){}

    // Getters for the properties
    std::string getName() const { return name; }
    int getDamage() const { return damage; }
    int getHealth() const { return health; }
    int getPrice() const { return price; }

    float getDamageMultiplier() const { return damageMultiplier; }

private:
    int price;
    std::string name;        
    int damage;                // Damage value of the item (if it's a weapon)
    int health;                // Health value (if it's a healing item)
    float damageMultiplier;    // Damage multiplier
};

#endif // !ITEM_H
