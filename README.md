# 💻 Processamento Gráfico 2026/2

Repositório pessoal para o desenvolvimento e entrega das 
atividades da disciplina de Processamento Gráfico: Fundamentos, do curso de 
Ciência da Computação da Unisinos.

## Atividades

* [Lista 1](src/Exercicios/Lista1/)

## ⚙️ Como compilar e executar

Este projeto usa **CMake** para compilação (Windows + VS Code).

1. Siga as instruções detalhadas em [GettingStarted.md](GettingStarted.md)
2. **Importante:** é necessário baixar a GLAD manualmente antes de compilar 
   (veja a seção abaixo)

## Baixando a GLAD manualmente

Acesse o [GLAD Generator](https://glad.dav1d.de/) com a configuração:
- API: OpenGL
- Version: 3.3+
- Profile: Core
- Language: C/C++

Depois, distribua os arquivos gerados:
- `glad.h` → `include/glad/`
- `khrplatform.h` → `include/glad/KHR/`
- `glad.c` → `common/`
