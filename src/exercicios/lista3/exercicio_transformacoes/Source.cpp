/*
 * Exercicio 1 (Transformacoes) - Lista 3
 * Desenha uma mesma geometria 3 vezes na tela aplicando transformacoes diferentes
 * (mesmo VAO, matrizes model distintas e 3 chamadas de desenho).
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

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Exercicio 1 - Transformacoes Multiplas - Lorrana Lasch", nullptr, nullptr);
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
    cout << "==========================================================================" << endl;
    cout << "Exercicio 1 (Lista 3): 3 Instancias com Transformacoes - Lorrana Lasch" << endl;
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL Version: " << version << endl;
    cout << "--------------------------------------------------------------------------" << endl;
    cout << "Mesmo VAO renderizado 3 vezes com matrizes model, escalas, rotacoes e cores" << endl;
    cout << "distintas a cada chamada de desenho (glDrawArrays)." << endl;
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
    GLuint VAO = setupCircleGeometry(60.0f, 64, vertexCount);

    glUseProgram(shaderProgram);

    mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(projection));

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.88f, 0.88f, 0.90f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float time = (float)glfwGetTime();

        glBindVertexArray(VAO);

        // -------------------------------------------------------------
        // Instância 1: Esquerda inferior, escala menor (0.7), pulso suave
        // -------------------------------------------------------------
        mat4 model1 = mat4(1.0f);
        model1 = translate(model1, vec3(200.0f, 200.0f, 0.0f));
        model1 = rotate(model1, time * 0.8f, vec3(0.0f, 0.0f, 1.0f));
        model1 = scale(model1, vec3(0.7f + 0.1f * sin(time * 2.0f), 0.7f + 0.1f * sin(time * 2.0f), 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(model1));
        glUniform4f(colorLoc, 0.2f, 0.5f, 0.85f, 1.0f); // Azul
        glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCount);

        // -------------------------------------------------------------
        // Instância 2: Centro, escala normal (1.0), rotação média
        // -------------------------------------------------------------
        mat4 model2 = mat4(1.0f);
        model2 = translate(model2, vec3(400.0f, 300.0f, 0.0f));
        model2 = rotate(model2, -time * 1.2f, vec3(0.0f, 0.0f, 1.0f));
        model2 = scale(model2, vec3(1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(model2));
        glUniform4f(colorLoc, 0.85f, 0.25f, 0.25f, 1.0f); // Vermelho
        glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCount);

        // -------------------------------------------------------------
        // Instância 3: Direita superior, escala maior (1.3), rotação rápida
        // -------------------------------------------------------------
        mat4 model3 = mat4(1.0f);
        model3 = translate(model3, vec3(600.0f, 400.0f, 0.0f));
        model3 = rotate(model3, time * 2.0f, vec3(0.0f, 0.0f, 1.0f));
        model3 = scale(model3, vec3(1.3f, 1.3f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(model3));
        glUniform4f(colorLoc, 0.2f, 0.75f, 0.35f, 1.0f); // Verde
        glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCount);

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
