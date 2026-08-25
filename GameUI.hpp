#ifndef GAMEUI_HPP
#define GAMEUI_HPP

#include "common.hpp"
#include <vector>
#include <string>

struct MenuButton {
    sf::FloatRect bounds;
    int actionId;
};

struct SplashCard {
    float gridX;
    float gridY;
    float flipAngle;
    float flipSpeed;
    int assetTextureId;
    bool displayingBack;
};

class GameUI {
private:
    sf::Font font;
    bool hasFont;
    float animationTime;

    sf::Text headerText;
    sf::Text hudText;

    // Cyberpunk Color Palette Properties
    sf::Color colorBackground;
    sf::Color colorHiddenCard;
    sf::Color colorHiddenBorder;
    sf::Color colorHUDPanel;

    std::vector<MenuButton> activeMenuButtons;
    sf::FloatRect quitButtonBounds;
    sf::FloatRect yesButtonBounds;
    sf::FloatRect noButtonBounds;
    sf::FloatRect endMenuButtonBounds;
    sf::FloatRect nextLevelButtonBounds;
    sf::FloatRect restartButtonBounds; 
    sf::FloatRect restartConfirmationBounds;
    sf::Texture splashTexture;
    std::vector<SplashCard> splashCards;

    // Array pool upgraded to support 25 distinct texture slots
    sf::Texture cardTextures[25]; 


    
public:
    // Dynamic Grid Layout Geometry Tracking Attributes
    int ROWS;
    int COLS;
    int PAIRS;
    float CARD_WIDTH;
    float CARD_HEIGHT;
    float SPACING;
    float startX;
    float startY;

    GameUI();
    
    // Setup and dynamic recalculation framework 
    void loadLevelSettings(const LevelConfig& config);
    void updateViewportView(sf::RenderWindow& window);
    
    void drawPersistentLogo(sf::RenderWindow& window);

    // Core Graphics Pipeline Calls
    void drawBackground(sf::RenderWindow& window);
    void drawSplashScreen(sf::RenderWindow& window, float currentAlpha, float tProgress = 0.0f);
    void loadAssets(sf::Texture splashTexture, sf::Sprite splashSprite);
    void drawMainMenu(sf::RenderWindow& window);
    void drawGrid(sf::RenderWindow& window, const std::vector<Card>& deck, GameState state);
    void drawHUD(sf::RenderWindow& window, GameState gameState, int moves, int matches, float timeSeconds, int currentLevel);
    void drawQuitConfirmation(sf::RenderWindow& window);
    void drawLevelClearScreen(sf::RenderWindow& window, int clearedLevel, int moves, float timeRemaining);
    void drawEndGameScreen(sf::RenderWindow& window, GameState state, int moves, float time);

    // Coordinate Intersection Checks
    int getClickedMenuOption(sf::Vector2f mousePos);
    bool isQuitButtonClicked(sf::Vector2f mousePos);
    bool isRestartButtonClicked(sf::Vector2f m);
    bool isYesClicked(sf::Vector2f mousePos);
    bool isNoClicked(sf::Vector2f mousePos);
    bool isEndMenuButtonClicked(sf::Vector2f mousePos);
    bool isNextLevelButtonClicked(sf::Vector2f mousePos);
    bool isRestartConfirmationClicked(sf::Vector2f m);
    
};

#endif