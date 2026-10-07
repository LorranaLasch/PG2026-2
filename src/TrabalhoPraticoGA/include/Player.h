#pragma once

#include "Sprite.h"

class Player {
public:
    Player();
    ~Player();

    void init(GLuint textureID, float groundY);
    void update(float deltaTime);
    void jump();
    void reset();

    Sprite& getSprite() { return sprite; }
    glm::vec2 getPosition() const { return sprite.getPosition(); }
    bool isGrounded() const { return onGround; }

private:
    Sprite sprite;
    float groundLevelY;
    float velocityY;
    float gravity;
    float jumpSpeed;
    bool onGround;

    // Animação de corrida
    float animTimer;
    float frameDuration;
    int currentFrame;
    int totalFrames;
};
