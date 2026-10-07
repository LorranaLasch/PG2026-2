# Lista 1 - Primitivas Gráficas, Shaders & Buffers

**Aluna:** Lorrana Lasch  
**Disciplina:** Processamento Gráfico: Fundamentos — Unisinos

Este diretório contém a implementação completa da **Lista de Exercícios 1**, abordando primitivas gráficas do OpenGL, shaders em GLSL, buffers (VBO/VAO), geometria paramétrica e desenho livre.

---

## Exercício 1: Desenho de dois triângulos
**Objetivo:** Praticar a utilização de diferentes primitivas gráficas do OpenGL (preenchido, contorno, pontos e as 3 formas juntas).

**Descrição e Interatividade:**
A geometria define dois triângulos opostos formando uma figura semelhante a uma gravata borboleta. Para que todas as opções solicitadas possam ser avaliadas dinamicamente na mesma execução, implementou-se controle interativo pelo teclado:
- **`[1]`** - Apenas polígono preenchido (`GL_TRIANGLES`)
- **`[2]`** - Apenas contorno (`GL_LINE_LOOP`)
- **`[3]`** - Apenas pontos (`GL_POINTS`)
- **`[4]`** - As 3 formas de desenho juntas (`GL_TRIANGLES` + `GL_LINE_LOOP` + `GL_POINTS`) **[Padrão]**

---

## Exercício 2: Geometria Paramétrica & Desafios
**Objetivo:** Gerar formas circulares e derivadas a partir da equação paramétrica do círculo:
$$x = r \cdot \cos(\theta), \quad y = r \cdot \sin(\theta)$$

**Formas Implementadas e Controles pelo Teclado:**
O programa permite visualizar todas as formas solicitadas alternando pelas seguintes teclas:
- **`[C]` ou `[1]`** - **Círculo base:** Gerado com 64 fatias angulares em `GL_TRIANGLE_FAN`.
- **`[8]` ou `[2]`** - **a) Octógono:** Polígono regular de 8 vértices no contorno.
- **`[5]` ou `[3]`** - **b) Pentágono:** Polígono regular de 5 vértices no contorno.
- **`[P]` ou `[4]`** - **c) Pac-man:** Arco circular variando de $30^\circ$ a $330^\circ$, simulando a boca aberta.
- **`[F]` ou `[5]`** - **d) Fatia de Pizza:** Setor circular de $60^\circ$ conectado ao vértice central.
- **`[E]` ou `[6]`** - **e) DESAFIO 1 (Estrela):** 10 vértices de contorno com raios alternados (maior e menor) usando `GL_TRIANGLE_FAN`.
- **`[S]` ou `[7]`** - **f) DESAFIO 2 (Espiral):** Espiral de Arquimedes com 1000 pontos em `GL_LINE_STRIP`, incrementando o ângulo ($\theta$) e o raio ($r$) continuamente.
- **`[T]` ou `[8]`** - **Modo Comparativo:** Renderiza a Estrela à esquerda e a Espiral à direita simultaneamente.

---

## Exercício 3: Triângulo Interpolado RGB
**Objetivo:** Compreender a configuração de buffers intercalados (*interleaved buffers*) e a passagem de atributos por vértice para interpolação na GPU.

### Respostas Teóricas:

#### a) Descreva uma possível configuração dos buffers (VBO, VAO) para representá-lo:
> Utilizamos um único VBO (Vertex Buffer Object) armazenando as coordenadas de posição $(x, y, z)$ e os canais de cor $(r, g, b)$ de forma sequencial e intercalada no mesmo array de `GLfloat`:
> ```
> [x, y, z, r, g, b,  x, y, z, r, g, b,  x, y, z, r, g, b]
> ```
> Cada vértice consome 6 floats (24 bytes).  
> No VAO (Vertex Array Object), configuramos dois ponteiros de atributos através da função `glVertexAttribPointer`:
> 1. **Atributo 0 (Posição):** tamanho 3 floats, tipo `GL_FLOAT`, stride de $6 \times \text{sizeof(GLfloat)}$, offset no byte 0.
> 2. **Atributo 1 (Cor):** tamanho 3 floats, tipo `GL_FLOAT`, stride de $6 \times \text{sizeof(GLfloat)}$, offset de $3 \times \text{sizeof(GLfloat)}$ (12 bytes).

#### b) Como estes atributos seriam identificados no vertex shader?
> No Vertex Shader (GLSL), declaramos os atributos com as localizações correspondentes:
> ```glsl
> #version 330 core
> layout (location = 0) in vec3 position; // Recebe (x, y, z)
> layout (location = 1) in vec3 vcolor;   // Recebe (r, g, b)
> 
> out vec4 interpolatedColor; // Exporta a cor para a fase de rasterização
> 
> void main() {
>     gl_Position = vec4(position, 1.0);
>     interpolatedColor = vec4(vcolor, 1.0);
> }
> ```
> O hardware de rasterização da GPU calcula os gradientes barycêntricos e entrega a cor suavemente interpolada para o Fragment Shader através da variável de entrada `in vec4 interpolatedColor;`.

---

## Exercício 4: Desenho Livre (Chapéu de Bruxo)
**Objetivo:** Criar uma composição gráfica original utilizando primitivas do OpenGL, múltiplas chamadas de desenho (`draw calls`) e controle de cores via variáveis `uniform`.

**Composição do Chapéu:**
1. **Aba:** Retângulo largo na base (dois triângulos formando um quad) com coloração cinza grafite escuro (`GL_TRIANGLES`, 6 vértices).
2. **Cone:** Triângulo alto com o vértice superior intencionalmente inclinado para a direita, conferindo o aspecto clássico de chapéu de bruxo pontudo dobrado (`GL_TRIANGLES`, 3 vértices).
3. **Faixa:** Faixa decorativa ajustada logo acima da aba, renderizada em roxo místico (`GL_TRIANGLES`, 6 vértices).
4. **Fivela:** Quadrado centralizado sobre a faixa, colorido em amarelo dourado via `uniform inputColor` (`GL_TRIANGLES`, 6 vértices).
