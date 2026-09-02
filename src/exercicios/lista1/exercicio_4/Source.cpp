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
int setupGeometry();

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 600, HEIGHT = 600;

int main()
{
	// Inicialização da GLFW
	glfwInit();

	// Criação da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Exercicio 4 - Lorrana Lasch", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);

	// GLAD: carrega todos os ponteiros das funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;

	}

	// Obtendo as informações de versão
	const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Compilando e buildando o programa de shader
	Shader shader("../commonfiles/shaders/vertex.vs","../commonfiles/shaders/fragment.fs");

	// Gerando um buffer simples, com a geometria de um triângulo
	GLuint VAO = setupGeometry();
	
	// Enviando a cor desejada (vec4) para o fragment shader
	// Utilizamos a variáveis do tipo uniform em GLSL para armazenar esse tipo de info
	// que não estão nos buffers
	GLint colorLoc = glGetUniformLocation(shader.ID, "inputColor");
	assert(colorLoc > -1);
	
	glUseProgram(shader.ID);

	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(0.8f, 0.8f, 0.8f, 1.0f); // cor de fundo
		glClear(GL_COLOR_BUFFER_BIT);

		glLineWidth(3);
		glPointSize(10);

		glBindVertexArray(VAO);

		// 1. Aba do chapéu (Preto/Cinza Escuro)
		glUniform4f(colorLoc, 0.15f, 0.15f, 0.15f, 1.0f);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// 2. Cone do chapéu (Preto/Cinza Escuro)
		glDrawArrays(GL_TRIANGLES, 6, 3);

		// 3. Faixa do chapéu (Roxo)
		glUniform4f(colorLoc, 0.5f, 0.0f, 0.5f, 1.0f);
		glDrawArrays(GL_TRIANGLES, 9, 6);

		// 4. Fivela (Amarelo/Dourado)
		glUniform4f(colorLoc, 0.9f, 0.8f, 0.1f, 1.0f);
		glDrawArrays(GL_TRIANGLES, 15, 6);

		glBindVertexArray(0);

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	// Pede pra OpenGL desalocar os buffers
	glDeleteVertexArrays(1, &VAO);
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

int setupGeometry()
{
	GLfloat vertices[] = {
		// 1. Aba do chapéu (Retângulo na base) - 6 vértices
		-0.7f, -0.3f, 0.0f,
		 0.7f, -0.3f, 0.0f,
		 0.7f, -0.2f, 0.0f,
		-0.7f, -0.3f, 0.0f,
		 0.7f, -0.2f, 0.0f,
		-0.7f, -0.2f, 0.0f,

		// 2. Cone do chapéu (Triângulo inclinado) - 3 vértices
		-0.3f, -0.2f, 0.0f,  // Base esquerda
		 0.3f, -0.2f, 0.0f,  // Base direita
		 0.4f,  0.6f, 0.0f,  // Ponta do chapéu inclinada para a direita

		// 3. Faixa do chapéu (Quad logo acima da aba) - 6 vértices
		-0.3f,   -0.2f,  0.0f,
		 0.3f,   -0.2f,  0.0f,
		 0.32f,  -0.05f, 0.0f,
		-0.3f,   -0.2f,  0.0f,
		 0.32f,  -0.05f, 0.0f,
		-0.17f,  -0.05f, 0.0f,

		// 4. Fivela dourada (Quad no meio da faixa) - 6 vértices
		-0.08f, -0.18f, 0.0f,
		 0.08f, -0.18f, 0.0f,
		 0.08f, -0.07f, 0.0f,
		-0.08f, -0.18f, 0.0f,
		 0.08f, -0.07f, 0.0f,
		-0.08f, -0.07f, 0.0f
	};

	GLuint VBO, VAO;

	//Geração do identificador do VBO
	glGenBuffers(1, &VBO);
	//Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);
	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);
	//Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando: 
	// Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
	// Numero de valores que o atributo tem (por ex, 3 coordenadas xyz) 
	// Tipo do dado
	// Se está normalizado (entre zero e um)
	// Tamanho em bytes 
	// Deslocamento a partir do byte zero 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice 
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0); 

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0); 

	return VAO;
}