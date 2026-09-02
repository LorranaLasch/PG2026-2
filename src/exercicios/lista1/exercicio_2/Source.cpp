#include <iostream>
#include <string>
#include <assert.h>
#include <cmath>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// Biblioteca manual pra implementação dos shaders
#include "../../../commonfiles/Shader.h"

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupGeometryFormas();
int setupGeometryEspiral();

// Dimensões da janela
const GLuint WIDTH = 800, HEIGHT = 600;
const float pi = 3.14159;

int main()
{
	glfwInit();

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Exercicio 2 e Desafio - Lorrana Lasch", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Ajustando o caminho relativo: como você roda de dentro da pasta "build", voltamos apenas 1 nível ("../")
	Shader shader("../commonfiles/shaders/vertex.vs","../commonfiles/shaders/fragment.fs");

	// Gerando os dois buffers separados
	GLuint VAO_formas = setupGeometryFormas();
	GLuint VAO_espiral = setupGeometryEspiral();
	
	GLint colorLoc = glGetUniformLocation(shader.ID, "inputColor");
	assert(colorLoc > -1);
	
	glUseProgram(shader.ID);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glClearColor(0.8f, 0.8f, 0.8f, 1.0f); // cor de fundo
		glClear(GL_COLOR_BUFFER_BIT);

		glLineWidth(2);
		glPointSize(10);

		// ==========================================
		// DESENHANDO O EXERCÍCIO 2 (Estrela) na esquerda
		// ==========================================
		glBindVertexArray(VAO_formas);
		glUniform4f(colorLoc, 0.4f, 0.4f, 0.4f, 1.0f); // Cor cinza escuro
		glDrawArrays(GL_TRIANGLE_FAN, 0, 12); // 10 pontos + centro + 1 pra fechar

		// ==========================================
		// DESENHANDO O DESAFIO 2 (Espiral) na direita
		// ==========================================
		glBindVertexArray(VAO_espiral);
		glUniform4f(colorLoc, 0.8f, 0.0f, 0.0f, 1.0f); // Cor vermelha
		glDrawArrays(GL_LINE_STRIP, 1, 1000); // 1000 pontos da espiral

		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}
	
	glDeleteVertexArrays(1, &VAO_formas);
	glDeleteVertexArrays(1, &VAO_espiral);
	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

// Geometria do Exercício 2 (Estrela) deslocada para a ESQUERDA
int setupGeometryFormas()
{
	const int nPoints = 10 + 1 + 1; // Exercicio 6e (Estrela de 5 pontas = 10 vértices no contorno)
	GLfloat* vertices = new GLfloat[nPoints * 3];

	float angle = 0.0;
	float deltaAngle = 2 * pi / (float)(nPoints - 2);
	float radius = 0.5;
	
	// Adicionar o centro (deslocado para a esquerda em X = -0.5)
	vertices[0] = -0.5; // x
	vertices[1] = 0.0;  // y
	vertices[2] = 0.0;  // z

	for (int i = 3; i < nPoints * 3; i += 3)
	{
		if(i % 2 == 0)
			radius = 0.2;
		else
			radius = 0.5;

		vertices[i] = (radius * cos(angle)) - 0.5; // Deslocado para a esquerda
		vertices[i+1] = radius * sin(angle);
		vertices[i+2] = 0.0;

		angle += deltaAngle;
	}

	GLuint VBO, VAO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, nPoints * 3 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0); 
	glBindVertexArray(0); 
	delete[] vertices; // Libera a memória

	return VAO;
}

// Geometria do Desafio 2 (Espiral) deslocada para a DIREITA
int setupGeometryEspiral()
{
	const int nPoints = 1000 + 1 + 1;
	GLfloat* vertices = new GLfloat[nPoints * 3];

	float angle = 0.0;
	float deltaAngle = 6 * pi / (float)(nPoints - 2);
	float radius = 0.0;
	float radiusIncrement = 0.4 / (float)(nPoints - 2); // Deixei um pouco menor pra caber melhor
	
	// Adicionar o centro (deslocado para a direita em X = +0.5)
	vertices[0] = 0.5; // x
	vertices[1] = 0.0; // y
	vertices[2] = 0.0; // z

	for (int i = 3; i < nPoints * 3; i += 3)
	{
		vertices[i] = (radius * cos(angle)) + 0.5; // Deslocado para a direita
		vertices[i+1] = radius * sin(angle);
		vertices[i+2] = 0.0;

		angle += deltaAngle;
		radius += radiusIncrement;
	}

	GLuint VBO, VAO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, nPoints * 3 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0); 
	glBindVertexArray(0); 
	delete[] vertices; // Libera a memória

	return VAO;
}