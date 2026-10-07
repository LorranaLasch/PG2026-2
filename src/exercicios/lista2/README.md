# Lista 2 - Sistemas de Coordenadas, Câmera 2D e Viewports

**Aluna:** Lorrana Lasch  
**Disciplina:** Processamento Gráfico: Fundamentos — Unisinos

Este diretório contém a resolução completa da **Lista 2**, abordando a Matriz de Projeção Ortográfica (`glm::ortho`), mapeamento de coordenadas do mundo para a tela, manipulação de múltiplos viewports (`glViewport`) e interação em tempo real com eventos de mouse da GLFW.

---

## Exercícios 1 a 5: Câmera Ortográfica & Viewports

O executável correspondente está em `exercicio_1_a_5/` e permite alternar interativamente entre as configurações solicitadas usando as teclas **`[1]`** a **`[5]`**.

### 1. Janela do mundo com limites: $x_{\min}=-10, x_{\max}=10, y_{\min}=-10, y_{\max}=10$
* **Implementação:** `projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -1.0f, 1.0f);`

---

### 2. Janela do mundo com limites: $x_{\min}=0, x_{\max}=800, y_{\min}=600, y_{\max}=0$
* **Implementação:** `projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);`

---

### 3. O que acontece quando posicionamos os objetos? Por que é útil essa configuração?
> **Resposta:**
> Ao definir a matriz ortográfica com os limites $X \in [0, 800]$ e $Y \in [600, 0]$, estabelecemos uma correspondência direta de **1 unidade do mundo para 1 pixel físico da tela**.
> 
> Além disso, ao inverter o eixo vertical ($y_{\min}=600$ e $y_{\max}=0$), alinhamos o sistema Cartesiano ao padrão de coordenadas de telas digitais e janelas do sistema operacional (onde a origem $(0,0)$ situa-se no **canto superior esquerdo**, o eixo X cresce para a direita e o eixo Y cresce para baixo).
> 
> **Utilidade prática:**
> - Elimina a necessidade de converter manualmente porcentagens normalizadas (NDC entre -1 e 1) ao posicionar elementos de UI (interfaces de usuário), botões, fontes e sprites 2D.
> - Facilita o tratamento de eventos de mouse, pois as coordenadas capturadas pelo cursor (`glfwGetCursorPos`) coincidem diretamente com as coordenadas dos objetos no mundo.

---

### 4. Modificação do Viewport para um único quadrante
* **Implementação:** `glViewport(width / 2, height / 2, width / 2, height / 2);`

---

### 5. Desenho da mesma cena nos 4 quadrantes
* **Implementação:** Laço iterativo particionando a janela em 4 sub-regiões de dimensões `(width/2, height/2)`:
  - Quadrante Inferior Esquerdo: `(0, 0)`
  - Quadrante Superior Esquerdo: `(0, height/2)`
  - Quadrante Inferior Direito: `(width/2, 0)`
  - Quadrante Superior Direito: `(width/2, height/2)`

---

## Exercício 6: Criação de Triângulos com Clique do Mouse

O executável correspondente está em `exercicio_6/` e foi construído com base nos exemplos da professora (`6.cpp`).

### Requisitos Atendidos:
1. **1 Vértice por Clique:** Cada clique com o botão esquerdo do mouse (`GLFW_MOUSE_BUTTON_LEFT`) registra um novo vértice nas coordenadas exatas do cursor.
2. **Triângulo a cada 3 Vértices:** Ao completar 3 cliques consecutivos, os vértices são conectados formando um novo triângulo preenchido (`GL_TRIANGLES`).
3. **Cor Nova para cada Triângulo:** Cada triângulo recebe automaticamente uma nova cor distinta a partir de uma paleta vibrante (`colorPalette`).
4. **Feedback Visual Imediato:** Enquanto o triângulo não é concluído (1º ou 2º clique), o programa exibe pontos brancos destacados e a linha guia de conexão, permitindo ao usuário visualizar a construção em tempo real.
5. **Mapeamento de Coordenadas:** As coordenadas do cursor da GLFW são convertidas para o sistema do OpenGL considerando a altura da janela:
   $$x = x_{\text{cursor}}, \quad y = 600 - y_{\text{cursor}}$$

### Controles:
- **Botão Esquerdo do Mouse:** Cria vértices e forma triângulos.
- **Teclas `[C]` ou `[R]`:** Limpa a tela (remove todos os triângulos).
- **Tecla `[ESC]`:** Fecha o programa.
