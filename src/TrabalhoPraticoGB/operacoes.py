"""
Operações aritméticas entre duas imagens.

Cada operação recebe a imagem atual (img), a segunda imagem (img2, já
redimensionada e com o mesmo número de canais) e o parâmetro p1 (0..100)
usado como peso alpha. Devolve uma imagem uint8.
"""

import cv2 as cv
import numpy as np


def adicao(img, img2, p1):
    # cv.add satura em 255. Alpha controla a intensidade da 2ª imagem.
    alpha = p1 / 100.0
    b = cv.convertScaleAbs(img2, alpha=alpha)
    return cv.add(img, b)


def subtracao_ponderada(img, img2, p1):
    # resultado = img - alpha * img2
    alpha = p1 / 100.0
    return cv.addWeighted(img, 1.0, img2, -alpha, 0)


def blending(img, img2, p1):
    # Fórmula da mistura: resultado = (1 - alpha) * img + alpha * img2
    alpha = p1 / 100.0
    return cv.addWeighted(img, 1.0 - alpha, img2, alpha, 0)


def multiplicacao(img, img2, p1):
    # Modo Multiply: alpha controla a força da multiplicação
    alpha = p1 / 100.0
    a = img.astype(np.float32) / 255.0
    b = (img2.astype(np.float32) / 255.0) * alpha + (1.0 - alpha)
    return np.clip(a * b * 255.0, 0, 255).astype(np.uint8)


def diferenca_absoluta(img, img2, p1):
    # |img - img2| pixel a pixel
    alpha = p1 / 100.0
    diff = cv.absdiff(img, img2)
    return cv.addWeighted(img, 1.0 - alpha, diff, alpha, 0)


OPERACOES = [
    {
        "nome": "Adição",
        "func": adicao, "p1": 50,
        "descricao": "Soma das duas imagens (cv.add), saturando em 255.",
        "params": "Intensidade da 2ª imagem (Alpha).",
    },
    {
        "nome": "Subtração ponderada",
        "func": subtracao_ponderada, "p1": 50,
        "descricao": "Subtrai a 2ª imagem da foto atual (cv.addWeighted).",
        "params": "Peso da subtração (Alpha).",
    },
    {
        "nome": "Blending (mistura)",
        "func": blending, "p1": 50,
        "descricao": "Transição suave e gradual entre as duas imagens.",
        "params": "Transição da 2ª imagem (Alpha).",
    },
    {
        "nome": "Multiplicação",
        "func": multiplicacao, "p1": 50,
        "descricao": "Multiplica as cores das imagens (efeito Multiply).",
        "params": "Intensidade do efeito (Alpha).",
    },
    {
        "nome": "Diferença absoluta",
        "func": diferenca_absoluta, "p1": 50,
        "descricao": "Destaca a diferença visual entre as duas imagens (cv.absdiff).",
        "params": "Intensidade da diferença (Alpha).",
    },
]


def ajustar_segunda(img2, referencia):
    """Redimensiona a segunda imagem e ajusta os canais para casar com a referência."""
    h, w = referencia.shape[:2]
    b = cv.resize(img2, (w, h), interpolation=cv.INTER_AREA)
    if referencia.ndim == 2 and b.ndim == 3:
        b = cv.cvtColor(b, cv.COLOR_BGR2GRAY)
    elif referencia.ndim == 3 and b.ndim == 2:
        b = cv.cvtColor(b, cv.COLOR_GRAY2BGR)
    return b
