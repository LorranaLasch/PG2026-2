# Lista 3 - Processamento Gráfico: Fundamentos

**Aluna:** Lorrana Lasch  
**Disciplina:** Processamento Gráfico: Fundamentos — Unisinos

Este diretório contém a resolução da **Lista 3**, cobrindo o enunciado oficial da disciplina de 2026/2 (**"Criando Triângulos a partir do Clique do Mouse"**) e o conjunto de práticas de **Transformações Geométricas 2D** (matrizes `model`, translação, escala, rotação e controles por teclado e física básica).

---

## 1. Entrega Oficial: Criando Triângulos a partir do Clique do Mouse
*(Conforme especificação do PDF da disciplina — `Lista 3.pdf`)*

**Diretório:** `exercicio_mouse_triangulos/`  
**Baseado em:** Código oficial da professora Rossana Baptista Queiroz (`6.cpp`)

### Requisitos Implementados:
1. **1 Vértice por Clique:** Cada clique com o botão esquerdo do mouse captura a posição atual do cursor e registra um vértice pontual na coordenada $(x, y)$.
2. **Triângulo a cada 3 Vértices:** Ao registrar o 3º vértice consecutivo, o programa fecha e renderiza um novo triângulo preenchido (`GL_TRIANGLES`) com contorno destacado.
3. **Cor Nova para cada Triângulo:** Cada triângulo recebe automaticamente uma nova tonalidade vibrante a partir de uma paleta de cores (`colorPalette`).
4. **Mapeamento de Coordenadas:** Utiliza projeção paralela ortográfica com dimensões $800 \times 600$ unidades, convertendo a origem vertical da GLFW (topo) para o padrão cartesiano do OpenGL:
   $$x = x_{\text{cursor}}, \quad y = 600 - y_{\text{cursor}}$$
5. **Comandos:**
   - **Botão Esquerdo do Mouse:** Marca vértices e constrói triângulos.
   - **Teclas `[C]` ou `[R]`:** Limpa a tela.
   - **Tecla `[ESC]`:** Fecha a aplicação.

---

## 2. Práticas de Transformações Geométricas em Objetos
*(Baseadas no exemplo oficial `HelloTransforms.cpp` da professora)*

### Exercício 1: Três Instâncias com Transformações Diferentes
**Diretório:** `exercicio_transformacoes/`  
Desenha uma mesma geometria base 3 vezes na tela aplicando transformações distintas em cada chamada (mesmo VAO, 3 matrizes `model`, translações, escalas e rotações independentes).

---

### Exercício 2: Controle Interativo de Posição via Teclado (WASD / Setas)
**Diretório:** `exercicio_movimento_teclado/`  
Altera dinamicamente a matriz `model` do objeto nas 4 direções cardeais usando teclas do teclado, com movimentação fluida e verificação contínua dos limites das bordas da janela:

* **Comandos:**
  - `[W]` / `[Seta Cima]`: Mover para cima
  - `[S]` / `[Seta Baixo]`: Mover para baixo
  - `[A]` / `[Seta Esquerda]`: Mover para a esquerda
  - `[D]` / `[Seta Direita]`: Mover para a direita

---

### Exercício 3 (Desafio-Extra): Screensaver "Pong" com Reflexão de Vetor
**Diretório:** `exercicio_screensaver_pong/`  
Implementação autônoma de um screensaver estilo clássico: o objeto move-se em linha reta e, ao colidir com qualquer uma das 4 extremidades da tela, inverte a componente do vetor de velocidade correspondente e altera sua cor.
