#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <Windows.h>
#include <vector>
#include <iostream>

#include "PlayerCharacter.h"
#include "Ground.h"
#include "Enemy.h"
#include "Object.h"

#include "Item.cpp"
#include "Levels.cpp"
#include "Utilities.cpp"
using namespace sf;

// Constants
const std::string gameName = "Ninja Survivor";
const double movementSpeed = 500;

const float HEALTH_BAR_WIDTH = 200.0f;
const float HEALTH_BAR_HEIGHT = 20.0f;

const float SCREEN_WIDTH = 1280;
const float SCREEN_HEIGHT = 720;

// Global Variables
int totalLevels;
int LevelNumber = -1;
int gravity = 500;
int currency = 0;
bool isShopping = false;
bool isPaused = false;
bool gameOver = false;
bool forceReload = false;

// Function Prototypes
void MainMenu(RenderWindow& window, bool& inMainMenu);
void DisplayInstructions(RenderWindow& window);
Texture& TextureManager(const std::string& texturePath);
static void LevelManager(Player& player, Level& level, int& prev, float deltaTime, RenderWindow& window, std::vector<Enemy>& enemies, std::vector<Object>& objects);
void DeathMenu(Player& player, Level& level, int& prev, float deltaTime, RenderWindow& window, std::vector<Enemy>& enemies, std::vector<Object>& objects);
void enforceBounds(Player& player, int enemies, Level& level);
void PauseMenu(RenderWindow& window, bool& isShopping);

static void AttachConsole() {
    AllocConsole();
    FILE* stream;
    freopen_s(&stream, "CONOUT$", "w", stdout);
}

int main()
{
    // Debug
    //AttachConsole();

    RenderWindow window(VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), gameName);

    // Game objects
    bool inMainMenu = true;
    Player player(Vector2f(10, 10), TextureManager("Textures/Player.png"), TextureManager("Textures/Weapon1.png"), movementSpeed);
    Level level(-1, SCREEN_WIDTH, SCREEN_HEIGHT);
    std::vector<Enemy> enemies;
    std::vector<Object> objects;
    int previousLevel = LevelNumber;

    // Font and Text Setup
    Font font;
    if (!font.loadFromFile("Textures/font.ttf")) {
        std::cerr << "Failed to load font!" << std::endl;
        return -1;
    }
    Text currencyText;
    currencyText.setFont(font);
    currencyText.setCharacterSize(24); // Font size
    currencyText.setFillColor(Color::White);
    currencyText.setStyle(Text::Bold);

    // Health Bar Setup
    Text healthBarText;
    healthBarText.setFont(font);
    healthBarText.setCharacterSize(24); // Font size
    healthBarText.setFillColor(Color::White);
    healthBarText.setStyle(Text::Bold);
    healthBarText.setString("Health:");
    healthBarText.setPosition(0, 0);

    RectangleShape healthBarBackground(Vector2f(HEALTH_BAR_WIDTH, HEALTH_BAR_HEIGHT));
    healthBarBackground.setFillColor(Color(50, 50, 50)); // Background color
    healthBarBackground.setPosition(20, 40);

    RectangleShape healthBar(Vector2f(HEALTH_BAR_WIDTH, HEALTH_BAR_HEIGHT));
    healthBar.setFillColor(Color::Red); // Health bar color
    healthBar.setPosition(20, 40);

    Clock clock;

    // Main game loop
    while (window.isOpen())
    {
        // Handle events
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
        }

        float deltaTime = clock.restart().asSeconds();

        if (LevelNumber == -1) {
            MainMenu(window, inMainMenu);
            if (!inMainMenu) {
                LevelNumber = 0;
                level = Level(0, SCREEN_WIDTH, SCREEN_HEIGHT);
                player.SetPosition(level.spawnPosition);
                previousLevel = LevelNumber;
            }
        }
        else if (!isShopping && !gameOver && !isPaused)
        {
            // Update game

            player.update(deltaTime, level.grounds);
            player.handleCollision(enemies, deltaTime);

            // Update currency text
            currencyText.setString("Currency: " + std::to_string(currency));
            currencyText.setPosition(SCREEN_WIDTH - currencyText.getLocalBounds().width - 10, 10);

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
                isPaused = true;
            }

            // Update health bar
            float healthPercentage = static_cast<float>(player.getHealth()/10);
            healthBar.setSize(Vector2f(HEALTH_BAR_WIDTH * healthPercentage, HEALTH_BAR_HEIGHT));

            // Clear, draw, and display
            window.clear(Color(18, 32, 32));

            for (Ground& ground : level.grounds)
                ground.draw(window);

            LevelManager(player, level, previousLevel, deltaTime, window, enemies,objects);
            // Display instructions if in level 0
            if (LevelNumber == 0) {
                DisplayInstructions(window);
            }

            player.draw(window);

            window.draw(currencyText);
            window.draw(healthBarText);
            window.draw(healthBarBackground);
            window.draw(healthBar);

            window.display();

        }
        else if (gameOver)
        {
            DeathMenu(player, level, previousLevel, deltaTime, window, enemies, objects);
        }
        else if (isPaused) {
            PauseMenu(window, isPaused);
        }
        else {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Enter))
            {
                isShopping = false;
            }
        }
        
    }

    return 0;
}

static void LevelManager(Player& player, Level& level, int& prev, float deltaTime, RenderWindow& window, std::vector<Enemy>& enemies, std::vector<Object>& objects)
{

    // Restart the game if the player's health is 0
    if (player.getHealth() <= 0) {
        gameOver = true;
        return;
    }

    Texture Enemy1 = TextureManager("Textures/Enemy1.png");
    Texture Enemy2 = TextureManager("Textures/Enemy2.png");
    Texture Enemy3 = TextureManager("Textures/Enemy3.png");
    Texture Enemy4 = TextureManager("Textures/Enemy4.png");
    Texture Chest = TextureManager("Textures/Chest.png");


    // Checking if level changed or force reload
    if (LevelNumber != prev || forceReload)
    {
        std::cout << LevelNumber << std::endl;
        objects.clear();
        level = Level(LevelNumber, SCREEN_WIDTH, SCREEN_HEIGHT);
        prev = LevelNumber;
        player.SetPosition(level.spawnPosition);
        switch (LevelNumber) {
        case 0:
            objects = { Object(sf::Vector2f(SCREEN_WIDTH / 2 - 81,SCREEN_HEIGHT * 7 / 8 - 60),Chest,true) };
            break;
        case 1:
            enemies = { Enemy(sf::Vector2f(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2), Enemy1,100,10,false, false),
                Enemy(sf::Vector2f(SCREEN_WIDTH * 3 / 4, SCREEN_HEIGHT * 7/8 ), Enemy3,100,3,false, false) };
            break;
        case 9:
        case 4:
            enemies = { Enemy(sf::Vector2f(SCREEN_WIDTH / 4, SCREEN_HEIGHT * 7 / 8), Enemy1,100,10,false, false),
                Enemy(sf::Vector2f(SCREEN_WIDTH * 3 / 4, SCREEN_HEIGHT / 2 ), Enemy3,100,3,false, false) };
            break;
        case 2:
            enemies = { Enemy(sf::Vector2f(SCREEN_WIDTH * 3 / 4, SCREEN_HEIGHT / 2), Enemy4,150,3,true, false), 
           Enemy(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 4), Enemy4, 150 ,3,true, false) };
            break;
        case 3:
            enemies = { Enemy(sf::Vector2f(SCREEN_WIDTH * 3 / 4, SCREEN_HEIGHT / 4), Enemy4,150,3,true, false),
            Enemy(sf::Vector2f(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2), Enemy4,150,3,true, false) };
            break;
        case 6:
            enemies = { Enemy(sf::Vector2f(SCREEN_WIDTH * 3 / 4, SCREEN_HEIGHT * 7 / 8), Enemy2,150,20,false,true) };
            break;
        case 7:
            enemies = { Enemy(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 3), Enemy3,100,3,false,false),
            Enemy(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT * 7 / 8), Enemy1,100,10,false,false),
            Enemy(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT * 7 / 8), Enemy1,100,10,false,false) };
            break;
        case 8:
            enemies = { Enemy(sf::Vector2f(SCREEN_WIDTH * 3 / 4, SCREEN_HEIGHT / 2), Enemy2,150,20,false,true),
                Enemy(sf::Vector2f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 4), Enemy4, 150 ,3,true, false)
            };
            break;
        case 5:
        case 10:
            objects = { Object(sf::Vector2f(SCREEN_WIDTH / 2 - 81,SCREEN_HEIGHT / 2 - 60),Chest,true) };
            break;
        }
        forceReload = false;
    }
    
    enforceBounds(player, enemies.size(),level);

    for (Object& object: objects)
    {
        if (player.getBounds().intersects(object.getBounds()) && sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
            object.interact(player, player.items, window, currency); // Interact with the object
            isShopping = true;
        }
        object.draw(window);
    }
    // Enemy Management
    for (Enemy& enemy : enemies)
    {
        if (!enemy.isAlive())
            continue;
        enemy.update(deltaTime, level.grounds, currency);
        enemy.setTarget(player.position());
        enemy.draw(window);
    }

    enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](Enemy& enemy) {
        return !enemy.isAlive(); // Remove if the enemy is not alive
        }), enemies.end());
    objects.erase(std::remove_if(objects.begin(), objects.end(), [](Object& object) {
        return object.isInteracted(); // Remove if the enemy is not alive
        }), objects.end());

}

Texture& TextureManager(const std::string& texturePath) {
    static std::map<std::string, sf::Texture> textureCache;  // Cache of textures

    // Check if texture is already loaded
    if (textureCache.find(texturePath) != textureCache.end()) {
        return textureCache[texturePath];  // Return cached texture
    }

    // Load new texture and cache it
    sf::Texture newTexture;
    if (!newTexture.loadFromFile(texturePath)) {
        std::cerr << "Failed to load texture!" << std::endl;
    }
    else {
        textureCache[texturePath] = newTexture;  // Store it in cache
    }

    return textureCache[texturePath];
}

void DeathMenu(Player& player, Level& level, int& prev, float deltaTime, RenderWindow& window, std::vector<Enemy>& enemies, std::vector<Object>& objects)
{
    // Create Game Over menu
    Font font;
    if (!font.loadFromFile("Textures/font.ttf")) {
        std::cerr << "Failed to load font for Game Over menu!" << std::endl;
        return;
    }

    // "Game Over" text
    sf::Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setCharacterSize(64);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setString("Game Over");
    gameOverText.setPosition(SCREEN_WIDTH / 2 - gameOverText.getLocalBounds().width / 2, SCREEN_HEIGHT / 3);

    // Final Score text
    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(48);
    scoreText.setFillColor(sf::Color::Yellow);
    scoreText.setString("Final Score: " + std::to_string(currency));
    scoreText.setPosition(
        SCREEN_WIDTH / 2 - scoreText.getLocalBounds().width / 2,
        SCREEN_HEIGHT / 2 - scoreText.getLocalBounds().height / 2
    );

    // "Restart" button text
    sf::Text restartText;
    restartText.setFont(font);
    restartText.setCharacterSize(36);
    restartText.setFillColor(sf::Color::White);
    restartText.setString("Restart");
    restartText.setPosition(
        SCREEN_WIDTH / 2 - restartText.getLocalBounds().width / 2,
        SCREEN_HEIGHT * 2 / 3
    );

    // Display the Game Over menu
    window.clear(sf::Color(0, 0, 0));
    window.draw(gameOverText);
    window.draw(scoreText);
    window.draw(restartText);
    window.display();

    // Handle events for restarting
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();

        if ((event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left &&
            restartText.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y)) || (
                sf::Keyboard::isKeyPressed(sf::Keyboard::Escape) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Enter))) {
            // Reset level
            player.SetHealth(10);
            currency = 0;
            LevelNumber = 0;
            level = Level(LevelNumber, SCREEN_WIDTH, SCREEN_HEIGHT);
            player.SetPosition(level.spawnPosition);
            player.items.clear();
            player.recalculateStats();
            prev = LevelNumber;
            enemies.clear();
            objects.clear();
            gameOver = false; // Exit game-over state
            return;
        }
    }
}


void enforceBounds(Player& player, int enemies, Level& level) {
    sf::Vector2f position = player.position();
    if (enemies) {
        if (position.x > SCREEN_WIDTH)
            position.x = SCREEN_WIDTH;
        player.SetPosition(position);
    }
    else if (position.x > SCREEN_WIDTH)
    {
        srand(time(0));
        LevelNumber = rand() % 10 + 1;
        forceReload = true;
    }
    if (position.x < 0)
    {
        position.x = 0;
        player.SetPosition(position);
    }
    if (position.y > SCREEN_HEIGHT)
    {
        player.ChangeHealth(-1);
        player.SetPosition(level.spawnPosition);
    }
}

void MainMenu(RenderWindow& window, bool& inMainMenu) {
    Font font;
    if (!font.loadFromFile("Textures/font.ttf")) {
        std::cerr << "Failed to load font for Main Menu!" << std::endl;
        return;
    }

    // Title
    Text titleText;
    titleText.setFont(font);
    titleText.setCharacterSize(72);
    titleText.setFillColor(Color::Red);
    titleText.setString("Ninja Survivor");
    titleText.setPosition(
        SCREEN_WIDTH / 2 - titleText.getLocalBounds().width / 2,
        SCREEN_HEIGHT / 3
    );

    // Play Button
    RectangleShape playButton(Vector2f(200, 50));
    playButton.setFillColor(Color(100, 100, 100));
    playButton.setPosition(
        SCREEN_WIDTH / 2 - 100,
        SCREEN_HEIGHT / 2 + 50
    );

    Text playText;
    playText.setFont(font);
    playText.setCharacterSize(36);
    playText.setFillColor(Color::White);
    playText.setString("Press Enter to Play");
    playText.setPosition(
        SCREEN_WIDTH / 2 - playText.getLocalBounds().width / 2,
        SCREEN_HEIGHT / 2 + 55
    );

    window.clear(Color(18, 32, 32));
    window.draw(titleText);
    window.draw(playText);
    window.display();

    // Handle button click
    if (Mouse::isButtonPressed(Mouse::Left)) {
        Vector2i mousePos = Mouse::getPosition(window);
        if (playText.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
            inMainMenu = false;
        }
    }

    // Allow keyboard input to start game
    if (Keyboard::isKeyPressed(Keyboard::Enter) ||
        Keyboard::isKeyPressed(Keyboard::Space)) {
        inMainMenu = false;
    }
}

void PauseMenu(RenderWindow& window, bool& isShopping) {

    Font font;
    if (!font.loadFromFile("Textures/font.ttf")) {
        std::cerr << "Failed to load font for Pause Menu!" << std::endl;
        return;
    }

    // Semi-transparent background
    RectangleShape overlay(Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT));
    overlay.setFillColor(Color(0, 0, 0, 150)); // Black with 150 alpha for transparency

    // "Paused" text
    Text pausedText;
    pausedText.setFont(font);
    pausedText.setCharacterSize(64);
    pausedText.setFillColor(Color::White);
    pausedText.setString("Paused");
    pausedText.setPosition(SCREEN_WIDTH / 2 - pausedText.getLocalBounds().width / 2, SCREEN_HEIGHT / 3);

    // Resume button
    Text resumeText;
    resumeText.setFont(font);
    resumeText.setCharacterSize(36);
    resumeText.setFillColor(Color::White);
    resumeText.setString("Resume (Enter)");
    resumeText.setPosition(SCREEN_WIDTH / 2 - resumeText.getLocalBounds().width / 2, SCREEN_HEIGHT / 2);

    // Quit button
    Text quitText;
    quitText.setFont(font);
    quitText.setCharacterSize(36);
    quitText.setFillColor(Color::White);
    quitText.setString("Quit (Esc)");
    quitText.setPosition(SCREEN_WIDTH / 2 - quitText.getLocalBounds().width / 2, SCREEN_HEIGHT / 2 + 50);

    // Draw the pause menu
    window.draw(overlay);
    window.draw(pausedText);
    window.draw(resumeText);
    window.draw(quitText);

    window.display();

    // Handle user input for the pause menu
    Event event;
    while (window.pollEvent(event)) {
        if (event.type == Event::Closed)
            window.close();

        if (Keyboard::isKeyPressed(Keyboard::Enter)) {
            isShopping = false; // Resume the game
        }
        if (Keyboard::isKeyPressed(Keyboard::Escape)) {
            window.close(); // Quit the game
        }
    }
}

void DisplayInstructions(RenderWindow& window) {
    Font font;
    if (!font.loadFromFile("Textures/font.ttf")) {
        return;
    }

    Text instructions;
    instructions.setFont(font);
    instructions.setCharacterSize(24);
    instructions.setFillColor(Color::White);
    instructions.setString(
        "Controls:\n"
        "WASD / Arrow Keys - Move\n"
        "Space - Jump\n"
        "Left Mouse Click / J / X - Swing Weapon\n"
        "E - Open Chest/Shop\n"
        "Shift - Dash"
    );

    // Center the text
    FloatRect textBounds = instructions.getLocalBounds();
    instructions.setPosition(SCREEN_WIDTH / 2 - textBounds.width / 2, SCREEN_HEIGHT / 4);

    window.draw(instructions);
}

