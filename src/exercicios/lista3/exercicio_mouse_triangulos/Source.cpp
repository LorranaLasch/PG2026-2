/*
 * Lista de Exercicios 3 - Processamento Grafico: Fundamentos
 * Enunciado Oficial da Unisinos (Lista 3.pdf): "Criando Triangulos a partir do Clique do Mouse"
 * Aluna: Lorrana Lasch
 * Baseado no codigo oficial da professora Rossana Baptista Queiroz
 *
 * Requisitos:
 * 1) Ao clicar na tela, cria-se apenas 1 vertice por clique.
 * 2) A cada 3 vertices criados (3 cliques), forma-se um triangulo.
 * 3) Para cada novo triangulo criado, seleciona-se uma cor nova.
 * 4) Projecao paralela ortografica com janela do mundo 800 x 600 (1 unidade = 1 pixel).
 */

#include <iostream>
#include <string>
#include <vector>
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

// Estrutura do triângulo
struct Triangle {
    vec3 p1, p2, p3;
    vec3 color;
};

// Protótipos das funções
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
int setupShader();
void initializeColors();
void updateTriangleBuffer();
void updatePendingBuffer();

// Dimensões da janela
const GLuint WIDTH = 800, HEIGHT = 600;

// Vetores para os triângulos concluídos e vértices em construção
vector<Triangle> triangles;
vector<vec3> pendingVertices;
vector<vec3> colorPalette;
int currentColorIndex = 0;

GLuint triVAO = 0, triVBO = 0;
GLuint pendingVAO = 0, pendingVBO = 0;

// Shaders GLSL embutidos
const GLchar* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 position;
uniform mat4 projection;
void main()
{
    gl_Position = projection * vec4(position, 1.0);
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

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Lista 3 - Triangulos via Mouse - Lorrana Lasch", nullptr, nullptr);
    if (!window)
    {
        cerr << "Falha ao criar janela GLFW" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cerr << "Falha ao inicializar GLAD" << endl;
        return -1;
    }

    initializeColors();

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    cout << "==========================================================================" << endl;
    cout << "Lista 3: Criando Triangulos a partir do Clique do Mouse - Lorrana Lasch" << endl;
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL Version: " << version << endl;
    cout << "--------------------------------------------------------------------------" << endl;
    cout << "Instrucoes:" << endl;
    cout << " - Clique com o BOTAO ESQUERDO na tela para marcar vertices." << endl;
    cout << " - Clique 1: 1 vertice adicionado (ponto branco destacado)" << endl;
    cout << " - Clique 2: 2 vertice adicionado (linha conectando)" << endl;
    cout << " - Clique 3: fecha o triangulo preenchido e sorteia uma cor nova!" << endl;
    cout << " - [C] ou [R]: Limpar todos os triangulos da tela" << endl;
    cout << " - [ESC]: Sair" << endl;
    cout << "==========================================================================" << endl;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    GLuint shaderProgram = setupShader();
    GLint colorLoc = glGetUniformLocation(shaderProgram, "inputColor");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    assert(colorLoc > -1 && projLoc > -1);

    glGenVertexArrays(1, &triVAO);
    glGenBuffers(1, &triVBO);

    glGenVertexArrays(1, &pendingVAO);
    glGenBuffers(1, &pendingVBO);

    glUseProgram(shaderProgram);

    // Mapeamento ortográfico 1 unidade = 1 pixel
    mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(projection));

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.12f, 0.12f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Renderiza triângulos finalizados
        if (!triangles.empty())
        {
            glBindVertexArray(triVAO);
            for (size_t i = 0; i < triangles.size(); ++i)
            {
                glUniform4f(colorLoc, triangles[i].color.r, triangles[i].color.g, triangles[i].color.b, 1.0f);
                glDrawArrays(GL_TRIANGLES, (GLint)(i * 3), 3);

                glLineWidth(1.5f);
                glUniform4f(colorLoc, 0.05f, 0.05f, 0.05f, 0.8f);
                glDrawArrays(GL_LINE_LOOP, (GLint)(i * 3), 3);
            }
        }

        // Renderiza vértices pendentes (1 ou 2 cliques)
        if (!pendingVertices.empty())
        {
            glBindVertexArray(pendingVAO);

            glPointSize(10.0f);
            glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
            glDrawArrays(GL_POINTS, 0, (GLsizei)pendingVertices.size());

            if (pendingVertices.size() == 2)
            {
                glLineWidth(2.0f);
                glUniform4f(colorLoc, 0.95f, 0.85f, 0.2f, 1.0f);
                glDrawArrays(GL_LINES, 0, 2);
            }
        }

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &triVAO);
    glDeleteBuffers(1, &triVBO);
    glDeleteVertexArrays(1, &pendingVAO);
    glDeleteBuffers(1, &pendingVBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Mapeia para o sistema de coordenadas do OpenGL com Y crescendo para cima
        float x = (float)xpos;
        float y = 600.0f - (float)ypos;

        pendingVertices.push_back(vec3(x, y, 0.0f));
        cout << "[Clique " << pendingVertices.size() << "/3] Vertice registrado em (" << (int)x << ", " << (int)y << ")" << endl;

        if (pendingVertices.size() == 3)
        {
            Triangle tri;
            tri.p1 = pendingVertices[0];
            tri.p2 = pendingVertices[1];
            tri.p3 = pendingVertices[2];
            tri.color = colorPalette[currentColorIndex];
            currentColorIndex = (currentColorIndex + 1) % colorPalette.size();

            triangles.push_back(tri);
            pendingVertices.clear();

            cout << ">>> Triangulo #" << triangles.size() << " formado com sucesso!" << endl;

            updateTriangleBuffer();
            updatePendingBuffer();
        }
        else
        {
            updatePendingBuffer();
        }
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_C || key == GLFW_KEY_R)
        {
            triangles.clear();
            pendingVertices.clear();
            updateTriangleBuffer();
            updatePendingBuffer();
            cout << "[Reset] Tela limpa." << endl;
        }
    }
}

void initializeColors()
{
    colorPalette.push_back(vec3(0.92f, 0.26f, 0.21f)); // Vermelho
    colorPalette.push_back(vec3(0.18f, 0.80f, 0.44f)); // Verde
    colorPalette.push_back(vec3(0.16f, 0.50f, 0.93f)); // Azul
    colorPalette.push_back(vec3(0.98f, 0.75f, 0.18f)); // Amarelo
    colorPalette.push_back(vec3(0.61f, 0.35f, 0.71f)); // Roxo
    colorPalette.push_back(vec3(0.00f, 0.81f, 0.82f)); // Turquesa
    colorPalette.push_back(vec3(0.95f, 0.45f, 0.20f)); // Laranja
    colorPalette.push_back(vec3(0.91f, 0.12f, 0.39f)); // Rosa
    colorPalette.push_back(vec3(0.55f, 0.76f, 0.29f)); // Verde Lima
}

void updateTriangleBuffer()
{
    vector<GLfloat> rawVertices;
    for (const auto& tri : triangles)
    {
        rawVertices.push_back(tri.p1.x); rawVertices.push_back(tri.p1.y); rawVertices.push_back(tri.p1.z);
        rawVertices.push_back(tri.p2.x); rawVertices.push_back(tri.p2.y); rawVertices.push_back(tri.p2.z);
        rawVertices.push_back(tri.p3.x); rawVertices.push_back(tri.p3.y); rawVertices.push_back(tri.p3.z);
    }

    glBindVertexArray(triVAO);
    glBindBuffer(GL_ARRAY_BUFFER, triVBO);
    glBufferData(GL_ARRAY_BUFFER, rawVertices.size() * sizeof(GLfloat), rawVertices.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void updatePendingBuffer()
{
    vector<GLfloat> rawVertices;
    for (const auto& pt : pendingVertices)
    {
        rawVertices.push_back(pt.x); rawVertices.push_back(pt.y); rawVertices.push_back(pt.z);
    }

    glBindVertexArray(pendingVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pendingVBO);
    glBufferData(GL_ARRAY_BUFFER, rawVertices.size() * sizeof(GLfloat), rawVertices.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
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
