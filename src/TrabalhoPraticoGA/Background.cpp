#include "Background.h"

Background::Background() {
}

Background::~Background() {
}

void Background::addLayer(GLuint texID, const glm::vec2& pos, const glm::vec2& size, float speedMult) {
    ParallaxLayer layer;
    layer.speedMultiplier = speedMult;
    layer.width = size.x;

    layer.spriteA.setTexture(texID);
    layer.spriteA.setPosition(pos);
    layer.spriteA.setSize(size);

    layer.spriteB.setTexture(texID);
    layer.spriteB.setPosition(glm::vec2(pos.x + size.x, pos.y));
    layer.spriteB.setSize(size);

    layers.push_back(layer);
}

void Background::update(float baseSpeed, float deltaTime) {
    for (auto& layer : layers) {
        float moveAmount = baseSpeed * layer.speedMultiplier * deltaTime;

        glm::vec2 posA = layer.spriteA.getPosition();
        glm::vec2 posB = layer.spriteB.getPosition();

        posA.x -= moveAmount;
        posB.x -= moveAmount;

        // Se o sprite A saiu da tela pela esquerda, reposiciona após o sprite B
        if (posA.x + layer.width * 0.5f <= 0.0f) {
            posA.x = posB.x + layer.width;
        }

        // Se o sprite B saiu da tela pela esquerda, reposiciona após o sprite A
        if (posB.x + layer.width * 0.5f <= 0.0f) {
            posB.x = posA.x + layer.width;
        }

        layer.spriteA.setPosition(posA);
        layer.spriteB.setPosition(posB);
    }
}

void Background::draw(GLuint shaderProgram) {
    for (auto& layer : layers) {
        layer.spriteA.draw(shaderProgram);
        layer.spriteB.draw(shaderProgram);
    }
}
