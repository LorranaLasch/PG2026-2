#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "Sprite.h"
#include <iostream>

GLuint Sprite::quadVAO = 0;
GLuint Sprite::quadVBO = 0;
GLuint Sprite::quadEBO = 0;
bool Sprite::isGeometryInitialized = false;

Sprite::Sprite()
    : textureID(0),
      position(0.0f, 0.0f),
      size(50.0f, 50.0f),
      rotation(0.0f),
      color(1.0f, 1.0f, 1.0f, 1.0f),
      uvOffset(0.0f, 0.0f),
      uvScale(1.0f, 1.0f),
      totalFramesX(1),
      totalFramesY(1),
      currentFrame(0)
{
}

Sprite::~Sprite() {
}

void Sprite::initQuadGeometry() {
    if (isGeometryInitialized) return;

    // Vértices do quad centralizado em (0,0) com tamanho 1x1
    // Layout: Posição (x, y, z) + Coordenada de Textura (u, v)
    float vertices[] = {
        // x       y      z       u     v
        -0.5f, -0.5f, 0.0f,   0.0f, 1.0f, // Inferior esquerdo
         0.5f, -0.5f, 0.0f,   1.0f, 1.0f, // Inferior direito
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, // Superior direito
        -0.5f,  0.5f, 0.0f,   0.0f, 0.0f  // Superior esquerdo
    };

    unsigned int indices[] = {
        0, 1, 2, // Primeiro triângulo
        2, 3, 0  // Segundo triângulo
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glGenBuffers(1, &quadEBO);

    glBindVertexArray(quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Atributo 0: Posição (x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Atributo 1: Coordenada de Textura (u, v)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    isGeometryInitialized = true;
}

void Sprite::cleanupQuadGeometry() {
    if (isGeometryInitialized) {
        glDeleteVertexArrays(1, &quadVAO);
        glDeleteBuffers(1, &quadVBO);
        glDeleteBuffers(1, &quadEBO);
        isGeometryInitialized = false;
    }
}

GLuint Sprite::loadTexture(const std::string& path, bool repeat) {
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    // Configurações de wrapping
    GLint wrapMode = repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

    // Filtragem linear/nearest para visualização pixel-art nítida
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(false);

    // Tenta carregar do caminho direto ou adicionando prefixo se necessário
    std::string actualPath = path;
    unsigned char* data = stbi_load(actualPath.c_str(), &width, &height, &nrChannels, 0);
    if (!data) {
        actualPath = "src/TrabalhoPraticoGA/" + path;
        data = stbi_load(actualPath.c_str(), &width, &height, &nrChannels, 0);
    }

    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << "[Sprite] Textura carregada: " << actualPath << " (" << width << "x" << height << ")" << std::endl;
    } else {
        std::cerr << "[Sprite] ERRO ao carregar textura: " << path << std::endl;
    }

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texID;
}

void Sprite::setSpritesheet(int totalFramesX, int totalFramesY) {
    this->totalFramesX = totalFramesX > 0 ? totalFramesX : 1;
    this->totalFramesY = totalFramesY > 0 ? totalFramesY : 1;
    this->uvScale = glm::vec2(1.0f / (float)this->totalFramesX, 1.0f / (float)this->totalFramesY);
    setFrame(0);
}

void Sprite::setFrame(int frameIndex) {
    this->currentFrame = frameIndex;
    int fx = currentFrame % totalFramesX;
    int fy = (currentFrame / totalFramesX) % totalFramesY;
    this->uvOffset = glm::vec2(fx * uvScale.x, fy * uvScale.y);
}

void Sprite::draw(GLuint shaderProgram) {
    if (textureID == 0) return;

    // Constrói matriz model: T * R * S
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));

    if (rotation != 0.0f) {
        model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    }

    model = glm::scale(model, glm::vec3(size, 1.0f));

    // Uniforms do Shader
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint uvOffsetLoc = glGetUniformLocation(shaderProgram, "uvOffset");
    GLint uvScaleLoc = glGetUniformLocation(shaderProgram, "uvScale");
    GLint colorLoc = glGetUniformLocation(shaderProgram, "spriteColor");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform2f(uvOffsetLoc, uvOffset.x, uvOffset.y);
    glUniform2f(uvScaleLoc, uvScale.x, uvScale.y);
    glUniform4f(colorLoc, color.r, color.g, color.b, color.a);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glBindVertexArray(quadVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

bool Sprite::checkCollision(const Sprite& other, float padding) const {
    // Cálculo de caixas alinhadas aos eixos (AABB)
    float halfW1 = (size.x - padding * 2.0f) * 0.5f;
    float halfH1 = (size.y - padding * 2.0f) * 0.5f;

    float halfW2 = other.size.x * 0.5f;
    float halfH2 = other.size.y * 0.5f;

    bool collisionX = (position.x + halfW1 >= other.position.x - halfW2) &&
                      (other.position.x + halfW2 >= position.x - halfW1);

    bool collisionY = (position.y + halfH1 >= other.position.y - halfH2) &&
                      (other.position.y + halfH2 >= position.y - halfH1);

    return collisionX && collisionY;
}
