#include <iostream>
#include <string>
#include <assert.h>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// Protótipos das funções
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
int setupShader();
int setupGeometry();

// Dimensões da janela
const GLuint WIDTH = 800, HEIGHT = 600;

/*
 =========================================================================
 RESPOSTAS TEÓRICAS DO EXERCÍCIO 3 (Solicitadas pela professora):
 -------------------------------------------------------------------------
 a) Descreva uma possível configuração dos buffers (VBO, VAO) para representá-lo:
    - Utilizamos um único VBO armazenando dados intercalados (interleaved):
      [x, y, z, r, g, b,  x, y, z, r, g, b,  x, y, z, r, g, b]
      Cada vértice ocupa 6 floats (24 bytes).
    - No VAO, habilitamos dois ponteiros de atributos (glVertexAttribPointer):
      1. Atributo 0 (Posição): 3 floats, stride = 6 * sizeof(float), offset = 0.
      2. Atributo 1 (Cor): 3 floats, stride = 6 * sizeof(float), offset = 3 * sizeof(float).

 b) Como estes atributos seriam identificados no vertex shader?
    - São declarados com layout (location = X) in vec3:
      layout (location = 0) in vec3 position; // Recebe coordenada (x,y,z)
      layout (location = 1) in vec3 vcolor;   // Recebe componente (r,g,b)
    - O vertex shader exporta essa cor para o fragment shader através de
      uma variável 'out vec4 finalColor', garantindo que o rasterizador da
      GPU interpole suavemente os gradientes entre os vértices.
 =========================================================================
*/

// Código fonte do Vertex Shader em GLSL
const GLchar *vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 position; // Atributo 0: posição
layout (location = 1) in vec3 vcolor;   // Atributo 1: cor RGB

out vec4 interpolatedColor; // Variável de saída para interpolar no Fragment Shader

void main()
{
    gl_Position = vec4(position.x, position.y, position.z, 1.0);
    interpolatedColor = vec4(vcolor, 1.0);
}
)glsl";

// Código fonte do Fragment Shader em GLSL
const GLchar *fragmentShaderSource = R"glsl(
#version 330 core
in vec4 interpolatedColor; // Recebe a cor já interpolada pela GPU
out vec4 color;

void main()
{
    color = interpolatedColor;
}
)glsl";

int main()
{
    if (!glfwInit())
    {
        cerr << "Falha ao inicializar GLFW" << endl;
        return -1;
    }

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Exercicio 3 - Triangulo Interpolado RGB - Lorrana Lasch", nullptr, nullptr);
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
    cout << "=========================================================================" << endl;
    cout << "Exercicio 3 - Triangulo Interpolado RGB - Lorrana Lasch" << endl;
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL Version: " << version << endl;
    cout << "-------------------------------------------------------------------------" << endl;
    cout << "Respostas Teoricas:" << endl;
    cout << " a) Configuracao VBO/VAO: buffer unico intercalado (stride 6 floats)," << endl;
    cout << "    posicao no offset 0 e cor no offset 3 * sizeof(float)." << endl;
    cout << " b) No Vertex Shader: layout(location = 0) in vec3 position;" << endl;
    cout << "                      layout(location = 1) in vec3 vcolor;" << endl;
    cout << "                      out vec4 interpolatedColor;" << endl;
    cout << " [ESC] - Sair" << endl;
    cout << "=========================================================================" << endl;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    GLuint shaderProgram = setupShader();
    GLuint VAO = setupGeometry();

    glUseProgram(shaderProgram);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Fundo cinza escuro para destacar as cores vivas do triângulo
        glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);

        // Desenha o triângulo (a GPU interpola automaticamente Vermelho, Verde e Azul)
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
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

int setupGeometry()
{
    // Vértices intercalados: Posição (x, y, z) + Cor (r, g, b)
    GLfloat vertices[] = {
        // x       y      z      r     g     b
         0.0f,   0.55f, 0.0f,  1.0f, 0.0f, 0.0f, // P1 (Topo)           - VERMELHO
        -0.55f, -0.45f, 0.0f,  0.0f, 1.0f, 0.0f, // P2 (Base Esquerda)  - VERDE
         0.55f, -0.45f, 0.0f,  0.0f, 0.0f, 1.0f  // P3 (Base Direita)   - AZUL
    };

    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Atributo 0: Posição (x, y, z) -> 3 floats, stride = 6 * sizeof(float), offset = 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Atributo 1: Cor (r, g, b) -> 3 floats, stride = 6 * sizeof(float), offset = 3 * sizeof(float)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}
