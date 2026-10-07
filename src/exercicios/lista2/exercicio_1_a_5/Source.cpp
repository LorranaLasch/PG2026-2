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

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
GLuint setupCircleGeometry(float cx, float cy, float radius, int nSegments, int& outCount);

// Dimensões da janela
const GLuint WIDTH = 800, HEIGHT = 600;
const float PI = 3.14159265358979323846f;

// Modo ativo (1 a 5 correspondente aos exercícios da Lista 2)
int currentMode = 5; // Padrão: Exercício 5 (4 quadrantes)

// Código fonte do Vertex Shader (baseado em HelloOrtho.cpp da professora)
const GLchar* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 position;
uniform mat4 projection;
void main()
{
    gl_Position = projection * vec4(position, 1.0);
}
)glsl";

// Código fonte do Fragment Shader (baseado em HelloOrtho.cpp da professora)
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

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Lista 2 (Exercicios 1 a 5) - Lorrana Lasch", nullptr, nullptr);
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
    cout << "====================================================================" << endl;
    cout << "Lista 2: Exercicios 1 a 5 (Camera 2D & Viewports) - Lorrana Lasch" << endl;
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL Version: " << version << endl;
    cout << "--------------------------------------------------------------------" << endl;
    cout << "Teclas para alternar entre os exercicios:" << endl;
    cout << " [1] - Exercicio 1: Janela do mundo ortho(-10, 10, -10, 10)" << endl;
    cout << " [2] - Exercicio 2: Janela do mundo ortho(0, 800, 600, 0) [pixels]" << endl;
    cout << " [3] - Exercicio 3: Cena desenhada com a camera 2D em tela cheia" << endl;
    cout << " [4] - Exercicio 4: Cena desenhada apenas no quadrante superior direito" << endl;
    cout << " [5] - Exercicio 5: Mesma cena desenhada nos 4 quadrantes [PADRAO]" << endl;
    cout << " [ESC] - Sair" << endl;
    cout << "====================================================================" << endl;

    GLuint shaderProgram = setupShader();
    GLint colorLoc = glGetUniformLocation(shaderProgram, "inputColor");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    assert(colorLoc > -1 && projLoc > -1);

    // Geometria 1: Círculo para o espaço [-10, 10] (raio = 4.0 unidades, centro em 0,0)
    int countEx1 = 0;
    GLuint vaoEx1 = setupCircleGeometry(0.0f, 0.0f, 4.0f, 64, countEx1);

    // Geometria 2: Círculo para o espaço em pixels [0..800, 600..0] (raio = 100 pixels, centro em 400, 300)
    int countPixel = 0;
    GLuint vaoPixel = setupCircleGeometry(400.0f, 300.0f, 100.0f, 64, countPixel);

    glUseProgram(shaderProgram);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Limpa buffer de cor
        glClearColor(0.85f, 0.85f, 0.85f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        mat4 projection = mat4(1.0f);

        if (currentMode == 1)
        {
            // Exercício 1: ortho xmin=-10, xmax=10, ymin=-10, ymax=10
            projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -1.0f, 1.0f);
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(projection));

            glViewport(0, 0, width, height);
            glBindVertexArray(vaoEx1);
            glUniform4f(colorLoc, 0.2f, 0.5f, 0.8f, 1.0f);
            glDrawArrays(GL_TRIANGLE_FAN, 0, countEx1);
        }
        else if (currentMode == 2 || currentMode == 3)
        {
            // Exercício 2 e 3: ortho xmin=0, xmax=800, ymin=600, ymax=0 (origem no topo-esquerdo)
            projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(projection));

            glViewport(0, 0, width, height);
            glBindVertexArray(vaoPixel);
            glUniform4f(colorLoc, 0.35f, 0.35f, 0.35f, 1.0f);
            glDrawArrays(GL_TRIANGLE_FAN, 0, countPixel);
        }
        else if (currentMode == 4)
        {
            // Exercício 4: Viewport apenas no quadrante superior direito
            projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(projection));

            // Quadrante superior direito: x = width/2, y = height/2
            glViewport(width / 2, height / 2, width / 2, height / 2);
            glBindVertexArray(vaoPixel);
            glUniform4f(colorLoc, 0.35f, 0.35f, 0.35f, 1.0f);
            glDrawArrays(GL_TRIANGLE_FAN, 0, countPixel);
        }
        else if (currentMode == 5)
        {
            // Exercício 5: Mesma cena desenhada nos 4 quadrantes
            projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(projection));

            glBindVertexArray(vaoPixel);

            for (int i = 0; i < 4; i++)
            {
                int vx = 0, vy = 0;
                switch (i)
                {
                    case 0: vx = 0; vy = 0; break;                 // Inferior Esquerdo
                    case 1: vx = 0; vy = height / 2; break;        // Superior Esquerdo
                    case 2: vx = width / 2; vy = 0; break;         // Inferior Direito
                    case 3: vx = width / 2; vy = height / 2; break;// Superior Direito
                }

                glViewport(vx, vy, width / 2, height / 2);

                // Alterna suavemente o tom de cinza em cada quadrante para distinguir
                float intensity = 0.3f + i * 0.15f;
                glUniform4f(colorLoc, intensity, intensity, intensity, 1.0f);
                glDrawArrays(GL_TRIANGLE_FAN, 0, countPixel);
            }
        }

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &vaoEx1);
    glDeleteVertexArrays(1, &vaoPixel);
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
        if (key == GLFW_KEY_1)
        {
            currentMode = 1;
            cout << "[Modo 1 selecionado]: Exercicio 1 - Ortho(-10, 10, -10, 10)" << endl;
        }
        else if (key == GLFW_KEY_2)
        {
            currentMode = 2;
            cout << "[Modo 2 selecionado]: Exercicio 2 - Ortho(0, 800, 600, 0) [pixels]" << endl;
        }
        else if (key == GLFW_KEY_3)
        {
            currentMode = 3;
            cout << "[Modo 3 selecionado]: Exercicio 3 - Camera 2D em tela cheia" << endl;
        }
        else if (key == GLFW_KEY_4)
        {
            currentMode = 4;
            cout << "[Modo 4 selecionado]: Exercicio 4 - Quadrante superior direito" << endl;
        }
        else if (key == GLFW_KEY_5)
        {
            currentMode = 5;
            cout << "[Modo 5 selecionado]: Exercicio 5 - 4 Quadrantes simultaneos" << endl;
        }
    }
}

GLuint setupCircleGeometry(float cx, float cy, float radius, int nSegments, int& outCount)
{
    vector<GLfloat> vertices;

    // Vértice central
    vertices.push_back(cx);
    vertices.push_back(cy);
    vertices.push_back(0.0f);

    float deltaAngle = 2.0f * PI / (float)nSegments;

    for (int i = 0; i <= nSegments; ++i)
    {
        float angle = i * deltaAngle;
        vertices.push_back(cx + radius * cos(angle));
        vertices.push_back(cy + radius * sin(angle));
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
