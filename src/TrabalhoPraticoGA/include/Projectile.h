#pragma once

#include "Sprite.h"

class Projectile {
public:
    Projectile(GLuint texID, const glm::vec2& startPos, float speed = 750.0f);
    ~Projectile();

    void update(float deltaTime);
    void draw(GLuint shaderProgram);

    bool isOffScreen() const { return sprite.getPosition().x > 1300.0f; }
    bool isActive() const { return active; }
    void deactivate() { active = false; }
    Sprite& getSprite() { return sprite; }

private:
    Sprite sprite;
    float speed;
    bool active;
};
