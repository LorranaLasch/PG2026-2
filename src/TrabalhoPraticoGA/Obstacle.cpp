#include "Obstacle.h"
#include <cmath>

Obstacle::Obstacle(GLuint texID, ObstacleType type, const glm::vec2& pos, float speed, const glm::vec2& size)
    : type(type),
      moveSpeed(speed),
      active(true),
      floatTimer(0.0f)
{
    sprite.setTexture(texID);
    sprite.setPosition(pos);
    sprite.setSize(size);
}

Obstacle::~Obstacle() {
}

void Obstacle::update(float deltaTime) {
    if (!active) return;

    glm::vec2 pos = sprite.getPosition();
    pos.x -= moveSpeed * deltaTime;

    // Se for vespa voadora (Tracker Jacker), faz um movimento ondulatório suave
    if (type == OBSTACLE_FLYING_WASP) {
        floatTimer += deltaTime * 5.0f;
        pos.y += std::sin(floatTimer) * 1.2f;
    }

    sprite.setPosition(pos);
}

void Obstacle::draw(GLuint shaderProgram) {
    if (active) {
        sprite.draw(shaderProgram);
    }
}
