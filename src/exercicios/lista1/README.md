# Lista 1 - Primitivas Gráficas, Shaders & Buffers

Este diretório contém as resoluções da Lista 1 da disciplina de Processamento Gráfico. Abaixo estão os detalhes de cada exercício desenvolvido, juntamente com as respostas teóricas.

---

## Exercício 1: Desenho de dois triângulos
**Objetivo:** Praticar a utilização de diferentes primitivas gráficas do OpenGL.

**Descrição:** 
A partir de vértices definidos manualmente no código, o programa desenha duas formas triangulares (semelhante a uma "gravata borboleta"). O código foi preparado para demonstrar a renderização do OpenGL de 4 maneiras diferentes através das chamadas de desenho:
- Polígono preenchido (`GL_TRIANGLES`)
- Apenas contorno (`GL_LINE_LOOP`)
- Apenas pontos (`GL_POINTS`)
- As três formas combinadas (todas as primitivas executadas em sequência)

---

## Exercício 2: Geometria Paramétrica (Desafios)
**Objetivo:** Utilizar equações paramétricas envolvendo trigonometria (seno e cosseno) para gerar as posições dos vértices e formar objetos circulares e derivados.

**Descrição:** 
Neste exercício, foram implementados os dois Desafios finais propostos. As duas formas são renderizadas lado a lado na mesma janela utilizando translações diretas na matemática de construção do VAO:
- **Estrela (Desafio 1):** Utilizando a primitiva `GL_TRIANGLE_FAN`, a lógica intercala o tamanho do raio a cada vértice processado pelo loop, formando as pontas da estrela.
- **Espiral (Desafio 2):** Utilizando a primitiva `GL_LINE_STRIP`, o laço de repetição aumenta continuamente não apenas o ângulo (θ), mas também o raio (r) de forma linear e gradativa.

---

## Exercício 3: Triângulo Interpolado RGB
**Objetivo:** Entender e configurar buffers (VAO e VBO) intercalados (interleaved data), passando tanto dados de posição quanto de cores diretamente pelo Vertex Shader.

**Respostas Teóricas:**

**a) Descreva uma possível configuração dos buffers (VBO, VAO) para representá-lo:**
> Foi utilizado um único VBO (Vertex Buffer Object) que armazena em um array sequencial e intercalado a posição e a cor de cada vértice: `[x, y, z, r, g, b, x, y, z, r, g, b...]`. 
> Para o OpenGL saber ler isso, o VAO é configurado com dois ponteiros de atributos (`glVertexAttribPointer`):
> 1. Ponteiro da Posição: Lê 3 floats, com offset começando no byte 0, tendo um passo (stride) total de 6 floats para encontrar a próxima posição.
> 2. Ponteiro da Cor: Lê 3 floats, com offset começando no tamanho equivalente aos 3 primeiros floats passados, tendo também o passo de 6 floats.

**b) Como estes atributos seriam identificados no vertex shader?**
> Eles são recebidos e identificados usando a diretiva `layout(location = X)`, correspondendo aos identificadores que habilitamos em C++ com `glEnableVertexAttribArray(X)`. No nosso código ficam assim:
> ```glsl
> layout (location = 0) in vec3 position; // recebe o (x,y,z)
> layout (location = 1) in vec3 color;    // recebe o (r,g,b)
> ```

**Implementação:** 
O código recria exatamente as posições pedidas no slide P1 (Vermelho), P2 (Verde) e P3 (Azul). A mágica da pintura acontece sozinha graças à forma com a qual o OpenGL rasteriza e interpola perfeitamente a variação das cores entre os pontos antes de enviá-las para o Fragment Shader.

---

## Exercício 4: Desenho Livre
**Objetivo:** Praticar múltiplas chamadas de desenho, gerenciamento de cores via Variáveis `Uniform` e utilização criativa das primitivas gráficas.

**Descrição:** 
O desenho livre elaborado no código é um **Chapéu de Bruxo**. Foram criados agrupamentos de vértices específicos para formar o objeto através de sobreposição e união de primitivas:
1. **Aba:** Retângulo achatado na base desenhado com `GL_TRIANGLES`.
2. **Cone do Chapéu:** Um triângulo alongado cuja ponta superior foi deslocada propositalmente para o eixo X positivo para simular o caimento de um chapéu torto.
3. **Faixa e Fivela:** Foram desenhados usando `GL_TRIANGLES` agrupando pequenos quadrados (quads), e pintados trocando a variável de estado Uniform para roxo e dourado, respectivamente, antes da renderização de cada etapa.
