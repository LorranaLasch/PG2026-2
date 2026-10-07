/*
 * Exercicio 6 - Lista 2: Criacao de Triangulos via Clique do Mouse
 * Aluna: Lorrana Lasch
 * Baseado no exemplo oficial da professora (6.cpp / Rossana Baptista Queiroz)
 *
 * Requisitos atendidos:
 * 1. Ao clicar na tela, cria-se apenas 1 vertice por clique.
 * 2. A cada 3 vertices (3 cliques), forma-se um triangulo preenchido completo.
 * 3. A cada novo triangulo criado, seleciona-se uma cor nova da paleta.
 * 4. Mapeamento 1 unidade = 1 pixel usando glm::ortho(0, 800, 0, 600, -1, 1).
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

// Estrutura que armazena os dados de um triângulo criado
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

// Vetores para armazenamento dos triângulos e dos cliques atuais
vector<Triangle> triangles;
vector<vec3> pendingVertices; // Armazena 1 ou 2 cliques antes de fechar o triângulo
vector<vec3> colorPalette;
int currentColorIndex = 0;

// Identificadores de buffers OpenGL
GLuint triVAO = 0, triVBO = 0;
GLuint pendingVAO = 0, pendingVBO = 0;

// Código fonte do Vertex Shader
const GLchar* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 position;
uniform mat4 projection;
void main()
{
    gl_Position = projection * vec4(position, 1.0);
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

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Exercicio 6 - Triangulos via Mouse - Lorrana Lasch", nullptr, nullptr);
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
    cout << "Exercicio 6 (Lista 2): Criacao de Triangulos com o Mouse - Lorrana Lasch" << endl;
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL Version: " << version << endl;
    cout << "--------------------------------------------------------------------------" << endl;
    cout << "Instrucoes:" << endl;
    cout << " - Clique com o BOTAO ESQUERDO na tela para adicionar vertices." << endl;
    cout << " - Clique 1: primeiro vertice (ponto branco na tela)" << endl;
    cout << " - Clique 2: segundo vertice (linha conectando)" << endl;
    cout << " - Clique 3: fecha o triangulo e aplica uma cor nova automaticamente!" << endl;
    cout << " - Tecla [C] ou [R]: Limpar todos os triangulos da tela" << endl;
    cout << " - Tecla [ESC]: Sair" << endl;
    cout << "==========================================================================" << endl;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    GLuint shaderProgram = setupShader();
    GLint colorLoc = glGetUniformLocation(shaderProgram, "inputColor");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    assert(colorLoc > -1 && projLoc > -1);

    // Inicializa buffers vazios
    glGenVertexArrays(1, &triVAO);
    glGenBuffers(1, &triVBO);

    glGenVertexArrays(1, &pendingVAO);
    glGenBuffers(1, &pendingVBO);

    glUseProgram(shaderProgram);

    // Matriz de projeção ortográfica com mapeamento de pixels (0..800 em X e 0..600 em Y)
    mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, value_ptr(projection));

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Fundo escuro suave
        glClearColor(0.12f, 0.12f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 1. Desenha todos os triângulos já concluídos
        if (!triangles.empty())
        {
            glBindVertexArray(triVAO);
            for (size_t i = 0; i < triangles.size(); ++i)
            {
                // Cor única do triângulo
                glUniform4f(colorLoc, triangles[i].color.r, triangles[i].color.g, triangles[i].color.b, 1.0f);
                glDrawArrays(GL_TRIANGLES, (GLint)(i * 3), 3);

                // Contorno fino para destacar as bordas
                glLineWidth(1.5f);
                glUniform4f(colorLoc, 0.05f, 0.05f, 0.05f, 0.8f);
                glDrawArrays(GL_LINE_LOOP, (GLint)(i * 3), 3);
            }
        }

        // 2. Desenha os vértices pendentes (em construção: 1 ou 2 cliques)
        if (!pendingVertices.empty())
        {
            glBindVertexArray(pendingVAO);

            // Desenha os pontos onde o usuário já clicou
            glPointSize(10.0f);
            glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f); // Pontos brancos
            glDrawArrays(GL_POINTS, 0, (GLsizei)pendingVertices.size());

            // Se já tem 2 cliques, desenha a linha conectando eles
            if (pendingVertices.size() == 2)
            {
                glLineWidth(2.0f);
                glUniform4f(colorLoc, 0.9f, 0.9f, 0.2f, 1.0f); // Linha amarela indicativa
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

        // Conversão de coordenadas da janela (origem topo-esquerdo)
        // para o sistema Cartesiano do OpenGL (origem no canto inferior-esquerdo)
        float x = (float)xpos;
        float y = 600.0f - (float)ypos;

        pendingVertices.push_back(vec3(x, y, 0.0f));
        cout << "[Clique do Mouse] Vertice " << pendingVertices.size() << "/3 registrado em (" << (int)x << ", " << (int)y << ")" << endl;

        // Ao completar 3 cliques, cria o triângulo!
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
        // Tecla C ou R para limpar todos os triângulos
        if (key == GLFW_KEY_C || key == GLFW_KEY_R)
        {
            triangles.clear();
            pendingVertices.clear();
            updateTriangleBuffer();
            updatePendingBuffer();
            cout << "[Reset] Todos os triangulos foram removidos da tela." << endl;
        }
    }
}

void initializeColors()
{
    // Paleta diversificada de cores vibrantes (RGB)
    colorPalette.push_back(vec3(0.92f, 0.26f, 0.21f)); // Vermelho
    colorPalette.push_back(vec3(0.18f, 0.80f, 0.44f)); // Verde esmeralda
    colorPalette.push_back(vec3(0.16f, 0.50f, 0.93f)); // Azul royal
    colorPalette.push_back(vec3(0.98f, 0.75f, 0.18f)); // Amarelo
    colorPalette.push_back(vec3(0.61f, 0.35f, 0.71f)); // Roxo
    colorPalette.push_back(vec3(0.00f, 0.81f, 0.82f)); // Turquesa / Ciano
    colorPalette.push_back(vec3(0.95f, 0.45f, 0.20f)); // Laranja
    colorPalette.push_back(vec3(0.91f, 0.12f, 0.39f)); // Rosa choque
    colorPalette.push_back(vec3(0.55f, 0.76f, 0.29f)); // Verde limão
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
