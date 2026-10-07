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

// Protótipos das funções
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
int setupShader();

// Funções para geração das geometrias paramétricas
GLuint createParametricShape(int nSlices, float startAngle, float endAngle, float r1, float r2, bool hasCenter, bool alternateRadius, int& outCount);
GLuint createSpiral(int nPoints, float maxAngle, float maxRadius, float centerX, int& outCount);

// Dimensões da janela
const GLuint WIDTH = 800, HEIGHT = 600;
const float PI = 3.14159265358979323846f;

// Identificadores de geometrias
enum ShapeType {
    SHAPE_CIRCLE = 0,     // Círculo base
    SHAPE_OCTAGON = 1,    // a) Octógono (8 lados)
    SHAPE_PENTAGON = 2,   // b) Pentágono (5 lados)
    SHAPE_PACMAN = 3,     // c) Pac-man
    SHAPE_PIZZA = 4,      // d) Fatia de pizza
    SHAPE_STAR = 5,       // e) Desafio 1: Estrela
    SHAPE_SPIRAL = 6,     // f) Desafio 2: Espiral
    SHAPE_DUAL = 7        // Estrela + Espiral lado a lado (modo original)
};

ShapeType currentShape = SHAPE_CIRCLE;

// Estrutura para armazenar cada forma
struct Geometry {
    GLuint vao;
    int vertexCount;
    GLenum primitive;
    float r, g, b;
    string name;
};

Geometry shapes[8];

// Código fonte do Vertex Shader
const GLchar* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 position;
void main()
{
    gl_Position = vec4(position.x, position.y, position.z, 1.0);
}
)glsl";

// Código fonte do Fragment Shader
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

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Exercicio 2 - Geometria Parametrica - Lorrana Lasch", nullptr, nullptr);
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

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    cout << "============================================================" << endl;
    cout << "Exercicio 2: Geometria Parametrica - Lorrana Lasch" << endl;
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL Version: " << version << endl;
    cout << "------------------------------------------------------------" << endl;
    cout << "Controles pelo teclado para navegar nas formas solicitadas:" << endl;
    cout << " [C] ou [1] - Circulo parametrico base" << endl;
    cout << " [8] ou [2] - a) Octogono (8 vertices)" << endl;
    cout << " [5] ou [3] - b) Pentagono (5 vertices)" << endl;
    cout << " [P] ou [4] - c) Pac-man" << endl;
    cout << " [F] ou [5] - d) Fatia de pizza" << endl;
    cout << " [E] ou [6] - e) DESAFIO 1: Estrela (raios alternados)" << endl;
    cout << " [S] ou [7] - f) DESAFIO 2: Espiral (raio crescente)" << endl;
    cout << " [T] ou [8] - Estrela e Espiral lado a lado (modo comparativo)" << endl;
    cout << " [ESC]      - Sair" << endl;
    cout << "============================================================" << endl;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    GLuint shaderProgram = setupShader();
    GLint colorLoc = glGetUniformLocation(shaderProgram, "inputColor");
    assert(colorLoc > -1);

    // 0. Círculo base (64 fatias)
    shapes[SHAPE_CIRCLE].vao = createParametricShape(64, 0.0f, 2.0f * PI, 0.5f, 0.5f, true, false, shapes[SHAPE_CIRCLE].vertexCount);
    shapes[SHAPE_CIRCLE].primitive = GL_TRIANGLE_FAN;
    shapes[SHAPE_CIRCLE].r = 0.2f; shapes[SHAPE_CIRCLE].g = 0.6f; shapes[SHAPE_CIRCLE].b = 0.9f;
    shapes[SHAPE_CIRCLE].name = "Circulo Parametrico Base";

    // 1. Octógono (8 fatias)
    shapes[SHAPE_OCTAGON].vao = createParametricShape(8, 0.0f, 2.0f * PI, 0.5f, 0.5f, true, false, shapes[SHAPE_OCTAGON].vertexCount);
    shapes[SHAPE_OCTAGON].primitive = GL_TRIANGLE_FAN;
    shapes[SHAPE_OCTAGON].r = 0.3f; shapes[SHAPE_OCTAGON].g = 0.8f; shapes[SHAPE_OCTAGON].b = 0.4f;
    shapes[SHAPE_OCTAGON].name = "a) Octogono";

    // 2. Pentágono (5 fatias)
    shapes[SHAPE_PENTAGON].vao = createParametricShape(5, 0.0f, 2.0f * PI, 0.5f, 0.5f, true, false, shapes[SHAPE_PENTAGON].vertexCount);
    shapes[SHAPE_PENTAGON].primitive = GL_TRIANGLE_FAN;
    shapes[SHAPE_PENTAGON].r = 0.9f; shapes[SHAPE_PENTAGON].g = 0.5f; shapes[SHAPE_PENTAGON].b = 0.2f;
    shapes[SHAPE_PENTAGON].name = "b) Pentagono";

    // 3. Pac-man (arco de 30 graus a 330 graus)
    shapes[SHAPE_PACMAN].vao = createParametricShape(50, PI / 6.0f, 11.0f * PI / 6.0f, 0.5f, 0.5f, true, false, shapes[SHAPE_PACMAN].vertexCount);
    shapes[SHAPE_PACMAN].primitive = GL_TRIANGLE_FAN;
    shapes[SHAPE_PACMAN].r = 1.0f; shapes[SHAPE_PACMAN].g = 0.9f; shapes[SHAPE_PACMAN].b = 0.0f; // Amarelo
    shapes[SHAPE_PACMAN].name = "c) Pac-man";

    // 4. Fatia de pizza (arco de 60 graus)
    shapes[SHAPE_PIZZA].vao = createParametricShape(16, -PI / 6.0f, PI / 6.0f, 0.55f, 0.55f, true, false, shapes[SHAPE_PIZZA].vertexCount);
    shapes[SHAPE_PIZZA].primitive = GL_TRIANGLE_FAN;
    shapes[SHAPE_PIZZA].r = 0.95f; shapes[SHAPE_PIZZA].g = 0.65f; shapes[SHAPE_PIZZA].b = 0.15f;
    shapes[SHAPE_PIZZA].name = "d) Fatia de Pizza";

    // 5. Estrela (10 pontos alternando r=0.5f e r=0.2f)
    shapes[SHAPE_STAR].vao = createParametricShape(10, 0.0f, 2.0f * PI, 0.5f, 0.2f, true, true, shapes[SHAPE_STAR].vertexCount);
    shapes[SHAPE_STAR].primitive = GL_TRIANGLE_FAN;
    shapes[SHAPE_STAR].r = 0.95f; shapes[SHAPE_STAR].g = 0.8f; shapes[SHAPE_STAR].b = 0.1f;
    shapes[SHAPE_STAR].name = "e) DESAFIO 1: Estrela";

    // 6. Espiral (1000 pontos, raio crescendo de 0 até 0.5)
    shapes[SHAPE_SPIRAL].vao = createSpiral(1000, 6.0f * PI, 0.5f, 0.0f, shapes[SHAPE_SPIRAL].vertexCount);
    shapes[SHAPE_SPIRAL].primitive = GL_LINE_STRIP;
    shapes[SHAPE_SPIRAL].r = 0.85f; shapes[SHAPE_SPIRAL].g = 0.15f; shapes[SHAPE_SPIRAL].b = 0.15f;
    shapes[SHAPE_SPIRAL].name = "f) DESAFIO 2: Espiral";

    // 7. Geometrias auxiliares para o modo dual (estrela à esquerda e espiral à direita)
    int starDualCount = 0;
    int spiralDualCount = 0;
    GLuint starDualVAO = createParametricShape(10, 0.0f, 2.0f * PI, 0.4f, 0.16f, true, true, starDualCount);
    GLuint spiralDualVAO = createSpiral(1000, 6.0f * PI, 0.38f, 0.5f, spiralDualCount);

    glUseProgram(shaderProgram);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.85f, 0.85f, 0.85f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glLineWidth(2.5f);
        glPointSize(8.0f);

        if (currentShape == SHAPE_DUAL)
        {
            // Modo comparativo: Estrela na esquerda e Espiral na direita
            glBindVertexArray(starDualVAO);
            glUniform4f(colorLoc, 0.35f, 0.35f, 0.35f, 1.0f);
            glDrawArrays(GL_TRIANGLE_FAN, 0, starDualCount);

            glBindVertexArray(spiralDualVAO);
            glUniform4f(colorLoc, 0.85f, 0.1f, 0.1f, 1.0f);
            glDrawArrays(GL_LINE_STRIP, 0, spiralDualCount);
        }
        else
        {
            // Desenha a forma atualmente selecionada centralizada
            const Geometry& g = shapes[currentShape];
            glBindVertexArray(g.vao);
            glUniform4f(colorLoc, g.r, g.g, g.b, 1.0f);
            glDrawArrays(g.primitive, 0, g.vertexCount);

            // Se for polígono, adiciona um contorno sutil preto para acabamento bonito
            if (g.primitive == GL_TRIANGLE_FAN)
            {
                glUniform4f(colorLoc, 0.1f, 0.1f, 0.1f, 1.0f);
                glDrawArrays(GL_LINE_LOOP, 1, g.vertexCount - 1);
            }
        }

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    for (int i = 0; i < 7; ++i)
        glDeleteVertexArrays(1, &shapes[i].vao);

    glDeleteVertexArrays(1, &starDualVAO);
    glDeleteVertexArrays(1, &spiralDualVAO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_C || key == GLFW_KEY_1)
        {
            currentShape = SHAPE_CIRCLE;
            cout << ">> " << shapes[currentShape].name << endl;
        }
        else if (key == GLFW_KEY_8 || key == GLFW_KEY_2)
        {
            currentShape = SHAPE_OCTAGON;
            cout << ">> " << shapes[currentShape].name << endl;
        }
        else if (key == GLFW_KEY_5 || key == GLFW_KEY_3)
        {
            currentShape = SHAPE_PENTAGON;
            cout << ">> " << shapes[currentShape].name << endl;
        }
        else if (key == GLFW_KEY_P || key == GLFW_KEY_4)
        {
            currentShape = SHAPE_PACMAN;
            cout << ">> " << shapes[currentShape].name << endl;
        }
        else if (key == GLFW_KEY_F || key == GLFW_KEY_5)
        {
            currentShape = SHAPE_PIZZA;
            cout << ">> " << shapes[currentShape].name << endl;
        }
        else if (key == GLFW_KEY_E || key == GLFW_KEY_6)
        {
            currentShape = SHAPE_STAR;
            cout << ">> " << shapes[currentShape].name << endl;
        }
        else if (key == GLFW_KEY_S || key == GLFW_KEY_7)
        {
            currentShape = SHAPE_SPIRAL;
            cout << ">> " << shapes[currentShape].name << endl;
        }
        else if (key == GLFW_KEY_T || key == GLFW_KEY_D || key == GLFW_KEY_8)
        {
            currentShape = SHAPE_DUAL;
            cout << ">> Modo comparativo: Estrela + Espiral lado a lado" << endl;
        }
    }
}

// Criação de formas paramétricas circulares e derivadas com GL_TRIANGLE_FAN
GLuint createParametricShape(int nSlices, float startAngle, float endAngle, float r1, float r2, bool hasCenter, bool alternateRadius, int& outCount)
{
    vector<GLfloat> vertices;

    // Vértice central
    if (hasCenter)
    {
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
    }

    float deltaAngle = (endAngle - startAngle) / (float)nSlices;
    int totalSteps = nSlices + (abs(endAngle - startAngle - 2.0f * PI) < 0.001f ? 1 : 1);

    for (int i = 0; i <= nSlices; ++i)
    {
        float angle = startAngle + i * deltaAngle;
        float radius = (alternateRadius && (i % 2 == 1)) ? r2 : r1;

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

// Criação da espiral paramétrica de Arquimedes (r = a * theta)
GLuint createSpiral(int nPoints, float maxAngle, float maxRadius, float centerX, int& outCount)
{
    vector<GLfloat> vertices;
    float deltaAngle = maxAngle / (float)(nPoints - 1);
    float radiusIncrement = maxRadius / (float)(nPoints - 1);

    for (int i = 0; i < nPoints; ++i)
    {
        float angle = i * deltaAngle;
        float radius = i * radiusIncrement;

        vertices.push_back(centerX + radius * cos(angle));
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
        cerr << "ERRO: Compilacao Vertex Shader falhou:\n" << infoLog << endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cerr << "ERRO: Compilacao Fragment Shader falhou:\n" << infoLog << endl;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cerr << "ERRO: Link do Shader Program falhou:\n" << infoLog << endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}