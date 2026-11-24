"""
Processamento Gráfico: Fundamentos - Trabalho do Grau B
Versão alternativa com a HighGUI do OpenCV + teclado/console (duas janelas:
imagem e "Controles"). A versão principal é a de main.py / app.py.

Uso:
    python highgui.py                 # começa numa tela escura: O abre foto, V liga a webcam
    python highgui.py caminho/foto.jpg

Pressione H na janela para ver todos os comandos no console.
"""

import os
import sys
import time
import unicodedata

import cv2 as cv
import numpy as np

from filtros import FILTROS
from operacoes import OPERACOES
from processamento import (CANAIS, PASTA_IMAGENS, PASTA_SAIDAS, PASTA_STICKERS,
                           ajustar_tamanho, processar, tela_inicial, ler_imagem,
                           salvar_imagem, carregar_segunda_imagem)
from stickers import DetectorFace, carregar_stickers, sobrepor

JANELA = "Grau B - Efeitos em Imagens e Video"
CONTROLES = "Controles"

AJUDA = """
==================================  COMANDOS  ==================================
 MODOS
   O ............ abrir foto do disco (Modo Foto)
   V ............ iniciar / parar a webcam (Modo Video)
 FILTROS (podem ser combinados: cada um e aplicado sobre o resultado anterior)
   A / D ........ filtro anterior / proximo  (mostra a descricao no console)
   ESPACO ....... liga/desliga a PREVIA do filtro selecionado
   ENTER ou E ... aplicar o filtro selecionado (entra na pilha de filtros)
   Z ............ desfazer o ultimo filtro/operacao aplicado
   Trackbars .... "Param 1" e "Param 2" ajustam os parametros do filtro
 CANAIS
   C ............ alterna o canal: RGB -> Cinza -> R -> G -> B
 OPERACOES ARITMETICAS COM DUAS IMAGENS
   K ............ proxima operacao (mostra a descricao)
   X ............ aplicar a operacao selecionada com a 2a imagem
   I ............ carregar outra 2a imagem do disco
 STICKERS
   T ............ proximo sticker
   Clique esq ... coloca o sticker selecionado na posicao do mouse
   Clique dir ... remove o ultimo sticker colocado
   + / - ........ aumenta / diminui o sticker (ou trackbar "Escala %")
   F ............ stickers automaticos no rosto (deteccao de face Haar)
 GERAL
   S ............ salvar a imagem / frame atual em saidas/
   R ............ resetar (volta a imagem original)
   U ............ mostrar/esconder o painel de informacoes
   H ............ mostrar esta ajuda
   Q ou ESC ..... sair
================================================================================
"""


def sem_acentos(texto):
    """cv.putText não desenha acentos; removemos para o painel da janela."""
    return unicodedata.normalize("NFKD", texto).encode("ascii", "ignore").decode("ascii")


def escolher_arquivo(titulo):
    """Abre um seletor de arquivos (Tkinter); se não der, pergunta no console."""
    caminho = ""
    if sys.platform != "darwin":   # no macOS, Tk + HighGUI no mesmo processo é instável
        try:
            import tkinter as tk
            from tkinter import filedialog
            raiz = tk.Tk()
            raiz.withdraw()
            raiz.attributes("-topmost", True)
            caminho = filedialog.askopenfilename(
                title=titulo,
                filetypes=[("Imagens", "*.png *.jpg *.jpeg *.bmp *.tif *.tiff *.webp"),
                           ("Todos", "*.*")])
            raiz.destroy()
            return caminho
        except Exception:
            pass
    try:
        caminho = input(f"{titulo} - digite o caminho do arquivo: ").strip().strip('"')
    except EOFError:
        caminho = ""
    return caminho


class App:
    def __init__(self, caminho_inicial=None):
        self.modo = "foto"
        self.cap = None
        self.original = None
        self.caminho = ""
        self.pilha = []            # passos aplicados: (tipo, indice, p1, p2)
        self.canal = "RGB"
        self.filtro_sel = 0
        self.op_sel = 2            # começa no blending
        self.previa = False
        self.mostrar_hud = True

        self.stickers = carregar_stickers(PASTA_STICKERS)
        self.sticker_por_nome = {s.nome: s for s in self.stickers}
        self.sticker_sel = 0
        self.colocados = []        # (indice_sticker, x, y, escala)
        self.detector = DetectorFace()
        self.modo_face = 0
        self.rostos_cache = None

        self.img2 = carregar_segunda_imagem()
        self.versao = 0            # muda sempre que a foto original muda (cache)
        self.cache_chave = None
        self.cache_img = None
        self.ultimo_resultado = None
        self.mensagem = ""
        self.mensagem_ate = 0

        cv.namedWindow(JANELA, cv.WINDOW_AUTOSIZE)
        cv.namedWindow(CONTROLES, cv.WINDOW_NORMAL)
        cv.resizeWindow(CONTROLES, 420, 160)
        cv.createTrackbar("Param 1", CONTROLES, 0, 100, lambda v: None)
        cv.createTrackbar("Param 2", CONTROLES, 0, 100, lambda v: None)
        cv.createTrackbar("Escala %", CONTROLES, 100, 300, lambda v: None)
        cv.setMouseCallback(JANELA, self.mouse)

        if not (caminho_inicial and self.carregar_foto(caminho_inicial)):
            self.original = tela_inicial(linhas=("Abra uma foto (tecla O) ou ligue a camera (tecla V)",
                                                 "H mostra a ajuda no console"))
        print(AJUDA)
        self.selecionar_filtro(0)

    # ------------------------------------------------------------------ util
    def avisar(self, texto, segundos=2.5):
        print(">>", texto)
        self.mensagem = sem_acentos(texto)
        self.mensagem_ate = time.time() + segundos

    def params(self):
        return (cv.getTrackbarPos("Param 1", CONTROLES),
                cv.getTrackbarPos("Param 2", CONTROLES))

    def escala_sticker(self):
        return max(10, cv.getTrackbarPos("Escala %", CONTROLES)) / 100.0

    # ----------------------------------------------------------------- modos
    def carregar_foto(self, caminho):
        img = ler_imagem(caminho) if caminho else None
        if img is None:
            self.avisar(f"Nao foi possivel abrir a imagem: {caminho}")
            return False
        self.parar_webcam(manter_frame=False)
        self.original = ajustar_tamanho(img)
        self.versao += 1
        self.caminho = caminho
        self.modo = "foto"
        self.rostos_cache = None
        self.colocados.clear()
        self.avisar(f"Modo Foto: {os.path.basename(caminho)}")
        return True

    def iniciar_webcam(self):
        cap = cv.VideoCapture(0)
        if not cap.isOpened():
            self.avisar("Nenhuma webcam encontrada.")
            return
        cap.set(cv.CAP_PROP_FRAME_WIDTH, 640)
        cap.set(cv.CAP_PROP_FRAME_HEIGHT, 480)
        self.cap = cap
        self.modo = "video"
        self.colocados.clear()
        self.avisar("Modo Video: webcam iniciada (V para parar)")

    def parar_webcam(self, manter_frame=True):
        if self.cap is not None:
            self.cap.release()
            self.cap = None
            self.modo = "foto"
            self.rostos_cache = None
            if manter_frame:
                self.avisar("Webcam parada. O ultimo frame virou a foto atual.")

    # --------------------------------------------------------------- seleção
    def selecionar_filtro(self, indice):
        self.filtro_sel = indice % len(FILTROS)
        f = FILTROS[self.filtro_sel]
        cv.setTrackbarPos("Param 1", CONTROLES, f["p1"])
        cv.setTrackbarPos("Param 2", CONTROLES, f["p2"])
        print(f"\n[FILTRO {self.filtro_sel + 1}/{len(FILTROS)}] {f['nome']}  ({f['tipo']})")
        print(f"  O que faz : {f['descricao']}")
        print(f"  Parametros: {f['params']}")
        print("  -> ESPACO para previa, ENTER para aplicar.")
        self.avisar(f"Filtro: {f['nome']}")

    def selecionar_operacao(self, indice):
        self.op_sel = indice % len(OPERACOES)
        o = OPERACOES[self.op_sel]
        cv.setTrackbarPos("Param 1", CONTROLES, o["p1"])
        print(f"\n[OPERACAO {self.op_sel + 1}/{len(OPERACOES)}] {o['nome']}")
        print(f"  O que faz : {o['descricao']}")
        print(f"  Parametros: {o['params']}")
        print("  -> X para aplicar com a 2a imagem (I para trocar a 2a imagem).")
        self.avisar(f"Operacao: {o['nome']}")

    # ----------------------------------------------------------------- mouse
    def mouse(self, evento, x, y, flags, param):
        if evento == cv.EVENT_LBUTTONDOWN and self.stickers:
            self.colocados.append((self.sticker_sel, x, y, self.escala_sticker()))
            self.avisar(f"Sticker {self.stickers[self.sticker_sel].nome} em ({x}, {y})", 1.2)
        elif evento == cv.EVENT_RBUTTONDOWN and self.colocados:
            self.colocados.pop()
            self.avisar("Ultimo sticker removido", 1.2)

    # ------------------------------------------------------------ renderizar
    def resultado(self, quadro):
        """Imagem final (sem o painel): filtros + stickers + face."""
        p1, p2 = self.params()
        previa = ("filtro", self.filtro_sel, p1, p2) if self.previa else None
        chave = (self.versao, tuple(self.pilha), self.canal, previa)
        if self.modo == "foto" and chave == self.cache_chave:
            base = self.cache_img            # evita reprocessar a foto a cada quadro
        else:
            base = processar(quadro, self.pilha, self.canal, self.img2, previa)
            self.cache_chave, self.cache_img = chave, base
        img = base.copy()

        if self.modo_face:
            if self.modo == "video" or self.rostos_cache is None:
                self.rostos_cache = self.detector.detectar(quadro)
            self.detector.aplicar(img, DetectorFace.MODOS[self.modo_face],
                                  self.sticker_por_nome, self.rostos_cache)
        for (i, x, y, esc) in self.colocados:
            sobrepor(img, self.stickers[i].quadro_atual(), x, y, esc)
        return img

    def painel(self, img):
        tela = img.copy()
        if self.mostrar_hud:
            f = FILTROS[self.filtro_sel]
            o = OPERACOES[self.op_sel]
            p1, p2 = self.params()
            st = self.stickers[self.sticker_sel].nome if self.stickers else "-"
            linhas = [
                f"Modo: {self.modo.upper()}   Canal: {self.canal}   Pilha: {len(self.pilha)} passo(s)",
                f"Filtro [A/D]: {f['nome']}  P1={p1} P2={p2}  {'(PREVIA)' if self.previa else ''}",
                f"Operacao [K]: {o['nome']}   Sticker [T]: {st} x{self.escala_sticker():.1f}",
                f"Face [F]: {DetectorFace.MODOS[self.modo_face]}   H = ajuda no console",
            ]
            altura = 22 * len(linhas) + 10
            faixa = tela[:altura].copy()
            cv.rectangle(faixa, (0, 0), (tela.shape[1], altura), (0, 0, 0), -1)
            tela[:altura] = cv.addWeighted(tela[:altura], 0.4, faixa, 0.6, 0)
            for i, t in enumerate(linhas):
                cv.putText(tela, sem_acentos(t), (8, 22 + 22 * i), cv.FONT_HERSHEY_SIMPLEX,
                           0.5, (255, 255, 255), 1, cv.LINE_AA)
        if time.time() < self.mensagem_ate:
            h = tela.shape[0]
            cv.putText(tela, self.mensagem, (10, h - 14), cv.FONT_HERSHEY_SIMPLEX, 0.6,
                       (0, 0, 0), 3, cv.LINE_AA)
            cv.putText(tela, self.mensagem, (10, h - 14), cv.FONT_HERSHEY_SIMPLEX, 0.6,
                       (0, 255, 255), 1, cv.LINE_AA)
        return tela

    # --------------------------------------------------------------- teclado
    def salvar(self):
        if self.ultimo_resultado is None:
            return
        os.makedirs(PASTA_SAIDAS, exist_ok=True)
        nome = time.strftime("resultado_%Y%m%d_%H%M%S.png")
        caminho = os.path.join(PASTA_SAIDAS, nome)
        if salvar_imagem(caminho, self.ultimo_resultado):
            self.avisar(f"Salvo em saidas/{nome}")
        else:
            self.avisar(f"Erro ao salvar em saidas/{nome}")

    def resetar(self):
        self.pilha.clear()
        self.colocados.clear()
        self.canal = "RGB"
        self.previa = False
        self.modo_face = 0
        self.selecionar_filtro(self.filtro_sel)
        self.avisar("Reset: voltou a imagem original")

    def tecla(self, k):
        if k in (27, ord("q")):
            return False
        c = chr(k).lower() if 0 <= k < 256 else ""
        if c == "h":
            print(AJUDA)
        elif c == "o":
            caminho = escolher_arquivo("Abrir foto")
            if caminho:
                self.carregar_foto(caminho)
        elif c == "v":
            if self.modo == "video":
                self.parar_webcam()
            else:
                self.iniciar_webcam()
        elif c == "d":
            self.selecionar_filtro(self.filtro_sel + 1)
        elif c == "a":
            self.selecionar_filtro(self.filtro_sel - 1)
        elif c == " ":
            self.previa = not self.previa
            self.avisar("Previa " + ("ligada" if self.previa else "desligada"))
        elif k in (13, 10) or c == "e":
            p1, p2 = self.params()
            self.pilha.append(("filtro", self.filtro_sel, p1, p2))
            self.previa = False
            self.avisar(f"Aplicado: {FILTROS[self.filtro_sel]['nome']} (P1={p1}, P2={p2})")
        elif c == "z":
            if self.pilha:
                passo = self.pilha.pop()
                nome = (FILTROS if passo[0] == "filtro" else OPERACOES)[passo[1]]["nome"]
                self.avisar(f"Desfeito: {nome}")
        elif c == "c":
            self.canal = CANAIS[(CANAIS.index(self.canal) + 1) % len(CANAIS)]
            self.avisar(f"Canal: {self.canal}")
        elif c == "k":
            self.selecionar_operacao(self.op_sel + 1)
        elif c == "x":
            if self.img2 is None:
                self.avisar("Carregue a 2a imagem com I")
            else:
                p1, _ = self.params()
                self.pilha.append(("op", self.op_sel, p1, 0))
                self.avisar(f"Aplicado: {OPERACOES[self.op_sel]['nome']} (alpha={p1 / 100:.2f})")
        elif c == "i":
            caminho = escolher_arquivo("Abrir 2a imagem")
            img = ler_imagem(caminho) if caminho else None
            if img is not None:
                self.img2 = img
                self.cache_chave = None
                self.avisar(f"2a imagem: {os.path.basename(caminho)}")
        elif c == "t" and self.stickers:
            self.sticker_sel = (self.sticker_sel + 1) % len(self.stickers)
            s = self.stickers[self.sticker_sel]
            self.avisar(f"Sticker: {s.nome}{' (animado)' if s.animado else ''} - clique para colocar")
        elif c in ("+", "="):
            cv.setTrackbarPos("Escala %", CONTROLES, min(300, int(self.escala_sticker() * 100) + 10))
        elif c in ("-", "_"):
            cv.setTrackbarPos("Escala %", CONTROLES, max(10, int(self.escala_sticker() * 100) - 10))
        elif c == "f":
            self.modo_face = (self.modo_face + 1) % len(DetectorFace.MODOS)
            self.rostos_cache = None
            msg = DetectorFace.MODOS[self.modo_face]
            if self.modo_face and not self.detector.ok:
                msg += " (Haar cascade nao encontrado!)"
            self.avisar(f"Stickers no rosto: {msg}")
        elif c == "s":
            self.salvar()
        elif c == "r":
            self.resetar()
        elif c == "u":
            self.mostrar_hud = not self.mostrar_hud
        return True

    # ---------------------------------------------------------- laço principal
    def executar(self):
        while True:
            if self.modo == "video" and self.cap is not None:
                ok, frame = self.cap.read()
                if not ok:
                    self.avisar("Falha ao ler a webcam.")
                    self.parar_webcam()
                else:
                    self.original = cv.flip(frame, 1)   # efeito espelho
                    self.versao += 1
            self.ultimo_resultado = self.resultado(self.original)
            cv.imshow(JANELA, self.painel(self.ultimo_resultado))
            k = cv.waitKey(30)
            if k != -1 and not self.tecla(k & 0xFF if k not in (13, 10) else k):
                break
            if cv.getWindowProperty(JANELA, cv.WND_PROP_VISIBLE) < 1:
                break
        self.parar_webcam(manter_frame=False)
        cv.destroyAllWindows()


if __name__ == "__main__":
    App(sys.argv[1] if len(sys.argv) > 1 else None).executar()
