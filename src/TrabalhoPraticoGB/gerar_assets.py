"""
Gera os stickers (PNG com canal alfa) e a 2ª imagem padrão das operações.

Execute uma vez (os arquivos já vêm prontos no repositório, mas o script
fica aqui para mostrar como foram feitos):

    python gerar_assets.py

Os desenhos são feitos em 4x o tamanho final e depois reduzidos com
INTER_AREA, o que suaviza as bordas (antialiasing) do canal alfa.
"""

import math
import os

import cv2 as cv
import numpy as np

from processamento import salvar_imagem

PASTA = os.path.dirname(os.path.abspath(__file__))
PASTA_STICKERS = os.path.join(PASTA, "assets", "stickers")
PASTA_IMAGENS = os.path.join(PASTA, "assets", "imagens")
ESCALA = 4


def tela(w, h):
    """Imagem BGRA totalmente transparente, já na escala ampliada."""
    return np.zeros((h * ESCALA, w * ESCALA, 4), np.uint8)


def finalizar(img):
    h, w = img.shape[:2]
    return cv.resize(img, (w // ESCALA, h // ESCALA), interpolation=cv.INTER_AREA)


def pts(lista):
    return np.array([[int(x * ESCALA), int(y * ESCALA)] for x, y in lista], np.int32)


def poligono_estrela(cx, cy, r_ext, r_int, n=5, rot=-math.pi / 2):
    p = []
    for i in range(n * 2):
        r = r_ext if i % 2 == 0 else r_int
        a = rot + i * math.pi / n
        p.append((cx + r * math.cos(a), cy + r * math.sin(a)))
    return p


def estrela(rot=-math.pi / 2, tamanho=160):
    img = tela(tamanho, tamanho)
    c = tamanho / 2
    contorno = pts(poligono_estrela(c, c, c * 0.95, c * 0.42, rot=rot))
    miolo = pts(poligono_estrela(c, c, c * 0.80, c * 0.34, rot=rot))
    cv.fillPoly(img, [contorno], (20, 120, 230, 255), cv.LINE_AA)
    cv.fillPoly(img, [miolo], (40, 215, 255, 255), cv.LINE_AA)
    # brilho
    cv.circle(img, (int(c * 0.8 * ESCALA), int(c * 0.75 * ESCALA)), int(6 * ESCALA),
              (220, 250, 255, 255), -1, cv.LINE_AA)
    return finalizar(img)


def coracao(tamanho=160):
    img = tela(tamanho, tamanho)
    p = []
    for i in range(200):
        t = i / 200 * 2 * math.pi
        x = 16 * math.sin(t) ** 3
        y = 13 * math.cos(t) - 5 * math.cos(2 * t) - 2 * math.cos(3 * t) - math.cos(4 * t)
        p.append((tamanho / 2 + x * tamanho / 36, tamanho / 2 - y * tamanho / 36 + 6))
    cv.fillPoly(img, [pts(p)], (60, 30, 200, 255), cv.LINE_AA)
    p2 = [(tamanho / 2 + (x - tamanho / 2) * 0.82, tamanho / 2 + (y - tamanho / 2) * 0.82)
          for x, y in p]
    cv.fillPoly(img, [pts(p2)], (90, 60, 240, 255), cv.LINE_AA)
    cv.ellipse(img, (int(tamanho * 0.36 * ESCALA), int(tamanho * 0.36 * ESCALA)),
               (int(10 * ESCALA), int(6 * ESCALA)), -35, 0, 360, (210, 200, 255, 255), -1,
               cv.LINE_AA)
    return finalizar(img)


def oculos(w=300, h=110):
    img = tela(w, h)
    preto = (25, 25, 25, 255)
    lente = (70, 40, 20, 200)  # lente semitransparente
    for cx in (w * 0.27, w * 0.73):
        centro = (int(cx * ESCALA), int(h * 0.55 * ESCALA))
        eixos = (int(w * 0.21 * ESCALA), int(h * 0.36 * ESCALA))
        cv.ellipse(img, centro, eixos, 0, 0, 360, preto, -1, cv.LINE_AA)
        eixos_in = (int((w * 0.21 - 7) * ESCALA), int((h * 0.36 - 7) * ESCALA))
        img_l = img.copy()
        cv.ellipse(img_l, centro, eixos_in, 0, 0, 360, lente, -1, cv.LINE_AA)
        img[:] = img_l
        cv.ellipse(img, (centro[0] - int(14 * ESCALA), centro[1] - int(10 * ESCALA)),
                   (int(10 * ESCALA), int(5 * ESCALA)), -25, 0, 360, (230, 230, 230, 170),
                   -1, cv.LINE_AA)
    # ponte e hastes
    cv.line(img, pts([(w * 0.45, h * 0.45)])[0], pts([(w * 0.55, h * 0.45)])[0], preto,
            int(7 * ESCALA), cv.LINE_AA)
    cv.line(img, pts([(w * 0.06, h * 0.45)])[0], pts([(w * 0.01, h * 0.25)])[0], preto,
            int(6 * ESCALA), cv.LINE_AA)
    cv.line(img, pts([(w * 0.94, h * 0.45)])[0], pts([(w * 0.99, h * 0.25)])[0], preto,
            int(6 * ESCALA), cv.LINE_AA)
    return finalizar(img)


def orelhas_gato(w=320, h=150):
    img = tela(w, h)
    pelo = (60, 90, 120, 255)
    rosa = (170, 150, 245, 255)
    for lado in (0, 1):
        if lado == 0:
            ext = [(w * 0.05, h * 0.98), (w * 0.16, h * 0.04), (w * 0.38, h * 0.80)]
            inn = [(w * 0.11, h * 0.86), (w * 0.17, h * 0.26), (w * 0.31, h * 0.74)]
        else:
            ext = [(w * 0.95, h * 0.98), (w * 0.84, h * 0.04), (w * 0.62, h * 0.80)]
            inn = [(w * 0.89, h * 0.86), (w * 0.83, h * 0.26), (w * 0.69, h * 0.74)]
        cv.fillPoly(img, [pts(ext)], pelo, cv.LINE_AA)
        cv.fillPoly(img, [pts(inn)], rosa, cv.LINE_AA)
    return finalizar(img)


def coroa(w=240, h=150):
    img = tela(w, h)
    ouro = (30, 190, 245, 255)
    escuro = (20, 130, 200, 255)
    corpo = [(w * 0.08, h * 0.92), (w * 0.04, h * 0.25), (w * 0.28, h * 0.55),
             (w * 0.50, h * 0.08), (w * 0.72, h * 0.55), (w * 0.96, h * 0.25),
             (w * 0.92, h * 0.92)]
    cv.fillPoly(img, [pts(corpo)], escuro, cv.LINE_AA)
    interno = [(w * 0.12, h * 0.86), (w * 0.09, h * 0.36), (w * 0.29, h * 0.62),
               (w * 0.50, h * 0.18), (w * 0.71, h * 0.62), (w * 0.91, h * 0.36),
               (w * 0.88, h * 0.86)]
    cv.fillPoly(img, [pts(interno)], ouro, cv.LINE_AA)
    for (x, y, cor) in ((0.04, 0.22, (200, 60, 230)), (0.50, 0.06, (60, 40, 220)),
                        (0.96, 0.22, (200, 60, 230)), (0.30, 0.76, (180, 120, 40)),
                        (0.50, 0.76, (60, 40, 220)), (0.70, 0.76, (180, 120, 40))):
        cv.circle(img, (int(w * x * ESCALA), int(h * y * ESCALA)), int(9 * ESCALA),
                  cor + (255,), -1, cv.LINE_AA)
    return finalizar(img)


def balao(texto="UAU!", w=260, h=170):
    img = tela(w, h)
    branco = (255, 255, 255, 255)
    preto = (30, 30, 30, 255)
    centro = (int(w * 0.5 * ESCALA), int(h * 0.42 * ESCALA))
    eixos = (int(w * 0.46 * ESCALA), int(h * 0.36 * ESCALA))
    rabo = pts([(w * 0.30, h * 0.66), (w * 0.16, h * 0.98), (w * 0.48, h * 0.72)])
    cv.ellipse(img, centro, eixos, 0, 0, 360, preto, -1, cv.LINE_AA)
    cv.fillPoly(img, [rabo], preto, cv.LINE_AA)
    eixos_in = (eixos[0] - 6 * ESCALA, eixos[1] - 6 * ESCALA)
    cv.ellipse(img, centro, eixos_in, 0, 0, 360, branco, -1, cv.LINE_AA)
    rabo_in = pts([(w * 0.32, h * 0.64), (w * 0.21, h * 0.88), (w * 0.45, h * 0.68)])
    cv.fillPoly(img, [rabo_in], branco, cv.LINE_AA)
    fonte = cv.FONT_HERSHEY_DUPLEX
    esc = 1.6 * ESCALA
    esp = 3 * ESCALA
    (tw, th), _ = cv.getTextSize(texto, fonte, esc, esp)
    cv.putText(img, texto, (centro[0] - tw // 2, centro[1] + th // 2), fonte, esc,
               (40, 40, 220, 255), esp, cv.LINE_AA)
    return finalizar(img)


def spritesheet_estrela(quadros=8, tamanho=120):
    """Spritesheet horizontal: estrela girando e pulsando (sticker animado)."""
    folha = np.zeros((tamanho, tamanho * quadros, 4), np.uint8)
    for i in range(quadros):
        rot = -math.pi / 2 + i * (2 * math.pi / 5) / quadros
        q = estrela(rot=rot, tamanho=tamanho)
        escala = 0.75 + 0.25 * math.sin(i / quadros * 2 * math.pi)
        s = max(2, int(tamanho * escala))
        q = cv.resize(q, (s, s), interpolation=cv.INTER_AREA)
        o = (tamanho - s) // 2
        folha[o:o + s, i * tamanho + o:i * tamanho + o + s] = q
    return folha


def imagem_segunda(w=960, h=640):
    """Segunda imagem padrão das operações aritméticas: degradê suave lilás/rosa com bolhas."""
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
        c = (int(rng.integers(0, w)), int(rng.integers(0, h)))
        cv.circle(bolhas, c, int(rng.integers(30, 110)), (255, 230, 250), -1, cv.LINE_AA)
    img = cv.addWeighted(img, 0.75, bolhas, 0.25, 0)
    return cv.GaussianBlur(img, (0, 0), 6)


def main():
    os.makedirs(PASTA_STICKERS, exist_ok=True)
    os.makedirs(PASTA_IMAGENS, exist_ok=True)
    stickers = {
        "01_estrela.png": estrela(),
        "02_coracao.png": coracao(),
        "03_oculos.png": oculos(),
        "04_orelhas_gato.png": orelhas_gato(),
        "05_coroa.png": coroa(),
        "06_balao_uau.png": balao(),
        "anim_estrela_8x1.png": spritesheet_estrela(),
    }
    for nome, img in stickers.items():
        salvar_imagem(os.path.join(PASTA_STICKERS, nome), img)
        print("gerado:", nome, img.shape)
    salvar_imagem(os.path.join(PASTA_IMAGENS, "segunda.png"), imagem_segunda())
    print("2a imagem gerada em", PASTA_IMAGENS)


if __name__ == "__main__":
    main()
