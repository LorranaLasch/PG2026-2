"""
Funções compartilhadas por todas as interfaces (app.py, gui.py e highgui.py):
caminhos, seleção de canal e o pipeline de filtros/operações.
"""

import os

import cv2 as cv
import numpy as np

from filtros import FILTROS
from operacoes import OPERACOES

PASTA = os.path.dirname(os.path.abspath(__file__))
PASTA_STICKERS = os.path.join(PASTA, "assets", "stickers")
PASTA_IMAGENS = os.path.join(PASTA, "assets", "imagens")
PASTA_SAIDAS = os.path.join(PASTA, "saidas")

MAX_W, MAX_H = 1100, 720          # tamanho máximo da foto exibida
CANAIS = ["RGB", "Cinza", "R", "G", "B"]
INDICE_CANAL = {"B": 0, "G": 1, "R": 2}  # o OpenCV guarda as cores em BGR


def ler_imagem(caminho, flags=cv.IMREAD_COLOR):
    """
    Lê uma imagem com suporte total a caminhos com acentos / Unicode no Windows.
    O cv.imread padrão falha no Windows quando o caminho possui acentos.
    """
    if not caminho or not os.path.isfile(caminho):
        return None
    try:
        with open(caminho, "rb") as f:
            dados = np.frombuffer(f.read(), dtype=np.uint8)
        return cv.imdecode(dados, flags)
    except Exception as erro:
        print(f"Erro ao ler imagem {caminho}: {erro}")
        return None


def salvar_imagem(caminho, img):
    """
    Salva uma imagem com suporte total a caminhos com acentos / Unicode no Windows.
    O cv.imwrite padrão falha no Windows quando o caminho possui acentos.
    """
    if img is None or not caminho:
        return False
    try:
        ext = os.path.splitext(caminho)[1]
        if not ext:
            ext = ".png"
        sucesso, buf = cv.imencode(ext, img)
        if sucesso:
            os.makedirs(os.path.dirname(os.path.abspath(caminho)), exist_ok=True)
            with open(caminho, "wb") as f:
                f.write(buf)
            return True
        return False
    except Exception as erro:
        print(f"Erro ao salvar imagem {caminho}: {erro}")
        return False


def imagem_segunda_padrao(w=960, h=640):
    """Gera a segunda imagem padrão (lilás/rosa com bolhas) caso o arquivo não exista."""
    x = np.linspace(0, 1, w)[None, :]
    y = np.linspace(0, 1, h)[:, None]
    t = (x * 0.6 + y * 0.4)
    b = 200 - 60 * t + 0 * y
    g = 120 - 50 * t + 0 * y
    r = 150 + 90 * t + 0 * y
    img = np.dstack([b, g, r]).clip(0, 255).astype(np.uint8)
    rng = np.random.default_rng(3)
    bolhas = img.copy()
    for _ in range(18):
        cx, cy = int(rng.uniform(0, w)), int(rng.uniform(0, h))
        raio = int(rng.uniform(25, 90))
        cor = (int(rng.uniform(210, 255)), int(rng.uniform(160, 220)), int(rng.uniform(220, 255)))
        cv.circle(bolhas, (cx, cy), raio, cor, -1, cv.LINE_AA)
    return cv.addWeighted(img, 0.75, bolhas, 0.25, 0)


def carregar_segunda_imagem():
    """Carrega a 2ª imagem das operações aritméticas ou gera a padrão."""
    caminho = os.path.join(PASTA_IMAGENS, "segunda.png")
    img = ler_imagem(caminho)
    if img is None:
        img = imagem_segunda_padrao()
        salvar_imagem(caminho, img)
    return img


def ajustar_tamanho(img, max_w=MAX_W, max_h=MAX_H):
    h, w = img.shape[:2]
    f = min(max_w / w, max_h / h, 1.0)
    if f < 1.0:
        img = cv.resize(img, (int(w * f), int(h * f)), interpolation=cv.INTER_AREA)
    return img


def tela_inicial(w=900, h=600, linhas=("Abra uma foto ou ligue a camera",)):
    """Imagem escura (tema roxo) mostrada enquanto nenhuma foto foi aberta."""
    img = np.full((h, w, 3), (38, 16, 26), np.uint8)       # roxo bem escuro (BGR)
    cv.rectangle(img, (40, 40), (w - 40, h - 40), (120, 60, 90), 2, cv.LINE_AA)
    y = h // 2 - 20 * (len(linhas) - 1)
    for i, t in enumerate(linhas):
        esc = 0.9 if i == 0 else 0.6
        (tw, th), _ = cv.getTextSize(t, cv.FONT_HERSHEY_SIMPLEX, esc, 2)
        cv.putText(img, t, ((w - tw) // 2, y + i * 40), cv.FONT_HERSHEY_SIMPLEX, esc,
                   (245, 200, 225) if i == 0 else (200, 160, 185), 2 if i == 0 else 1, cv.LINE_AA)
    return img


def aplicar_canal(img, canal):
    """Seleciona o que será processado: imagem inteira, cinza ou um canal."""
    if canal == "RGB":
        return img.copy()
    if canal == "Cinza":
        return cv.cvtColor(img, cv.COLOR_BGR2GRAY)
    return img[:, :, INDICE_CANAL[canal]].copy()


def aplicar_passo(img, passo, img2, canal):
    """Aplica um passo da pilha (filtro ou operação) sobre img (1 ou 3 canais)."""
    tipo, indice, p1, p2 = passo
    if tipo == "filtro":
        return FILTROS[indice]["func"](img, p1, p2)
    # operação aritmética: a 2a imagem passa pela mesma seleção de canal
    h, w = img.shape[:2]
    b = cv.resize(img2, (w, h), interpolation=cv.INTER_AREA)
    b = aplicar_canal(b, canal)
    if b.ndim != img.ndim:   # ex.: filtro anterior devolveu 1 canal
        b = cv.cvtColor(b, cv.COLOR_BGR2GRAY) if img.ndim == 2 else cv.cvtColor(b, cv.COLOR_GRAY2BGR)
    return OPERACOES[indice]["func"](img, b, p1)


def processar(original, pilha, canal, img2, previa=None):
    """Pipeline completo: seleção de canal -> filtros/operações em sequência."""
    img = aplicar_canal(original, canal)
    for passo in pilha:
        img = aplicar_passo(img, passo, img2, canal)
    if previa is not None:
        img = aplicar_passo(img, previa, img2, canal)
    if img.ndim == 2:   # canal único / cinza: exibe o resultado daquele canal
        img = cv.cvtColor(img, cv.COLOR_GRAY2BGR)
    return img
