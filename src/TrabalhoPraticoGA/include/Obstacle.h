#pragma once

#include "Sprite.h"

enum ObstacleType {
    OBSTACLE_GROUND_STUMP = 0,
    OBSTACLE_FLYING_WASP = 1,
    ITEM_PIN = 2
};

class Obstacle {
public:
    Obstacle(GLuint texID, ObstacleType type, const glm::vec2& pos, float speed, const glm::vec2& size);
    ~Obstacle();

    void update(float deltaTime);
    void draw(GLuint shaderProgram);

    bool isOffScreen() const { return sprite.getPosition().x < -100.0f; }
    ObstacleType getType() const { return type; }
    Sprite& getSprite() { return sprite; }
    bool isActive() const { return active; }
    void deactivate() { active = false; }

private:
    Sprite sprite;
    ObstacleType type;
    float moveSpeed;
    bool active;
    float floatTimer;
};
