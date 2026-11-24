"""
Interface gráfica (Tkinter) do Trabalho do Grau B - tema roxo.

Usa os mesmos módulos da versão HighGUI (filtros.py, operacoes.py,
stickers.py e a função processar() de main.py), só troca a interface:
menus, botões, listas, sliders e uma área de imagem clicável.

Uso:
    python gui.py
    python gui.py caminho/da/foto.jpg
"""

import os
import sys
import time
import tkinter as tk
from tkinter import filedialog, messagebox, ttk

import cv2 as cv
import numpy as np

from filtros import FILTROS
from processamento import (CANAIS, PASTA_IMAGENS, PASTA_SAIDAS, PASTA_STICKERS, ajustar_tamanho,
                           processar, tela_inicial, ler_imagem, salvar_imagem, carregar_segunda_imagem)
from operacoes import OPERACOES
from stickers import DetectorFace, carregar_stickers, sobrepor

# ----------------------------------------------------------------- tema roxo
COR = {
    "fundo": "#1e1030",        # fundo da janela
    "painel": "#2d1a47",       # painel lateral
    "cartao": "#3a2260",       # caixas de cada seção
    "destaque": "#9b5de5",     # roxo principal (botões)
    "destaque2": "#b98af5",    # roxo claro (hover / seleção)
    "escuro": "#6a35b8",       # roxo pressionado
    "texto": "#f3eaff",
    "texto2": "#c9b3ec",       # texto secundário
    "area": "#140a22",         # fundo da área da imagem
}
FONTE = ("Segoe UI", 10)
FONTE_B = ("Segoe UI", 10, "bold")
FONTE_T = ("Segoe UI", 12, "bold")

MAX_W, MAX_H = 960, 680
TIPOS_IMAGEM = [("Imagens", "*.png *.jpg *.jpeg *.bmp *.tif *.tiff *.webp"), ("Todos", "*.*")]


def para_photoimage(img_bgr):
    """Converte uma imagem BGR do OpenCV em PhotoImage do Tk (sem precisar do Pillow)."""
    rgb = cv.cvtColor(img_bgr, cv.COLOR_BGR2RGB)
    h, w = rgb.shape[:2]
    ppm = f"P6 {w} {h} 255 ".encode() + rgb.tobytes()
    try:
        return tk.PhotoImage(data=ppm, format="PPM")
    except tk.TclError:   # alternativa: PNG em base64
        ok, png = cv.imencode(".png", img_bgr, [cv.IMWRITE_PNG_COMPRESSION, 0])
        import base64
        return tk.PhotoImage(data=base64.b64encode(png.tobytes()))


def miniatura_sticker(sticker_bgra, lado=46):
    """Miniatura do sticker sobre o fundo roxo do painel (para os botões)."""
    h, w = sticker_bgra.shape[:2]
    f = lado / max(h, w)
    s = cv.resize(sticker_bgra, (max(1, int(w * f)), max(1, int(h * f))), interpolation=cv.INTER_AREA)
    cor = COR["cartao"].lstrip("#")
    r, g, b = (int(cor[i:i + 2], 16) for i in (0, 2, 4))
    fundo = np.full((lado, lado, 3), (b, g, r), np.uint8)
    return sobrepor(fundo, s, lado / 2, lado / 2)


class PainelRolavel(tk.Frame):
    """Frame com barra de rolagem (o painel lateral pode ser maior que a tela)."""

    def __init__(self, pai, largura):
        super().__init__(pai, bg=COR["painel"])
        self.canvas = tk.Canvas(self, bg=COR["painel"], highlightthickness=0, width=largura)
        barra = ttk.Scrollbar(self, orient="vertical", command=self.canvas.yview,
                              style="Roxo.Vertical.TScrollbar")
        self.interno = tk.Frame(self.canvas, bg=COR["painel"])
        self.interno.bind("<Configure>",
                          lambda e: self.canvas.configure(scrollregion=self.canvas.bbox("all")))
        self.canvas.create_window((0, 0), window=self.interno, anchor="nw", width=largura)
        self.canvas.configure(yscrollcommand=barra.set)
        self.canvas.pack(side="left", fill="both", expand=True)
        barra.pack(side="right", fill="y")
        # rolagem com a roda do mouse só quando o mouse está sobre o painel
        self.interno.bind("<Enter>", lambda e: self._roda(True))
        self.interno.bind("<Leave>", lambda e: self._roda(False))

    def _roda(self, ligar):
        if ligar:
            self.canvas.bind_all("<MouseWheel>", self._rolar)
            self.canvas.bind_all("<Button-4>", lambda e: self.canvas.yview_scroll(-2, "units"))
            self.canvas.bind_all("<Button-5>", lambda e: self.canvas.yview_scroll(2, "units"))
        else:
            for ev in ("<MouseWheel>", "<Button-4>", "<Button-5>"):
                self.canvas.unbind_all(ev)

    def _rolar(self, e):
        passo = -1 * (e.delta // 120 if abs(e.delta) >= 120 else e.delta)
        self.canvas.yview_scroll(passo, "units")


class AppGUI:
    def __init__(self, raiz, caminho_inicial=None):
        self.raiz = raiz
        raiz.title("Stories FX - Grau B (Processamento Gráfico)")
        raiz.configure(bg=COR["fundo"])
        raiz.minsize(1000, 620)
        raiz.geometry("1320x800")

        # ---------------- estado (mesma lógica da versão HighGUI)
        self.modo = "foto"
        self.cap = None
        self.original = None
        self.versao = 0
        self.pilha = []                       # (tipo, indice, p1, p2)
        self.stickers = carregar_stickers(PASTA_STICKERS)
        self.sticker_por_nome = {s.nome: s for s in self.stickers}
        self.colocados = []                   # (indice_sticker, x, y, escala)
        self.detector = DetectorFace()
        self.rostos_cache = None
        self.img2 = carregar_segunda_imagem()
        self.cache_chave = None
        self.cache_img = None
        self.ultimo_resultado = None
        self.foto_tk = None
        self.offset = (0, 0)

        # ---------------- variáveis da interface
        self.v_filtro = tk.IntVar(value=0)
        self.v_p1 = tk.IntVar(value=FILTROS[0]["p1"])
        self.v_p2 = tk.IntVar(value=FILTROS[0]["p2"])
        self.v_previa = tk.BooleanVar(value=False)
        self.v_canal = tk.StringVar(value="RGB")
        self.v_op = tk.StringVar(value=OPERACOES[2]["nome"])
        self.v_alpha = tk.IntVar(value=50)
        self.v_sticker = tk.IntVar(value=0)
        self.v_escala = tk.IntVar(value=100)
        self.v_face = tk.StringVar(value=DetectorFace.MODOS[0])
        self.v_status = tk.StringVar(value="Pronto.")

        self._estilos()
        self._menus()
        self._layout()
        self._atalhos()

        if not (caminho_inicial and self.carregar_foto(caminho_inicial)):
            self.original = tela_inicial(MAX_W, MAX_H, ("Abra uma foto ou ligue a webcam",
                                                        "Menu Arquivo > Abrir foto  |  Video > Webcam"))
        self.selecionar_filtro(0)
        self.selecionar_operacao()
        self._atualizar_pilha()
        self.raiz.protocol("WM_DELETE_WINDOW", self.sair)
        self.laco()

    # ================================================================ visual
    def _estilos(self):
        st = ttk.Style(self.raiz)
        st.theme_use("clam")
        st.configure(".", background=COR["painel"], foreground=COR["texto"], font=FONTE,
                     fieldbackground=COR["cartao"], bordercolor=COR["escuro"],
                     lightcolor=COR["cartao"], darkcolor=COR["cartao"])
        st.configure("Roxo.TButton", background=COR["destaque"], foreground="white",
                     font=FONTE_B, borderwidth=0, padding=(10, 6), focusthickness=0)
        st.map("Roxo.TButton",
               background=[("pressed", COR["escuro"]), ("active", COR["destaque2"]),
                           ("disabled", COR["cartao"])],
               foreground=[("disabled", COR["texto2"])])
        st.configure("Sec.TButton", background=COR["cartao"], foreground=COR["texto"],
                     font=FONTE, borderwidth=1, padding=(8, 5))
        st.map("Sec.TButton", background=[("pressed", COR["escuro"]), ("active", "#4b2d7a")])
        st.configure("TLabel", background=COR["cartao"], foreground=COR["texto"])
        st.configure("Sub.TLabel", background=COR["cartao"], foreground=COR["texto2"],
                     font=("Segoe UI", 9))
        st.configure("TRadiobutton", background=COR["cartao"], foreground=COR["texto"])
        st.map("TRadiobutton", background=[("active", COR["cartao"])],
               indicatorcolor=[("selected", COR["destaque2"]), ("!selected", COR["fundo"])])
        st.configure("TCheckbutton", background=COR["cartao"], foreground=COR["texto"])
        st.map("TCheckbutton", background=[("active", COR["cartao"])],
               indicatorcolor=[("selected", COR["destaque2"]), ("!selected", COR["fundo"])])
        st.configure("Horizontal.TScale", background=COR["destaque"], troughcolor=COR["fundo"])
        st.map("Horizontal.TScale", background=[("active", COR["destaque2"])])
        st.configure("TCombobox", fieldbackground=COR["fundo"], background=COR["destaque"],
                     foreground=COR["texto"], arrowcolor="white", selectbackground=COR["escuro"])
        st.map("TCombobox", fieldbackground=[("readonly", COR["fundo"])],
               foreground=[("readonly", COR["texto"])],
               selectbackground=[("readonly", COR["fundo"])])
        self.raiz.option_add("*TCombobox*Listbox.background", COR["fundo"])
        self.raiz.option_add("*TCombobox*Listbox.foreground", COR["texto"])
        self.raiz.option_add("*TCombobox*Listbox.selectBackground", COR["destaque"])
        st.configure("Roxo.Vertical.TScrollbar", background=COR["destaque"],
                     troughcolor=COR["painel"], arrowcolor="white", borderwidth=0)

    def _menus(self):
        opc = dict(bg=COR["painel"], fg=COR["texto"], activebackground=COR["destaque"],
                   activeforeground="white", tearoff=0)
        barra = tk.Menu(self.raiz, **opc)
        m = tk.Menu(barra, **opc)
        m.add_command(label="Abrir foto...", accelerator="Ctrl+O", command=self.abrir_foto)
        m.add_command(label="Abrir 2ª imagem...", command=self.abrir_segunda)
        m.add_command(label="Salvar resultado", accelerator="Ctrl+S", command=self.salvar)
        m.add_command(label="Salvar como...", command=lambda: self.salvar(perguntar=True))
        m.add_separator()
        m.add_command(label="Sair", command=self.sair)
        barra.add_cascade(label="Arquivo", menu=m)

        m = tk.Menu(barra, **opc)
        m.add_command(label="Desfazer último passo", accelerator="Ctrl+Z", command=self.desfazer)
        m.add_command(label="Resetar tudo", accelerator="Ctrl+R", command=self.resetar)
        m.add_command(label="Remover último sticker", command=self.remover_sticker)
        barra.add_cascade(label="Editar", menu=m)

        m = tk.Menu(barra, **opc)
        m.add_command(label="Iniciar / parar webcam", accelerator="Ctrl+W", command=self.alternar_webcam)
        barra.add_cascade(label="Vídeo", menu=m)

        m = tk.Menu(barra, **opc)
        for i, f in enumerate(FILTROS):
            m.add_radiobutton(label=f["nome"], variable=self.v_filtro, value=i,
                              command=lambda i=i: self.selecionar_filtro(i))
        barra.add_cascade(label="Filtros", menu=m)

        m = tk.Menu(barra, **opc)
        m.add_command(label="Como usar", command=self.ajuda)
        barra.add_cascade(label="Ajuda", menu=m)
        self.raiz.config(menu=barra)

    def _secao(self, pai, titulo):
        caixa = tk.Frame(pai, bg=COR["cartao"], padx=10, pady=8,
                         highlightbackground=COR["escuro"], highlightthickness=1)
        caixa.pack(fill="x", padx=10, pady=(8, 0))
        tk.Label(caixa, text=titulo, bg=COR["cartao"], fg=COR["destaque2"], font=FONTE_T,
                 anchor="w").pack(fill="x", pady=(0, 4))
        return caixa

    def _slider(self, pai, texto, var, de=0, ate=100):
        linha = tk.Frame(pai, bg=COR["cartao"])
        linha.pack(fill="x", pady=2)
        rotulo = ttk.Label(linha, text=texto, width=11)
        rotulo.pack(side="left")
        valor = ttk.Label(linha, width=4, anchor="e")
        valor.pack(side="right")
        escala = ttk.Scale(linha, from_=de, to=ate, orient="horizontal",
                           command=lambda v: (var.set(int(float(v))), valor.config(text=str(var.get()))))
        escala.pack(side="left", fill="x", expand=True, padx=4)
        escala.set(var.get())
        valor.config(text=str(var.get()))
        return escala, rotulo

    def _layout(self):
        # ----- cabeçalho
        topo = tk.Frame(self.raiz, bg=COR["destaque"], height=46)
        topo.pack(fill="x")
        tk.Label(topo, text="✦ Stories FX", bg=COR["destaque"], fg="white",
                 font=("Segoe UI", 16, "bold")).pack(side="left", padx=14, pady=6)
        self.lbl_modo = tk.Label(topo, text="", bg=COR["escuro"], fg="white", font=FONTE_B,
                                 padx=10, pady=3)
        self.lbl_modo.pack(side="left", padx=6)
        for texto, cmd in (("Resetar", self.resetar), ("Salvar", self.salvar),
                           ("Webcam", self.alternar_webcam), ("Abrir foto", self.abrir_foto)):
            tk.Button(topo, text=texto, command=cmd, bg=COR["escuro"], fg="white", bd=0,
                      activebackground=COR["destaque2"], activeforeground="white",
                      font=FONTE_B, padx=12, pady=4, cursor="hand2").pack(side="right", padx=4, pady=6)

        # ----- barra de status (empacotada antes do corpo para não ser espremida)
        tk.Label(self.raiz, textvariable=self.v_status, bg=COR["painel"], fg=COR["texto2"],
                 anchor="w", padx=10, font=("Segoe UI", 9)).pack(fill="x", side="bottom")

        corpo = tk.Frame(self.raiz, bg=COR["fundo"])
        corpo.pack(fill="both", expand=True)

        # ----- painel lateral
        lateral = PainelRolavel(corpo, 330)
        lateral.pack(side="left", fill="y")
        p = lateral.interno

        # Filtros
        s = self._secao(p, "Filtros")
        self.cmb_filtro = ttk.Combobox(s, state="readonly", values=[f["nome"] for f in FILTROS])
        self.cmb_filtro.pack(fill="x")
        self.cmb_filtro.bind("<<ComboboxSelected>>",
                             lambda e: self.selecionar_filtro(self.cmb_filtro.current()))
        self.lbl_tipo = ttk.Label(s, style="Sub.TLabel")
        self.lbl_tipo.pack(fill="x", pady=(4, 0))
        self.lbl_desc = tk.Label(s, bg=COR["fundo"], fg=COR["texto"], font=("Segoe UI", 9),
                                 wraplength=280, justify="left", anchor="w", padx=6, pady=6)
        self.lbl_desc.pack(fill="x", pady=4)
        self.lbl_params = tk.Label(s, bg=COR["cartao"], fg=COR["texto2"], font=("Segoe UI", 9),
                                   wraplength=280, justify="left", anchor="w")
        self.lbl_params.pack(fill="x")
        self.sl_p1, _ = self._slider(s, "Param 1", self.v_p1)
        self.sl_p2, _ = self._slider(s, "Param 2", self.v_p2)
        ttk.Checkbutton(s, text="Mostrar prévia antes de aplicar (Espaço)",
                        variable=self.v_previa).pack(anchor="w", pady=2)
        linha = tk.Frame(s, bg=COR["cartao"])
        linha.pack(fill="x", pady=(4, 0))
        ttk.Button(linha, text="Aplicar filtro", style="Roxo.TButton",
                   command=self.aplicar_filtro).pack(side="left", fill="x", expand=True)
        ttk.Button(linha, text="Desfazer", style="Sec.TButton",
                   command=self.desfazer).pack(side="left", padx=(6, 0))

        # Canal
        s = self._secao(p, "Canal")
        linha = tk.Frame(s, bg=COR["cartao"])
        linha.pack(fill="x")
        for c in CANAIS:
            ttk.Radiobutton(linha, text=c, value=c, variable=self.v_canal,
                            command=self.mudou_canal).pack(side="left", padx=2)

        # Pilha de filtros aplicados
        s = self._secao(p, "Filtros aplicados (em sequência)")
        self.lista_pilha = tk.Listbox(s, height=5, bg=COR["fundo"], fg=COR["texto"],
                                      selectbackground=COR["destaque"], bd=0,
                                      highlightthickness=0, font=("Segoe UI", 9))
        self.lista_pilha.pack(fill="x")

        # Operações
        s = self._secao(p, "Operações com 2 imagens")
        self.cmb_op = ttk.Combobox(s, state="readonly", textvariable=self.v_op,
                                   values=[o["nome"] for o in OPERACOES])
        self.cmb_op.pack(fill="x")
        self.cmb_op.bind("<<ComboboxSelected>>", lambda e: self.selecionar_operacao())
        self.lbl_op = tk.Label(s, bg=COR["fundo"], fg=COR["texto"], font=("Segoe UI", 9),
                               wraplength=280, justify="left", anchor="w", padx=6, pady=6)
        self.lbl_op.pack(fill="x", pady=4)
        self._slider(s, "Alpha", self.v_alpha)
        linha = tk.Frame(s, bg=COR["cartao"])
        linha.pack(fill="x", pady=(4, 0))
        ttk.Button(linha, text="Aplicar operação", style="Roxo.TButton",
                   command=self.aplicar_operacao).pack(side="left", fill="x", expand=True)
        ttk.Button(linha, text="2ª imagem...", style="Sec.TButton",
                   command=self.abrir_segunda).pack(side="left", padx=(6, 0))
        self.mini_img2 = tk.Label(s, bg=COR["cartao"])
        self.mini_img2.pack(anchor="w", pady=(6, 0))
        self._atualizar_mini_img2()

        # Stickers
        s = self._secao(p, "Stickers (clique na imagem)")
        grade = tk.Frame(s, bg=COR["cartao"])
        grade.pack(fill="x")
        self.miniaturas = []
        self.botoes_sticker = []
        for i, st in enumerate(self.stickers):
            foto = para_photoimage(miniatura_sticker(st.quadros[0]))
            self.miniaturas.append(foto)
            b = tk.Radiobutton(grade, image=foto, variable=self.v_sticker, value=i,
                               indicatoron=False, bg=COR["cartao"], selectcolor=COR["destaque"],
                               activebackground=COR["destaque2"], bd=2, relief="flat",
                               offrelief="flat", cursor="hand2",
                               command=self.selecionou_sticker)
            b.grid(row=i // 5, column=i % 5, padx=2, pady=2)
            self.botoes_sticker.append(b)
        self.lbl_sticker = ttk.Label(s, style="Sub.TLabel")
        self.lbl_sticker.pack(fill="x", pady=(4, 0))
        self._slider(s, "Escala %", self.v_escala, 10, 300)
        ttk.Button(s, text="Remover último sticker", style="Sec.TButton",
                   command=self.remover_sticker).pack(fill="x", pady=(4, 0))
        ttk.Label(s, text="Stickers automáticos no rosto (Haar):").pack(anchor="w", pady=(8, 2))
        cmb = ttk.Combobox(s, state="readonly", textvariable=self.v_face, values=DetectorFace.MODOS)
        cmb.pack(fill="x")
        cmb.bind("<<ComboboxSelected>>", lambda e: self.mudou_face())

        # Salvar / resetar
        s = self._secao(p, "Resultado")
        ttk.Button(s, text="💾  Salvar imagem / frame", style="Roxo.TButton",
                   command=self.salvar).pack(fill="x")
        ttk.Button(s, text="↺  Resetar (voltar ao original)", style="Sec.TButton",
                   command=self.resetar).pack(fill="x", pady=(6, 0))
        tk.Frame(p, bg=COR["painel"], height=12).pack()

        # ----- área da imagem
        self.area = tk.Canvas(corpo, bg=COR["area"], highlightthickness=0, cursor="crosshair")
        self.area.pack(side="left", fill="both", expand=True)
        self.area.bind("<Button-1>", self.clique)
        self.area.bind("<Button-3>", lambda e: self.remover_sticker())
        self.area.bind("<Button-2>", lambda e: self.remover_sticker())   # macOS


        self.selecionou_sticker()

    def _atalhos(self):
        r = self.raiz
        r.bind("<Control-o>", lambda e: self.abrir_foto())
        r.bind("<Control-s>", lambda e: self.salvar())
        r.bind("<Control-z>", lambda e: self.desfazer())
        r.bind("<Control-r>", lambda e: self.resetar())
        r.bind("<Control-w>", lambda e: self.alternar_webcam())
        r.bind("<Return>", lambda e: self.aplicar_filtro())
        r.bind("<space>", lambda e: self.v_previa.set(not self.v_previa.get()))

    # ============================================================ utilidades
    def status(self, texto):
        print(">>", texto)
        self.v_status.set(texto)

    def _atualizar_mini_img2(self):
        if self.img2 is None:
            return
        h, w = self.img2.shape[:2]
        mini = cv.resize(self.img2, (90, max(1, int(90 * h / w))), interpolation=cv.INTER_AREA)
        self.foto_img2 = para_photoimage(mini)
        self.mini_img2.config(image=self.foto_img2)

    def _atualizar_pilha(self):
        self.lista_pilha.delete(0, "end")
        for n, (tipo, i, p1, p2) in enumerate(self.pilha, 1):
            if tipo == "filtro":
                texto = f"{n}. {FILTROS[i]['nome']}  (P1={p1}, P2={p2})"
            else:
                texto = f"{n}. {OPERACOES[i]['nome']}  (alpha={p1 / 100:.2f})"
            self.lista_pilha.insert("end", texto)
        if not self.pilha:
            self.lista_pilha.insert("end", "  (nenhum ainda)")

    # ============================================================== ações
    def carregar_foto(self, caminho):
        img = ler_imagem(caminho) if caminho else None
        if img is None:
            self.status(f"Não foi possível abrir: {caminho}")
            return False
        self.parar_webcam(silencioso=True)
        self.original = ajustar_tamanho(img, MAX_W, MAX_H)
        self.versao += 1
        self.rostos_cache = None
        self.colocados.clear()
        self.status(f"Modo Foto: {os.path.basename(caminho)}")
        return True

    def abrir_foto(self):
        caminho = filedialog.askopenfilename(title="Abrir foto", filetypes=TIPOS_IMAGEM)
        if caminho:
            self.carregar_foto(caminho)

    def abrir_segunda(self):
        caminho = filedialog.askopenfilename(title="Abrir 2ª imagem", filetypes=TIPOS_IMAGEM)
        img = ler_imagem(caminho) if caminho else None
        if img is not None:
            self.img2 = img
            self.cache_chave = None
            self._atualizar_mini_img2()
            self.status(f"2ª imagem: {os.path.basename(caminho)}")

    def alternar_webcam(self):
        if self.modo == "video":
            self.parar_webcam()
            return
        cap = cv.VideoCapture(0)
        if not cap.isOpened():
            messagebox.showwarning("Webcam", "Nenhuma webcam encontrada.")
            return
        cap.set(cv.CAP_PROP_FRAME_WIDTH, 640)
        cap.set(cv.CAP_PROP_FRAME_HEIGHT, 480)
        self.cap = cap
        self.modo = "video"
        self.colocados.clear()
        self.status("Modo Vídeo: webcam iniciada.")

    def parar_webcam(self, silencioso=False):
        if self.cap is not None:
            self.cap.release()
            self.cap = None
            self.modo = "foto"
            self.rostos_cache = None
            if not silencioso:
                self.status("Webcam parada. O último frame virou a foto atual.")

    def selecionar_filtro(self, indice):
        f = FILTROS[indice]
        self.v_filtro.set(indice)
        self.cmb_filtro.current(indice)
        self.lbl_tipo.config(text=f"{indice + 1}/{len(FILTROS)} · {f['tipo']}")
        self.lbl_desc.config(text=f["descricao"])
        self.lbl_params.config(text=f["params"])
        self.sl_p1.set(f["p1"])
        self.sl_p2.set(f["p2"])
        # a descrição também vai para o console, como na versão HighGUI
        print(f"\n[FILTRO] {f['nome']} ({f['tipo']})\n  {f['descricao']}\n  {f['params']}")
        self.status(f"Filtro selecionado: {f['nome']}. Ajuste os parâmetros e clique em Aplicar.")

    def aplicar_filtro(self):
        i = self.v_filtro.get()
        self.pilha.append(("filtro", i, self.v_p1.get(), self.v_p2.get()))
        self.v_previa.set(False)
        self._atualizar_pilha()
        self.status(f"Aplicado: {FILTROS[i]['nome']}")

    def selecionar_operacao(self):
        o = OPERACOES[self.cmb_op.current()]
        self.lbl_op.config(text=f"{o['descricao']}\n{o['params']}")
        print(f"\n[OPERACAO] {o['nome']}\n  {o['descricao']}\n  {o['params']}")

    def aplicar_operacao(self):
        if self.img2 is None:
            self.status("Carregue a 2ª imagem primeiro.")
            return
        i = self.cmb_op.current()
        self.pilha.append(("op", i, self.v_alpha.get(), 0))
        self._atualizar_pilha()
        self.status(f"Aplicado: {OPERACOES[i]['nome']}")

    def desfazer(self):
        if self.pilha:
            self.pilha.pop()
            self._atualizar_pilha()
            self.status("Último passo desfeito.")

    def mudou_canal(self):
        self.status(f"Canal: {self.v_canal.get()}")

    def selecionou_sticker(self):
        if not self.stickers:
            return
        s = self.stickers[self.v_sticker.get()]
        self.lbl_sticker.config(text=f"Selecionado: {s.nome}" + ("  (animado)" if s.animado else ""))

    def mudou_face(self):
        self.rostos_cache = None
        if self.v_face.get() != "desligado" and not self.detector.ok:
            messagebox.showwarning("Detecção de face",
                                   "Haar Cascade indisponível. Instale opencv-python < 5.")
        self.status(f"Stickers no rosto: {self.v_face.get()}")

    def clique(self, e):
        if not self.stickers or self.original is None:
            return
        x, y = e.x - self.offset[0], e.y - self.offset[1]
        h, w = self.original.shape[:2]
        if 0 <= x < w and 0 <= y < h:
            esc = max(10, self.v_escala.get()) / 100.0
            self.colocados.append((self.v_sticker.get(), x, y, esc))
            self.status(f"Sticker {self.stickers[self.v_sticker.get()].nome} em ({x}, {y})")

    def remover_sticker(self):
        if self.colocados:
            self.colocados.pop()
            self.status("Último sticker removido.")

    def salvar(self, perguntar=False):
        if self.ultimo_resultado is None:
            return
        if perguntar:
            caminho = filedialog.asksaveasfilename(defaultextension=".png", filetypes=TIPOS_IMAGEM)
            if not caminho:
                return
        else:
            os.makedirs(PASTA_SAIDAS, exist_ok=True)
            caminho = os.path.join(PASTA_SAIDAS, time.strftime("resultado_%Y%m%d_%H%M%S.png"))
        if salvar_imagem(caminho, self.ultimo_resultado):
            self.status(f"Salvo em {caminho}")
        else:
            self.status(f"Erro ao salvar: {caminho}")

    def resetar(self):
        self.pilha.clear()
        self.colocados.clear()
        self.v_canal.set("RGB")
        self.v_previa.set(False)
        self.v_face.set("desligado")
        self._atualizar_pilha()
        self.selecionar_filtro(self.v_filtro.get())
        self.status("Reset: voltou à imagem original.")

    def ajuda(self):
        messagebox.showinfo("Como usar", (
            "1. Abra uma foto (Arquivo › Abrir foto) ou inicie a webcam.\n"
            "2. Escolha um filtro: a descrição e os parâmetros aparecem no painel.\n"
            "3. Ajuste Param 1/Param 2, marque a prévia se quiser e clique em Aplicar.\n"
            "   Os filtros se acumulam em sequência; Desfazer remove o último.\n"
            "4. Canal: aplica em RGB, Cinza ou só em R, G ou B.\n"
            "5. Operações: escolha a operação e o alpha e aplique com a 2ª imagem.\n"
            "6. Stickers: escolha um e clique na imagem (botão direito remove).\n"
            "7. Salvar grava em saidas/. Resetar volta à imagem original.\n\n"
            "Atalhos: Ctrl+O abrir, Ctrl+S salvar, Ctrl+Z desfazer, Ctrl+R resetar,\n"
            "Ctrl+W webcam, Enter aplicar filtro, Espaço prévia."))

    def sair(self):
        self.parar_webcam(silencioso=True)
        self.raiz.destroy()

    # ============================================================ desenho
    def resultado(self):
        canal = self.v_canal.get()
        previa = ("filtro", self.v_filtro.get(), self.v_p1.get(), self.v_p2.get()) \
            if self.v_previa.get() else None
        chave = (self.versao, tuple(self.pilha), canal, previa)
        if self.modo == "foto" and chave == self.cache_chave:
            base = self.cache_img
        else:
            base = processar(self.original, self.pilha, canal, self.img2, previa)
            self.cache_chave, self.cache_img = chave, base
        img = base.copy()
        modo_face = self.v_face.get()
        if modo_face != "desligado":
            if self.modo == "video" or self.rostos_cache is None:
                self.rostos_cache = self.detector.detectar(self.original)
            self.detector.aplicar(img, modo_face, self.sticker_por_nome, self.rostos_cache)
        for (i, x, y, esc) in self.colocados:
            sobrepor(img, self.stickers[i].quadro_atual(), x, y, esc)
        return img

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
            self.foto_tk = para_photoimage(self.ultimo_resultado)
            cw, ch = self.area.winfo_width(), self.area.winfo_height()
            h, w = self.ultimo_resultado.shape[:2]
            self.offset = (max(0, (cw - w) // 2), max(0, (ch - h) // 2))
            self.area.delete("all")
            self.area.create_image(*self.offset, image=self.foto_tk, anchor="nw")
        except Exception as erro:   # não deixa a interface travar por um erro de filtro
            self.status(f"Erro: {erro}")
        self.lbl_modo.config(text=f"MODO {'VÍDEO' if self.modo == 'video' else 'FOTO'} · "
                                  f"canal {self.v_canal.get()}")
        self.raiz.after(30, self.laco)


def main():
    raiz = tk.Tk()
    AppGUI(raiz, sys.argv[1] if len(sys.argv) > 1 else None)
    raiz.mainloop()


if __name__ == "__main__":
    main()
