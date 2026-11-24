"""
Stickers: carregamento de PNGs com canal alfa, sobreposição (alpha blending),
stickers animados por spritesheet e posicionamento automático por detecção
de face (Haar Cascade).
"""

import glob
import os
import time

import cv2 as cv
import numpy as np

from processamento import ler_imagem


class Sticker:
    """Um sticker estático (1 quadro) ou animado (spritesheet horizontal)."""

    def __init__(self, caminho):
        img = ler_imagem(caminho, cv.IMREAD_UNCHANGED)
        if img is None:
            raise ValueError(f"Não foi possível ler o sticker: {caminho}")
        # Garante 4 canais (BGRA) mesmo se a imagem for JPG ou PNG sem alfa
        if img.ndim == 2:
            img = cv.cvtColor(img, cv.COLOR_GRAY2BGRA)
        elif img.ndim == 3:
            if img.shape[2] == 3:
                img = cv.cvtColor(img, cv.COLOR_BGR2BGRA)
            elif img.shape[2] != 4:
                raise ValueError(f"Sticker com número inválido de canais ({img.shape[2]}): {caminho}")
        else:
            raise ValueError(f"Formato de sticker inválido: {caminho}")

        self.nome = os.path.splitext(os.path.basename(caminho))[0]
        self.quadros = [img]
        self.fps = 10
        # Convenção: "anim_<nome>_<colunas>x<linhas>.png" é um spritesheet
        if self.nome.startswith("anim_"):
            try:
                cols, lins = self.nome.rsplit("_", 1)[1].split("x")
                cols, lins = int(cols), int(lins)
                h, w = img.shape[0] // lins, img.shape[1] // cols
                self.quadros = [img[l * h:(l + 1) * h, c * w:(c + 1) * w]
                                for l in range(lins) for c in range(cols)]
            except ValueError:
                pass

        # Padroniza o tamanho base de todos os stickers para ~180px
        # (evita que imagens de 1080p fiquem gigantes e todas fiquem uniformes)
        TAM_BASE = 180
        quadros_norm = []
        for q in self.quadros:
            qh, qw = q.shape[:2]
            maior = max(qh, qw)
            if maior > TAM_BASE:
                f = TAM_BASE / maior
                nw, nh = max(1, int(qw * f)), max(1, int(qh * f))
                q = cv.resize(q, (nw, nh), interpolation=cv.INTER_AREA)
            quadros_norm.append(q)
        self.quadros = quadros_norm

    @property
    def animado(self):
        return len(self.quadros) > 1

    def quadro_atual(self, t=None):
        """Escolhe o quadro do spritesheet a partir do tempo (animação)."""
        if not self.animado:
            return self.quadros[0]
        t = time.time() if t is None else t
        return self.quadros[int(t * self.fps) % len(self.quadros)]


def carregar_stickers(pasta):
    stickers = []
    extensoes = ("*.png", "*.jpg", "*.jpeg", "*.webp")
    caminhos = []
    for ext in extensoes:
        caminhos.extend(glob.glob(os.path.join(pasta, ext)))
    for caminho in sorted(set(caminhos)):
        try:
            stickers.append(Sticker(caminho))
        except Exception as e:
            print("[aviso]", e)
    return stickers


def sobrepor(fundo, sticker_bgra, cx, cy, escala=1.0):
    """
    Desenha o sticker (BGRA) centralizado em (cx, cy) sobre o fundo (BGR),
    usando o canal alfa: resultado = alfa * sticker + (1 - alfa) * fundo.
    Trata stickers que saem parcialmente para fora da imagem.
    """
    if escala != 1.0:
        w = max(1, int(sticker_bgra.shape[1] * escala))
        h = max(1, int(sticker_bgra.shape[0] * escala))
        sticker_bgra = cv.resize(sticker_bgra, (w, h), interpolation=cv.INTER_AREA)
    sh, sw = sticker_bgra.shape[:2]
    fh, fw = fundo.shape[:2]
    x0, y0 = int(cx - sw / 2), int(cy - sh / 2)
    # recorte da região que realmente cai dentro da imagem
    fx0, fy0 = max(0, x0), max(0, y0)
    fx1, fy1 = min(fw, x0 + sw), min(fh, y0 + sh)
    if fx0 >= fx1 or fy0 >= fy1:
        return fundo
    sx0, sy0 = fx0 - x0, fy0 - y0
    sx1, sy1 = sx0 + (fx1 - fx0), sy0 + (fy1 - fy0)

    pedaco = sticker_bgra[sy0:sy1, sx0:sx1]
    alfa = pedaco[:, :, 3:4].astype(np.float32) / 255.0
    regiao = fundo[fy0:fy1, fx0:fx1].astype(np.float32)
    mistura = alfa * pedaco[:, :, :3].astype(np.float32) + (1 - alfa) * regiao
    fundo[fy0:fy1, fx0:fx1] = mistura.astype(np.uint8)
    return fundo


class DetectorFace:
    """Detecta rostos com Haar Cascade e posiciona stickers automaticamente."""

    MODOS = [
        "desligado",
        "oculos pixel",
        "orelhas gato preto",
        "bigodes gato",
        "gato preto completo",
        "raiva anime",
    ]

    def __init__(self):
        # Haar Cascades fazem parte do OpenCV 4.x (no 5.x foram removidos do pacote
        # principal); por isso o requirements.txt fixa opencv-python < 5.
        self.ok = False
        if hasattr(cv, "CascadeClassifier"):
            caminho = os.path.join(cv.data.haarcascades, "haarcascade_frontalface_default.xml")
            self.cascade = cv.CascadeClassifier(caminho)
            self.ok = not self.cascade.empty()

    def detectar(self, img_bgr):
        if not self.ok:
            return []
        cinza = cv.cvtColor(img_bgr, cv.COLOR_BGR2GRAY)
        cinza = cv.equalizeHist(cinza)
        menor = max(40, min(cinza.shape[:2]) // 8)
        return self.cascade.detectMultiScale(cinza, scaleFactor=1.1, minNeighbors=5,
                                             minSize=(menor, menor))

    def aplicar(self, img_bgr, modo, stickers_por_nome, rostos=None):
        """Desenha os stickers do modo escolhido sobre cada rosto encontrado."""
        if modo == "desligado":
            return img_bgr
        if rostos is None:
            rostos = self.detectar(img_bgr)

        oculos_px = stickers_por_nome.get("st_oculos_pixel")
        bigodes = stickers_por_nome.get("st_bigodes_gato")
        orelhas_pretas = stickers_por_nome.get("st_orelhas_gato_preto")
        raiva = stickers_por_nome.get("st_raiva_anime")

        for (x, y, w, h) in rostos:
            # 1. Orelhas de gato pretas (no topo da cabeça)
            if modo in ("orelhas gato preto", "gato preto completo") and orelhas_pretas is not None:
                s = orelhas_pretas.quadro_atual()
                esc = (w * 1.05) / s.shape[1]
                sobrepor(img_bgr, s, x + w / 2, y - h * 0.12, esc)

            # 3. Bigodes de gato (nas bochechas)
            if modo in ("bigodes gato", "gato preto completo") and bigodes is not None:
                s = bigodes.quadro_atual()
                esc = (w * 0.95) / s.shape[1]
                sobrepor(img_bgr, s, x + w / 2, y + h * 0.65, esc)

            # 4. Narizinho do gato preto completo
            if modo == "gato preto completo":
                nx, ny = int(x + w / 2), int(y + h * 0.63)
                cv.circle(img_bgr, (nx, ny), max(3, w // 26), (35, 35, 35), -1, cv.LINE_AA)

            # 5. Óculos Pixel / Thug Life (nos olhos)
            if modo == "oculos pixel" and oculos_px is not None:
                s = oculos_px.quadro_atual()
                esc = (w * 1.05) / s.shape[1]
                sobrepor(img_bgr, s, x + w / 2, y + h * 0.40, esc)

            # 6. Símbolo de raiva (anime) no canto da testa
            if modo == "raiva anime" and raiva is not None:
                s = raiva.quadro_atual()
                esc = (w * 0.38) / s.shape[1]
                sobrepor(img_bgr, s, x + w * 0.85, y + h * 0.10, esc)

        return img_bgr
