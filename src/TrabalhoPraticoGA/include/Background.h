#pragma once

#include "Sprite.h"
#include <vector>

struct ParallaxLayer {
    Sprite spriteA;
    Sprite spriteB;
    float speedMultiplier;
    float width;
};

class Background {
public:
    Background();
    ~Background();

    void addLayer(GLuint texID, const glm::vec2& pos, const glm::vec2& size, float speedMult);
    void update(float baseSpeed, float deltaTime);
    void draw(GLuint shaderProgram);

private:
    std::vector<ParallaxLayer> layers;
};
