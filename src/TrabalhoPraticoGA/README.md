# Jogos Vorazes: A Fuga de Katniss na Arena (Trabalho Prático - Grau A)

**Aluna:** Lorrana Lasch  
**Disciplina:** Processamento Gráfico: Fundamentos  
**Curso:** Ciência da Computação — Unisinos  

---

## Sobre o Jogo

*Jogos Vorazes: A Fuga de Katniss na Arena* é um jogo 2D de plataforma/sobrevivência estilo **Endless Runner** desenvolvido em **C++** com **OpenGL 3.3 Moderno** (Pipeline Programável com Shaders).

No controle de **Katniss Everdeen**, o jogador precisa correr pelas florestas densas da Arena dos Jogos Vorazes, pulando sobre obstáculos mortais da Capital (tocos pontiagudos), abatendo vespas venenosas (*Tracker Jackers*) com seu arco e flecha e resgatando os icônicos **Broches do Tordo** (*Mockingjay Pins*) para aumentar sua pontuação de sobrevivência.

## Controles

| Tecla | Ação |
| :--- | :--- |
| **`[ESPAÇO]`** / **`[W]`** / **`[SETA CIMA]`** | Pular obstáculos no chão |
| **`[F]`** / **`[ENTER]`** | Disparar flecha com o arco para abater vespas no ar |
| **`[R]`** | Reiniciar a partida após o Game Over |
| **`[ESC]`** | Fechar a aplicação |

---

## Demonstração em Vídeo

* **Arquivo de vídeo (local):** [`video/katniss_gameplay.mp4`](video/katniss_gameplay.mp4) (incluso no repositório com o gameplay gravado, saltos, combate com arco e tela de pontuação).

---

## Como Compilar e Executar

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
