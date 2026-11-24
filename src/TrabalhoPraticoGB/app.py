"""
Stories FX - versão "aplicativo" (Tkinter, tema roxo).

Tela pensada para ser simples, como um app de stories:
  * imagem grande no centro;
  * faixa de filtros embaixo, com miniaturas de prévia: toque para ver a
    descrição e a prévia, depois "Aplicar" ou "Cancelar";
  * ícones grandes à esquerda (foto, câmera, stickers, rosto, ajustes,
    misturar, desfazer, resetar, salvar);
  * as opções avançadas (canal, operações, tamanho do sticker...) ficam em
    gavetas que só abrem quando o ícone é tocado.

Usa os mesmos módulos das outras versões (filtros.py, operacoes.py,
stickers.py e processar() de main.py).

Uso:
    python app.py
    python app.py caminho/da/foto.jpg
"""

import os
import sys
import time
import tkinter as tk
from tkinter import filedialog, messagebox

import cv2 as cv
import numpy as np

from filtros import FILTROS
from processamento import (PASTA_IMAGENS, PASTA_SAIDAS, PASTA_STICKERS, ajustar_tamanho,
                           processar, ler_imagem, salvar_imagem, carregar_segunda_imagem)
from operacoes import OPERACOES
from stickers import DetectorFace, carregar_stickers, sobrepor

COR = {
    "fundo": "#170b26",
    "barra": "#24123c",
    "cartao": "#321a52",
    "roxo": "#9b5de5",
    "roxo_claro": "#c39bff",
    "roxo_escuro": "#4a2a7a",
    "texto": "#f6efff",
    "texto2": "#bda6e0",
    "area": "#0f0719",
}
FONTE = "Segoe UI"
MAX_W, MAX_H = 900, 600
MINI_W, MINI_H = 104, 74
TIPOS_IMAGEM = [("Imagens", "*.png *.jpg *.jpeg *.bmp *.tif *.tiff *.webp"), ("Todos", "*.*")]

CANAIS_APP = [("Cor", "RGB"), ("Cinza", "Cinza"), ("Vermelho", "R"), ("Verde", "G"), ("Azul", "B")]
ICONES_ROSTO = {
    "desligado": "🚫  Desligado",
    "oculos pixel": "🕶  Óculos Thug Life",
    "orelhas gato preto": "🐱  Orelhas de gato",
    "bigodes gato": "😺  Bigodes de gato",
    "gato preto completo": "🐾  Gatinho completo",
    "raiva anime": "💢  Raiva (Anime)",
}


# ----------------------------------------------------------------- utilidades
def para_photoimage(img_bgr):
    """BGR do OpenCV -> PhotoImage do Tk (formato PPM, sem precisar do Pillow)."""
    rgb = cv.cvtColor(img_bgr, cv.COLOR_BGR2RGB)
    h, w = rgb.shape[:2]
    try:
        return tk.PhotoImage(data=f"P6 {w} {h} 255 ".encode() + rgb.tobytes(), format="PPM")
    except tk.TclError:
        import base64
        _, png = cv.imencode(".png", img_bgr)
        return tk.PhotoImage(data=base64.b64encode(png.tobytes()))


def hex_bgr(cor):
    cor = cor.lstrip("#")
    r, g, b = (int(cor[i:i + 2], 16) for i in (0, 2, 4))
    return (b, g, r)


def miniatura(img, w=MINI_W, h=MINI_H):
    """Recorta o centro mantendo a proporção e reduz para o tamanho da miniatura."""
    ih, iw = img.shape[:2]
    f = max(w / iw, h / ih)
    r = cv.resize(img, (max(w, int(iw * f)), max(h, int(ih * f))), interpolation=cv.INTER_AREA)
    y0, x0 = (r.shape[0] - h) // 2, (r.shape[1] - w) // 2
    return r[y0:y0 + h, x0:x0 + w].copy()


def miniatura_sticker(bgra, lado=64, fundo=COR["cartao"]):
    h, w = bgra.shape[:2]
    f = (lado - 8) / max(h, w)
    s = cv.resize(bgra, (max(1, int(w * f)), max(1, int(h * f))), interpolation=cv.INTER_AREA)
    base = np.full((lado, lado, 3), hex_bgr(fundo), np.uint8)
    return sobrepor(base, s, lado / 2, lado / 2)


def texto_param(params, n):
    """Extrai 'tamanho do kernel (3 a 31)' de 'Param 1: tamanho do kernel (3 a 31). ...'."""
    chave = f"Param {n}: "
    if chave not in params:
        return None
    t = params.split(chave, 1)[1].split("Param", 1)[0].strip().rstrip(".")
    return None if t.startswith("não usado") else t


class Botao(tk.Label):
    """Botão plano feito com Label (fica roxo também no macOS, onde tk.Button ignora a cor)."""

    def __init__(self, pai, texto, comando, cor=COR["roxo"], cor_hover=COR["roxo_claro"],
                 fg="white", fonte=(FONTE, 11, "bold"), **kw):
        super().__init__(pai, text=texto, bg=cor, fg=fg, font=fonte, cursor="hand2", **kw)
        self.cor, self.cor_hover, self.comando = cor, cor_hover, comando
        self.bind("<Enter>", lambda e: self.config(bg=self.cor_hover))
        self.bind("<Leave>", lambda e: self.config(bg=self.cor))
        self.bind("<Button-1>", lambda e: self.comando())

    def definir_cor(self, cor):
        self.cor = cor
        self.config(bg=cor)


# ------------------------------------------------------------------------ app
class StoriesApp:
    def __init__(self, raiz, caminho_inicial=None):
        self.raiz = raiz
        raiz.title("Stories FX")
        raiz.configure(bg=COR["fundo"])
        try:
            raiz.state("zoomed")   # Abre direto maximizado (tela cheia com barra de título) no Windows
        except Exception:
            raiz.geometry("1280x860")
        raiz.minsize(900, 580)

        # estado de processamento (mesma lógica das outras versões)
        self.modo = "foto"
        self.cap = None
        self.original = None
        self.versao = 0
        self.pilha = []                    # (tipo, indice, p1, p2)
        self.canal = "RGB"
        self.img2 = carregar_segunda_imagem()
        self.stickers = carregar_stickers(PASTA_STICKERS)
        self.sticker_por_nome = {s.nome: s for s in self.stickers}
        self.sticker_sel = None            # sticker "na mão", pronto para colocar
        self.colocados = []
        self.escala = tk.IntVar(value=100)
        self.detector = DetectorFace()
        self.modo_face = "desligado"
        self.rostos_cache = None

        self.filtro_sel = None             # filtro em prévia (ainda não aplicado)
        self.p1 = tk.IntVar()
        self.p2 = tk.IntVar()
        self.op_sel = 2
        self.alpha = tk.IntVar(value=50)

        self.cache_chave = None
        self.cache_img = None
        self.chave_minis = None
        self.ultima_minis = 0
        self.ultimo_resultado = None
        self.foto_tk = None
        self.offset = (0, 0)
        self.escala_tela = 1.0
        self.toast_txt, self.toast_ate = "", 0
        self.gaveta_aberta = None

        self._montar()
        if caminho_inicial:
            self.carregar_foto(caminho_inicial)   # sem foto: tela inicial pedindo foto/câmera
        raiz.protocol("WM_DELETE_WINDOW", self.sair)
        raiz.bind("<Escape>", lambda e: self.cancelar_filtro())
        raiz.bind("<Return>", lambda e: self.aplicar_filtro())
        raiz.bind("<Control-z>", lambda e: self.desfazer())
        raiz.bind("<Control-s>", lambda e: self.salvar())
        self.laco()

    # ============================================================ montagem
    def _montar(self):
        r = self.raiz
        # ---- barra de cima
        topo = tk.Frame(r, bg=COR["barra"], height=54)
        topo.pack(side="top", fill="x")
        tk.Label(topo, text="✦ Stories FX", bg=COR["barra"], fg=COR["roxo_claro"],
                 font=(FONTE, 18, "bold")).pack(side="left", padx=16, pady=8)
        self.pilula = tk.Label(topo, bg=COR["roxo_escuro"], fg="white", font=(FONTE, 10, "bold"),
                               padx=12, pady=4)
        self.pilula.pack(side="left", padx=4)

        # Botão Salvar no topo (em destaque, NUNCA fica cortado em nenhuma tela!)
        Botao(topo, "💾  Salvar resultado", self.salvar, cor=COR["roxo"], cor_hover=COR["roxo_claro"],
              padx=16, pady=6, fonte=(FONTE, 11, "bold")).pack(side="right", padx=16, pady=8)

        self.dica = tk.Label(topo, bg=COR["barra"], fg=COR["texto2"], font=(FONTE, 10))
        self.dica.pack(side="right", padx=10)

        # ---- faixa de filtros (embaixo)
        self.faixa = tk.Frame(r, bg=COR["barra"])
        self.faixa.pack(side="bottom", fill="x")
        tk.Label(self.faixa, text="Filtros  ·  toque em um para ver a prévia", bg=COR["barra"],
                 fg=COR["texto2"], font=(FONTE, 10), anchor="w").pack(fill="x", padx=14, pady=(8, 2))
        rolagem = tk.Canvas(self.faixa, bg=COR["barra"], height=MINI_H + 36, highlightthickness=0)
        barra_h = tk.Scrollbar(self.faixa, orient="horizontal", command=rolagem.xview,
                               bg=COR["roxo"], troughcolor=COR["barra"], bd=0, width=10)
        rolagem.configure(xscrollcommand=barra_h.set)
        rolagem.pack(fill="x", padx=10)
        barra_h.pack(fill="x", padx=10, pady=(0, 6))
        linha = tk.Frame(rolagem, bg=COR["barra"])
        rolagem.create_window((0, 0), window=linha, anchor="nw")
        linha.bind("<Configure>", lambda e: rolagem.configure(scrollregion=rolagem.bbox("all")))
        rolar = lambda e: rolagem.xview_scroll(-1 if (getattr(e, "delta", 0) > 0 or e.num == 4) else 1, "units")
        self.mini_labels = []
        self.mini_fotos = [None] * len(FILTROS)
        for i, f in enumerate(FILTROS):
            caixa = tk.Frame(linha, bg=COR["barra"], padx=3, pady=3, cursor="hand2")
            caixa.pack(side="left", padx=3)
            img = tk.Label(caixa, bg=COR["barra"], bd=0, highlightthickness=3,
                           highlightbackground=COR["barra"])
            img.pack()
            exclusivo = f["tipo"] == "Efeito exclusivo"
            nome = tk.Label(caixa, text=f["nome"].split(" (")[0], bg=COR["barra"],
                            fg=COR["roxo_claro"] if exclusivo else COR["texto"],
                            font=(FONTE, 9, "bold" if exclusivo else "normal"))
            nome.pack()
            for w in (caixa, img, nome):
                w.bind("<Button-1>", lambda e, i=i: self.escolher_filtro(i))
                w.bind("<MouseWheel>", rolar)
                w.bind("<Button-4>", rolar)
                w.bind("<Button-5>", rolar)
            self.mini_labels.append(img)

        # ---- cartão do filtro (aparece acima da faixa quando um filtro é tocado)
        self.cartao = tk.Frame(r, bg=COR["cartao"], padx=16, pady=10)
        esq = tk.Frame(self.cartao, bg=COR["cartao"])
        esq.pack(side="left", fill="both", expand=True)
        self.c_titulo = tk.Label(esq, bg=COR["cartao"], fg="white", font=(FONTE, 14, "bold"), anchor="w")
        self.c_titulo.pack(fill="x")
        self.c_desc = tk.Label(esq, bg=COR["cartao"], fg=COR["texto"], font=(FONTE, 10),
                               anchor="w", justify="left", wraplength=560)
        self.c_desc.pack(fill="x", pady=(2, 4))
        self.c_sl1 = self._slider(esq, self.p1)
        self.c_sl2 = self._slider(esq, self.p2)
        dir_ = tk.Frame(self.cartao, bg=COR["cartao"])
        dir_.pack(side="right", padx=(16, 0))
        Botao(dir_, "✓  Aplicar", self.aplicar_filtro, padx=26, pady=10,
              fonte=(FONTE, 13, "bold")).pack(fill="x")
        Botao(dir_, "✕  Cancelar", self.cancelar_filtro, cor=COR["cartao"],
              cor_hover=COR["roxo_escuro"], fg=COR["texto2"], padx=26, pady=6,
              fonte=(FONTE, 11)).pack(fill="x", pady=(6, 0))

        # ---- miolo: ícones à esquerda, imagem, gaveta à direita
        self.miolo = tk.Frame(r, bg=COR["fundo"])
        self.miolo.pack(side="top", fill="both", expand=True)

        icones = tk.Frame(self.miolo, bg=COR["barra"], padx=6, pady=4)
        icones.pack(side="left", fill="y")
        self.botoes_icone = {}
        for chave, emoji, texto, cmd in (
                ("foto", "🖼", "Foto", self.abrir_foto),
                ("camera", "🎥", "Câmera", self.alternar_webcam),
                ("stickers", "⭐", "Stickers", lambda: self.alternar_gaveta("stickers")),
                ("rosto", "😺", "Rosto", lambda: self.alternar_gaveta("rosto")),
                ("ajustes", "🎨", "Cor/Canal", lambda: self.alternar_gaveta("ajustes")),
                ("misturar", "➕", "Misturar", lambda: self.alternar_gaveta("misturar")),
                ("desfazer", "↶", "Desfazer", self.desfazer),
                ("resetar", "⟲", "Resetar", self.resetar),
                ("salvar", "💾", "Salvar", self.salvar)):
            b = Botao(icones, f"{emoji}\n{texto}", cmd, cor=COR["barra"], cor_hover=COR["roxo_escuro"],
                      fg=COR["texto"], fonte=(FONTE, 9, "bold"), width=8, pady=3)
            b.pack(pady=1)
            self.botoes_icone[chave] = b
        self.botoes_icone["salvar"].definir_cor(COR["roxo"])

        self.tela = tk.Canvas(self.miolo, bg=COR["area"], highlightthickness=0, cursor="crosshair")
        self.tela.pack(side="left", fill="both", expand=True)
        self.tela.bind("<Button-1>", self.toque)
        self.tela.bind("<Button-3>", lambda e: self.remover_sticker())
        self.tela.bind("<Button-2>", lambda e: self.remover_sticker())

        self.gavetas = {nome: self._gaveta(nome) for nome in ("stickers", "rosto", "ajustes", "misturar")}

        # ---- tela inicial (enquanto não há foto nem câmera)
        self.vazio = tk.Frame(self.tela, bg=COR["area"])
        tk.Label(self.vazio, text="✦", bg=COR["area"], fg=COR["roxo"],
                 font=(FONTE, 40)).pack()
        tk.Label(self.vazio, text="Vamos começar?", bg=COR["area"], fg="white",
                 font=(FONTE, 22, "bold")).pack()
        tk.Label(self.vazio, text="Abra uma foto do computador ou ligue a câmera.", bg=COR["area"],
                 fg=COR["texto2"], font=(FONTE, 12)).pack(pady=(4, 18))
        botoes = tk.Frame(self.vazio, bg=COR["area"])
        botoes.pack()
        Botao(botoes, "🖼  Abrir foto", self.abrir_foto, padx=24, pady=12,
              fonte=(FONTE, 13, "bold")).pack(side="left", padx=8)
        Botao(botoes, "🎥  Ligar câmera", self.alternar_webcam, cor=COR["roxo_escuro"], padx=24, pady=12,
              fonte=(FONTE, 13, "bold")).pack(side="left", padx=8)

    def _slider(self, pai, var, de=0, ate=100):
        linha = tk.Frame(pai, bg=pai["bg"])
        rotulo = tk.Label(linha, bg=pai["bg"], fg=COR["texto2"], font=(FONTE, 10), anchor="w", width=30)
        rotulo.pack(side="left")
        tk.Scale(linha, variable=var, from_=de, to=ate, orient="horizontal", showvalue=True,
                 bg=pai["bg"], fg=COR["texto"], troughcolor=COR["fundo"], activebackground=COR["roxo_claro"],
                 highlightthickness=0, bd=0, sliderrelief="flat", length=260,
                 font=(FONTE, 9)).pack(side="left", fill="x", expand=True)
        linha.rotulo = rotulo
        return linha

    def _titulo(self, pai, texto, sub=""):
        tk.Label(pai, text=texto, bg=COR["cartao"], fg="white", font=(FONTE, 15, "bold"),
                 anchor="w").pack(fill="x")
        if sub:
            tk.Label(pai, text=sub, bg=COR["cartao"], fg=COR["texto2"], font=(FONTE, 10),
                     anchor="w", justify="left", wraplength=250).pack(fill="x", pady=(0, 10))

    def _gaveta(self, nome):
        g = tk.Frame(self.miolo, bg=COR["cartao"], padx=14, pady=12, width=290)
        g.pack_propagate(False)
        Botao(g, "✕", lambda: self.alternar_gaveta(None), cor=COR["cartao"],
              cor_hover=COR["roxo_escuro"], fg=COR["texto2"], padx=8).place(relx=1.0, x=0, y=0, anchor="ne")

        if nome == "stickers":
            self._titulo(g, "Stickers", "Escolha um sticker e toque na imagem para colocar. "
                                        "Botão direito remove o último.")

            # Controles inferiores (fixados na parte inferior da gaveta para NUNCA sumirem)
            Botao(g, "Remover último sticker", self.remover_sticker, cor=COR["roxo_escuro"],
                  pady=7).pack(side="bottom", fill="x", pady=(6, 0))
            tk.Scale(g, variable=self.escala, from_=20, to=300, orient="horizontal", bg=COR["cartao"],
                     fg=COR["texto"], troughcolor=COR["fundo"], highlightthickness=0, bd=0,
                     activebackground=COR["roxo_claro"]).pack(side="bottom", fill="x")
            tk.Label(g, text="Tamanho do sticker (%)", bg=COR["cartao"], fg=COR["texto"],
                     font=(FONTE, 10, "bold"), anchor="w").pack(side="bottom", fill="x", pady=(6, 0))
            self.lbl_st_nome = tk.Label(g, bg=COR["cartao"], fg=COR["texto2"], font=(FONTE, 10), anchor="w")
            self.lbl_st_nome.pack(side="bottom", fill="x", pady=(2, 0))

            # Área de stickers rolável preenchendo o espaço superior restante
            area_rol = tk.Frame(g, bg=COR["cartao"])
            area_rol.pack(side="top", fill="both", expand=True, pady=(6, 4))
            cv_st = tk.Canvas(area_rol, bg=COR["cartao"], highlightthickness=0)
            sb = tk.Scrollbar(area_rol, orient="vertical", command=cv_st.yview,
                              bg=COR["roxo"], troughcolor=COR["cartao"], bd=0, width=8)
            cv_st.configure(yscrollcommand=sb.set)
            sb.pack(side="right", fill="y")
            cv_st.pack(side="left", fill="both", expand=True)

            grade = tk.Frame(cv_st, bg=COR["cartao"])
            cv_st.create_window((0, 0), window=grade, anchor="nw")
            grade.bind("<Configure>", lambda e: cv_st.configure(scrollregion=cv_st.bbox("all")))
            rolar = lambda e: cv_st.yview_scroll(-1 if (getattr(e, "delta", 0) > 0 or e.num == 4) else 1, "units")
            cv_st.bind("<MouseWheel>", rolar)
            grade.bind("<MouseWheel>", rolar)

            self.mini_st, self.lbl_st = [], []
            for i, st in enumerate(self.stickers):
                foto = para_photoimage(miniatura_sticker(st.quadros[0]))
                self.mini_st.append(foto)
                lbl = tk.Label(grade, image=foto, bg=COR["cartao"], highlightthickness=3,
                               highlightbackground=COR["cartao"], cursor="hand2")
                lbl.grid(row=i // 3, column=i % 3, padx=4, pady=4)
                lbl.bind("<Button-1>", lambda e, i=i: self.escolher_sticker(i))
                lbl.bind("<MouseWheel>", rolar)
                self.lbl_st.append(lbl)

        elif nome == "rosto":
            self._titulo(g, "Rosto", "Detecta rostos (Haar Cascade) e coloca os stickers sozinho. "
                                     "Funciona na foto e na câmera.")
            area_rosto = tk.Frame(g, bg=COR["cartao"])
            area_rosto.pack(fill="both", expand=True)
            cv_r = tk.Canvas(area_rosto, bg=COR["cartao"], highlightthickness=0, height=450)
            sb_r = tk.Scrollbar(area_rosto, orient="vertical", command=cv_r.yview,
                                bg=COR["roxo"], troughcolor=COR["cartao"], bd=0, width=8)
            cv_r.configure(yscrollcommand=sb_r.set)
            sb_r.pack(side="right", fill="y")
            cv_r.pack(side="left", fill="both", expand=True)

            lista_r = tk.Frame(cv_r, bg=COR["cartao"])
            cv_r.create_window((0, 0), window=lista_r, anchor="nw")
            lista_r.bind("<Configure>", lambda e: cv_r.configure(scrollregion=cv_r.bbox("all")))
            rolar_r = lambda e: cv_r.yview_scroll(-1 if (getattr(e, "delta", 0) > 0 or e.num == 4) else 1, "units")
            cv_r.bind("<MouseWheel>", rolar_r)
            lista_r.bind("<MouseWheel>", rolar_r)

            self.bt_rosto = {}
            for modo in DetectorFace.MODOS:
                b = Botao(lista_r, ICONES_ROSTO.get(modo, modo), lambda m=modo: self.escolher_rosto(m),
                          cor=COR["roxo_escuro"], fonte=(FONTE, 11, "bold"), anchor="w", padx=12, pady=7)
                b.pack(fill="x", pady=2)
                b.bind("<MouseWheel>", rolar_r)
                self.bt_rosto[modo] = b

        elif nome == "ajustes":
            self._titulo(g, "Cor / Canal", "Escolha onde os filtros agem: a imagem colorida, em "
                                           "tons de cinza ou só um canal (R, G ou B).")
            self.bt_canal = {}
            for texto, canal in CANAIS_APP:
                b = Botao(g, texto, lambda c=canal: self.escolher_canal(c), cor=COR["roxo_escuro"],
                          pady=7)
                b.pack(fill="x", pady=2)
                self.bt_canal[canal] = b
            tk.Label(g, text="Filtros já aplicados", bg=COR["cartao"], fg="white",
                     font=(FONTE, 12, "bold"), anchor="w").pack(fill="x", pady=(14, 4))
            self.lista = tk.Label(g, bg=COR["fundo"], fg=COR["texto"], font=(FONTE, 10), anchor="nw",
                                  justify="left", padx=8, pady=6, height=7)
            self.lista.pack(fill="x")

        elif nome == "misturar":
            self._titulo(g, "Misturar imagens", "Operações aritméticas entre a imagem atual e uma 2ª imagem.")

            # Controles de baixo (sempre visíveis!)
            Botao(g, "✓  Aplicar mistura", self.aplicar_operacao, pady=9,
                  fonte=(FONTE, 12, "bold")).pack(side="bottom", fill="x", pady=(8, 0))

            lin = tk.Frame(g, bg=COR["cartao"])
            lin.pack(side="bottom", fill="x", pady=(6, 0))
            self.mini_img2 = tk.Label(lin, bg=COR["cartao"])
            self.mini_img2.pack(side="left")
            Botao(lin, "Trocar\n2ª imagem", self.abrir_segunda, cor=COR["roxo_escuro"], padx=8,
                  pady=4, fonte=(FONTE, 9, "bold")).pack(side="right")

            self.sl_alpha = tk.Scale(g, variable=self.alpha, from_=0, to=100, orient="horizontal",
                                     label="Intensidade / Alpha (%)", bg=COR["cartao"], fg=COR["texto"],
                                     troughcolor=COR["fundo"], highlightthickness=0, bd=0,
                                     activebackground=COR["roxo_claro"])
            self.sl_alpha.pack(side="bottom", fill="x", pady=(4, 0))

            self.lbl_op = tk.Label(g, bg=COR["fundo"], fg=COR["texto"], font=(FONTE, 9), anchor="w",
                                   justify="left", wraplength=250, padx=8, pady=6)
            self.lbl_op.pack(side="bottom", fill="x", pady=(6, 4))

            # Botões das 5 operações no topo da gaveta
            self.bt_op = []
            for i, o in enumerate(OPERACOES):
                b = Botao(g, o["nome"], lambda i=i: self.escolher_operacao(i), cor=COR["roxo_escuro"],
                          pady=5)
                b.pack(fill="x", pady=2)
                self.bt_op.append(b)
        return g

    # ============================================================ gavetas
    def alternar_gaveta(self, nome):
        self.cache_chave = None
        if self.gaveta_aberta:
            self.gavetas[self.gaveta_aberta].pack_forget()
            self.botoes_icone[self.gaveta_aberta].definir_cor(COR["barra"])
        if nome is None or nome == self.gaveta_aberta:
            self.gaveta_aberta = None
            if nome is None or self.sticker_sel is not None:
                pass
            return
        self.gaveta_aberta = nome
        self.gavetas[nome].pack(side="right", fill="y", before=self.tela)
        self.botoes_icone[nome].definir_cor(COR["roxo_escuro"])
        if nome == "stickers" and self.sticker_sel is None and self.stickers:
            self.escolher_sticker(0)
        if nome == "misturar":
            self.escolher_operacao(self.op_sel)
            self._mini_img2()
        if nome == "ajustes":
            self._atualizar_botoes()
        if nome == "rosto":
            self._atualizar_botoes()

    def _atualizar_botoes(self):
        if hasattr(self, "bt_canal"):
            for c, b in self.bt_canal.items():
                b.definir_cor(COR["roxo"] if c == self.canal else COR["roxo_escuro"])
        if hasattr(self, "bt_rosto"):
            for m, b in self.bt_rosto.items():
                b.definir_cor(COR["roxo"] if m == self.modo_face else COR["roxo_escuro"])
        if hasattr(self, "bt_op"):
            for i, b in enumerate(self.bt_op):
                b.definir_cor(COR["roxo"] if i == self.op_sel else COR["roxo_escuro"])
        if hasattr(self, "lista"):
            linhas = []
            for n, (tipo, i, p1, p2) in enumerate(self.pilha, 1):
                nome = FILTROS[i]["nome"] if tipo == "filtro" else "Mistura: " + OPERACOES[i]["nome"]
                linhas.append(f"{n}. {nome}")
            self.lista.config(text="\n".join(linhas[-7:]) if linhas else "(nenhum ainda)")

    # ============================================================ ações
    def toast(self, texto, seg=2.2):
        print(">>", texto)
        self.toast_txt, self.toast_ate = texto, time.time() + seg

    def carregar_foto(self, caminho):
        img = ler_imagem(caminho) if caminho else None
        if img is None:
            self.toast("Não foi possível abrir a imagem.")
            return False
        self.parar_webcam(silencioso=True)
        self.original = ajustar_tamanho(img, MAX_W, MAX_H)
        self.versao += 1
        self.rostos_cache = None
        self.colocados.clear()
        self.toast(f"Foto aberta: {os.path.basename(caminho)}")
        return True

    def abrir_foto(self):
        caminho = filedialog.askopenfilename(title="Escolher foto", filetypes=TIPOS_IMAGEM)
        if caminho:
            self.carregar_foto(caminho)

    def abrir_segunda(self):
        caminho = filedialog.askopenfilename(title="Escolher 2ª imagem", filetypes=TIPOS_IMAGEM)
        img = ler_imagem(caminho) if caminho else None
        if img is not None:
            self.img2 = img
            self.cache_chave = None
            self._mini_img2()

    def _mini_img2(self):
        if self.img2 is not None:
            self.foto_img2 = para_photoimage(miniatura(self.img2, 120, 80))
            self.mini_img2.config(image=self.foto_img2)

    def alternar_webcam(self):
        if self.modo == "video":
            self.parar_webcam()
            return
        cap = cv.VideoCapture(0)
        if not cap.isOpened():
            messagebox.showwarning("Câmera", "Nenhuma webcam encontrada.")
            return
        cap.set(cv.CAP_PROP_FRAME_WIDTH, 640)
        cap.set(cv.CAP_PROP_FRAME_HEIGHT, 480)
        self.cap = cap
        self.modo = "video"
        self.colocados.clear()
        self.botoes_icone["camera"].config(text="⏹\nParar")
        self.toast("Câmera ligada")

    def parar_webcam(self, silencioso=False):
        if self.cap is not None:
            self.cap.release()
            self.cap = None
            self.modo = "foto"
            self.rostos_cache = None
            self.botoes_icone["camera"].config(text="🎥\nCâmera")
            if not silencioso:
                self.toast("Câmera parada: a última imagem virou a foto")

    def escolher_filtro(self, i):
        if self.original is None:
            self.toast("Abra uma foto ou ligue a câmera primeiro")
            return
        f = FILTROS[i]
        self.filtro_sel = i
        self.p1.set(f["p1"])
        self.p2.set(f["p2"])
        self.c_titulo.config(text=f"{f['nome']}   ·   {f['tipo']}")
        self.c_desc.config(text=f["descricao"])
        for n, (sl, var) in enumerate(((self.c_sl1, self.p1), (self.c_sl2, self.p2)), 1):
            t = texto_param(f["params"], n)
            if t:
                sl.rotulo.config(text=t[0].upper() + t[1:])
                sl.pack(fill="x")
            else:
                sl.pack_forget()
        if not texto_param(f["params"], 1):
            self.c_desc.config(text=f["descricao"] + "\n(Este filtro não tem parâmetros.)")
        self.cartao.pack(side="bottom", fill="x", before=self.miolo)
        for j, lbl in enumerate(self.mini_labels):
            lbl.config(highlightbackground=COR["roxo_claro"] if j == i else COR["barra"])
        print(f"\n[FILTRO] {f['nome']} ({f['tipo']})\n  {f['descricao']}\n  {f['params']}")

    def aplicar_filtro(self):
        if self.filtro_sel is None:
            return
        self.pilha.append(("filtro", self.filtro_sel, self.p1.get(), self.p2.get()))
        self.toast(f"✓ {FILTROS[self.filtro_sel]['nome']} aplicado")
        self.cancelar_filtro()

    def cancelar_filtro(self):
        self.filtro_sel = None
        self.cartao.pack_forget()
        for lbl in self.mini_labels:
            lbl.config(highlightbackground=COR["barra"])
        self._atualizar_botoes()

    def escolher_operacao(self, i):
        self.op_sel = i
        o = OPERACOES[i]
        self.lbl_op.config(text=f"{o['descricao']}\n{o['params']}")
        self.cache_chave = None
        self._atualizar_botoes()

    def aplicar_operacao(self):
        if self.img2 is None:
            self.toast("Escolha a 2ª imagem primeiro")
            return
        self.pilha.append(("op", self.op_sel, self.alpha.get(), 0))
        self.cache_chave = None
        self._atualizar_botoes()
        self.toast(f"✓ {OPERACOES[self.op_sel]['nome']} aplicada")

    def escolher_canal(self, canal):
        self.canal = canal
        self._atualizar_botoes()
        self.toast(f"Canal: {dict((c, t) for t, c in CANAIS_APP)[canal]}")

    def escolher_sticker(self, i):
        self.sticker_sel = i
        for j, lbl in enumerate(self.lbl_st):
            lbl.config(highlightbackground=COR["roxo_claro"] if j == i else COR["cartao"])
        s = self.stickers[i]
        self.lbl_st_nome.config(text=s.nome.split("_", 1)[1].replace("_", " ")
                                + ("  (animado)" if s.animado else ""))

    def escolher_rosto(self, modo):
        if modo != "desligado" and not self.detector.ok:
            messagebox.showwarning("Rosto", "Detecção de face indisponível. Instale opencv-python < 5.")
            return
        self.modo_face = modo
        self.rostos_cache = None
        self._atualizar_botoes()

    def toque(self, e):
        if self.sticker_sel is None or self.gaveta_aberta != "stickers" or self.original is None:
            if self.gaveta_aberta != "stickers":
                self.toast("Abra ⭐ Stickers e escolha um para colocar")
            return
        x = int((e.x - self.offset[0]) / self.escala_tela)
        y = int((e.y - self.offset[1]) / self.escala_tela)
        h, w = self.original.shape[:2]
        if 0 <= x < w and 0 <= y < h:
            self.colocados.append((self.sticker_sel, x, y, self.escala.get() / 100.0))

    def remover_sticker(self):
        if self.colocados:
            self.colocados.pop()
            self.toast("Sticker removido")

    def desfazer(self):
        if self.pilha:
            tipo, i, _, _ = self.pilha.pop()
            self.toast(f"↶ Desfeito: {(FILTROS if tipo == 'filtro' else OPERACOES)[i]['nome']}")
            self._atualizar_botoes()
        elif self.colocados:
            self.remover_sticker()

    def resetar(self):
        self.pilha.clear()
        self.colocados.clear()
        self.canal = "RGB"
        self.modo_face = "desligado"
        self.cancelar_filtro()
        self.toast("⟲ Tudo limpo: imagem original")

    def salvar(self):
        if self.ultimo_resultado is None:
            return
        os.makedirs(PASTA_SAIDAS, exist_ok=True)
        nome = time.strftime("resultado_%Y%m%d_%H%M%S.png")
        caminho = os.path.join(PASTA_SAIDAS, nome)
        if salvar_imagem(caminho, self.ultimo_resultado):
            self.toast(f"💾 Salvo em saidas/{nome}", 3)
        else:
            self.toast("Erro ao salvar imagem.")

    def sair(self):
        self.parar_webcam(silencioso=True)
        self.raiz.destroy()

    # ============================================================ desenho
    def resultado(self):
        if self.original is None:
            return None
        previa = None
        if self.filtro_sel is not None:
            previa = ("filtro", self.filtro_sel, self.p1.get(), self.p2.get())
        elif self.gaveta_aberta == "misturar" and self.img2 is not None:
            previa = ("op", self.op_sel, self.alpha.get(), 0)

        chave = (self.versao, tuple(self.pilha), self.canal, previa)
        if self.modo == "foto" and chave == self.cache_chave:
            base = self.cache_img
        else:
            base = processar(self.original, self.pilha, self.canal, self.img2, previa)
            self.cache_chave, self.cache_img = chave, base
        img = base.copy()
        if self.modo_face != "desligado":
            if self.modo == "video" or self.rostos_cache is None:
                self.rostos_cache = self.detector.detectar(self.original)
            self.detector.aplicar(img, self.modo_face, self.sticker_por_nome, self.rostos_cache)
        for (i, x, y, esc) in self.colocados:
            sobrepor(img, self.stickers[i].quadro_atual(), x, y, esc)
        return img

    def atualizar_miniaturas(self):
        """Prévia de cada filtro aplicada sobre a imagem atual (pequena)."""
        chave = (self.versao, tuple(self.pilha), self.canal)
        agora = time.time()
        if self.original is None:
            if self.chave_minis != "vazio":
                self.chave_minis = "vazio"
                vazia = np.full((MINI_H, MINI_W, 3), hex_bgr(COR["cartao"]), np.uint8)
                for i in range(len(FILTROS)):
                    self.mini_fotos[i] = para_photoimage(vazia)
                    self.mini_labels[i].config(image=self.mini_fotos[i])
            return
        if chave == self.chave_minis or (self.modo == "video" and agora - self.ultima_minis < 1.0):
            return
        self.chave_minis, self.ultima_minis = chave, agora
        pequena = miniatura(self.original)
        base = processar(pequena, self.pilha, self.canal, self.img2)
        for i, f in enumerate(FILTROS):
            try:
                m = processar(base, [("filtro", i, f["p1"], f["p2"])], "RGB", self.img2)
            except cv.error:
                m = base
            self.mini_fotos[i] = para_photoimage(m)
            self.mini_labels[i].config(image=self.mini_fotos[i])

    def laco(self):
        if self.modo == "video" and self.cap is not None:
            ok, frame = self.cap.read()
            if ok:
                self.original = cv.flip(frame, 1)
                self.versao += 1
            else:
                self.parar_webcam()
        try:
            self.ultimo_resultado = self.resultado()
            self.atualizar_miniaturas()
            if self.ultimo_resultado is None:          # tela inicial
                cw, ch = max(1, self.tela.winfo_width()), max(1, self.tela.winfo_height())
                self.tela.delete("all")
                self.tela.create_window(cw // 2, ch // 2, window=self.vazio)
                if time.time() < self.toast_ate:
                    self._etiqueta(cw // 2, ch - 24, self.toast_txt, "s")
                raise StopIteration
            # ajusta a imagem ao espaço disponível
            cw, ch = max(1, self.tela.winfo_width()), max(1, self.tela.winfo_height())
            h, w = self.ultimo_resultado.shape[:2]
            self.escala_tela = min((cw - 20) / w, (ch - 20) / h, 1.6)
            self.escala_tela = max(self.escala_tela, 0.1)
            mostrar = self.ultimo_resultado
            if abs(self.escala_tela - 1.0) > 0.01:
                mostrar = cv.resize(mostrar, (int(w * self.escala_tela), int(h * self.escala_tela)),
                                    interpolation=cv.INTER_AREA if self.escala_tela < 1 else cv.INTER_LINEAR)
            self.foto_tk = para_photoimage(mostrar)
            self.offset = ((cw - mostrar.shape[1]) // 2, (ch - mostrar.shape[0]) // 2)
            self.tela.delete("all")
            self.tela.create_image(*self.offset, image=self.foto_tk, anchor="nw")
            if self.filtro_sel is not None or (self.gaveta_aberta == "misturar" and self.img2 is not None):
                self._etiqueta(self.offset[0] + 12, self.offset[1] + 12, "PRÉVIA", "nw")
            if time.time() < self.toast_ate:
                self._etiqueta(cw // 2, ch - 24, self.toast_txt, "s")
        except StopIteration:
            pass
        except Exception as erro:
            print("Erro:", erro)
        self.pilula.config(text="🔴  AO VIVO" if self.modo == "video" else
                           ("🖼  FOTO" if self.original is not None else "SEM FOTO"))
        if self.original is None:
            dica = "Comece abrindo uma foto ou ligando a câmera"
        elif self.gaveta_aberta == "stickers":
            dica = "Toque na imagem para colar o sticker"
        else:
            dica = "Escolha um filtro na faixa de baixo"
        self.dica.config(text=dica)
        self.raiz.after(30, self.laco)

    def _etiqueta(self, x, y, texto, ancora):
        t = self.tela.create_text(x, y, text=texto, fill="white", font=(FONTE, 11, "bold"), anchor=ancora)
        x0, y0, x1, y1 = self.tela.bbox(t)
        r = self.tela.create_rectangle(x0 - 10, y0 - 5, x1 + 10, y1 + 5, fill=COR["roxo_escuro"],
                                       outline=COR["roxo_claro"])
        self.tela.tag_raise(t, r)


def main():
    raiz = tk.Tk()
    StoriesApp(raiz, sys.argv[1] if len(sys.argv) > 1 else None)
    raiz.mainloop()


if __name__ == "__main__":
    main()
