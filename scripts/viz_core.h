#ifndef VIZ_CORE_H
#define VIZ_CORE_H

#include "../scripts/arvore_avl.h"
#include "../scripts/arvore_rubro_negra.h"

#define POOL_MAX     50000
#define PATH_MAX_LEN 128
#define NOME_MAX_RES 300

typedef struct Pool { Dados* v; int n, cap; } Pool;

static bool pool_linha(char* campos[], int n, int numLinha, void* ctx) {
    Pool* p = (Pool*)ctx;
    Dados d; int trunc = 0;
    if (!dadosDeCampos(campos, n, numLinha, &d, &trunc)) return false;
    if (p->n >= POOL_MAX) return false;
    if (p->n == p->cap) {
        int nc = p->cap ? p->cap * 2 : 1024;
        Dados* nv = (Dados*)realloc(p->v, (size_t)nc * sizeof(Dados));
        if (!nv) return false;
        p->v = nv; p->cap = nc;
    }
    p->v[p->n++] = d;
    return true;
}

typedef struct Busca {
    bool     feita, achou;
    unsigned id;
    int      comps;
    int      profundidade;
    unsigned path[PATH_MAX_LEN];
    int      npath;
    const Dados* dados;
} Busca;

typedef struct App {
    Pool     pool;
    No*      avl;
    ArvoreRN* rn;
    unsigned proximo;
    Busca    ba, br;
} App;

static App G;

static void app_limpar_arvores(void) {
    if (G.avl) { liberarArvore(G.avl); G.avl = NULL; }
    if (G.rn)  rn_liberarArvore(G.rn);
    G.rn = rn_criarArvore();
    memset(&G.ba, 0, sizeof G.ba);
    memset(&G.br, 0, sizeof G.br);
}

static int app_ler_dataset(const char* caminho) {
    Pool novo = {0};
    CsvConfig cfg = { ',', true, COL_TOTAL };
    CsvResultado res;
    int st = csvCarregar(caminho, &cfg, pool_linha, &novo, &res);
    if (st != CSV_OK || novo.n == 0) { free(novo.v); return -1; }
    free(G.pool.v);
    G.pool = novo;
    return G.pool.n;
}

static void app_carregar(int n, bool embaralhar) {
    app_limpar_arvores();
    if (n > G.pool.n) n = G.pool.n;
    if (n <= 0) { G.proximo = 1; return; }
    unsigned* ord = (unsigned*)malloc((size_t)n * sizeof(unsigned));
    for (int i = 0; i < n; i++) ord[i] = (unsigned)i + 1;
    if (embaralhar) {
        unsigned long long s = 88172645463325252ULL;
        for (int i = n - 1; i > 0; i--) {
            s ^= s << 13; s ^= s >> 7; s ^= s << 17;
            int j = (int)(s % (unsigned long long)(i + 1));
            unsigned t = ord[i]; ord[i] = ord[j]; ord[j] = t;
        }
    }
    for (int i = 0; i < n; i++) {
        G.avl = inserirNo(G.avl, ord[i], &G.pool.v[ord[i] - 1]);
        rn_inserirNo(G.rn, ord[i], &G.pool.v[ord[i] - 1]);
    }
    G.proximo = (unsigned)n + 1;
    free(ord);
}

static bool app_inserir_proximo(unsigned* idInserido) {
    if (G.proximo == 0 || G.proximo > (unsigned)G.pool.n) return false;
    unsigned id = G.proximo++;
    G.avl = inserirNo(G.avl, id, &G.pool.v[id - 1]);
    rn_inserirNo(G.rn, id, &G.pool.v[id - 1]);
    if (idInserido) *idInserido = id;
    return true;
}

static bool app_remover(unsigned id) {
    bool existe = buscar(G.avl, id);
    if (!existe) return false;
    G.avl = removerNo(G.avl, id);
    rn_removerNo(G.rn, id);
    memset(&G.ba, 0, sizeof G.ba);
    memset(&G.br, 0, sizeof G.br);
    return true;
}

static Busca busca_avl(No* r, unsigned id) {
    Busca b; memset(&b, 0, sizeof b);
    b.feita = true; b.id = id;
    while (r && b.npath < PATH_MAX_LEN) {
        b.path[b.npath++] = r->id;
        b.comps++;
        if (id == r->id) { b.achou = true; b.dados = &r->dados; break; }
        r = id < r->id ? r->esquerda : r->direita;
    }
    b.profundidade = b.npath - 1;
    return b;
}

static Busca busca_rn(ArvoreRN* t, unsigned id) {
    Busca b; memset(&b, 0, sizeof b);
    b.feita = true; b.id = id;
    NoRN* r = t->raiz;
    while (r != t->nil && b.npath < PATH_MAX_LEN) {
        b.path[b.npath++] = r->id;
        b.comps++;
        if (id == r->id) { b.achou = true; b.dados = &r->dados; break; }
        r = id < r->id ? r->esquerda : r->direita;
    }
    b.profundidade = b.npath - 1;
    return b;
}

static void app_buscar_id(unsigned id) {
    G.ba = busca_avl(G.avl, id);
    G.br = busca_rn(G.rn, id);
}

static bool contem_ci(const char* txt, const char* sub) {
    size_t n = strlen(sub);
    if (n == 0) return true;
    for (; *txt; txt++) {
        size_t i = 0;
        for (; i < n && txt[i]; i++) {
            int a = (unsigned char)txt[i], b = (unsigned char)sub[i];
            if (a >= 'A' && a <= 'Z') a += 32;
            if (b >= 'A' && b <= 'Z') b += 32;
            if (a != b) break;
        }
        if (i == n) return true;
    }
    return false;
}

static void nome_rec(const No* r, const char* q, unsigned* ids, int* n, int max) {
    if (!r || *n >= max) return;
    nome_rec(r->esquerda, q, ids, n, max);
    if (*n < max && (contem_ci(r->dados.title, q) || contem_ci(r->dados.performer, q)))
        ids[(*n)++] = r->id;
    nome_rec(r->direita, q, ids, n, max);
}

static int app_buscar_nome(const char* q, unsigned* ids, int max) {
    int n = 0;
    nome_rec(G.avl, q, ids, &n, max);
    return n;
}

typedef struct VNode {
    unsigned id;
    int  l, r;
    int  col, depth;
    int  vermelho;
    int  fb;
} VNode;

typedef struct VTree { VNode* v; int n, maxDepth; } VTree;

static void vt_free(VTree* t) { free(t->v); t->v = NULL; t->n = 0; t->maxDepth = 0; }

static int vt_avl_rec(VTree* t, No* r, int depth, int* col) {
    if (!r) return -1;
    int idx = t->n++;
    t->v[idx].id = r->id; t->v[idx].depth = depth;
    t->v[idx].fb = fatorBalanceamento(r); t->v[idx].vermelho = 0;
    if (depth > t->maxDepth) t->maxDepth = depth;
    int l = vt_avl_rec(t, r->esquerda, depth + 1, col);
    t->v[idx].col = (*col)++;
    int rr = vt_avl_rec(t, r->direita, depth + 1, col);
    t->v[idx].l = l; t->v[idx].r = rr;
    return idx;
}

static int vt_rn_rec(VTree* t, ArvoreRN* a, NoRN* r, int depth, int* col) {
    if (r == a->nil) return -1;
    int idx = t->n++;
    t->v[idx].id = r->id; t->v[idx].depth = depth;
    t->v[idx].vermelho = (r->cor == VERMELHO); t->v[idx].fb = 0;
    if (depth > t->maxDepth) t->maxDepth = depth;
    int l = vt_rn_rec(t, a, r->esquerda, depth + 1, col);
    t->v[idx].col = (*col)++;
    int rr = vt_rn_rec(t, a, r->direita, depth + 1, col);
    t->v[idx].l = l; t->v[idx].r = rr;
    return idx;
}

static void vt_de_avl(VTree* t, No* raiz) {
    vt_free(t);
    int n = contarNos(raiz);
    if (n == 0) return;
    t->v = (VNode*)calloc((size_t)n, sizeof(VNode));
    int col = 0;
    vt_avl_rec(t, raiz, 0, &col);
}

static void vt_de_rn(VTree* t, ArvoreRN* a) {
    vt_free(t);
    int n = rn_contarNos(a);
    if (n == 0) return;
    t->v = (VNode*)calloc((size_t)n, sizeof(VNode));
    int col = 0;
    vt_rn_rec(t, a, a->raiz, 0, &col);
}

#endif
