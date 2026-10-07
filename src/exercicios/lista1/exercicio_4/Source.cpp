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

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Exercicio 4 - Desenho Livre (Chapeu de Bruxo) - Lorrana Lasch", nullptr, nullptr);
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
    cout << "Exercicio 4 - Desenho Livre: Chapeu de Bruxo - Lorrana Lasch" << endl;
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL Version: " << version << endl;
    cout << "------------------------------------------------------------" << endl;
    cout << "Desenho composto por 4 partes distintas e cores uniformes:" << endl;
    cout << " 1. Aba do chapeu (retangulo na base, cinza grafite)" << endl;
    cout << " 2. Cone do chapeu (triangulo inclinado)" << endl;
    cout << " 3. Faixa decorativa (quad roxo)" << endl;
    cout << " 4. Fivela frontal (quad dourado)" << endl;
    cout << " [ESC] - Sair" << endl;
    cout << "============================================================" << endl;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    GLuint shaderProgram = setupShader();
    GLuint VAO = setupGeometry();

    GLint colorLoc = glGetUniformLocation(shaderProgram, "inputColor");
    assert(colorLoc > -1);

    glUseProgram(shaderProgram);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Fundo em tom pastel suave para destacar a silhueta do chapéu
        glClearColor(0.88f, 0.88f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);

        // 1. Aba do chapéu (Cinza Grafite / Quase Preto) - 6 vértices
        glUniform4f(colorLoc, 0.15f, 0.15f, 0.18f, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 2. Cone do chapéu (Cinza Grafite / Quase Preto) - 3 vértices
        glDrawArrays(GL_TRIANGLES, 6, 3);

        // 3. Faixa do chapéu (Roxo Místico) - 6 vértices
        glUniform4f(colorLoc, 0.55f, 0.15f, 0.65f, 1.0f);
        glDrawArrays(GL_TRIANGLES, 9, 6);

        // 4. Fivela dourada (Amarelo Ouro) - 6 vértices
        glUniform4f(colorLoc, 0.95f, 0.82f, 0.15f, 1.0f);
        glDrawArrays(GL_TRIANGLES, 15, 6);

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
    GLfloat vertices[] = {
        // 1. Aba do chapéu (Retângulo na base) - 6 vértices
        -0.70f, -0.30f, 0.0f,
         0.70f, -0.30f, 0.0f,
         0.70f, -0.20f, 0.0f,
        -0.70f, -0.30f, 0.0f,
         0.70f, -0.20f, 0.0f,
        -0.70f, -0.20f, 0.0f,

        // 2. Cone do chapéu (Triângulo inclinado para a direita) - 3 vértices
        -0.35f, -0.20f, 0.0f,  // Base esquerda
         0.35f, -0.20f, 0.0f,  // Base direita
         0.38f,  0.65f, 0.0f,  // Ponta curva do chapéu

        // 3. Faixa do chapéu (Faixa logo acima da aba) - 6 vértices
        -0.32f, -0.20f, 0.0f,
         0.32f, -0.20f, 0.0f,
         0.33f, -0.06f, 0.0f,
        -0.32f, -0.20f, 0.0f,
         0.33f, -0.06f, 0.0f,
        -0.20f, -0.06f, 0.0f,

        // 4. Fivela dourada (Quadrado no meio da faixa) - 6 vértices
        -0.08f, -0.18f, 0.0f,
         0.08f, -0.18f, 0.0f,
         0.08f, -0.08f, 0.0f,
        -0.08f, -0.18f, 0.0f,
         0.08f, -0.08f, 0.0f,
        -0.08f, -0.08f, 0.0f
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