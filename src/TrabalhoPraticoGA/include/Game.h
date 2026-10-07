#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>

#include "Sprite.h"
#include "Player.h"
#include "Background.h"
#include "Obstacle.h"
#include "Projectile.h"

enum GameState {
    STATE_START = 0,
    STATE_PLAYING = 1,
    STATE_GAMEOVER = 2
};

class Game {
public:
    Game();
    ~Game();

    bool init(int width = 1200, int height = 600);
    void run();
    void reset();

    // Callbacks da GLFW
    void handleKey(int key, int action);

private:
    void initShaders();
    void loadAssets();
    void update(float deltaTime);
    void render();
    void spawnObstacle();
    void shootArrow();
    void drawNumber(int number, float startX, float y, float digitWidth = 26.0f, float digitHeight = 38.0f);
    void drawHUD();

    GLFWwindow* window;
    int screenWidth;
    int screenHeight;

    GLuint shaderProgram;
    glm::mat4 projection;

    GameState state;

    // Entidades do jogo
    Player player;
    Background background;
    std::vector<Obstacle> obstacles;
    std::vector<Projectile> projectiles;

    // Texturas
    GLuint texKatnissRun;
    GLuint texBgDistant;
    GLuint texBgTrees;
    GLuint texBgGround;
    GLuint texStump;
    GLuint texWasp;
    GLuint texPin;
    GLuint texArrow;

    // Texturas de UI e HUD
    GLuint texDigits;
    GLuint texUIGameOver;
    GLuint texUIStart;
    GLuint texUIPinIcon;

    Sprite spriteDigit;
    Sprite spriteBanner;
    Sprite spriteUIPin;

    // Parâmetros de gameplay
    float baseGameSpeed;
    float spawnTimer;
    float spawnInterval;
    float score;
    int pinsCollected;
    int arrowsFired;

    // Controle de tiro
    float shootCooldown;
};
