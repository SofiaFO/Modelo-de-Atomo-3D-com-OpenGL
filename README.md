# Modelo de Átomo 3D com OpenGL

Visualização interativa de um modelo atômico 3D desenvolvido em C++ utilizando OpenGL.

## 📋 Descrição

Este projeto simula um átomo com um núcleo central azul e cinco elétrons laranjas orbitando em diferentes planos. A visualização é renderizada em tempo real com iluminação dinâmica e permite interação com a câmera para explorar o modelo de diferentes ângulos.

## ✨ Características

- **Núcleo Atômico**: Esfera azul no centro representando o núcleo
- **Elétrons**: 5 elétrons laranjas em movimento circular contínuo
- **Órbitas Múltiplas**: Elétrons orbitam em diferentes planos:
  - Plano horizontal (XZ)
  - Plano vertical (YZ)
  - Planos diagonais inclinados
- **Iluminação Phong**: Modelo de iluminação com componentes ambiente, difuso e especular
- **Controles Interativos**:
  - 🖱️ **Mouse (arrastar)**: Rotacionar a câmera ao redor do átomo
  - 🖱️ **Scroll**: Zoom in/out (distância da câmera)
- **Animação em Tempo Real**: Elétrons se movem continuamente em suas órbitas

## 🛠️ Tecnologias Utilizadas

- **C++**: Linguagem principal
- **OpenGL 3.3**: API gráfica para renderização 3D
- **GLEW**: Gerenciamento de extensões OpenGL
- **GLFW**: Criação de janela e gerenciamento de input
- **GLM**: Biblioteca matemática para operações com matrizes e vetores

## 📦 Dependências

Para compilar e executar o projeto, você precisa ter instalado:

- OpenGL 3.3+
- GLEW
- GLFW3
- GLM (header-only)
- Compilador C++ com suporte a C++11 ou superior

## 🚀 Compilação

### Windows (MinGW/MSVC)
```bash
g++ atomo.cpp -o atomo -lglfw3 -lglew32 -lopengl32 -lgdi32
```

### Linux
```bash
g++ atomo.cpp -o atomo -lglfw -lGLEW -lGL -lm
```

### macOS
```bash
g++ atomo.cpp -o atomo -lglfw -lGLEW -framework OpenGL
```

## 🎮 Como Usar

1. Compile o projeto usando um dos comandos acima
2. Execute o arquivo gerado:
   - Windows: `atomo.exe`
   - Linux/macOS: `./atomo`
3. Use o mouse para interagir:
   - Clique e arraste para rotacionar a vista
   - Use o scroll do mouse para aproximar ou afastar