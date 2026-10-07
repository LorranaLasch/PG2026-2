#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <vector>

class Sprite {
public:
    Sprite();
    ~Sprite();

    // Inicialização da malha (Quad com VAO, VBO, EBO)
    static void initQuadGeometry();
    static void cleanupQuadGeometry();

    // Carregamento de textura
    static GLuint loadTexture(const std::string& path, bool repeat = false);

    // Configuração de propriedades do Sprite
    void setTexture(GLuint texID) { this->textureID = texID; }
    void setPosition(const glm::vec2& pos) { this->position = pos; }
    void setSize(const glm::vec2& sz) { this->size = sz; }
    void setRotation(float rot) { this->rotation = rot; }
    void setColor(const glm::vec4& col) { this->color = col; }

    glm::vec2 getPosition() const { return position; }
    glm::vec2 getSize() const { return size; }

    // Suporte a Spritesheet
    void setSpritesheet(int totalFramesX, int totalFramesY = 1);
    void setFrame(int frameIndex);

    // Tiling para texturas repetidas (ex: chão e parallax)
    void setUVScale(const glm::vec2& scale) { this->uvScale = scale; }
    void setUVOffset(const glm::vec2& offset) { this->uvOffset = offset; }

    // Desenho
    void draw(GLuint shaderProgram);

    // Bounding Box (AABB) para detecção de colisão
    bool checkCollision(const Sprite& other, float padding = 0.0f) const;

private:
    GLuint textureID;
    glm::vec2 position;
    glm::vec2 size;
    float rotation;
    glm::vec4 color;

    // UVs para spritesheets
    glm::vec2 uvOffset;
    glm::vec2 uvScale;
    int totalFramesX;
    int totalFramesY;
    int currentFrame;

    // VAO/VBO/EBO compartilhados por todos os sprites para performance máxima
    static GLuint quadVAO;
    static GLuint quadVBO;
    static GLuint quadEBO;
    static bool isGeometryInitialized;
};
