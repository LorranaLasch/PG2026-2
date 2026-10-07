/*
 * Exercicio 3 (Screensaver / Pong) - Lista 3
 * Geometria com comportamento de Pong: move-se autonomamente e espelha o vetor
 * de direcao sempre que colide com os limites da janela da aplicacao.
 *
 * Aluna: Lorrana Lasch
 * Baseado no codigo oficial HelloTransforms.cpp da professora Rossana Baptista Queiroz
 */

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <assert.h>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
int setupShader();
GLuint setupCircleGeometry(float radius, int nSegments, int& outCount);

const GLuint WIDTH = 800, HEIGHT = 600;
const float PI = 3.14159265358979323846f;
const float RADIUS = 35.0f;

// Posição e velocidade iniciais (pixels por segundo)
float posX = 400.0f, posY = 300.0f;
float speedX = 260.0f, speedY = 200.0f;
float rotationAngle = 0.0f;

// Cores dinâmicas a cada colisão
vector<vec3> bounceColors;
int colorIndex = 0;

const GLchar* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 position;
uniform mat4 projection;
uniform mat4 model;
void main()
{
    gl_Position = projection * model * vec4(position, 1.0);
}
)glsl";

const GLchar* fragmentShaderSource = R"glsl(
#version 330 core
uniform vec4 inputColor;
out vec4 color;
void main()
{
    color = inputColor;
}
)glsl";

int main()
{
    if (!glfwInit())
    {
        cerr << "Falha ao inicializar GLFW" << endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Exercicio 3 - Screensaver Pong - Lorrana Lasch", nullptr, nullptr);
    if (!window)
    {
        cerr << "Falha ao criar janela GLFW" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cerr << "Falha ao inicializar GLAD" << endl;
        return -1;
    }

    // Inicializa cores para as batidas
    bounceColors.push_back(vec3(0.92f, 0.26f, 0.21f)); // Vermelho
    bounceColors.push_back(vec3(0.18f, 0.80f, 0.44f)); // Verde
    bounceColors.push_back(vec3(0.16f, 0.50f, 0.93f)); // Azul
    bounceColors.push_back(vec3(0.98f, 0.75f, 0.18f)); // Dourado
    bounceColors.push_back(vec3(0.61f, 0.35f, 0.71f)); // Roxo
    bounceColors.push_back(vec3(0.00f, 0.81f, 0.82f)); // Turquesa

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    cout << "==========================================================================" << endl;
    cout << "Exercicio 3 (Lista 3): Screensaver Pong - Lorrana Lasch" << endl;
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL Version: " << version << endl;
    cout << "--------------------------------------------------------------------------" << endl;
    cout << "A geometria movimenta-se pela tela e espelha o vetor de direcao a cada" << endl;
    cout << "colisao com as bordas (efeito classico de Screensaver / DVD / Pong)." << endl;
    cout << " [ESC] - Sair" << endl;
    cout << "==========================================================================" << endl;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    GLuint shaderProgram = setupShader();
    GLint colorLoc = glGetUniformLocation(shaderProgram, "inputColor");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    assert(colorLoc > -1 && projLoc > -1 && modelLoc > -1);

    int vertexCount = 0;
    GLuint VAO = setupCircleGeometry(RADIUS, 32, vertexCount);

    glUseProgram(shaderProgram);

    mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(projection));

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        double currentTime = glfwGetTime();
        float deltaTime = (float)(currentTime - lastTime);
        lastTime = currentTime;

        // Atualização da física de colisão (espelhar vetor)
        posX += speedX * deltaTime;
        posY += speedY * deltaTime;
        rotationAngle += 1.5f * deltaTime;

        bool bounced = false;

        // Borda esquerda ou direita
        if (posX - RADIUS <= 0.0f)
        {
            posX = RADIUS;
            speedX = -speedX;
            bounced = true;
        }
        else if (posX + RADIUS >= 800.0f)
        {
            posX = 800.0f - RADIUS;
            speedX = -speedX;
            bounced = true;
        }

        // Borda inferior ou superior
        if (posY - RADIUS <= 0.0f)
        {
            posY = RADIUS;
            speedY = -speedY;
            bounced = true;
        }
        else if (posY + RADIUS >= 600.0f)
        {
            posY = 600.0f - RADIUS;
            speedY = -speedY;
            bounced = true;
        }

        if (bounced)
            colorIndex = (colorIndex + 1) % bounceColors.size();

        glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);

        mat4 model = mat4(1.0f);
        model = translate(model, vec3(posX, posY, 0.0f));
        model = rotate(model, rotationAngle, vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(model));

        vec3 curCol = bounceColors[colorIndex];
        glUniform4f(colorLoc, curCol.r, curCol.g, curCol.b, 1.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCount);

        // Contorno
        glLineWidth(2.5f);
        glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 0.9f);
        glDrawArrays(GL_LINE_LOOP, 1, vertexCount - 1);

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

GLuint setupCircleGeometry(float radius, int nSegments, int& outCount)
{
    vector<GLfloat> vertices;

    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);

    float deltaAngle = 2.0f * PI / (float)nSegments;
    for (int i = 0; i <= nSegments; ++i)
    {
        float angle = i * deltaAngle;
        vertices.push_back(radius * cos(angle));
        vertices.push_back(radius * sin(angle));
        vertices.push_back(0.0f);
    }

    outCount = vertices.size() / 3;

    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

int setupShader()
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cerr << "ERRO: Compilacao Vertex Shader:\n" << infoLog << endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cerr << "ERRO: Compilacao Fragment Shader:\n" << infoLog << endl;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cerr << "ERRO: Link Shader Program:\n" << infoLog << endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}
