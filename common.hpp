#ifndef COMMON_HPP
#define COMMON_HPP

#include <SFML/Graphics.hpp>
#include <vector>

// Global Layout Dimensions
inline constexpr int WINDOW_WIDTH = 800;
inline constexpr int WINDOW_HEIGHT = 600;

enum class GameState {
    SPLASH_SCREEN,
    TRANSITIONING,
    START_MENU,
    PLAYING,
    PAUSED_REVEAL,
    QUIT_CONFIRM,
    LEVEL_CLEAR,
    WON,
    GAME_OVER
};

enum class SplashStage {
    WAIT_INITIAL,    
    LOGO_FADE_IN,    
    LOGO_PAUSE,      
    LOGO_FADE_OUT    
};

enum class GameMode {
    CLASSIC,
    TIME_ATTACK,
    SHUFFLE_MODE
};

enum class CardState {
    HIDDEN,
    REVEALED,
    MATCHED
};

struct LevelConfig {
    int levelNum;
    int rows;
    int cols;
    float baseTime;
    float maxCardWidth;
    float maxCardHeight;
    float spacing;
};

struct Card {
    int textureId; 
    CardState state;
};

#endif // COMMON_HPP