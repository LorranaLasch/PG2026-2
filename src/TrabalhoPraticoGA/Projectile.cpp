#include "Projectile.h"

Projectile::Projectile(GLuint texID, const glm::vec2& startPos, float speed)
    : speed(speed),
      active(true)
{
    sprite.setTexture(texID);
    sprite.setPosition(startPos);
    sprite.setSize(glm::vec2(55.0f, 18.0f));
}

Projectile::~Projectile() {
}

void Projectile::update(float deltaTime) {
    if (!active) return;
    glm::vec2 pos = sprite.getPosition();
    pos.x += speed * deltaTime;
    sprite.setPosition(pos);
}

void Projectile::draw(GLuint shaderProgram) {
    if (active) {
        sprite.draw(shaderProgram);
    }
}
