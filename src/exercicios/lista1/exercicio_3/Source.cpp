#include <iostream>
#include <string>
#include <assert.h>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// Biblioteca manual pra implementação dos shaders
#include "../../../commonfiles/Shader.h"

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
int setupGeometry();

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 800, HEIGHT = 600;

int main()
{
	glfwInit();

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Exercicio 3 - Lorrana Lasch", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Falha ao inicializar GLAD" << std::endl;
		return -1;
	}

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

    // Ajustado para o caminho correto do shader
	Shader shader("../commonfiles/shaders/vertex.vs", "../commonfiles/shaders/fragment.fs");

	GLuint VAO = setupGeometry();

	glUseProgram(shader.ID);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glClearColor(0.8f, 0.8f, 0.8f, 1.0f); // fundo
		glClear(GL_COLOR_BUFFER_BIT);

		glBindVertexArray(VAO);

        // Desenhando o triângulo com as cores interpoladas
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}
	glDeleteVertexArrays(1, &VAO);
	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

int setupGeometry()
{
	/* 
    RESPOSTAS TEÓRICAS DO EXERCÍCIO 3:
    a) Descreva uma possível configuração dos buffers (VBO, VAO):
       - Usamos um único VBO armazenando a posição (x,y,z) e a cor (r,g,b) intercaladas no mesmo array.
       - Configuramos o VAO com dois ponteiros de atributos (glVertexAttribPointer):
         1. Para a posição: tamanho 3, tipo float, stride de 6*sizeof(float), offset de 0.
         2. Para a cor: tamanho 3, tipo float, stride de 6*sizeof(float), offset de 3*sizeof(float).
       
    b) Como estes atributos seriam identificados no vertex shader?
       - Usando a palavra reservada 'layout' indicando a localização definida no glVertexAttribPointer.
       - Exemplo: 
         layout (location = 0) in vec3 position;
         layout (location = 1) in vec3 color;
    */

	GLfloat vertices[] = {
		// x     y     z      r    g    b
		 0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f, // P1 (Topo, Vermelho)
		-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f, // P2 (Base Esq, Verde)
		 0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f  // P3 (Base Dir, Azul)
	};

	GLuint VBO, VAO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Atributo 0: Posição (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);

	// Atributo 1: Cor (r, g, b)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return VAO;
}
