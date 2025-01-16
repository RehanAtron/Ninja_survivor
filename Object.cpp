#include "Object.h"
#include "PlayerCharacter.h"
#include <iostream>
#include <random>
#include <ctime>
#include "Item.cpp"

const float SCREEN_WIDTH = 1280;
const float SCREEN_HEIGHT = 720;

Object::Object(const sf::Vector2f& position, const sf::Texture& textureFile, bool awarding)
    : texture(textureFile), chest(awarding), interacted(false)
{
    sprite.setTexture(textureFile);
    sprite.setPosition(position);

    std::vector<Item> predefinedItems = {
    Item("Flaming Sword", 5, 0, 1.5f, 100),       
    Item("Small Health Potion", 0, 2, 1.0f, 20),        
    Item("Full Health Potion", 0, 9, 1.0f, 50),       
    Item("Sword of Shadows", 0, 0, 1.2f, 100),      
    Item("Enchanted Sword", 4, 0, 1.1f, 75),    
    Item("Totem of Undying", 1, 0, 1.5f, 150),           
    };

    // Randomize storedItems
    std::srand(time(0));

    // Shuffle the predefined items
    std::random_shuffle(predefinedItems.begin(), predefinedItems.end());

    // Choose a random number of items (1 to 3) and add them to storedItems
    int numItems = std::rand() % 2 + 2; // Randomly select 1 to 3 items
    for (int i = 0; i < numItems; ++i) {
        storedItems.push_back(predefinedItems[i]);
    }
}

void Object::draw(sf::RenderWindow& window)
{
    window.draw(sprite);
}

void Object::interact(Player& player, std::vector<Item>& items, sf::RenderWindow& window, int& currency)
{
    sf::Font font;
    if (!font.loadFromFile("Textures/font.ttf"))
        return;

    // Background setup
    sf::RectangleShape background;
    background.setSize(sf::Vector2f(1000, 600));  // Adjust size as needed
    background.setFillColor(sf::Color(50, 50, 50, 200));  // Translucent gray
    background.setPosition((SCREEN_WIDTH - 1000) / 2, (SCREEN_HEIGHT - 600) / 2);  // Center the background

    // Shop title setup
    sf::Text titleText;
    titleText.setFont(font);
    titleText.setString("SHOP");
    titleText.setCharacterSize(36);
    titleText.setFillColor(sf::Color::White);
    titleText.setStyle(sf::Text::Bold);
    // Center the title
    titleText.setPosition(
        (SCREEN_WIDTH - titleText.getLocalBounds().width) / 2,
        background.getPosition().y + 20
    );

    // Menu text setup
    sf::Text menuText;
    menuText.setFont(font);
    menuText.setCharacterSize(24);
    menuText.setFillColor(sf::Color::White);
    menuText.setPosition(
        background.getPosition().x + 50,  // Padding from left edge of background
        background.getPosition().y + 80   // Below title
    );

    // Currency text setup
    sf::Text currencyText;
    currencyText.setFont(font);
    currencyText.setCharacterSize(24);
    currencyText.setFillColor(sf::Color::Yellow);
    currencyText.setStyle(sf::Text::Bold);

    // Exit instruction text setup
    sf::Text exitText;
    exitText.setFont(font);
    exitText.setCharacterSize(20);
    exitText.setFillColor(sf::Color::White);
    exitText.setString("Press Enter or Escape to exit");

    // Feedback message setup
    sf::Text feedbackText;
    feedbackText.setFont(font);
    feedbackText.setCharacterSize(24);
    feedbackText.setPosition(
        (SCREEN_WIDTH - feedbackText.getLocalBounds().width) / 2,
        background.getPosition().y + background.getSize().y - feedbackText.getLocalBounds().height
    );
    feedbackText.setFillColor(sf::Color::Green);

    std::string feedbackMessage;
    float feedbackTimer = 0.0f;
    const float FEEDBACK_DURATION = 2.0f;

    bool menuOpen = true;
    sf::Clock clock;

    while (menuOpen && window.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();

        if (feedbackTimer > 0.0f) {
            feedbackTimer -= deltaTime;
            if (feedbackTimer <= 0.0f) {
                feedbackMessage.clear();
            }
        }

        // Create the menu string with detailed item stats
        std::string menuString = "Available Items:\n\n";
        for (size_t i = 0; i < storedItems.size(); ++i) {
            const Item& item = storedItems[i];
            menuString += std::to_string(i + 1) + ". " + item.getName() + "\n";
            menuString += "   Price: " + std::to_string(item.getPrice()) + " coins\n";

            // Show stat changes with colored brackets
            if (item.getDamage() > 0) {
                menuString += "   [Damage: +" + std::to_string(item.getDamage()) + "]\n";
            }
            if (item.getHealth() > 0) {
                menuString += "   [Health: +" + std::to_string(item.getHealth()) + "]\n";
            }
            if (item.getDamageMultiplier() > 1.0f) {
                menuString += "   [Damage Multiplier: x" +
                    std::to_string(item.getDamageMultiplier()) + "]\n";
            }
            menuString += "\n";  // Add spacing between items
        }
        menuText.setString(menuString);

        currencyText.setString(std::to_string(currency));
        currencyText.setPosition(
            background.getPosition().x + background.getSize().x - currencyText.getLocalBounds().width - 20,
            background.getPosition().y + 20
        );

        exitText.setPosition(
            (SCREEN_WIDTH - exitText.getLocalBounds().width) / 2,
            background.getPosition().y + background.getSize().y - 40
        );

        feedbackText.setString(feedbackMessage);
        feedbackText.setPosition(
            (SCREEN_WIDTH - feedbackText.getLocalBounds().width) / 2,
            background.getPosition().y + background.getSize().y - 100
        );

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed)
            {
                int key = event.key.code - sf::Keyboard::Num1;
                if (key >= 0 && key < static_cast<int>(storedItems.size())) {
                    if (currency >= storedItems[key].getPrice()) {
                        // Store old stats for comparison
                        float oldMultiplier = player.calculateTotalDamageMultiplier();
                        int oldDamage = player.getDamage();

                        // Purchase and update
                        items.push_back(storedItems[key]);
                        currency -= storedItems[key].getPrice();


                        player.recalculateStats();

                        // Generate feedback with stat changes
                        feedbackMessage = "Purchased: " + storedItems[key].getName() + "\n";
                        if (player.getDamage() > oldDamage) {
                            feedbackMessage += "Damage: +" +
                                std::to_string(player.getDamage() - oldDamage) + "\n";
                        }
                        if (player.calculateTotalDamageMultiplier() > oldMultiplier) {
                            feedbackMessage += "Multiplier: +" +
                                std::to_string(player.calculateTotalDamageMultiplier() - oldMultiplier) + "x";
                        }
                        feedbackText.setFillColor(sf::Color::Green);
                        feedbackTimer = FEEDBACK_DURATION;

                        // Remove the purchased item from storedItems
                        storedItems.erase(storedItems.begin() + key);
                    }
                    else {
                        feedbackMessage = "Not enough currency for: " + storedItems[key].getName();
                        feedbackText.setFillColor(sf::Color::Red);
                        feedbackTimer = FEEDBACK_DURATION;
                    }
                }
                else if (event.key.code == sf::Keyboard::Escape ||
                    event.key.code == sf::Keyboard::Enter) {
                    menuOpen = false;
                }
            }
        }

        window.clear(sf::Color(0, 0, 0, 150));  // Slightly darken the game background
        window.draw(background);
        window.draw(titleText);
        window.draw(menuText);
        window.draw(currencyText);
        window.draw(exitText);
        if (!feedbackMessage.empty()) {
            window.draw(feedbackText);
        }
        window.display();
    }

    interacted = true;
}

sf::FloatRect Object::getBounds()
{
    return sprite.getGlobalBounds();
}

bool Object::isInteracted()
{
    return interacted;
}