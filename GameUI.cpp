#include "GameUI.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <vector>

GameUI::GameUI() : headerText(font), hudText(font) {
    hasFont = false;
    animationTime = 0.0f;
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    if (font.openFromFile("assets/PressStart2P.ttf")) {
        hasFont = true;
    } else if (font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
        hasFont = true;
    }

    // Load up to 25 image textures dynamically from your folder
    for (int i = 0; i < 25; ++i) {
        std::string path = "assets/img_" + std::to_string(i) + ".png";
        if (!cardTextures[i].loadFromFile(path)) {
            std::cerr << "[WARNING] Texture asset failed to load: " << path << std::endl;
        }
    }

    // Colors sampled from splash.png rearranged for Dark Mode
    colorBackground   = sf::Color(18, 16, 24);                   // Deep Midnight Slate
    colorHiddenCard   = sf::Color(36, 32, 48);                   // Dark Purple-Grey blocks
    colorHiddenBorder = sf::Color(85, 98, 142);                  // Controller Lavender Purple
    colorHUDPanel     = sf::Color(28, 25, 36, 245);              // Dark Charcoal panel background

    // Load splash screen texture only
    if (!splashTexture.loadFromFile("assets/splash.png")) {
        std::cerr << "[WARNING] Splash texture failed to load: assets/splash.png" << std::endl;
    }

    // --- Setup Splash Art Matrix Grid Struct Elements ---
    const int gridRows = 4;
    const int gridCols = 5;
    const float baseGridX = 365.0f; // Positioned safely to the right of your left logo frame area
    const float baseGridY = 90.0f;
    const float spacingX  = 82.0f;
    const float spacingY  = 102.0f;

    splashCards.clear(); 
    for (int r = 0; r < gridRows; ++r) {
        for (int c = 0; c < gridCols; ++c) {
            SplashCard card;
            card.gridX = baseGridX + (c * spacingX);
            card.gridY = baseGridY + (r * spacingY);
            card.flipAngle = static_cast<float>(std::rand() % 360);
            card.flipSpeed = 2.0f + static_cast<float>(std::rand() % 100) / 40.0f;
            card.assetTextureId = std::rand() % 25;
            card.displayingBack = true;
            splashCards.push_back(card);
        }
    }
}

// Dynamic Scaler Framework Engine
void GameUI::loadLevelSettings(const LevelConfig& config) {
    ROWS = config.rows;
    COLS = config.cols;
    PAIRS = (ROWS * COLS) / 2;
    SPACING = config.spacing;

    float availableWidth  = 800.f - 60.f;
    float availableHeight = 600.f - 240.f;

    float computedWidth  = (availableWidth  - (SPACING * (COLS - 1))) / COLS;
    float computedHeight = (availableHeight - (SPACING * (ROWS - 1))) / ROWS;

    float finalSize = std::min({computedWidth, computedHeight, config.maxCardWidth, config.maxCardHeight});

    CARD_WIDTH  = finalSize;
    CARD_HEIGHT = finalSize * 1.25f;

    float totalGridWidth  = (COLS * CARD_WIDTH)  + ((COLS - 1) * SPACING);
    float totalGridHeight = (ROWS * CARD_HEIGHT) + ((ROWS - 1) * SPACING);

    startX = (800.f - totalGridWidth) / 2.f;
    startY = 105.f;
}

void GameUI::updateViewportView(sf::RenderWindow& window) {
    sf::Vector2u windowSize = window.getSize();
    float targetRatio = 800.f / 600.f;
    float windowRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
    sf::View view(sf::FloatRect(sf::Vector2f(0.f, 0.f), sf::Vector2f(800.f, 600.f)));
    sf::FloatRect viewport(sf::Vector2f(0.f, 0.f), sf::Vector2f(1.f, 1.f));

    if (windowRatio > targetRatio) {
        float viewportWidth = targetRatio / windowRatio;
        viewport.position.x = (1.f - viewportWidth) / 2.f;
        viewport.size.x = viewportWidth;
    } else {
        float viewportHeight = windowRatio / targetRatio;
        viewport.position.y = (1.f - viewportHeight) / 2.f;
        viewport.size.y = viewportHeight;
    }
    view.setViewport(viewport);
    window.setView(view);
}

void GameUI::drawBackground(sf::RenderWindow& window) {
    updateViewportView(window);
    window.clear(colorBackground);
    animationTime += 0.01f;
    
    float screenW = 800.f;
    float screenH = 600.f;

    // 1. Dark Cobblestone Ground Platform at the bottom
    float groundHeight = 65.f;
    sf::RectangleShape groundBase(sf::Vector2f(screenW, groundHeight));
    groundBase.setPosition(sf::Vector2f(0.f, screenH - groundHeight));
    groundBase.setFillColor(sf::Color(32, 28, 44)); // Darker platform foundation
    window.draw(groundBase);

    // Top border line of the cobblestone structure
    sf::RectangleShape groundTrim(sf::Vector2f(screenW, 6.f));
    groundTrim.setPosition(sf::Vector2f(0.f, screenH - groundHeight));
    groundTrim.setFillColor(sf::Color(10, 8, 14)); // Hard black brick outline
    window.draw(groundTrim);

    // Procedural pseudo-pixel brick outlines
    for (int bx = 0; bx < screenW; bx += 40) {
        sf::RectangleShape brickCrack(sf::Vector2f(2.f, groundHeight - 6.f));
        brickCrack.setPosition(sf::Vector2f(static_cast<float>(bx + (bx % 3 == 0 ? 10 : 0)), screenH - groundHeight + 6.f));
        brickCrack.setFillColor(sf::Color(52, 46, 68, 150));
        window.draw(brickCrack);
    }

    // 2. Floating Orange/Yellow Game Pixels (Pop out brightly in dark mode)
    sf::Color arcadeOrange(255, 145, 0);
    sf::Color arcadeYellow(255, 200, 0);
   
    float orangePositionsX[] = { 120.f, 680.f, 240.f, 720.f, 90.f };
    float orangeBaseY[]      = { 150.f, 80.f,  420.f, 230.f, 310.f };
    float boxSizes[]         = { 24.f,  32.f,  16.f,  20.f,  14.f };

    for (int i = 0; i < 5; ++i) {
        float driftY = std::sin(animationTime * 1.5f + i) * 15.f;
        sf::RectangleShape pixelBlock(sf::Vector2f(boxSizes[i], boxSizes[i]));
        pixelBlock.setPosition(sf::Vector2f(orangePositionsX[i], orangeBaseY[i] + driftY));
        pixelBlock.setFillColor(i % 2 == 0 ? arcadeOrange : arcadeYellow);
        window.draw(pixelBlock);
    }
}

void GameUI::drawPersistentLogo(sf::RenderWindow& window) {
    // 1. Define badge dimensions and placement
    float radius = 22.f; 
    sf::CircleShape topLogoBadge(radius);
    topLogoBadge.setPosition(sf::Vector2f(20.f, 20.f));

    // 2. Safely verify and bind the splash screen texture
    if (splashTexture.getSize().x > 0 && splashTexture.getSize().y > 0) {
        topLogoBadge.setTexture(&splashTexture);

        // 3. SECURE RECTANGLE CROP: Map a perfect center-square to avoid texture stretching
        sf::Vector2u imgSize = splashTexture.getSize();
        unsigned int squareDim = std::min(imgSize.x, imgSize.y);
        unsigned int leftOffset = (imgSize.x - squareDim) / 2;
        unsigned int topOffset = (imgSize.y - squareDim) / 2;
        
        // Explicitly cast to sf::IntRect to prevent pipeline scaling artifacts
        topLogoBadge.setTextureRect(sf::IntRect(
            sf::Vector2i(static_cast<int>(leftOffset), static_cast<int>(topOffset)), 
            sf::Vector2i(static_cast<int>(squareDim), static_cast<int>(squareDim))
        ));
    } else {
        // Fallback color if the splash asset fails to bind
        topLogoBadge.setFillColor(sf::Color(255, 145, 0));
    }

    // 4. Set aesthetic borders
    topLogoBadge.setFillColor(sf::Color(255, 255, 255, 255));
    topLogoBadge.setOutlineThickness(2.f);
    topLogoBadge.setOutlineColor(sf::Color(255, 145, 0)); 

    // 5. Render to screen
    window.draw(topLogoBadge);
}

void GameUI::drawSplashScreen(sf::RenderWindow& window, float currentAlpha, float tProgress) {
    // Premium Cyber Void Background
    sf::RectangleShape bgTint(sf::Vector2f(800.f, 600.f));
    bgTint.setFillColor(sf::Color(11, 9, 15, 255));
    window.draw(bgTint);

    // Keep the engine timeline clock running continuous loops
    animationTime += 0.016f; 

    sf::Vector2f viewSize(800.f, 600.f);
    sf::Vector2f screenCenter(viewSize.x / 2.f, viewSize.y / 2.f);
    
    // Main Menu Layout coordinates
    float menuBtnW = 450.f;
    float menuBtnH = 50.f;
    float menuStartX = (viewSize.x - menuBtnW) / 2.f;
    float menuStartY = 180.f; 
    float menuSpacing = 65.f;

    // --- SWARMING ORBIT AND FUSION ENGINE ---
    for (size_t i = 0; i < splashCards.size(); ++i) {
        auto& card = splashCards[i];

        // Group cards explicitly into 4 menu rows (0 to 3)
        int targetRow = i % 4; 
        bool isRowLeader = (i / 4 == 0); // Only one card per group draws the menu text block

        // 1. CALCULATE SWARM ORBIT DATA (When sitting on Splash screen)
        // Give every single card a completely unique radius and rotational speed offset
        float uniqueAngle = (i * 18.0f) + (animationTime * 45.0f * (1.0f + (i % 3) * 0.2f));
        float rad = uniqueAngle * 0.0174533f;
        
        // Define orbital paths around the screen canvas area
        float orbitRadiusX = 180.f + (i % 4) * 35.f;
        float orbitRadiusY = 120.f + (i % 3) * 25.f;

        float swarmX = screenCenter.x + std::cos(rad) * orbitRadiusX;
        float swarmY = screenCenter.y + std::sin(rad) * orbitRadiusY + std::sin(animationTime + i) * 15.f;

        // 2. CALCULATE FINAL MAIN MENU TARGET DATA
        float destX = menuStartX + (menuBtnW / 2.f);
        float destY = menuStartY + (targetRow * menuSpacing) + (menuBtnH / 2.f);

        // 3. INTERPOLATE BETWEEN THE SWARM PATH AND MENU POSITION
        float currentX = swarmX;
        float currentY = swarmY;
        
        float cardW = 54.0f;
        float cardH = 70.0f;
        
        // Continuous organic rotation on the splash screen, snapping smoothly into flat cards on transition
        float targetFlipAngle = uniqueAngle; 

        if (tProgress > 0.0f) {
            // Smooth acceleration curve out of orbit into menu lines
            float blend = tProgress * tProgress * (3.0f - 2.0f * tProgress);
            
            currentX = swarmX + (destX - swarmX) * blend;
            currentY = swarmY + (destY - swarmY) * blend;
            
            cardW = 54.0f + (menuBtnW - 54.0f) * blend;
            cardH = 70.0f + (menuBtnH - 70.0f) * blend;
            
            // Turn all cards completely face up ($180^\circ$ flip transition)
            targetFlipAngle = uniqueAngle * (1.0f - blend) + (blend * 180.0f);
        }

        // Compute 3D width deformation math matrices
        float cosAngle = std::cos(targetFlipAngle * 0.0174533f);
        float visibleWidthScale = std::abs(cosAngle);
        bool displayingMenuText = (tProgress > 0.5f) || (std::abs(static_cast<int>(targetFlipAngle) % 360) > 90 && std::abs(static_cast<int>(targetFlipAngle) % 360) < 270);

        if (visibleWidthScale < 0.02f) continue; 
        float renderW = cardW * visibleWidthScale;

        // Construct primary rendering card component node shape
        sf::RectangleShape cardNode(sf::Vector2f(renderW, cardH));
        cardNode.setOrigin(sf::Vector2f(renderW / 2.0f, cardH / 2.0f));
        cardNode.setPosition(sf::Vector2f(currentX, currentY));
        cardNode.setOutlineThickness(1.5f);

        // 4. DRAW STATE DETERMINATION INTERFACE
        if (tProgress <= 0.5f) {
            // --- THE ACTIVE SWARM SPLASH VISUALS ---
            bool showFrontFace = (i % 2 == 0);
            
            if (!showFrontFace) {
                // Card Back Outer Frame Design Block
                cardNode.setFillColor(sf::Color(34, 30, 46, static_cast<uint8_t>(currentAlpha)));
                cardNode.setOutlineColor(sf::Color(80, 90, 135, static_cast<uint8_t>(currentAlpha)));
                window.draw(cardNode);

                // Tech Grid Line detailing
                float crossLen = 10.f * visibleWidthScale;
                sf::RectangleShape crossH(sf::Vector2f(crossLen, 2.f));
                crossH.setOrigin(sf::Vector2f(crossLen / 2.f, 1.f));
                crossH.setPosition(sf::Vector2f(currentX, currentY));
                crossH.setFillColor(sf::Color(0, 240, 255, static_cast<uint8_t>(currentAlpha))); // Glowing Cyan centers
                window.draw(crossH);
            } else {
                // Card Front Face Asset Render
                cardNode.setFillColor(sf::Color(242, 240, 235, static_cast<uint8_t>(currentAlpha)));
                cardNode.setOutlineColor(sf::Color(16, 12, 22, static_cast<uint8_t>(currentAlpha)));
                window.draw(cardNode);

                int id = card.assetTextureId;
                if (id >= 0 && id < 25 && cardTextures[id].getSize().x > 0) {
                    float iconSize = std::min(cardW, cardH) * 0.70f * visibleWidthScale;
                    sf::RectangleShape iconNode(sf::Vector2f(iconSize, iconSize));
                    iconNode.setOrigin(sf::Vector2f(iconSize / 2.f, iconSize / 2.f));
                    iconNode.setPosition(sf::Vector2f(currentX, currentY));
                    iconNode.setTexture(&cardTextures[id]);
                    iconNode.setFillColor(sf::Color(255, 255, 255, static_cast<uint8_t>(currentAlpha)));
                    window.draw(iconNode);
                }
            }
        } else {
            // --- MENU BUTTON COHESIVE FUSION RENDERING ---
            if (isRowLeader) {
                std::string menuOptions[] = { "1. CLASSIC MODE", "2. TIME ATTACK", "3. GOD MODE CHAOS", "4. EXIT GAME" };
                sf::Color menuThemes[] = { sf::Color(0, 240, 255), sf::Color(145, 90, 255), sf::Color(255, 110, 0), sf::Color(240, 50, 80) };

                // Snap bounding boxes flat into standard layout bars seamlessly
                cardNode.setSize(sf::Vector2f(menuBtnW * visibleWidthScale, menuBtnH));
                cardNode.setOrigin(sf::Vector2f((menuBtnW * visibleWidthScale) / 2.f, menuBtnH / 2.f));
                cardNode.setPosition(sf::Vector2f(currentX, currentY));
                
                cardNode.setFillColor(sf::Color(26, 22, 36, static_cast<uint8_t>(currentAlpha)));
                cardNode.setOutlineColor(menuThemes[targetRow]);
                window.draw(cardNode);

                if (hasFont && visibleWidthScale > 0.4f) {
                    sf::Text label(font, menuOptions[targetRow], 14);
                    label.setFillColor(menuThemes[targetRow]);
                    
                    sf::FloatRect textBounds = label.getLocalBounds();
                    label.setOrigin(sf::Vector2f(textBounds.size.x / 2.f, textBounds.size.y / 2.f));
                    label.setPosition(sf::Vector2f(currentX, currentY));
                    window.draw(label);
                }
            }
        }
    }

    // --- LOGO PANEL ---
    sf::Sprite splashSprite(splashTexture);
    sf::Vector2u texSize = splashTexture.getSize();
    float scaleX = 280.f / static_cast<float>(texSize.x);
    float scaleY = 130.f / static_cast<float>(texSize.y);
    float logoScale = std::min(scaleX, scaleY);
    splashSprite.setScale(sf::Vector2f(logoScale, logoScale));
    
    // Slid off-canvas dynamically as transition value ticks up
    float logoX = 25.f - (250.f * tProgress); 
    splashSprite.setPosition(sf::Vector2f(logoX, 25.f));
    splashSprite.setColor(sf::Color(255, 255, 255, static_cast<uint8_t>(currentAlpha * (1.0f - tProgress))));
    window.draw(splashSprite);
}

void GameUI::drawMainMenu(sf::RenderWindow& window) {
    activeMenuButtons.clear();
    sf::Vector2f viewSize(800.f, 600.f);

    float consoleW = 550.f, consoleH = 340.f;
    float consoleX = (viewSize.x - consoleW) / 2.f;
    float consoleY = 195.f;

    sf::RectangleShape glassPane(sf::Vector2f(consoleW, consoleH));
    glassPane.setPosition(sf::Vector2f(consoleX, consoleY));
    glassPane.setFillColor(colorHUDPanel);
    glassPane.setOutlineThickness(3.f);
    glassPane.setOutlineColor(sf::Color(10, 8, 14)); // Sharp retro black border
    window.draw(glassPane);

    if (hasFont) {
        headerText.setString("KIOKU");
        headerText.setCharacterSize(58);
        sf::FloatRect textBounds = headerText.getLocalBounds();
        float textX = (viewSize.x - textBounds.size.x) / 2.f;
        float textY = 60.f;

        // Pixel drop shadow offset
        headerText.setOutlineThickness(0.f);
        headerText.setFillColor(sf::Color(10, 8, 14));
        headerText.setPosition(sf::Vector2f(textX + 4.f, textY + 4.f));
        window.draw(headerText);

        headerText.setFillColor(sf::Color(255, 145, 0)); // High-visibility Retro Orange
        headerText.setPosition(sf::Vector2f(textX, textY));
        window.draw(headerText);

        sf::Text subtitleText(font, "GROUP 9", 9);
        subtitleText.setFillColor(sf::Color(140, 135, 180)); // Soft Lavender text accent
        subtitleText.setPosition(sf::Vector2f((viewSize.x - subtitleText.getLocalBounds().size.x) / 2.f, textY + 86.f));
        window.draw(subtitleText);
    }

    std::string options[] = { "1. CLASSIC MODE", "2. TIME ATTACK", "3. GOD MODE CHAOS", "4. EXIT GAME" };
   
    sf::Color retroThemes[] = {
        sf::Color(230, 225, 245),  // Soft Off-White text
        sf::Color(140, 135, 180),  // Lavender Purple
        sf::Color(255, 145, 0),    // Vibrant Arcade Orange
        sf::Color(220, 70, 70)     // Dark Mode Warning Red
    };
   
    float btnWidth = 500.f, btnHeight = 44.f, btnX = (viewSize.x - btnWidth) / 2.f;

    for (int i = 0; i < 4; ++i) {
        float btnY = 210.f + (i * 56.f);

        sf::RectangleShape btnBox(sf::Vector2f(btnWidth, btnHeight));
        btnBox.setPosition(sf::Vector2f(btnX, btnY));
        btnBox.setFillColor(sf::Color(44, 40, 56)); // Dark internal button face
        btnBox.setOutlineThickness(2.f);
        btnBox.setOutlineColor(retroThemes[i]);
        window.draw(btnBox);

        sf::RectangleShape indicator(sf::Vector2f(14.f, btnHeight));
        indicator.setPosition(sf::Vector2f(btnX, btnY));
        indicator.setFillColor(retroThemes[i]);
        window.draw(indicator);

        if (hasFont) {
            sf::Text label(font, options[i], 11);
            label.setFillColor(retroThemes[i]);
            label.setPosition(sf::Vector2f(btnX + 30.f, btnY + (btnHeight - label.getLocalBounds().size.y) / 2.5f));
            window.draw(label);
        }

        activeMenuButtons.push_back({sf::FloatRect(sf::Vector2f(btnX, btnY), sf::Vector2f(btnWidth, btnHeight)), i + 1});
    }
    drawPersistentLogo(window);
}

void GameUI::drawGrid(sf::RenderWindow& window, const std::vector<Card>& deck, GameState state) {
    sf::Color shadowColor(10, 8, 14, 160); // Flat black shadow blocks
    sf::Color solidOutlineColor(10, 8, 14);

    for (size_t i = 0; i < deck.size(); ++i) {
        int r = i / COLS;
        int c = i % COLS;
        float x = startX + (c * (CARD_WIDTH + SPACING));
        float y = startY + (r * (CARD_HEIGHT + SPACING));

        if (deck[i].state == CardState::MATCHED) continue;

        // Hard pixel shadow depth block
        sf::RectangleShape cardShadow(sf::Vector2f(CARD_WIDTH, CARD_HEIGHT));
        cardShadow.setPosition(sf::Vector2f(x + 5.f, y + 5.f));
        cardShadow.setFillColor(shadowColor);
        window.draw(cardShadow);

        sf::RectangleShape cardShape(sf::Vector2f(CARD_WIDTH, CARD_HEIGHT));
        cardShape.setPosition(sf::Vector2f(x, y));

        if (deck[i].state == CardState::REVEALED) {
            cardShape.setFillColor(sf::Color(240, 238, 233)); // Off-white/cream card base
            cardShape.setOutlineThickness(2.f);
            cardShape.setOutlineColor(solidOutlineColor);
            window.draw(cardShape);

            int id = deck[i].textureId;
            if (id >= 0 && id < 25) {
                float padding = CARD_WIDTH * 0.1f;
                sf::RectangleShape iconShape(sf::Vector2f(CARD_WIDTH - (padding * 2), CARD_HEIGHT - (padding * 2)));
                iconShape.setPosition(sf::Vector2f(x + padding, y + padding));
                iconShape.setTexture(&cardTextures[id]);
                sf::Vector2u texSize = cardTextures[id].getSize();
                iconShape.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(texSize)));
                window.draw(iconShape);
            }
        } else {
            cardShape.setFillColor(colorHiddenCard);
            cardShape.setOutlineThickness(2.f);
            cardShape.setOutlineColor(solidOutlineColor);
            window.draw(cardShape);

            // Miniature decorative center pixel layout block
            sf::RectangleShape coreDot(sf::Vector2f(8.f, 8.f));
            coreDot.setPosition(sf::Vector2f(x + (CARD_WIDTH - 8.f) / 2.f, y + (CARD_HEIGHT - 8.f) / 2.f));
            coreDot.setFillColor(colorHiddenBorder);
            window.draw(coreDot);
        }
    }
}

void GameUI::drawHUD(sf::RenderWindow& window, GameState gameState, int moves, int matches, float timeSeconds, int currentLevel) {
    if (!hasFont) return;

    float hudWidth = 430.f;
    float hudHeight = 45.f;
    float hudX = (800.f - hudWidth) / 2.f;
    float hudY = 20.f;

    sf::RectangleShape hudPanel(sf::Vector2f(hudWidth, hudHeight));
    hudPanel.setFillColor(colorHUDPanel);
    hudPanel.setOutlineThickness(2.f);
    hudPanel.setOutlineColor(sf::Color(10, 8, 14));
    hudPanel.setPosition(sf::Vector2f(hudX, hudY));
    window.draw(hudPanel);

    std::string hudString = "LVL: " + std::to_string(currentLevel) +
                            "  MOVES: " + std::to_string(moves) +
                            "  TIME: " + std::to_string((int)timeSeconds) + "S";
    hudText.setString(hudString);
    hudText.setCharacterSize(10);
    hudText.setFillColor(sf::Color(230, 225, 245));
    hudText.setPosition(sf::Vector2f(hudX + (hudWidth - hudText.getLocalBounds().size.x) / 2.f, hudY + 16.f));
    window.draw(hudText);

    float btnWidth = 120.f;
    float btnHeight = 45.f;
    float quitX = 640.f;
    float btnY = 20.f;

    quitButtonBounds = sf::FloatRect(sf::Vector2f(quitX, btnY), sf::Vector2f(btnWidth, btnHeight));

    sf::RectangleShape quitBtn(sf::Vector2f(btnWidth, btnHeight));
    quitBtn.setPosition(sf::Vector2f(quitX, btnY));
    quitBtn.setFillColor(sf::Color(180, 50, 50));
    quitBtn.setOutlineThickness(2.f);
    quitBtn.setOutlineColor(sf::Color(10, 8, 14));
    window.draw(quitBtn);

    sf::Text quitLabel(font, "QUIT", 10);
    quitLabel.setFillColor(sf::Color::White);
    quitLabel.setPosition(sf::Vector2f(quitX + (btnWidth - quitLabel.getLocalBounds().size.x) / 2.f, btnY + 16.f));
    window.draw(quitLabel);
}

void GameUI::drawLevelClearScreen(sf::RenderWindow& window, int clearedLevel, int moves, float timeRemaining) {
    sf::Vector2f viewSize(800.f, 600.f);
    sf::RectangleShape overlay(viewSize);
    overlay.setFillColor(sf::Color(10, 8, 14, 200));
    window.draw(overlay);

    float boxW = 500.f, boxH = 260.f;
    float boxX = (viewSize.x - boxW) / 2.f;
    float boxY = (viewSize.y - boxH) / 2.f;

    sf::RectangleShape msgBox(sf::Vector2f(boxW, boxH));
    msgBox.setPosition(sf::Vector2f(boxX, boxY));
    msgBox.setFillColor(colorHUDPanel);
    msgBox.setOutlineThickness(3.f);
    msgBox.setOutlineColor(sf::Color(255, 145, 0));
    window.draw(msgBox);

    if (hasFont) {
        sf::Text txt(font, "LEVEL " + std::to_string(clearedLevel) + " CLEAR", 16);
        txt.setFillColor(sf::Color(255, 145, 0));
        txt.setPosition(sf::Vector2f(boxX + (boxW - txt.getLocalBounds().size.x) / 2.f, boxY + 35.f));
        window.draw(txt);

        sf::Text stats(font, "MOVES CONCLUDED: " + std::to_string(moves), 11);
        stats.setFillColor(sf::Color(230, 225, 245));
        stats.setPosition(sf::Vector2f(boxX + 40.f, boxY + 100.f));
        window.draw(stats);

        sf::Text timeTxt(font, "TIME REMAINING: " + std::to_string((int)timeRemaining) + "S", 11);
        timeTxt.setFillColor(sf::Color(230, 225, 245));
        timeTxt.setPosition(sf::Vector2f(boxX + 40.f, boxY + 130.f));
        window.draw(timeTxt);
    }

    nextLevelButtonBounds = sf::FloatRect(sf::Vector2f(boxX + 100.f, boxY + 185.f), sf::Vector2f(300.f, 42.f));
    sf::RectangleShape nextBtn(sf::Vector2f(300.f, 42.f));
    nextBtn.setPosition(sf::Vector2f(boxX + 100.f, boxY + 185.f));
    nextBtn.setFillColor(sf::Color(85, 98, 142));
    nextBtn.setOutlineThickness(2.f);
    nextBtn.setOutlineColor(sf::Color(10, 8, 14));
    window.draw(nextBtn);

    if (hasFont) {
        sf::Text btnLbl(font, "Next Level na to lods", 10);
        btnLbl.setFillColor(sf::Color::White);
        btnLbl.setPosition(sf::Vector2f(boxX + 100.f + (300.f - btnLbl.getLocalBounds().size.x) / 2.f, boxY + 200.f));
        window.draw(btnLbl);
    }
}

void GameUI::drawQuitConfirmation(sf::RenderWindow& window) {
    if (!hasFont) return;

    sf::Vector2f viewSize(800.f, 600.f);

    sf::RectangleShape dimLayer(viewSize);
    dimLayer.setFillColor(sf::Color(10, 8, 14, 200));
    window.draw(dimLayer);

    float boxWidth = 520.f;
    float boxHeight = 220.f;
    float boxX = (viewSize.x - boxWidth) / 2.f;
    float boxY = (viewSize.y - boxHeight) / 2.f;

    sf::RectangleShape dialogBox(sf::Vector2f(boxWidth, boxHeight));
    dialogBox.setPosition(sf::Vector2f(boxX, boxY));
    dialogBox.setFillColor(colorHUDPanel);
    dialogBox.setOutlineThickness(3.f);
    dialogBox.setOutlineColor(sf::Color(10, 8, 14));
    window.draw(dialogBox);

    sf::Text promptText(font, "ABORT CURRENT MISSION?", 13);
    promptText.setFillColor(sf::Color(230, 225, 245));
    promptText.setPosition(sf::Vector2f(boxX + (boxWidth - promptText.getLocalBounds().size.x) / 2.f, boxY + 45.f));
    window.draw(promptText);

    float btnWidth = 120.f;
    float btnHeight = 40.f;
    float totalGaps = boxWidth - (btnWidth * 3.f);
    float gap = totalGaps / 4.f;

    float yesX     = boxX + gap;
    float restartX = yesX + btnWidth + gap;
    float noX      = restartX + btnWidth + gap;
    float btnY     = boxY + 120.f;

    // YES BUTTON
    yesButtonBounds = sf::FloatRect(sf::Vector2f(yesX, btnY), sf::Vector2f(btnWidth, btnHeight));
    sf::RectangleShape yesBtn(sf::Vector2f(btnWidth, btnHeight));
    yesBtn.setPosition(sf::Vector2f(yesX, btnY));
    yesBtn.setFillColor(sf::Color(85, 98, 142));
    yesBtn.setOutlineThickness(2.f);
    yesBtn.setOutlineColor(sf::Color(10, 8, 14));
    window.draw(yesBtn);

    sf::Text yesText(font, "YES", 11);
    yesText.setFillColor(sf::Color::White);
    yesText.setPosition(sf::Vector2f(yesX + (btnWidth - yesText.getLocalBounds().size.x) / 2.f, btnY + 14.f));
    window.draw(yesText);

    // RESTART BUTTON
    restartConfirmationBounds = sf::FloatRect(sf::Vector2f(restartX, btnY), sf::Vector2f(btnWidth, btnHeight));
    sf::RectangleShape restartBtn(sf::Vector2f(btnWidth, btnHeight));
    restartBtn.setPosition(sf::Vector2f(restartX, btnY));
    restartBtn.setFillColor(sf::Color(255, 145, 0));
    restartBtn.setOutlineThickness(2.f);
    restartBtn.setOutlineColor(sf::Color(10, 8, 14));
    window.draw(restartBtn);

    sf::Text restartText(font, "RESTART", 10);
    restartText.setFillColor(sf::Color::White);
    restartText.setPosition(sf::Vector2f(restartX + (btnWidth - restartText.getLocalBounds().size.x) / 2.f, btnY + 14.f));
    window.draw(restartText);

    // NO BUTTON
    noButtonBounds = sf::FloatRect(sf::Vector2f(noX, btnY), sf::Vector2f(btnWidth, btnHeight));
    sf::RectangleShape noBtn(sf::Vector2f(btnWidth, btnHeight));
    noBtn.setPosition(sf::Vector2f(noX, btnY));
    noBtn.setFillColor(sf::Color(44, 40, 56));
    noBtn.setOutlineThickness(2.f);
    noBtn.setOutlineColor(sf::Color(10, 8, 14));
    window.draw(noBtn);

    sf::Text noText(font, "NO", 11);
    noText.setFillColor(sf::Color::White);
    noText.setPosition(sf::Vector2f(noX + (btnWidth - noText.getLocalBounds().size.x) / 2.f, btnY + 14.f));
    window.draw(noText);
}

bool GameUI::isRestartConfirmationClicked(sf::Vector2f m) {
    return restartConfirmationBounds.contains(m);
}

void GameUI::drawEndGameScreen(sf::RenderWindow& window, GameState state, int moves, float time) {
    sf::Vector2f viewSize(800.f, 600.f);
    sf::RectangleShape overlay(viewSize);
    overlay.setFillColor(sf::Color(10, 8, 14, 220));
    window.draw(overlay);

    if (hasFont) {
        std::string resStr = (state == GameState::WON) ? "VICTORY ACHIEVED!" : "GAME OVER";
        sf::Text resText(font, resStr, 22);
        resText.setFillColor((state == GameState::WON) ? sf::Color(255, 145, 0) : sf::Color(220, 70, 70));
        resText.setPosition(sf::Vector2f((viewSize.x - resText.getLocalBounds().size.x) / 2.f, 150.f));
        window.draw(resText);

        std::string scoreStr = "MOVES: " + std::to_string(moves) +
                               " | TOTAL TIME: " + std::to_string((int)time) + "S";
        sf::Text scoreText(font, scoreStr, 12);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(sf::Vector2f((viewSize.x - scoreText.getLocalBounds().size.x) / 2.f, 240.f));
        window.draw(scoreText);
    }

    float goBtnWidth  = 180.f;
    float goBtnHeight = 45.f;
    float goLeftX     = 200.f;
    float goRightX    = 420.f;
    float goY         = 340.f;

    restartButtonBounds = sf::FloatRect(sf::Vector2f(goLeftX,  goY), sf::Vector2f(goBtnWidth, goBtnHeight));
    endMenuButtonBounds = sf::FloatRect(sf::Vector2f(goRightX, goY), sf::Vector2f(goBtnWidth, goBtnHeight));

    // PLAY AGAIN BUTTON
    sf::RectangleShape goRestart(sf::Vector2f(goBtnWidth, goBtnHeight));
    goRestart.setPosition(sf::Vector2f(goLeftX, goY));
    goRestart.setFillColor(sf::Color(85, 98, 142));
    goRestart.setOutlineThickness(2.f);
    goRestart.setOutlineColor(sf::Color::White);
    window.draw(goRestart);

    if (hasFont) {
        sf::Text goRestartText(font, "PLAY AGAIN", 11);
        goRestartText.setFillColor(sf::Color::White);
        goRestartText.setPosition(sf::Vector2f(goLeftX + (goBtnWidth - goRestartText.getLocalBounds().size.x) / 2.f, goY + 16.f));
        window.draw(goRestartText);
    }

    // MAIN MENU BUTTON
    sf::RectangleShape goQuit(sf::Vector2f(goBtnWidth, goBtnHeight));
    goQuit.setPosition(sf::Vector2f(goRightX, goY));
    goQuit.setFillColor(sf::Color(44, 40, 56));
    goQuit.setOutlineThickness(2.f);
    goQuit.setOutlineColor(sf::Color::White);
    window.draw(goQuit);

    if (hasFont) {
        sf::Text goQuitText(font, "MAIN MENU", 11);
        goQuitText.setFillColor(sf::Color::White);
        goQuitText.setPosition(sf::Vector2f(goRightX + (goBtnWidth - goQuitText.getLocalBounds().size.x) / 2.f, goY + 16.f));
        window.draw(goQuitText);
    }
}

int GameUI::getClickedMenuOption(sf::Vector2f m) {
    for (size_t i = 0; i < activeMenuButtons.size(); ++i) {
        if (activeMenuButtons[i].bounds.contains(m)) return activeMenuButtons[i].actionId;
    }
    return -1;
}

bool GameUI::isQuitButtonClicked(sf::Vector2f m)      { return quitButtonBounds.contains(m); }
bool GameUI::isRestartButtonClicked(sf::Vector2f m)   { return restartButtonBounds.contains(m); }
bool GameUI::isYesClicked(sf::Vector2f m)             { return yesButtonBounds.contains(m); }
bool GameUI::isNoClicked(sf::Vector2f m)              { return noButtonBounds.contains(m); }
bool GameUI::isEndMenuButtonClicked(sf::Vector2f m)   { return endMenuButtonBounds.contains(m); }
bool GameUI::isNextLevelButtonClicked(sf::Vector2f m) { return nextLevelButtonBounds.contains(m); }