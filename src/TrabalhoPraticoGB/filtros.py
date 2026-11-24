"""
Filtros de imagem (Processamento Gráfico: Fundamentos - Grau B).

Cada filtro recebe:
    img -> matriz NumPy uint8 com 1 canal (cinza / canal único) ou 3 canais (BGR)
    p1  -> parâmetro 1, valor inteiro de 0 a 100 (vem do trackbar "Param 1")
    p2  -> parâmetro 2, valor inteiro de 0 a 100 (vem do trackbar "Param 2")
e devolve uma imagem uint8 com o mesmo número de canais da entrada.

Os parâmetros de 0..100 são convertidos para a faixa útil de cada filtro
(tamanho de kernel, sigma, limiar etc.) dentro da própria função.
"""

import cv2 as cv
import numpy as np


# ---------------------------------------------------------------------------
# Funções auxiliares
# ---------------------------------------------------------------------------

def kernel_impar(p, minimo=1, maximo=31):
    """Converte um valor 0..100 em um tamanho de kernel ímpar (1, 3, 5, ...)."""
    k = int(minimo + (maximo - minimo) * p / 100.0)
    if k % 2 == 0:
        k += 1
    return max(minimo, min(k, maximo if maximo % 2 == 1 else maximo - 1))


def eh_cinza(img):
    return img.ndim == 2


def para_bgr(img):
    """Garante 3 canais (necessário para filtros que só fazem sentido em cor)."""
    return cv.cvtColor(img, cv.COLOR_GRAY2BGR) if eh_cinza(img) else img


def volta_formato(resultado, original):
    """Se a entrada tinha 1 canal, devolve o resultado também com 1 canal."""
    if eh_cinza(original) and resultado.ndim == 3:
        return cv.cvtColor(resultado, cv.COLOR_BGR2GRAY)
    return resultado


def para_cinza(img):
    return img if eh_cinza(img) else cv.cvtColor(img, cv.COLOR_BGR2GRAY)


# ---------------------------------------------------------------------------
# Filtros passa-baixa (suavização)
# ---------------------------------------------------------------------------

def media(img, p1, p2):
    k = kernel_impar(p1, 3, 31)
    return cv.blur(img, (k, k))


def gaussiano(img, p1, p2):
    k = kernel_impar(p1, 3, 31)
    sigma = p2 / 10.0  # 0 = OpenCV calcula a partir do tamanho do kernel
    return cv.GaussianBlur(img, (k, k), sigma)


def mediana(img, p1, p2):
    k = kernel_impar(p1, 3, 21)
    return cv.medianBlur(img, k)


def bilateral(img, p1, p2):
    d = int(5 + p1 / 100.0 * 10)          # diâmetro da vizinhança 5..15
    sigma = 10 + p2 / 100.0 * 140          # sigma de cor/espaço 10..150
    return cv.bilateralFilter(img, d, sigma, sigma)


# ---------------------------------------------------------------------------
# Filtros passa-alta (realce / bordas)
# ---------------------------------------------------------------------------

def sharpen(img, p1, p2):
    # Kernel de realce dos slides, com o peso central controlado por p1:
    #  [ 0 -a  0]
    #  [-a 1+4a -a]
    #  [ 0 -a  0]
    a = 0.25 + p1 / 100.0 * 2.75           # 0.25..3.0  (a = 1 -> kernel clássico)
    kernel = np.array([[0, -a, 0],
                       [-a, 1 + 4 * a, -a],
                       [0, -a, 0]], np.float32)
    return cv.filter2D(img, -1, kernel)


def laplaciano(img, p1, p2):
    k = kernel_impar(p1, 1, 7)
    lap = cv.Laplacian(img, cv.CV_64F, ksize=k)
    return cv.convertScaleAbs(lap)


def sobel(img, p1, p2):
    k = kernel_impar(p1, 1, 7)
    gx = cv.Sobel(img, cv.CV_64F, 1, 0, ksize=k)
    gy = cv.Sobel(img, cv.CV_64F, 0, 1, ksize=k)
    mag = cv.magnitude(gx, gy)
    return cv.convertScaleAbs(mag)


def canny(img, p1, p2):
    cinza = para_cinza(img)
    limiar1 = int(p1 / 100.0 * 255)
    limiar2 = int(p2 / 100.0 * 255)
    if limiar2 < limiar1:
        limiar1, limiar2 = limiar2, limiar1
    bordas = cv.Canny(cinza, limiar1, limiar2)
    return volta_formato(cv.cvtColor(bordas, cv.COLOR_GRAY2BGR), img)


def relevo(img, p1, p2):
    forca = 0.5 + p1 / 100.0 * 2.5
    kernel = np.array([[-2, -1, 0],
                       [-1, 1, 1],
                       [0, 1, 2]], np.float32) * forca
    kernel[1, 1] = 1
    return cv.filter2D(img, -1, kernel, delta=0)


# ---------------------------------------------------------------------------
# Filtros estatísticos / morfológicos
# ---------------------------------------------------------------------------

def dilatacao(img, p1, p2):
    k = kernel_impar(p1, 3, 21)
    return cv.dilate(img, np.ones((k, k), np.uint8))


def erosao(img, p1, p2):
    k = kernel_impar(p1, 3, 21)
    return cv.erode(img, np.ones((k, k), np.uint8))


# ---------------------------------------------------------------------------
# Filtros pontuais (pixel a pixel)
# ---------------------------------------------------------------------------

def negativo(img, p1, p2):
    return 255 - img          # mesmo que canal ^ 255 (XOR) visto em aula


def limiarizacao(img, p1, p2):
    limiar = int(p1 / 100.0 * 255)
    _, saida = cv.threshold(img, limiar, 255, cv.THRESH_BINARY)
    return saida


def brilho_contraste(img, p1, p2):
    alpha = p2 / 50.0                      # contraste 0..2   (50 = sem mudança)
    beta = (p1 - 50) * 2.55                # brilho -127..127 (50 = sem mudança)
    return cv.convertScaleAbs(img, alpha=alpha, beta=beta)


def gamma(img, p1, p2):
    g = max(0.1, p1 / 100.0 * 3.0)         # gamma 0.1..3.0
    tabela = np.array([((i / 255.0) ** (1.0 / g)) * 255 for i in range(256)],
                      np.float32).clip(0, 255).astype(np.uint8)
    return cv.LUT(img, tabela)


def sepia(img, p1, p2):
    bgr = para_bgr(img).astype(np.float32)
    # Matriz sépia clássica, escrita na ordem BGR do OpenCV
    m = np.array([[0.131, 0.534, 0.272],
                  [0.168, 0.686, 0.349],
                  [0.189, 0.769, 0.393]], np.float32)
    s = cv.transform(bgr, m)
    intensidade = p1 / 100.0
    saida = cv.addWeighted(bgr, 1 - intensidade, s, intensidade, 0)
    return volta_formato(np.clip(saida, 0, 255).astype(np.uint8), img)


def saturacao(img, p1, p2):
    if eh_cinza(img):
        return img.copy()                  # imagem de 1 canal não tem saturação
    hsv = cv.cvtColor(img, cv.COLOR_BGR2HSV)
    h, s, v = cv.split(hsv)
    fator = p1 / 50.0                      # 0..2 (50 = sem mudança)
    s = cv.convertScaleAbs(s, alpha=fator)
    return cv.cvtColor(cv.merge([h, s, v]), cv.COLOR_HSV2BGR)


def posterizacao(img, p1, p2):
    niveis = int(2 + p1 / 100.0 * 14)      # 2..16 níveis por canal
    passo = 256 / niveis
    return (np.floor(img / passo) * passo + passo / 2).clip(0, 255).astype(np.uint8)


def pixelizacao(img, p1, p2):
    bloco = int(2 + p1 / 100.0 * 38)       # tamanho do bloco 2..40 px
    h, w = img.shape[:2]
    pequena = cv.resize(img, (max(1, w // bloco), max(1, h // bloco)),
                        interpolation=cv.INTER_LINEAR)
    return cv.resize(pequena, (w, h), interpolation=cv.INTER_NEAREST)


def vinheta(img, p1, p2):
    h, w = img.shape[:2]
    sigma = 0.25 + (1 - p1 / 100.0) * 0.75
    kx = cv.getGaussianKernel(w, w * sigma)
    ky = cv.getGaussianKernel(h, h * sigma)
    mascara = ky @ kx.T
    mascara = mascara / mascara.max()
    if not eh_cinza(img):
        mascara = mascara[:, :, None]
    return (img * mascara).astype(np.uint8)


# ---------------------------------------------------------------------------
# Catálogo: nome, função, descrição e significado dos parâmetros
# ---------------------------------------------------------------------------

FILTROS = [
    {
        "nome": "Média (Box Blur)",
        "func": media, "p1": 20, "p2": 0,
        "tipo": "Suavização (passa-baixa)",
        "descricao": "Substitui cada pixel pela média dos vizinhos dentro de um kernel "
                     "k x k (cv.blur). Reduz ruído, mas também borra as bordas.",
        "params": "Param 1: tamanho do kernel (3 a 31). Param 2: não usado.",
    },
    {
        "nome": "Gaussiano",
        "func": gaussiano, "p1": 20, "p2": 15,
        "tipo": "Suavização (passa-baixa)",
        "descricao": "Média ponderada por uma gaussiana (cv.GaussianBlur): vizinhos "
                     "mais próximos pesam mais. Suaviza de forma mais natural que a média.",
        "params": "Param 1: tamanho do kernel (3 a 31). Param 2: sigma (0 a 10; 0 = automático).",
    },
    {
        "nome": "Mediana",
        "func": mediana, "p1": 10, "p2": 0,
        "tipo": "Suavização (estatístico)",
        "descricao": "Troca cada pixel pela mediana da vizinhança (cv.medianBlur). "
                     "Ótimo para remover ruído 'sal e pimenta' preservando bordas.",
        "params": "Param 1: tamanho do kernel (3 a 21). Param 2: não usado.",
    },
    {
        "nome": "Bilateral",
        "func": bilateral, "p1": 30, "p2": 40,
        "tipo": "Suavização que preserva bordas",
        "descricao": "Suaviza considerando distância espacial E diferença de cor "
                     "(cv.bilateralFilter). Gera efeito de 'pele lisa'.",
        "params": "Param 1: diâmetro da vizinhança (5 a 15). Param 2: sigma de cor/espaço (10 a 150).",
    },
    {
        "nome": "Realce (Sharpen)",
        "func": sharpen, "p1": 27, "p2": 0,
        "tipo": "Realce (passa-alta)",
        "descricao": "Kernel de nitidez aplicado com cv.filter2D: reforça o pixel "
                     "central e subtrai os vizinhos, destacando detalhes.",
        "params": "Param 1: força do realce (0.25 a 3.0). Param 2: não usado.",
    },
    {
        "nome": "Laplaciano",
        "func": laplaciano, "p1": 30, "p2": 0,
        "tipo": "Realce / detecção de bordas (passa-alta)",
        "descricao": "Segunda derivada da imagem (cv.Laplacian): destaca bordas em "
                     "todas as direções.",
        "params": "Param 1: tamanho do kernel (1 a 7). Param 2: não usado.",
    },
    {
        "nome": "Sobel",
        "func": sobel, "p1": 30, "p2": 0,
        "tipo": "Detecção de bordas (gradiente)",
        "descricao": "Calcula as derivadas horizontal e vertical (cv.Sobel) e "
                     "mostra a magnitude do gradiente.",
        "params": "Param 1: tamanho do kernel (1 a 7). Param 2: não usado.",
    },
    {
        "nome": "Canny",
        "func": canny, "p1": 20, "p2": 60,
        "tipo": "Detecção de bordas",
        "descricao": "Detector de bordas de Canny (cv.Canny): gera bordas finas e "
                     "binárias usando dois limiares (histerese).",
        "params": "Param 1: limiar inferior (0 a 255). Param 2: limiar superior (0 a 255).",
    },
    {
        "nome": "Relevo (Emboss)",
        "func": relevo, "p1": 20, "p2": 0,
        "tipo": "Convolução com kernel direcional",
        "descricao": "Kernel diagonal que dá a impressão de que a imagem foi "
                     "esculpida em relevo.",
        "params": "Param 1: força do relevo. Param 2: não usado.",
    },
    {
        "nome": "Dilatação (Máximo)",
        "func": dilatacao, "p1": 10, "p2": 0,
        "tipo": "Estatístico / morfológico",
        "descricao": "Filtro de máximo (cv.dilate): cada pixel recebe o maior valor "
                     "da vizinhança. Expande áreas claras.",
        "params": "Param 1: tamanho do kernel (3 a 21). Param 2: não usado.",
    },
    {
        "nome": "Erosão (Mínimo)",
        "func": erosao, "p1": 10, "p2": 0,
        "tipo": "Estatístico / morfológico",
        "descricao": "Filtro de mínimo (cv.erode): cada pixel recebe o menor valor "
                     "da vizinhança. Expande áreas escuras.",
        "params": "Param 1: tamanho do kernel (3 a 21). Param 2: não usado.",
    },
    {
        "nome": "Negativo",
        "func": negativo, "p1": 0, "p2": 0,
        "tipo": "Operação pontual",
        "descricao": "Inverte as intensidades: novo = 255 - valor (equivale a "
                     "canal ^ 255).",
        "params": "Sem parâmetros.",
    },
    {
        "nome": "Limiarização",
        "func": limiarizacao, "p1": 50, "p2": 0,
        "tipo": "Operação pontual",
        "descricao": "Binariza a imagem (cv.threshold): valores acima do limiar "
                     "viram 255, os demais viram 0.",
        "params": "Param 1: limiar (0 a 255). Param 2: não usado.",
    },
    {
        "nome": "Brilho e Contraste",
        "func": brilho_contraste, "p1": 60, "p2": 60,
        "tipo": "Operação pontual",
        "descricao": "Transformação linear novo = alpha * valor + beta "
                     "(cv.convertScaleAbs).",
        "params": "Param 1: brilho beta (50 = original). Param 2: contraste alpha (50 = original).",
    },
    {
        "nome": "Correção Gamma",
        "func": gamma, "p1": 50, "p2": 0,
        "tipo": "Operação pontual (não linear)",
        "descricao": "novo = 255 * (valor/255)^(1/gamma), via tabela (cv.LUT). "
                     "Gamma > 1 clareia os tons escuros; < 1 escurece.",
        "params": "Param 1: gamma (0.1 a 3.0; ~33 = 1.0). Param 2: não usado.",
    },
    {
        "nome": "Sépia",
        "func": sepia, "p1": 100, "p2": 0,
        "tipo": "Cor",
        "descricao": "Combina os canais com uma matriz de cor sépia (cv.transform), "
                     "dando aspecto de foto antiga.",
        "params": "Param 1: intensidade da mistura (0 a 100%). Param 2: não usado.",
    },
    {
        "nome": "Saturação (HSV)",
        "func": saturacao, "p1": 80, "p2": 0,
        "tipo": "Cor",
        "descricao": "Converte para HSV, multiplica o canal S e volta para BGR "
                     "(exemplo dos slides). Só altera imagens coloridas.",
        "params": "Param 1: fator de saturação (0 a 2; 50 = original). Param 2: não usado.",
    },
    {
        "nome": "Posterização",
        "func": posterizacao, "p1": 10, "p2": 0,
        "tipo": "Quantização",
        "descricao": "Reduz a quantidade de tons por canal, gerando um efeito de pôster.",
        "params": "Param 1: número de níveis por canal (2 a 16). Param 2: não usado.",
    },
    {
        "nome": "Pixelização",
        "func": pixelizacao, "p1": 20, "p2": 0,
        "tipo": "Amostragem",
        "descricao": "Reduz a resolução e amplia de volta com vizinho mais próximo, "
                     "formando blocos (efeito mosaico).",
        "params": "Param 1: tamanho do bloco (2 a 40 px). Param 2: não usado.",
    },
    {
        "nome": "Vinheta",
        "func": vinheta, "p1": 50, "p2": 0,
        "tipo": "Máscara",
        "descricao": "Multiplica a imagem por uma máscara gaussiana, escurecendo "
                     "as bordas como em fotos de câmera antiga.",
        "params": "Param 1: força da vinheta (0 a 100). Param 2: não usado.",
    },
]


# Os 5 efeitos exclusivos (efeitos.py) aparecem primeiro na lista de filtros.
from efeitos import EFEITOS  # noqa: E402

FILTROS = EFEITOS + FILTROS
