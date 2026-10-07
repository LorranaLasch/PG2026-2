# 💻 Processamento Gráfico 2026/2 - Unisinos

Repositório pessoal para o desenvolvimento e entrega das atividades práticas da disciplina de **Processamento Gráfico: Fundamentos**, do curso de Ciência da Computação da Unisinos.

**Aluna:** Lorrana Lasch

---

## 📂 Estrutura do Repositório

```plaintext
📁 PG2026-2-Lorrana_Lasch/
├── 📁 include/           # Cabeçalhos de bibliotecas (GLAD, GLFW, GLM, stb_image)
├── 📁 Common/            # Códigos utilitários compartilhados (glad.c, Shader, etc.)
├── 📁 src/               # Código-fonte
│   ├── 📁 exemplos/      # Exemplos de aula (HelloTriangle, HelloOrtho)
│   ├── 📁 exercicios/    # Listas práticas
│   │   ├── 📁 lista1/    # Lista 1 - Primitivas, Shaders e Buffers
│   │   ├── 📁 lista2/    # Lista 2 - Câmera 2D, Ortho e Viewports
│   │   └── 📁 lista3/    # Lista 3 - Mouse e Transformações Geométricas
│   └── 📁 TrabalhoPraticoGA/ # Trabalho Prático do Grau A (Jogo da Katniss)
├── 📄 .gitignore         # Arquivos ignorados pelo Git
├── 📄 CMakeLists.txt     # Script de compilação via CMake
└── 📄 README.md          # Apresentação do repositório
```

---

## ⚙️ Como compilar e executar

Este projeto utiliza **CMake** para configuração e compilação multiplataforma:

1. Abra a pasta do projeto no VS Code.
2. Certifique-se de que os arquivos da biblioteca **GLAD** estão presentes em `include/glad/` e `Common/glad.c`.
3. Pelo terminal ou extensão do CMake:
   ```bash
   cmake -B build
   cmake --build build
   ```
