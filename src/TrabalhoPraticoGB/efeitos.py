"""
Efeitos autorais do Stories FX (inspirados no tema roxo do app).

Diferente dos filtros clássicos de filtros.py, cada efeito aqui combina
várias operações vistas na disciplina (conversão de cor, convolução,
detecção de bordas, deslocamento de canais, blending, máscaras) para criar
um "look" de stories.

Mesma assinatura dos filtros: efeito(img, p1, p2) -> img, com img de
1 canal (cinza/canal único) ou 3 canais (BGR) e p1, p2 em 0..100.
"""

import cv2 as cv
import numpy as np


def para_bgr(img):
    return cv.cvtColor(img, cv.COLOR_GRAY2BGR) if img.ndim == 2 else img


def para_cinza(img):
    return img if img.ndim == 2 else cv.cvtColor(img, cv.COLOR_BGR2GRAY)


def volta_formato(resultado, original):
    """Se a entrada tinha 1 canal (cinza ou canal R/G/B), devolve 1 canal também."""
    if original.ndim == 2 and resultado.ndim == 3:
        return cv.cvtColor(resultado, cv.COLOR_BGR2GRAY)
    return resultado

# Cores do tema (BGR)
ROXO_ESCURO = np.array([70, 10, 45], np.float32)
ROXO = np.array([229, 93, 155], np.float32)
ROSA = np.array([200, 140, 255], np.float32)
LILAS = np.array([250, 190, 225], np.float32)


def _blur_rapido(img, sigma):
    """Desfoque gaussiano grande feito em 1/4 do tamanho e ampliado de volta (rápido p/ webcam)."""
    h, w = img.shape[:2]
    peq = cv.resize(img, (max(1, w // 4), max(1, h // 4)), interpolation=cv.INTER_AREA)
    peq = cv.GaussianBlur(peq, (0, 0), max(0.5, sigma / 4))
    return cv.resize(peq, (w, h), interpolation=cv.INTER_LINEAR)


def _tela(a, b):
    """Blending 'screen': 1 - (1-a)(1-b). Clareia sem estourar como a soma."""
    return 255.0 - (255.0 - a) * (255.0 - b) / 255.0


def duotone_roxo(img, p1, p2):
    """Pinta a imagem com só duas cores: sombras roxo-escuras e luzes rosa/lilás."""
    cinza = para_cinza(img).astype(np.float32)
    # contraste (P2) aplicado antes do mapeamento: 50 = original
    contraste = 0.5 + p2 / 50.0
    t = np.clip((cinza - 128) * contraste + 128, 0, 255)[:, :, None] / 255.0
    # gradiente de 3 pontos: roxo escuro -> roxo -> lilás
    meio = np.clip(t * 2, 0, 1)
    alto = np.clip(t * 2 - 1, 0, 1)
    duo = ROXO_ESCURO * (1 - meio) + ROXO * meio
    duo = duo * (1 - alto) + LILAS * alto
    original = para_bgr(img).astype(np.float32)
    mistura = p1 / 100.0
    saida = original * (1 - mistura) + duo * mistura
    return volta_formato(np.clip(saida, 0, 255).astype(np.uint8), img)


def glitch_rgb(img, p1, p2):
    """Separa os canais R e B para os lados e 'rasga' faixas horizontais da imagem."""
    bgr = para_bgr(img)
    h, w = bgr.shape[:2]
    desloc = int(p1 / 100.0 * w * 0.04)                   # até 4% da largura
    b, g, r = cv.split(bgr)
    if desloc:
        r = np.roll(r, -desloc, axis=1)                    # vermelho para a esquerda
        b = np.roll(b, desloc, axis=1)                     # azul para a direita
    saida = cv.merge([b, g, r])
    # faixas deslocadas (P2 = quantidade); semente fixa para a foto não "piscar"
    faixas = int(p2 / 100.0 * 14)
    rng = np.random.default_rng(42)
    for _ in range(faixas):
        y = int(rng.integers(0, h))
        alt = int(rng.integers(2, max(3, h // 25)))
        dx = int(rng.integers(-w // 12, w // 12))
        saida[y:y + alt] = np.roll(saida[y:y + alt], dx, axis=1)
    return volta_formato(saida, img)


def neon(img, p1, p2):
    """Bordas brilhando em roxo/rosa sobre a imagem escurecida."""
    cinza = cv.GaussianBlur(para_cinza(img), (5, 5), 0)
    bordas = cv.Canny(cinza, 50, 130)
    bordas = cv.dilate(bordas, np.ones((2, 2), np.uint8))
    # cor da borda varia na horizontal: roxo -> rosa
    h, w = bordas.shape
    t = np.linspace(0, 1, w, dtype=np.float32)[None, :, None]
    cor = ROXO * (1 - t) + ROSA * t
    linhas = (bordas.astype(np.float32)[:, :, None] / 255.0) * cor
    # brilho = várias versões borradas da borda somadas (P1 = tamanho do brilho)
    raio = 3 + p1 / 100.0 * 30
    brilho = _blur_rapido(linhas, raio) * 3.0 + cv.GaussianBlur(linhas, (0, 0), raio / 3) * 1.5
    # fundo escurecido (P2 = quanto da imagem original aparece)
    fundo = para_bgr(img).astype(np.float32) * (p2 / 100.0) * 0.6
    saida = _tela(_tela(fundo, np.clip(brilho, 0, 255)), linhas)
    return volta_formato(np.clip(saida, 0, 255).astype(np.uint8), img)


def vhs(img, p1, p2):
    """Aparência de fita VHS: cor 'vazando', linhas de varredura, ruído e cores lavadas."""
    bgr = para_bgr(img)
    h, w = bgr.shape[:2]
    # 1) separa luminância e cor (YCrCb) e borra/desloca só a cor (cor "vazando")
    ycc = cv.cvtColor(bgr, cv.COLOR_BGR2YCrCb)
    y, cr, cb = cv.split(ycc)
    desloc = 2 + int(p2 / 100.0 * 20)
    cr = np.roll(cv.blur(cr, (15, 1)), desloc, axis=1)
    cb = np.roll(cv.blur(cb, (15, 1)), -desloc // 2, axis=1)
    saida = cv.cvtColor(cv.merge([y, cr, cb]), cv.COLOR_YCrCb2BGR).astype(np.float32)
    # 2) leve desfoque horizontal (fita perde nitidez)
    saida = cv.blur(saida, (5, 1))
    forca = p1 / 100.0
    # 3) linhas de varredura: escurece uma linha a cada duas
    saida[::2] *= 1.0 - 0.45 * forca
    # 4) ruído granulado (semente pela posição: estável na foto)
    rng = np.random.default_rng(7)
    saida += rng.normal(0, 22 * forca, (h, w, 1)).astype(np.float32)
    # faixa clara de 'tracking' perto da parte de baixo, típica de VHS
    y0 = int(h * 0.82)
    alt = max(2, h // 40)
    saida[y0:y0 + alt] = saida[y0:y0 + alt] * (1 - 0.5 * forca) + 230 * 0.5 * forca
    saida[y0:y0 + alt] = np.roll(saida[y0:y0 + alt], int(w * 0.02), axis=1)
    # 5) cores lavadas com leve tom roxo
    media = saida.mean(axis=2, keepdims=True)
    saida = saida * (1 - 0.35 * forca) + media * 0.35 * forca          # dessatura
    saida = saida * (1 - 0.15 * forca) + np.array([50, 0, 30], np.float32) * forca
    return volta_formato(np.clip(saida, 0, 255).astype(np.uint8), img)


def sonho(img, p1, p2):
    """Brilho suave nas áreas claras (bloom), tom lilás e bordas escurecidas."""
    bgr = para_bgr(img).astype(np.float32)
    h, w = bgr.shape[:2]
    # 1) pega só as partes claras e borra bastante -> "halo" de luz
    claro = np.clip((bgr - 110) * 2.0, 0, 255)
    halo = _blur_rapido(claro, 6 + p1 / 100.0 * 25)
    suave = cv.GaussianBlur(bgr, (0, 0), 2)
    saida = _tela(bgr * 0.6 + suave * 0.4, halo * (0.6 + p1 / 100.0 * 1.2))
    # 2) tom lilás (P2) misturado com blending 'screen'
    tom = np.empty_like(saida)
    tom[:] = LILAS * 0.55 + ROXO * 0.25
    saida = saida * (1 - p2 / 130.0) + _tela(saida, tom * 0.5) * (p2 / 130.0)
    # 3) vinheta suave
    kx = cv.getGaussianKernel(w, w * 0.6)
    ky = cv.getGaussianKernel(h, h * 0.6)
    mascara = ky @ kx.T
    mascara = (0.55 + 0.45 * mascara / mascara.max())[:, :, None]
    return volta_formato(np.clip(saida * mascara, 0, 255).astype(np.uint8), img)


EFEITOS = [
    {
        "nome": "✦ Duotone Roxo",
        "func": duotone_roxo, "p1": 100, "p2": 60,
        "tipo": "Efeito exclusivo",
        "descricao": "Transforma a foto em só duas cores do tema: as sombras ficam roxo-escuras e "
                     "as luzes viram lilás. Converte para cinza e usa o brilho de cada pixel para "
                     "escolher a cor num degradê.",
        "params": "Param 1: intensidade, quanto do efeito aparece (0 a 100%). "
                  "Param 2: contraste antes de colorir (50 = original).",
    },
    {
        "nome": "✦ Glitch RGB",
        "func": glitch_rgb, "p1": 40, "p2": 50,
        "tipo": "Efeito exclusivo",
        "descricao": "Imita uma tela com defeito: separa os canais vermelho e azul, empurrando "
                     "cada um para um lado, e 'rasga' algumas faixas horizontais da imagem.",
        "params": "Param 1: distância entre os canais (até 4% da largura). "
                  "Param 2: quantidade de faixas rasgadas (0 a 14).",
    },
    {
        "nome": "✦ Neon",
        "func": neon, "p1": 40, "p2": 35,
        "tipo": "Efeito exclusivo",
        "descricao": "Encontra as bordas (Canny), pinta de roxo a rosa e espalha um brilho em volta "
                     "com desfoque gaussiano, como letreiro de neon sobre a foto escurecida.",
        "params": "Param 1: tamanho do brilho. Param 2: quanto da foto aparece no fundo.",
    },
    {
        "nome": "✦ VHS",
        "func": vhs, "p1": 75, "p2": 50,
        "tipo": "Efeito exclusivo",
        "descricao": "Aparência de fita de vídeo antiga: a cor 'vaza' para o lado (canais de cor "
                     "YCrCb deslocados), linhas de varredura escuras, ruído granulado e cores lavadas.",
        "params": "Param 1: força das linhas e do ruído. Param 2: quanto a cor vaza para o lado.",
    },
    {
        "nome": "✦ Sonho",
        "func": sonho, "p1": 50, "p2": 60,
        "tipo": "Efeito exclusivo",
        "descricao": "Clima de sonho: as partes claras ganham um halo de luz (bloom), a imagem "
                     "recebe um tom lilás e as bordas escurecem suavemente.",
        "params": "Param 1: força do halo de luz. Param 2: quantidade de tom lilás.",
    },
]
