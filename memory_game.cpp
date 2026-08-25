#include "common.hpp"
#include "GameUI.hpp"
#include <algorithm>
#include <random>
#include <iostream>
#include <vector>
#include <cstdlib> 

const LevelConfig classicLevels[5] = {
    {1, 4, 4, 0.0f, 100.f, 100.f, 15.f}, 
    {2, 4, 5, 0.0f, 90.f,  90.f,  12.f}, 
    {3, 4, 6, 0.0f, 85.f,  85.f,  12.f}, 
    {4, 6, 6, 0.0f, 70.f,  70.f,  10.f}, 
    {5, 6, 8, 0.0f, 62.f,  62.f,  8.f}   
};

const LevelConfig timeAttackLevels[5] = {
    {1, 4, 4, 150.f, 100.f, 100.f, 15.f}, 
    {2, 4, 5, 120.f, 90.f,  90.f,  12.f}, 
    {3, 4, 6, 60.f, 85.f,  85.f,  12.f}, 
    {4, 6, 6, 90.f, 70.f,  70.f,  10.f}, 
    {5, 6, 8, 40.f, 62.f,  62.f,  8.f}   
};

int main() {
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(800, 600)), "KIOKU - GROUP 9", sf::Style::Default);
    window.setFramerateLimit(60);



  



    sf::View gameView(sf::FloatRect(sf::Vector2f(0.f, 0.f), sf::Vector2f(800.f, 600.f)));
    window.setView(gameView);

    GameUI ui;
    std::vector<Card> deck;
    
    GameState gameState = GameState::SPLASH_SCREEN; 
    GameMode currentMode = GameMode::CLASSIC;
    
    int currentLevelIndex = 0; 
    int cumulativeMoves = 0;   
    float runningGameTime = 0.0f; 

    SplashStage splashStage = SplashStage::WAIT_INITIAL;
    float splashTimer = 0.0f, splashAlpha = 0.0f; 
    bool skipSplash = false;   

    int moves = 0, matches = 0, firstCardIndex = -1, secondCardIndex = -1, wrongAttempts = 0; 

    sf::Clock gameClock;
    sf::Clock stateClock; 
    
    float levelTimeLimit = 60.0f; 
    float levelTimePenalties = 0.0f; 

    auto setupActiveLevel = [&](int lvlIndex) {
        deck.clear();
        LevelConfig activeConfig = (currentMode == GameMode::CLASSIC) ? classicLevels[lvlIndex] : timeAttackLevels[lvlIndex];
        
        ui.loadLevelSettings(activeConfig);
        
        std::vector<int> imageIds;
        for (int p = 0; p < ui.PAIRS; ++p) {
            imageIds.push_back(p);
            imageIds.push_back(p);
        }
        
        std::random_device rd; std::mt19937 g(rd());
        std::shuffle(imageIds.begin(), imageIds.end(), g);

        for (int id : imageIds) { deck.push_back({id, CardState::HIDDEN}); }

        moves = 0; matches = 0; wrongAttempts = 0; levelTimePenalties = 0.0f;
        firstCardIndex = -1; secondCardIndex = -1;
        levelTimeLimit = activeConfig.baseTime;
        gameClock.restart();
    };

    while (window.isOpen()) {

        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (event->getIf<sf::Event::Resized>()) {
                gameView.setViewport(sf::FloatRect(sf::Vector2f(0.f, 0.f), sf::Vector2f(1.f, 1.f)));
                window.setView(gameView);
            }

            if (auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                if (gameState == GameState::SPLASH_SCREEN && (keyEvent->code == sf::Keyboard::Key::Space || keyEvent->code == sf::Keyboard::Key::Enter)) {
                    skipSplash = true;
                }
                if (keyEvent->code == sf::Keyboard::Key::Escape) window.close();
                if (keyEvent->code == sf::Keyboard::Key::Backspace && (gameState == GameState::PLAYING || gameState == GameState::PAUSED_REVEAL)) {
                    gameState = GameState::START_MENU;
                }
            }

            if (auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseEvent->button == sf::Mouse::Button::Left) {
                    if (gameState == GameState::SPLASH_SCREEN) skipSplash = true;
                    
                    sf::Vector2i pixelPos = {mouseEvent->position.x, mouseEvent->position.y};
                    sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos, window.getView());

                    if (gameState == GameState::START_MENU) {
                        int clickId = ui.getClickedMenuOption(worldPos);
                        if (clickId == 1 || clickId == 2) {
                            currentMode = (clickId == 1) ? GameMode::CLASSIC : GameMode::TIME_ATTACK;
                            currentLevelIndex = 0;
                            cumulativeMoves = 0;
                            runningGameTime = 0.0f;
                            setupActiveLevel(currentLevelIndex);
                            gameState = GameState::PLAYING;
                        } else if (clickId == 3) {
                            #ifdef _WIN32
                                std::system("start shuffle_game.html");
                            #else
                                std::system("open shuffle_game.html");
                            #endif
                        } else if (clickId == 4) {
                            window.close();
                        }
                    }
                    else if (gameState == GameState::PLAYING || gameState == GameState::PAUSED_REVEAL) {
                        // Check Top Nav Bar QUIT Button
                        if (ui.isQuitButtonClicked(worldPos)) {
                            gameState = GameState::QUIT_CONFIRM;
                        } 
                        // Process Card interactions if we're actively playing
                        else if (gameState == GameState::PLAYING) {
                            for (int i = 0; i < (int)deck.size(); ++i) {
                                if (deck[i].state == CardState::HIDDEN) {
                                    int r = i / ui.COLS; int c = i % ui.COLS;
                                    float x = ui.startX + c * (ui.CARD_WIDTH + ui.SPACING);
                                    float y = ui.startY + r * (ui.CARD_HEIGHT + ui.SPACING);

                                    if (worldPos.x >= x && worldPos.x <= x + ui.CARD_WIDTH && worldPos.y >= y && worldPos.y <= y + ui.CARD_HEIGHT) {
                                        if (firstCardIndex == -1) {
                                            firstCardIndex = i; deck[i].state = CardState::REVEALED;
                                        } else if (secondCardIndex == -1 && i != firstCardIndex) {
                                            secondCardIndex = i; deck[i].state = CardState::REVEALED; moves++;

                                            if (deck[firstCardIndex].textureId == deck[secondCardIndex].textureId) {
                                                deck[firstCardIndex].state = CardState::MATCHED;
                                                deck[secondCardIndex].state = CardState::MATCHED;
                                                matches++; wrongAttempts = 0; firstCardIndex = -1; secondCardIndex = -1;

                                                if (matches == ui.PAIRS) {
                                                    cumulativeMoves += moves;
                                                    float segmentTime = gameClock.getElapsedTime().asSeconds();
                                                    
                                                    if (currentMode == GameMode::TIME_ATTACK) {
                                                        float rem = levelTimeLimit - segmentTime - levelTimePenalties;
                                                        runningGameTime += (levelTimeLimit - rem);
                                                    } else {
                                                        runningGameTime += segmentTime;
                                                    }

                                                    if (currentLevelIndex < 4) {
                                                        gameState = GameState::LEVEL_CLEAR;
                                                    } else {
                                                        gameState = GameState::WON;
                                                        stateClock.restart();
                                                    }
                                                }
                                            } else {
                                                if (currentMode == GameMode::TIME_ATTACK) {
                                                    wrongAttempts++;
                                                    levelTimePenalties += static_cast<float>((wrongAttempts > 4) ? 4 : wrongAttempts);
                                                }
                                                gameState = GameState::PAUSED_REVEAL;
                                                stateClock.restart();
                                            }
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    } 
                    else if (gameState == GameState::LEVEL_CLEAR) {
                        if (ui.isNextLevelButtonClicked(worldPos)) {
                            currentLevelIndex++;
                            setupActiveLevel(currentLevelIndex);
                            gameState = GameState::PLAYING;
                        }
                    }
                    else if (gameState == GameState::QUIT_CONFIRM) {
                        if (ui.isYesClicked(worldPos)) { 
                            gameState = GameState::START_MENU; 
                        }
                        else if (ui.isRestartConfirmationClicked(worldPos)) { 
                            // Restarts current active level directly via Confirmation Pop-up
                            setupActiveLevel(currentLevelIndex);
                            gameState = GameState::PLAYING; 
                        }
                        else if (ui.isNoClicked(worldPos)) { 
                            gameState = GameState::PLAYING; 
                        }
                    }
                    else if (gameState == GameState::WON || gameState == GameState::GAME_OVER) {
                        if (ui.isRestartButtonClicked(worldPos)) {
                            currentLevelIndex = 0;
                            cumulativeMoves = 0;
                            runningGameTime = 0.0f;
                            setupActiveLevel(currentLevelIndex);
                            gameState = GameState::PLAYING;
                        }
                        else if (ui.isEndMenuButtonClicked(worldPos)) {
                            gameState = GameState::START_MENU;
                        }
                    }
                }
            }
        }

        if (gameState == GameState::SPLASH_SCREEN) {
            float dt = gameClock.restart().asSeconds();
            splashTimer += dt;
            if (skipSplash) { gameState = GameState::START_MENU; }
            else {
                switch (splashStage) {
                    case SplashStage::WAIT_INITIAL: if (splashTimer >= 0.3f) { splashStage = SplashStage::LOGO_FADE_IN; splashTimer = 0.0f; } break;
                    case SplashStage::LOGO_FADE_IN:
                        splashAlpha = std::min(splashTimer / 1.0f, 1.0f) * 255.f;
                        if (splashTimer >= 1.0f) { splashStage = SplashStage::LOGO_PAUSE; splashTimer = 0.0f; } break;
                    case SplashStage::LOGO_PAUSE: splashAlpha = 255.f; if (splashTimer >= 1.5f) { splashStage = SplashStage::LOGO_FADE_OUT; splashTimer = 0.0f; } break;
                    case SplashStage::LOGO_FADE_OUT:
                        splashAlpha = (1.0f - std::min(splashTimer / 1.0f, 1.0f)) * 255.f;
                        if (splashTimer >= 1.0f) gameState = GameState::START_MENU; break;
                }
            }
        }

        float displayTime = 0.0f;
        if (gameState == GameState::SPLASH_SCREEN) { gameClock.restart(); }
        else if (gameState == GameState::WON || gameState == GameState::LEVEL_CLEAR) { displayTime = runningGameTime; }
        else if (currentMode == GameMode::TIME_ATTACK) {
            float elapsed = gameClock.getElapsedTime().asSeconds();
            if (gameState == GameState::PLAYING || gameState == GameState::PAUSED_REVEAL) {
                displayTime = levelTimeLimit - elapsed - levelTimePenalties;
                if (displayTime <= 0.0f) { displayTime = 0.0f; gameState = GameState::GAME_OVER; stateClock.restart(); }
            }
        } else {
            displayTime = gameClock.getElapsedTime().asSeconds();
        }

        if (gameState == GameState::PAUSED_REVEAL && stateClock.getElapsedTime().asMilliseconds() >= 250) {
            deck[firstCardIndex].state = CardState::HIDDEN; deck[secondCardIndex].state = CardState::HIDDEN;
            firstCardIndex = -1; secondCardIndex = -1; gameState = GameState::PLAYING;
        }

        window.clear();
        ui.drawBackground(window);

        if (gameState == GameState::SPLASH_SCREEN) ui.drawSplashScreen(window, splashAlpha);
        else if (gameState == GameState::START_MENU) ui.drawMainMenu(window);
        else if (gameState == GameState::PLAYING || gameState == GameState::PAUSED_REVEAL) {
            ui.drawGrid(window, deck, gameState);
            ui.drawHUD(window, gameState, moves, matches, displayTime, currentLevelIndex + 1);
        }
        else if (gameState == GameState::LEVEL_CLEAR) {
            ui.drawLevelClearScreen(window, currentLevelIndex + 1, moves, displayTime);
        }
        else if (gameState == GameState::QUIT_CONFIRM) ui.drawQuitConfirmation(window);
        else if (gameState == GameState::WON || gameState == GameState::GAME_OVER) {
            ui.drawEndGameScreen(window, gameState, cumulativeMoves, displayTime);
        }

        window.display();
    }
    return 0;
}