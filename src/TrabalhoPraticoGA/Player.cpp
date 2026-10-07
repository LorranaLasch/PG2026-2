#include "Player.h"

Player::Player()
    : groundLevelY(140.0f),
      velocityY(0.0f),
      gravity(-1400.0f),   // Gravidade natural em pixels/s²
      jumpSpeed(550.0f),   // Impulso inicial do pulo
      onGround(true),
      animTimer(0.0f),
      frameDuration(0.085f), // Ritmo atlético fluido
      currentFrame(0),
      totalFrames(6)
{
}

Player::~Player() {
}

void Player::init(GLuint textureID, float groundY) {
    this->groundLevelY = groundY;
    sprite.setTexture(textureID);
    sprite.setSize(glm::vec2(90.0f, 90.0f));
    sprite.setSpritesheet(totalFrames, 1);
    reset();
}

void Player::reset() {
    sprite.setPosition(glm::vec2(150.0f, groundLevelY));
    velocityY = 0.0f;
    onGround = true;
    currentFrame = 0;
    animTimer = 0.0f;
    sprite.setFrame(0);
}

void Player::jump() {
    if (onGround) {
        velocityY = jumpSpeed;
        onGround = false;
        // Frame 2 (pose estendida no ar)
        sprite.setFrame(2);
    }
}

void Player::update(float deltaTime) {
    // Física do pulo
    if (!onGround) {
        velocityY += gravity * deltaTime;
        glm::vec2 pos = sprite.getPosition();
        pos.y += velocityY * deltaTime;

        if (pos.y <= groundLevelY) {
            pos.y = groundLevelY;
            velocityY = 0.0f;
            onGround = true;
        }

        sprite.setPosition(pos);
    } else {
        // Animação de corrida no chão
        animTimer += deltaTime;
        if (animTimer >= frameDuration) {
            animTimer = 0.0f;
            currentFrame = (currentFrame + 1) % totalFrames;
            sprite.setFrame(currentFrame);
        }
    }
}
