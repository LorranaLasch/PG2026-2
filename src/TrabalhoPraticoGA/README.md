# 🏹 Jogos Vorazes: A Fuga de Katniss na Arena (Trabalho Prático - Grau A)

**Aluna:** Lorrana Lasch  
**Disciplina:** Processamento Gráfico: Fundamentos  
**Curso:** Ciência da Computação — Unisinos  

---

## 🎮 Sobre o Jogo

*Jogos Vorazes: A Fuga de Katniss na Arena* é um jogo 2D de plataforma/sobrevivência estilo **Endless Runner** desenvolvido em **C++** com **OpenGL 3.3 Moderno** (Pipeline Programável com Shaders).

No controle de **Katniss Everdeen**, o jogador precisa correr pelas florestas densas da Arena dos Jogos Vorazes, pulando sobre obstáculos mortais da Capital (tocos pontiagudos), abatendo vespas venenosas (*Tracker Jackers*) com seu arco e flecha e resgatando os icônicos **Broches do Tordo** (*Mockingjay Pins*) para aumentar sua pontuação de sobrevivência.

---

## 📋 Requisitos do Grau A Atendidos

| Requisito do Enunciado | Implementação Técnica |
| :--- | :--- |
| **OpenGL 3.3+ & Shaders** | Implementado com Shaders GLSL (`sprite.vs` e `sprite.fs`), compilados e linkados dinamicamente via C++. |
| **Buffers de Geometria** | Classe `Sprite` utiliza **VAO**, **VBO** (posições e coordenadas de textura) e **EBO** (2 triângulos indexados para compor o quad). |
| **Projeção Ortográfica 2D** | Câmera configurada com `glm::ortho(0.0f, 1200.0f, 0.0f, 600.0f, -1.0f, 1.0f)` mapeando o espaço do mundo para a tela da aplicação. |
| **Matrizes de Transformação (`model`)** | Matrizes construídas com translações, rotações e escalas aplicadas por vértice no Vertex Shader para todas as entidades. |
| **Spritesheet Animado** | Ciclo de corrida da Katniss com **6 quadros de animação** alinhados horizontalmente (`katniss_run.png`), com cálculo dinâmico de coordenadas de textura (UV Offset e UV Scale) no shader. |
| **Parallax Scrolling (Camadas de Fundo)** | 3 camadas independentes de cenário rolando infinitamente com velocidades distintas: Céu/Montanhas distantes (0.15x), Pinheiros da floresta (0.50x) e Chão (1.00x). |
| **Controle de Movimentação** | Input via GLFW: pulo com simulação de gravidade física (`Espaço` / `W` / `Seta Cima`) e disparo de flechas (`F` / `Enter`). |
| **Detecção de Colisões (AABB)** | Caixas delimitadoras alinhadas aos eixos (*Axis-Aligned Bounding Box*) com tolerâncias de hitbox para colisões entre Katniss, obstáculos, flechas e itens coletáveis. |

---

## ⌨️ Controles

| Tecla | Ação |
| :--- | :--- |
| **`[ESPAÇO]`** / **`[W]`** / **`[SETA CIMA]`** | Pular obstáculos no chão |
| **`[F]`** / **`[ENTER]`** | Disparar flecha com o arco para abater vespas no ar |
| **`[R]`** | Reiniciar a partida após o Game Over |
| **`[ESC]`** | Fechar a aplicação |

---

## 🎥 Demonstração em Vídeo

* **Arquivo de vídeo (local):** [`video/katniss_gameplay.mp4`](video/katniss_gameplay.mp4) (incluso no repositório com o gameplay gravado, saltos, combate com arco e tela de pontuação).

---

## ⚙️ Como Compilar e Executar

### Opção 1: Executável Pronto (Windows)
Basta navegar até a pasta e executar:
```bash
./katniss_game.exe
```

### Opção 2: Compilação via g++ (Terminal / MinGW)
Dentro da pasta `src/TrabalhoPraticoGA`:
```bash
g++ -std=c++17 -I../../include -I../../Common -I../../include/glad -Iinclude *.cpp ../../Common/glad.c -L../../Common/lib -lglfw3 -lopengl32 -lgdi32 -o katniss_game.exe
```

### Opção 3: Compilação via CMake (Raiz do Repositório)
Na raiz do projeto:
```bash
cmake -B build
cmake --build build --target TrabalhoPraticoGA
```
