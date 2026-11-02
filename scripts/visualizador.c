#define UNICODE
#define _UNICODE
#define NOMINMAX
#include "viz_core.h"
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <stdarg.h>

static int imin(int a, int b) { return a < b ? a : b; }
static int imax(int a, int b) { return a > b ? a : b; }

static wchar_t* W(const char* s) {
    static wchar_t buf[8][1100];
    static int k;
    wchar_t* b = buf[k++ & 7];
    b[0] = 0;
    if (!MultiByteToWideChar(CP_UTF8, 0, s, -1, b, 1099)) b[0] = 0;
    b[1099] = 0;
    return b;
}

static void cortar(const char* s, int max, char* out) {
    int n = (int)strlen(s);
    if (n <= max) { strcpy(out, s); return; }
    int i = max;
    while (i > 0 && ((unsigned char)s[i] & 0xC0) == 0x80) i--;
    memcpy(out, s, (size_t)i);
    strcpy(out + i, "...");
}

static int ncar(const char* s) { int n = 0; for (; *s; s++) if (((unsigned char)*s & 0xC0) != 0x80) n++; return n; }
static void padr(char* dst, const char* s, int w) { int p = w - ncar(s); strcpy(dst, s); dst += strlen(dst); while (p-- > 0) *dst++ = ' '; *dst = 0; }
static void padl(char* dst, const char* s, int w) { int p = w - ncar(s); while (p-- > 0) *dst++ = ' '; strcpy(dst, s); }

enum {
    ID_LBL_BUSCA = 100, ID_EDIT, ID_BTN_BUSCAR, ID_BTN_INSERIR, ID_BTN_REMOVER,
    ID_LBL_QTD, ID_COMBO, ID_BTN_CARREGAR, ID_CHK_EMB, ID_BTN_DATASET,
    ID_GRP_FICHA, ID_FICHA, ID_GRP_RES, ID_LISTA, ID_GRP_STATS, ID_STATS, ID_LOG
};
#define WM_ABRIR_DATASET (WM_APP + 1)

static HINSTANCE g_inst;
static HWND g_main, g_edit, g_combo, g_chk, g_ficha, g_lista, g_stats, g_log, g_panA, g_panR;
static HWND g_ctl[32]; static int g_nctl;
static HFONT g_fUI, g_fMono, g_fBold, g_fNo, g_fNoSm, g_fHdr, g_fHdr2;
static WNDPROC g_oldEdit;
static VTree g_vta, g_vtr;
static unsigned g_resIds[NOME_MAX_RES];
static int g_nres;
static char g_dataset[MAX_PATH * 2];

static void log_add(const char* fmt, ...) {
    char tmp[1024];
    va_list ap; va_start(ap, fmt); vsnprintf(tmp, sizeof tmp, fmt, ap); va_end(ap);
    wchar_t linha[1200];
    wcscpy(linha, W(tmp)); wcscat(linha, L"\r\n");
    int len = GetWindowTextLengthW(g_log);
    SendMessageW(g_log, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessageW(g_log, EM_REPLACESEL, FALSE, (LPARAM)linha);
    SendMessageW(g_log, EM_SCROLLCARET, 0, 0);
}

static void set_text(HWND h, const char* utf8) { SetWindowTextW(h, W(utf8)); }

static void txt_float(char* o, size_t n, float v, const char* f) {
    if (v == DADO_AUSENTE_FLOAT) snprintf(o, n, "-"); else snprintf(o, n, f, (double)v);
}
static void txt_int(char* o, size_t n, int v) {
    if (v == DADO_AUSENTE_INT) snprintf(o, n, "-"); else snprintf(o, n, "%d", v);
}

static void atualizar_ficha(void) {
    char t[512];
    const Busca* b = &G.ba;
    if (!b->feita) {
        snprintf(t, sizeof t, "Digite um ID (1 a %d) ou parte do nome\r\n"
                 "da música/artista e clique em Buscar.\r\n\r\n"
                 "O caminho percorrido aparece destacado\r\n"
                 "nas duas árvores.", G.pool.n);
    } else if (!b->achou) {
        snprintf(t, sizeof t, "ID #%u não encontrado nas árvores.", b->id);
    } else {
        const Dados* d = b->dados;
        char ti[100], ar[100], ge[100], dn[24], en[24], te[24], va[24], po[24], pp[24], pk[24], wk[24], ps[24];
        cortar(d->title, 44, ti); cortar(d->performer, 44, ar); cortar(d->genre, 52, ge);
        txt_float(dn, sizeof dn, d->danceability, "%.3f"); txt_float(en, sizeof en, d->energy, "%.3f");
        txt_float(te, sizeof te, d->tempo, "%.1f");        txt_float(va, sizeof va, d->valence, "%.3f");
        txt_int(po, sizeof po, d->popularity); txt_int(pp, sizeof pp, d->position);
        txt_int(pk, sizeof pk, d->peak_pos);   txt_int(wk, sizeof wk, d->wks_on_chart);
        (void)ps;
        snprintf(t, sizeof t,
                 "#%u  %s\r\nArtista: %s\r\nGênero: %s\r\nSemana: %s   Pos: %s   Pico: %s   Sem.: %s\r\n"
                 "Dance: %s  Energy: %s  Tempo: %s\r\nValence: %s  Popularidade: %s\r\n"
                 "Chave: (%u, %s)",
                 b->id, ti, ar, ge[0] ? ge : "-", d->chart_week, pp, pk, wk,
                 dn, en, te, va, po, b->id, ti);
    }
    set_text(g_ficha, t);
}

static void atualizar_stats(void) {
    char out[1024], a[64], b[64], c[64];
    int n_a = contarNos(G.avl), n_r = rn_contarNos(G.rn);
    char l[2][40];
    out[0] = 0;
    padr(a, "", 14); padl(b, "AVL", 10); padl(c, "Rubro-Negra", 14);
    strcat(out, a); strcat(out, b); strcat(out, c); strcat(out, "\r\n");

    #define LINHA(rot, va, vr) do { padr(a, rot, 14); padl(b, va, 10); padl(c, vr, 14); \
        strcat(out, a); strcat(out, b); strcat(out, c); strcat(out, "\r\n"); } while (0)

    snprintf(l[0], 40, "%d", n_a); snprintf(l[1], 40, "%d", n_r);                 LINHA("Nós", l[0], l[1]);
    snprintf(l[0], 40, "%d", altura(G.avl)); snprintf(l[1], 40, "%d", rn_altura(G.rn)); LINHA("Altura", l[0], l[1]);
    if (G.ba.feita) { snprintf(l[0], 40, "%d", G.ba.comps); snprintf(l[1], 40, "%d", G.br.comps); }
    else            { strcpy(l[0], "-"); strcpy(l[1], "-"); }
    LINHA("Comparações", l[0], l[1]);
    if (G.ba.feita && G.ba.achou) { snprintf(l[0], 40, "%d", G.ba.profundidade); snprintf(l[1], 40, "%d", G.br.profundidade); }
    else                          { strcpy(l[0], "-"); strcpy(l[1], "-"); }
    LINHA("Profundidade", l[0], l[1]);
    if (G.ba.feita) snprintf(l[0], 40, "#%u", G.ba.id); else strcpy(l[0], "-");
    LINHA("Últ. busca", l[0], l[0]);
    snprintf(l[0], 40, "%.1f KB", n_a * (double)sizeof(No) / 1024.0);
    snprintf(l[1], 40, "%.1f KB", ((double)n_r * sizeof(NoRN) + sizeof(NoRN) + sizeof(ArvoreRN)) / 1024.0);
    LINHA("Memória", l[0], l[1]);
    set_text(g_stats, out);
}

static void refresh_all(void) {
    vt_de_avl(&g_vta, G.avl);
    vt_de_rn(&g_vtr, G.rn);
    atualizar_ficha();
    atualizar_stats();
    InvalidateRect(g_panA, NULL, FALSE);
    InvalidateRect(g_panR, NULL, FALSE);
}

static int on_path(const Busca* b, unsigned id) {
    for (int i = 0; i < b->npath; i++) if (b->path[i] == id) return 1;
    return 0;
}
static int edge_on_path(const Busca* b, unsigned pai, unsigned filho) {
    for (int i = 0; i + 1 < b->npath; i++) if (b->path[i] == pai && b->path[i + 1] == filho) return 1;
    return 0;
}

static void draw_tree(HDC dc, int W_, int H_, int top, const VTree* t, int rn, const Busca* b) {
    if (t->n == 0) {
        RECT r = { 0, top, W_, H_ };
        SelectObject(dc, g_fUI); SetTextColor(dc, RGB(120, 120, 120));
        DrawTextW(dc, L"Árvore vazia", -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        return;
    }
    const int margem = 26, rodape = 16;
    double availW = W_ - 2.0 * margem, availH = H_ - top - 2.0 * margem - rodape;
    double dx = t->n > 1 ? availW / (t->n - 1) : 0;  if (dx > 46) dx = 46;
    double dy = t->maxDepth > 0 ? availH / t->maxDepth : 0;  if (dy > 56) dy = 56;
    double x0 = (W_ - dx * (t->n - 1)) / 2.0;
    double y0 = top + margem;
    double rd = dx * 0.46; if (dy > 0 && dy * 0.36 < rd) rd = dy * 0.36; if (rd > 15) rd = 15; if (rd < 2) rd = 2;
    int R = (int)(rd + 0.5);

    #define PX(v) ((int)(x0 + (v)->col * dx + 0.5))
    #define PY(v) ((int)(y0 + (v)->depth * dy + 0.5))

    COLORREF corCam = b->achou ? RGB(240, 150, 20) : RGB(205, 40, 40);

    HPEN penFino = CreatePen(PS_SOLID, 1, RGB(120, 120, 120));
    HPEN penCam  = CreatePen(PS_SOLID, 3, corCam);
    SelectObject(dc, penFino);
    for (int i = 0; i < t->n; i++) {
        const VNode* v = &t->v[i];
        if (v->l >= 0) { MoveToEx(dc, PX(v), PY(v), NULL); LineTo(dc, PX(&t->v[v->l]), PY(&t->v[v->l])); }
        if (v->r >= 0) { MoveToEx(dc, PX(v), PY(v), NULL); LineTo(dc, PX(&t->v[v->r]), PY(&t->v[v->r])); }
    }
    if (b->feita) {
        SelectObject(dc, penCam);
        for (int i = 0; i < t->n; i++) {
            const VNode* v = &t->v[i];
            if (v->l >= 0 && edge_on_path(b, v->id, t->v[v->l].id)) { MoveToEx(dc, PX(v), PY(v), NULL); LineTo(dc, PX(&t->v[v->l]), PY(&t->v[v->l])); }
            if (v->r >= 0 && edge_on_path(b, v->id, t->v[v->r].id)) { MoveToEx(dc, PX(v), PY(v), NULL); LineTo(dc, PX(&t->v[v->r]), PY(&t->v[v->r])); }
        }
    }
    SelectObject(dc, GetStockObject(NULL_PEN));
    DeleteObject(penFino); DeleteObject(penCam);

    SetBkMode(dc, TRANSPARENT);
    for (int i = 0; i < t->n; i++) {
        const VNode* v = &t->v[i];
        int cx = PX(v), cy = PY(v);
        int noCam = b->feita && on_path(b, v->id);
        int achado = b->feita && b->achou && v->id == b->id;
        COLORREF fill, borda, txt;
        int larg = 1;
        if (rn) { fill = v->vermelho ? RGB(205, 35, 35) : RGB(30, 30, 30); borda = RGB(20, 20, 20); txt = RGB(255, 255, 255); }
        else    { fill = RGB(190, 220, 250); borda = RGB(70, 130, 200); txt = RGB(20, 20, 20); }
        if (noCam) { borda = corCam; larg = R >= 6 ? 3 : 2; if (!rn && achado) fill = RGB(250, 190, 60); }
        if (achado) { borda = RGB(230, 60, 150); larg = R >= 6 ? 3 : 2; }
        HPEN p = CreatePen(PS_SOLID, larg, borda);
        HBRUSH br = CreateSolidBrush(fill);
        HGDIOBJ op = SelectObject(dc, p), ob = SelectObject(dc, br);
        Ellipse(dc, cx - R, cy - R, cx + R + 1, cy + R + 1);
        SelectObject(dc, op); SelectObject(dc, ob);
        DeleteObject(p); DeleteObject(br);

        if (R >= 8) {
            wchar_t s[16]; wsprintfW(s, L"%u", v->id);
            RECT rc = { cx - R, cy - R, cx + R + 1, cy + R + 1 };
            SelectObject(dc, (R < 12 || v->id >= 1000) ? g_fNoSm : g_fNo);
            SetTextColor(dc, txt);
            DrawTextW(dc, s, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
        }
        if (!rn && R >= 9 && dx >= 14) {
            wchar_t s[8];
            if (v->fb > 0) wsprintfW(s, L"+%d", v->fb); else wsprintfW(s, L"%d", v->fb);
            RECT rc = { cx - 14, cy + R + 1, cx + 15, cy + R + 14 };
            SelectObject(dc, g_fNoSm); SetTextColor(dc, RGB(100, 100, 100));
            DrawTextW(dc, s, -1, &rc, DT_CENTER | DT_TOP | DT_SINGLELINE | DT_NOCLIP);
        }
    }
    #undef PX
    #undef PY
}

static void paint_panel(HWND hwnd, int rn) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    RECT rc; GetClientRect(hwnd, &rc);
    int w = rc.right, h = rc.bottom;
    HDC dc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, imax(w, 1), imax(h, 1));
    HGDIOBJ old = SelectObject(dc, bmp);

    HBRUSH white = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(dc, &rc, white); DeleteObject(white);

    const Busca* b = rn ? &G.br : &G.ba;
    const VTree* t = rn ? &g_vtr : &g_vta;

    RECT hd = { 0, 0, w, 50 };
    HBRUSH hb = CreateSolidBrush(rn ? RGB(120, 28, 30) : RGB(30, 70, 130));
    FillRect(dc, &hd, hb); DeleteObject(hb);
    SetBkMode(dc, TRANSPARENT);
    char l1[128];
    snprintf(l1, sizeof l1, "%s  -  %d nós, altura %d", rn ? "Rubro-Negra" : "AVL",
             rn ? rn_contarNos(G.rn) : contarNos(G.avl), rn ? rn_altura(G.rn) : altura(G.avl));
    SelectObject(dc, g_fHdr); SetTextColor(dc, RGB(255, 255, 255));
    TextOutW(dc, 8, 6, W(l1), (int)wcslen(W(l1)));
    if (b->feita) {
        char l2[160];
        if (b->achou) snprintf(l2, sizeof l2, "ENCONTRADO: %d comparações (profundidade %d)", b->comps, b->profundidade);
        else          snprintf(l2, sizeof l2, "NÃO ENCONTRADO: %d comparações", b->comps);
        SelectObject(dc, g_fHdr2);
        SetTextColor(dc, b->achou ? RGB(150, 255, 170) : RGB(255, 170, 160));
        TextOutW(dc, 8, 28, W(l2), (int)wcslen(W(l2)));
    }
    draw_tree(dc, w, h, 50, t, rn, b);

    BitBlt(hdc, 0, 0, w, h, dc, 0, 0, SRCCOPY);
    SelectObject(dc, old); DeleteObject(bmp); DeleteDC(dc);
    EndPaint(hwnd, &ps);
}

static LRESULT CALLBACK PanelProc(HWND hwnd, UINT m, WPARAM wp, LPARAM lp) {
    switch (m) {
    case WM_CREATE:
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCTW*)lp)->lpCreateParams);
        return 0;
    case WM_ERASEBKGND: return 1;
    case WM_PAINT: paint_panel(hwnd, (int)GetWindowLongPtrW(hwnd, GWLP_USERDATA)); return 0;
    }
    return DefWindowProcW(hwnd, m, wp, lp);
}

static void ler_edit(char* out, size_t n) {
    wchar_t w[256]; w[0] = 0;
    GetWindowTextW(g_edit, w, 255);
    WideCharToMultiByte(CP_UTF8, 0, w, -1, out, (int)n, NULL, NULL);
    out[n - 1] = 0;
    char* p = out; while (*p == ' ') p++;
    memmove(out, p, strlen(p) + 1);
    size_t L = strlen(out); while (L > 0 && out[L - 1] == ' ') out[--L] = 0;
}

static int so_digitos(const char* s) {
    if (!*s) return 0;
    for (; *s; s++) if (*s < '0' || *s > '9') return 0;
    return 1;
}

static void log_resultado_busca(void) {
    char nome[120] = "";
    if (G.ba.achou) { char ti[80]; cortar(G.ba.dados->title, 40, ti); snprintf(nome, sizeof nome, " %s", ti); }
    log_add("Busca por #%u%s:  AVL = %s (%d comp.)  |  Rubro-Negra = %s (%d comp.)", G.ba.id, nome,
            G.ba.achou ? "encontrado" : "não encontrado", G.ba.comps,
            G.br.achou ? "encontrado" : "não encontrado", G.br.comps);
    if (!G.ba.achou && G.ba.id >= 1 && G.ba.id <= (unsigned)G.pool.n)
        log_add("  (o ID #%u existe no dataset, mas não está nas árvores: use \"Inserir próximo\" ou aumente a Qtd. inicial)", G.ba.id);
}

static void buscar_id(unsigned id) {
    app_buscar_id(id);
    log_resultado_busca();
    refresh_all();
}

static void acao_buscar(void) {
    char q[256];
    ler_edit(q, sizeof q);
    if (!q[0]) { log_add("Digite um ID ou parte do nome."); return; }
    if (so_digitos(q) && strlen(q) <= 9) {
        SendMessageW(g_lista, LB_RESETCONTENT, 0, 0); g_nres = 0;
        buscar_id((unsigned)strtoul(q, NULL, 10));
        return;
    }
    unsigned ids[NOME_MAX_RES];
    int n = app_buscar_nome(q, ids, NOME_MAX_RES);
    SendMessageW(g_lista, LB_RESETCONTENT, 0, 0);
    g_nres = n;
    if (n == 0) { log_add("Nenhum registro nas árvores com \"%s\" no título/artista.", q); return; }
    for (int i = 0; i < n; i++) {
        Busca t = busca_avl(G.avl, ids[i]);
        char ti[90], ar[70], linha[256];
        cortar(t.dados->title, 40, ti); cortar(t.dados->performer, 30, ar);
        snprintf(linha, sizeof linha, "#%-5u %s - %s  [%s]", ids[i], ti, ar, t.dados->chart_week);
        g_resIds[i] = ids[i];
        SendMessageW(g_lista, LB_ADDSTRING, 0, (LPARAM)W(linha));
    }
    log_add("%d resultado(s) para \"%s\"%s. Clique em um deles para buscar o ID.", n, q,
            n >= NOME_MAX_RES ? " (limitado)" : "");
    if (n == 1) { SendMessageW(g_lista, LB_SETCURSEL, 0, 0); buscar_id(ids[0]); }
}

static void acao_inserir(void) {
    unsigned id;
    if (!app_inserir_proximo(&id)) { log_add("Não há mais registros no dataset para inserir."); return; }
    log_add("Inserido #%u nas duas árvores.", id);
    app_buscar_id(id);
    log_resultado_busca();
    refresh_all();
}

static void acao_remover(void) {
    char q[256]; ler_edit(q, sizeof q);
    if (!so_digitos(q) || strlen(q) > 9) { log_add("Remover: digite o ID numérico no campo de busca."); return; }
    unsigned id = (unsigned)strtoul(q, NULL, 10);
    if (!app_remover(id)) { log_add("Remover #%u: ID não existe nas árvores.", id); return; }
    log_add("Removido #%u das duas árvores (AVL: %d nós | Rubro-Negra: %d nós).", id, contarNos(G.avl), rn_contarNos(G.rn));
    refresh_all();
}

static void acao_carregar(void) {
    if (G.pool.n == 0) { log_add("Nenhum dataset carregado. Use o botão \"Dataset...\"."); return; }
    wchar_t w[32]; w[0] = 0;
    GetWindowTextW(g_combo, w, 31);
    int n = _wtoi(w); if (n <= 0) n = 31;
    int emb = (SendMessageW(g_chk, BM_GETCHECK, 0, 0) == BST_CHECKED);
    app_carregar(n, emb);
    SendMessageW(g_lista, LB_RESETCONTENT, 0, 0); g_nres = 0;
    log_add("Carregados %d registros (IDs 1..%d) em ordem %s nas duas árvores.",
            imin(n, G.pool.n), imin(n, G.pool.n), emb ? "embaralhada" : "sequencial");
    refresh_all();
}

static void carregar_arquivo(const char* caminho_ansi) {
    HCURSOR oc = SetCursor(LoadCursor(NULL, IDC_WAIT));
    int n = app_ler_dataset(caminho_ansi);
    SetCursor(oc);
    if (n < 0) { log_add("Falha ao ler \"%s\" (arquivo inexistente, UTF-16 ou formato diferente de 13 colunas).", caminho_ansi); return; }
    strncpy(g_dataset, caminho_ansi, sizeof g_dataset - 1);
    log_add("Dataset lido: %d registros válidos em memória (limite %d).", n, POOL_MAX);
    acao_carregar();
}

static void acao_dataset(void) {
    wchar_t arq[MAX_PATH * 2]; arq[0] = 0;
    OPENFILENAMEW ofn; memset(&ofn, 0, sizeof ofn);
    ofn.lStructSize = sizeof ofn; ofn.hwndOwner = g_main;
    ofn.lpstrFilter = L"CSV / texto (*.csv;*.txt)\0*.csv;*.txt\0Todos os arquivos\0*.*\0";
    ofn.lpstrFile = arq; ofn.nMaxFile = MAX_PATH * 2;
    ofn.lpstrTitle = L"Escolha o dataset";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    if (!GetOpenFileNameW(&ofn)) return;
    char ansi[MAX_PATH * 2];
    WideCharToMultiByte(CP_ACP, 0, arq, -1, ansi, sizeof ansi, NULL, NULL);
    carregar_arquivo(ansi);
}

static HWND mk(const wchar_t* cls, const wchar_t* txt, DWORD style, DWORD ex, int id) {
    HWND h = CreateWindowExW(ex, cls, txt, WS_CHILD | WS_VISIBLE | style, 0, 0, 10, 10, g_main,
                             (HMENU)(INT_PTR)id, g_inst, NULL);
    if (g_nctl < 32) g_ctl[g_nctl++] = h;
    return h;
}

static void layout(void) {
    RECT rc; GetClientRect(g_main, &rc);
    int W_ = rc.right, H_ = rc.bottom;
    int logH = 110, topH = 172, pad = 8;
    int y1 = 8, h1 = 26;
    #define POS(id, x, y, w, h) MoveWindow(GetDlgItem(g_main, id), x, y, w, h, TRUE)
    POS(ID_LBL_BUSCA, 8, y1 + 4, 128, 20);   POS(ID_EDIT, 138, y1, 190, h1);
    POS(ID_BTN_BUSCAR, 338, y1, 90, h1);     POS(ID_BTN_INSERIR, 436, y1, 130, h1);
    POS(ID_BTN_REMOVER, 574, y1, 110, h1);   POS(ID_LBL_QTD, 704, y1 + 4, 84, 20);
    POS(ID_COMBO, 790, y1, 70, 200);         POS(ID_BTN_CARREGAR, 868, y1, 90, h1);
    POS(ID_CHK_EMB, 968, y1 + 3, 100, 22);   POS(ID_BTN_DATASET, 1074, y1, 100, h1);

    int y2 = 42, aw = W_ - 4 * pad;
    int w1 = aw * 34 / 100, w3 = aw * 27 / 100, w2 = aw - w1 - w3;
    int x1 = pad, x2 = x1 + w1 + pad, x3 = x2 + w2 + pad;
    POS(ID_GRP_FICHA, x1, y2, w1, topH);  POS(ID_FICHA, x1 + 8, y2 + 18, w1 - 16, topH - 24);
    POS(ID_GRP_RES, x2, y2, w2, topH);    POS(ID_LISTA, x2 + 8, y2 + 18, w2 - 16, topH - 26);
    POS(ID_GRP_STATS, x3, y2, w3, topH);  POS(ID_STATS, x3 + 8, y2 + 18, w3 - 16, topH - 24);

    int yp = y2 + topH + pad, hp = H_ - yp - logH - 2 * pad;
    int pw = (W_ - 3 * pad) / 2;
    MoveWindow(g_panA, pad, yp, pw, imax(hp, 50), TRUE);
    MoveWindow(g_panR, pad * 2 + pw, yp, pw, imax(hp, 50), TRUE);
    MoveWindow(g_log, pad, H_ - logH - pad, W_ - 2 * pad, logH, TRUE);
    #undef POS
}

static LRESULT CALLBACK EditProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_CHAR && w == '\r') { acao_buscar(); return 0; }
    return CallWindowProcW(g_oldEdit, h, m, w, l);
}

static void criar_controles(void) {
    LOGFONTW lf; (void)lf;
    g_fUI    = CreateFontW(-13, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    g_fBold  = CreateFontW(-13, 0, 0, 0, FW_BOLD,   0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    g_fMono  = CreateFontW(-13, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, FIXED_PITCH, L"Consolas");
    g_fNo    = CreateFontW(-11, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    g_fNoSm  = CreateFontW(-9,  0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    g_fHdr   = CreateFontW(-14, 0, 0, 0, FW_BOLD,   0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    g_fHdr2  = CreateFontW(-13, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");

    mk(L"STATIC", L"Buscar (ID ou nome):", SS_NOPREFIX, 0, ID_LBL_BUSCA);
    g_edit = mk(L"EDIT", L"", WS_TABSTOP | ES_AUTOHSCROLL, WS_EX_CLIENTEDGE, ID_EDIT);
    mk(L"BUTTON", L"Buscar", WS_TABSTOP | BS_PUSHBUTTON, 0, ID_BTN_BUSCAR);
    mk(L"BUTTON", L"Inserir próximo", WS_TABSTOP | BS_PUSHBUTTON, 0, ID_BTN_INSERIR);
    mk(L"BUTTON", L"Remover (ID)", WS_TABSTOP | BS_PUSHBUTTON, 0, ID_BTN_REMOVER);
    mk(L"STATIC", L"Qtd. inicial:", SS_NOPREFIX, 0, ID_LBL_QTD);
    g_combo = mk(L"COMBOBOX", L"", WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST, 0, ID_COMBO);
    mk(L"BUTTON", L"Carregar", WS_TABSTOP | BS_PUSHBUTTON, 0, ID_BTN_CARREGAR);
    g_chk = mk(L"BUTTON", L"Embaralhar", WS_TABSTOP | BS_AUTOCHECKBOX, 0, ID_CHK_EMB);
    mk(L"BUTTON", L"Dataset...", WS_TABSTOP | BS_PUSHBUTTON, 0, ID_BTN_DATASET);

    mk(L"BUTTON", L"FICHA DO REGISTRO", BS_GROUPBOX, 0, ID_GRP_FICHA);
    g_ficha = mk(L"STATIC", L"", SS_NOPREFIX, 0, ID_FICHA);
    mk(L"BUTTON", L"Resultados da busca por nome", BS_GROUPBOX, 0, ID_GRP_RES);
    g_lista = mk(L"LISTBOX", L"", WS_VSCROLL | WS_HSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT, WS_EX_CLIENTEDGE, ID_LISTA);
    mk(L"BUTTON", L"AVL x Rubro-Negra", BS_GROUPBOX, 0, ID_GRP_STATS);
    g_stats = mk(L"STATIC", L"", SS_NOPREFIX, 0, ID_STATS);
    g_log = mk(L"EDIT", L"", WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL, WS_EX_CLIENTEDGE, ID_LOG);
    SendMessageW(g_log, EM_SETLIMITTEXT, 4000000, 0);

    g_panA = CreateWindowExW(WS_EX_CLIENTEDGE, L"ArvoreVizPanel", L"", WS_CHILD | WS_VISIBLE, 0, 0, 10, 10, g_main, NULL, g_inst, (LPVOID)0);
    g_panR = CreateWindowExW(WS_EX_CLIENTEDGE, L"ArvoreVizPanel", L"", WS_CHILD | WS_VISIBLE, 0, 0, 10, 10, g_main, NULL, g_inst, (LPVOID)1);

    for (int i = 0; i < g_nctl; i++) SendMessageW(g_ctl[i], WM_SETFONT, (WPARAM)g_fUI, TRUE);
    SendMessageW(g_ficha, WM_SETFONT, (WPARAM)g_fMono, TRUE);
    SendMessageW(g_stats, WM_SETFONT, (WPARAM)g_fMono, TRUE);
    SendMessageW(g_lista, WM_SETFONT, (WPARAM)g_fMono, TRUE);
    SendMessageW(g_log,   WM_SETFONT, (WPARAM)g_fMono, TRUE);

    static const wchar_t* qtds[] = { L"7", L"15", L"31", L"63", L"127", L"255", L"511" };
    for (int i = 0; i < 7; i++) SendMessageW(g_combo, CB_ADDSTRING, 0, (LPARAM)qtds[i]);
    SendMessageW(g_combo, CB_SETCURSEL, 2, 0);
    SendMessageW(g_chk, BM_SETCHECK, BST_CHECKED, 0);

    g_oldEdit = (WNDPROC)SetWindowLongPtrW(g_edit, GWLP_WNDPROC, (LONG_PTR)EditProc);
}

static LRESULT CALLBACK MainProc(HWND hwnd, UINT m, WPARAM wp, LPARAM lp) {
    switch (m) {
    case WM_CREATE:
        g_main = hwnd;
        criar_controles();
        return 0;
    case WM_SIZE:
        if (g_panR) layout();
        return 0;
    case WM_GETMINMAXINFO: {
        MINMAXINFO* mi = (MINMAXINFO*)lp;
        mi->ptMinTrackSize.x = 1200; mi->ptMinTrackSize.y = 700;
        return 0;
    }
    case WM_ABRIR_DATASET:
        acao_dataset();
        return 0;
    case WM_COMMAND: {
        int id = LOWORD(wp), nt = HIWORD(wp);
        switch (id) {
        case ID_BTN_BUSCAR:   acao_buscar();  break;
        case ID_BTN_INSERIR:  acao_inserir(); break;
        case ID_BTN_REMOVER:  acao_remover(); break;
        case ID_BTN_CARREGAR: acao_carregar(); break;
        case ID_BTN_DATASET:  acao_dataset(); break;
        case ID_LISTA:
            if (nt == LBN_SELCHANGE) {
                int i = (int)SendMessageW(g_lista, LB_GETCURSEL, 0, 0);
                if (i >= 0 && i < g_nres) buscar_id(g_resIds[i]);
            }
            break;
        }
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, m, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR cmd, int nShow) {
    (void)hPrev; (void)cmd;
    g_inst = hInst;
    app_limpar_arvores();

    WNDCLASSEXW pc; memset(&pc, 0, sizeof pc);
    pc.cbSize = sizeof pc; pc.lpfnWndProc = PanelProc; pc.hInstance = hInst;
    pc.hCursor = LoadCursor(NULL, IDC_ARROW); pc.lpszClassName = L"ArvoreVizPanel";
    RegisterClassExW(&pc);

    WNDCLASSEXW wc; memset(&wc, 0, sizeof wc);
    wc.cbSize = sizeof wc; wc.lpfnWndProc = MainProc; wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = L"ArvoreVizMain";
    RegisterClassExW(&wc);

    HWND h = CreateWindowExW(0, L"ArvoreVizMain", L"BillTree - AVL x Rubro-Negra (Estruturas de Dados II - UNICID)",
                             WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1300, 860,
                             NULL, NULL, hInst, NULL);
    if (!h) return 1;
    ShowWindow(h, nShow);
    UpdateWindow(h);
    layout();

    int argc = 0; wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    char ansi[MAX_PATH * 2]; ansi[0] = 0;
    if (argv && argc > 1) WideCharToMultiByte(CP_ACP, 0, argv[1], -1, ansi, sizeof ansi, NULL, NULL);
    if (argv) LocalFree(argv);
    if (!ansi[0]) {
        FILE* t = fopen("dataset.csv", "rb");
        if (t) { fclose(t); strcpy(ansi, "dataset.csv"); }
    }
    refresh_all();
    log_add("Visualizador pronto. Comparações = nós visitados no caminho da busca.");
    if (ansi[0]) carregar_arquivo(ansi);
    else { log_add("Escolha o arquivo do dataset..."); PostMessageW(h, WM_ABRIR_DATASET, 0, 0); }

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) { TranslateMessage(&msg); DispatchMessageW(&msg); }

    liberarArvore(G.avl); rn_liberarArvore(G.rn); free(G.pool.v);
    vt_free(&g_vta); vt_free(&g_vtr);
    return (int)msg.wParam;
}
