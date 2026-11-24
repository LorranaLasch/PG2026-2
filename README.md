# Processamento Gráfico 2026/2 - Unisinos

Repositório pessoal para o desenvolvimento e entrega das atividades práticas da disciplina de **Processamento Gráfico: Fundamentos**, do curso de Ciência da Computação da Unisinos.

**Aluna:** Lorrana Lasch (Trabalho Individual)

---

## Estrutura do Repositório

```plaintext
📁 PG2026-2-Lorrana_Lasch/
├── 📁 include/               # Cabeçalhos de bibliotecas (GLAD, GLFW, GLM, stb_image)
├── 📁 Common/                # Códigos utilitários compartilhados (glad.c, Shader, etc.)
├── 📁 src/                   # Código-fonte
│   ├── 📁 exemplos/          # Exemplos de aula (HelloTriangle, HelloOrtho)
│   ├── 📁 exercicios/        # Listas práticas
│   │   ├── 📁 lista1/        # Lista 1 - Primitivas, Shaders e Buffers
│   │   ├── 📁 lista2/        # Lista 2 - Câmera 2D, Ortho e Viewports
│   │   └── 📁 lista3/        # Lista 3 - Mouse e Transformações Geométricas
│   ├── 📁 TrabalhoPraticoGA/ # Trabalho Prático do Grau A (Jogo da Katniss - C++ / OpenGL)
│   └── 📁 TrabalhoPraticoGB/ # Trabalho Prático do Grau B (Stories FX - Python / OpenCV)
├── 📄 .gitignore             # Arquivos ignorados pelo Git
├── 📄 CMakeLists.txt         # Script de compilação via CMake (Grau A e listas em C++)
└── 📄 README.md              # Apresentação do repositório
```

---

## Trabalhos Práticos

### Grau A: Jogo da Katniss (C++ & OpenGL)
* **Diretório:** `src/TrabalhoPraticoGA`
* Jogo 2D interativo desenvolvido em C++ com OpenGL (GLFW, GLAD e GLM), explorando o pipeline gráfico, shaders e transformações geométricas.
* **Compilação via CMake:**
  ```bash
  cmake -B build
  cmake --build build
  ```

---

### Grau B: Stories FX - Efeitos em Imagens e Vídeo (Python & OpenCV)
* **Diretório:** `src/TrabalhoPraticoGB`
* Aplicativo protótipo de edição de imagens e fluxo de vídeo inspirado nos *stories* do Instagram, desenvolvido em **Python 3** com **OpenCV** e interface gráfica.
* Recursos implementados:
  * Mais de 10 filtros (suavização, realce, operacões pontuais, cor, máscaras e efeitos criativos)
  * Aplicação em canais individuais (R, G, B) ou na imagem completa
  * Operações aritméticas entre imagens (adição, subtração ponderada, blending, multiplicação e diferença)
  * Sistema de stickers com transparência (canal alfa) posicionados via clique do mouse
  * Modo Foto e Modo Vídeo (webcam ao vivo)
  * Recursos extras: Interface gráfica completa (Tkinter), detecção facial via Haar Cascade, stickers animados por spritesheet e encadeamento de múltiplos filtros.
* Para instruções detalhadas de como instalar e executar, consulte o [README do Grau B](src/TrabalhoPraticoGB/README.md).
