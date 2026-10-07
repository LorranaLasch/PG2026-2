#include <iostream>
#include <string>
#include <assert.h>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// Protótipos das funções
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
int setupShader();
int setupGeometry();

// Dimensões da janela
const GLuint WIDTH = 800, HEIGHT = 600;

// Modo de desenho (1 = preenchido, 2 = contorno, 3 = pontos, 4 = os 3 juntos)
int drawMode = 4;

// Código fonte do Vertex Shader em GLSL
const GLchar* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 position;
void main()
{
    gl_Position = vec4(position.x, position.y, position.z, 1.0);
}
)glsl";

// Código fonte do Fragment Shader em GLSL
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
    // Inicialização da GLFW
    if (!glfwInit())
    {
        cerr << "Falha ao inicializar GLFW" << endl;
        return -1;
    }

    // Criação da janela GLFW
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Exercicio 1 - Lorrana Lasch", nullptr, nullptr);
    if (!window)
    {
        cerr << "Falha ao criar janela GLFW" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Registra função de callback de teclado
    glfwSetKeyCallback(window, key_callback);

    // GLAD: carrega ponteiros de funções da OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cerr << "Falha ao inicializar GLAD" << endl;
        return -1;
    }

    // Informações da GPU e OpenGL
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    cout << "=====================================================" << endl;
    cout << "Exercicio 1 - Lorrana Lasch" << endl;
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL Version: " << version << endl;
    cout << "-----------------------------------------------------" << endl;
    cout << "Instrucoes do teclado:" << endl;
    cout << " [1] - Apenas poligono preenchido (a)" << endl;
    cout << " [2] - Apenas contorno (b)" << endl;
    cout << " [3] - Apenas pontos (c)" << endl;
    cout << " [4] - As 3 formas de desenho juntas (d) [PADRAO]" << endl;
    cout << " [ESC] - Sair" << endl;
    cout << "=====================================================" << endl;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Compila e linka os shaders
    GLuint shaderProgram = setupShader();

    // Geometria dos 2 triângulos
    GLuint VAO = setupGeometry();

    GLint colorLoc = glGetUniformLocation(shaderProgram, "inputColor");
    assert(colorLoc > -1);

    glUseProgram(shaderProgram);

    // Loop principal da aplicação
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Limpa a tela com fundo cinza claro
        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glLineWidth(6.0f);
        glPointSize(14.0f);

        glBindVertexArray(VAO);

        // a) Polígono preenchido (se modo 1 ou 4)
        if (drawMode == 1 || drawMode == 4)
        {
            // Triângulo 1 (cinza escuro)
            glUniform4f(colorLoc, 0.35f, 0.35f, 0.35f, 1.0f);
            glDrawArrays(GL_TRIANGLES, 0, 3);

            // Triângulo 2 (cinza médio)
            glUniform4f(colorLoc, 0.55f, 0.55f, 0.55f, 1.0f);
            glDrawArrays(GL_TRIANGLES, 3, 3);
        }

        // b) Contorno (se modo 2 ou 4)
        if (drawMode == 2 || drawMode == 4)
        {
            // Contorno do Triângulo 1 (preto)
            glUniform4f(colorLoc, 0.0f, 0.0f, 0.0f, 1.0f);
            glDrawArrays(GL_LINE_LOOP, 0, 3);

            // Contorno do Triângulo 2 (preto)
            glUniform4f(colorLoc, 0.0f, 0.0f, 0.0f, 1.0f);
            glDrawArrays(GL_LINE_LOOP, 3, 3);
        }

        // c) Pontos (se modo 3 ou 4)
        if (drawMode == 3 || drawMode == 4)
        {
            // Pontos do Triângulo 1 (azul escuro / ciano)
            glUniform4f(colorLoc, 0.1f, 0.2f, 0.8f, 1.0f);
            glDrawArrays(GL_POINTS, 0, 3);

            // Pontos do Triângulo 2 (vermelho)
            glUniform4f(colorLoc, 0.85f, 0.15f, 0.15f, 1.0f);
            glDrawArrays(GL_POINTS, 3, 3);
        }

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

    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        drawMode = 1;
        cout << "[Modo 1 selecionado]: Apenas poligono preenchido (a)" << endl;
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        drawMode = 2;
        cout << "[Modo 2 selecionado]: Apenas contorno (b)" << endl;
    }
    if (key == GLFW_KEY_3 && action == GLFW_PRESS)
    {
        drawMode = 3;
        cout << "[Modo 3 selecionado]: Apenas pontos (c)" << endl;
    }
    if (key == GLFW_KEY_4 && action == GLFW_PRESS)
    {
        drawMode = 4;
        cout << "[Modo 4 selecionado]: As 3 formas de desenho juntas (d)" << endl;
    }
}

int setupShader()
{
    // Vertex Shader
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

    // Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cerr << "ERRO: Compilacao Fragment Shader falhou:\n" << infoLog << endl;
    }

    // Link do Programa de Shader
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
    GLfloat vertices[] = {
        // Triângulo 1 (Esquerda)
        -0.7f, -0.5f, 0.0f,
         0.1f, -0.5f, 0.0f,
        -0.3f,  0.5f, 0.0f,

        // Triângulo 2 (Direita)
        -0.1f,  0.5f, 0.0f,
         0.3f, -0.5f, 0.0f,
         0.7f,  0.5f, 0.0f
    };

    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}
