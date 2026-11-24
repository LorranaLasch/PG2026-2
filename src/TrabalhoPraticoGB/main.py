"""
Processamento Gráfico: Fundamentos - Trabalho do Grau B
Stories FX: app de efeitos em imagens e vídeo com OpenCV.

    python main.py                  # abre o app (uma janela só, tema roxo)
    python main.py caminho/foto.jpg # já abre com uma foto

O app fica em app.py. Outras interfaces (opcionais):
    python gui.py      # painel completo em Tkinter, com menus
    python highgui.py  # versão HighGUI do OpenCV + teclado
"""

from app import main

if __name__ == "__main__":
    main()
