#ifndef ARVORE_RUBRO_NEGRA_H
#define ARVORE_RUBRO_NEGRA_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>

#include "leitor_dataset.h"

#ifndef TAM_TITULO
#define TAM_TITULO   80
#endif
#ifndef TAM_ARTISTA
#define TAM_ARTISTA  120
#endif
#ifndef TAM_GENERO
#define TAM_GENERO   240
#endif

#ifndef DADOS_ESTRUTURA_DEFINIDA
#define DADOS_ESTRUTURA_DEFINIDA

typedef unsigned int posint;

#ifndef DADO_AUSENTE_INT
#define DADO_AUSENTE_INT (-1)
#endif
#ifndef DADO_AUSENTE_FLOAT
#define DADO_AUSENTE_FLOAT (-1.0f)
#endif

typedef struct Dados {
    char chart_week[30];
    int position;
    int peak_pos;
    int wks_on_chart;
    char title[TAM_TITULO];
    char performer[TAM_ARTISTA];
    char genre[TAM_GENERO];
    float danceability;
    float energy;
    float tempo;
    float valence;
    int popularity;
} Dados;

#endif

typedef enum {
    VERMELHO,
    PRETO
} Cor;

typedef struct NoRN {
    posint id;
    Dados dados;
    Cor cor;
    struct NoRN* pai;
    struct NoRN* esquerda;
    struct NoRN* direita;
} NoRN;

typedef struct ArvoreRN {
    NoRN* raiz;
    NoRN* nil;
} ArvoreRN;

static NoRN* rn_novoNo(ArvoreRN* arvore, posint v, const Dados* dados) {
    NoRN* no = (NoRN*)malloc(sizeof(NoRN));
    if (!no) {
        printf("Falha ao alocar nó da árvore rubro-negra.\n");
        exit(EXIT_FAILURE);
    }

    no->id = v;
    if (dados) no->dados = *dados;
    else memset(&no->dados, 0, sizeof no->dados);

    no->cor = VERMELHO;
    no->pai = arvore->nil;
    no->esquerda = arvore->nil;
    no->direita = arvore->nil;
    return no;
}

static ArvoreRN* rn_criarArvore(void) {
    ArvoreRN* arvore = (ArvoreRN*)malloc(sizeof(ArvoreRN));
    if (!arvore) {
        printf("Falha ao alocar árvore rubro-negra.\n");
        exit(EXIT_FAILURE);
    }

    NoRN* nil = (NoRN*)malloc(sizeof(NoRN));
    if (!nil) {
        printf("Falha ao alocar sentinela nil da árvore rubro-negra.\n");
        exit(EXIT_FAILURE);
    }

    nil->id = 0;
    memset(&nil->dados, 0, sizeof nil->dados);
    nil->cor = PRETO;
    nil->pai = nil;
    nil->esquerda = nil;
    nil->direita = nil;

    arvore->nil = nil;
    arvore->raiz = nil;
    return arvore;
}

static bool rn_arvoreVazia(const ArvoreRN* arvore) {
    return arvore == NULL || arvore->raiz == arvore->nil;
}

static void rn_rotacaoEsquerda(ArvoreRN* arvore, NoRN* no) {
    NoRN* filhoDireito = no->direita;
    no->direita = filhoDireito->esquerda;

    if (filhoDireito->esquerda != arvore->nil)
        filhoDireito->esquerda->pai = no;

    filhoDireito->pai = no->pai;
    if (no->pai == arvore->nil)
        arvore->raiz = filhoDireito;
    else if (no == no->pai->esquerda)
        no->pai->esquerda = filhoDireito;
    else
        no->pai->direita = filhoDireito;

    filhoDireito->esquerda = no;
    no->pai = filhoDireito;
}

static void rn_rotacaoDireita(ArvoreRN* arvore, NoRN* no) {
    NoRN* filhoEsquerdo = no->esquerda;
    no->esquerda = filhoEsquerdo->direita;

    if (filhoEsquerdo->direita != arvore->nil)
        filhoEsquerdo->direita->pai = no;

    filhoEsquerdo->pai = no->pai;
    if (no->pai == arvore->nil)
        arvore->raiz = filhoEsquerdo;
    else if (no == no->pai->direita)
        no->pai->direita = filhoEsquerdo;
    else
        no->pai->esquerda = filhoEsquerdo;

    filhoEsquerdo->direita = no;
    no->pai = filhoEsquerdo;
}

static void rn_corrigirInsercao(ArvoreRN* arvore, NoRN* no) {
    while (no->pai->cor == VERMELHO) {
        if (no->pai == no->pai->pai->esquerda) {
            NoRN* tio = no->pai->pai->direita;
            if (tio->cor == VERMELHO) {
                no->pai->cor = PRETO;
                tio->cor = PRETO;
                no->pai->pai->cor = VERMELHO;
                no = no->pai->pai;
            } else {
                if (no == no->pai->direita) {
                    no = no->pai;
                    rn_rotacaoEsquerda(arvore, no);
                }
                no->pai->cor = PRETO;
                no->pai->pai->cor = VERMELHO;
                rn_rotacaoDireita(arvore, no->pai->pai);
            }
        } else {
            NoRN* tio = no->pai->pai->esquerda;
            if (tio->cor == VERMELHO) {
                no->pai->cor = PRETO;
                tio->cor = PRETO;
                no->pai->pai->cor = VERMELHO;
                no = no->pai->pai;
            } else {
                if (no == no->pai->esquerda) {
                    no = no->pai;
                    rn_rotacaoDireita(arvore, no);
                }
                no->pai->cor = PRETO;
                no->pai->pai->cor = VERMELHO;
                rn_rotacaoEsquerda(arvore, no->pai->pai);
            }
        }

        if (no == arvore->raiz)
            break;
    }

    arvore->raiz->cor = PRETO;
}

static void rn_inserirNo(ArvoreRN* arvore, posint v, const Dados* dados) {
    if (!arvore) return;

    NoRN* pai = arvore->nil;
    NoRN* atual = arvore->raiz;

    while (atual != arvore->nil) {
        pai = atual;
        if (v < atual->id)
            atual = atual->esquerda;
        else if (v > atual->id)
            atual = atual->direita;
        else
            return;
    }

    NoRN* no = rn_novoNo(arvore, v, dados);
    no->pai = pai;

    if (pai == arvore->nil)
        arvore->raiz = no;
    else if (v < pai->id)
        pai->esquerda = no;
    else
        pai->direita = no;

    rn_corrigirInsercao(arvore, no);
}

static NoRN* rn_buscarNo(ArvoreRN* arvore, posint v) {
    NoRN* atual = arvore->raiz;
    while (atual != arvore->nil && v != atual->id) {
        if (v < atual->id)
            atual = atual->esquerda;
        else
            atual = atual->direita;
    }
    return atual;
}

static bool rn_buscar(ArvoreRN* arvore, posint v) {
    return rn_buscarNo(arvore, v) != arvore->nil;
}

static void rn_normalizarTexto(const char* origem, char* destino, size_t tam) {
    if (!destino || tam == 0) return;
    size_t i = 0;
    while (origem[i] != '\0' && i + 1 < tam) {
        destino[i] = (char)tolower((unsigned char)origem[i]);
        i++;
    }
    destino[i] = '\0';
}

static int rn_compararTextoIgnorandoCaixa(const char* a, const char* b) {
    char ca[256];
    char cb[256];
    snprintf(ca, sizeof(ca), "%s", a ? a : "");
    snprintf(cb, sizeof(cb), "%s", b ? b : "");
    rn_normalizarTexto(ca, ca, sizeof(ca));
    rn_normalizarTexto(cb, cb, sizeof(cb));
    return strcmp(ca, cb);
}

static NoRN* rn_buscarPorChaveCompostaRec(ArvoreRN* arvore, NoRN* no,
                                         const char* title, const char* performer) {
    if (no == NULL || no == arvore->nil) return arvore->nil;

    if (rn_compararTextoIgnorandoCaixa(no->dados.title, title) == 0 &&
        rn_compararTextoIgnorandoCaixa(no->dados.performer, performer) == 0) {
        return no;
    }

    NoRN* encontradoEsq = rn_buscarPorChaveCompostaRec(arvore, no->esquerda, title, performer);
    if (encontradoEsq != arvore->nil)
        return encontradoEsq;

    return rn_buscarPorChaveCompostaRec(arvore, no->direita, title, performer);
}

static NoRN* rn_buscarPorChaveComposta(ArvoreRN* arvore,
                                       const char* title,
                                       const char* performer) {
    if (!arvore || rn_arvoreVazia(arvore) || !title || !performer) return arvore ? arvore->nil : NULL;
    return rn_buscarPorChaveCompostaRec(arvore, arvore->raiz, title, performer);
}

static bool rn_existeChaveComposta(ArvoreRN* arvore,
                                   const char* title,
                                   const char* performer) {
    if (!arvore || rn_arvoreVazia(arvore)) return false;
    NoRN* encontrado = rn_buscarPorChaveComposta(arvore, title, performer);
    return encontrado != NULL && encontrado != arvore->nil;
}

static NoRN* rn_minimo(ArvoreRN* arvore, NoRN* no) {
    if (no == NULL || no == arvore->nil) return arvore->nil;
    while (no->esquerda != arvore->nil)
        no = no->esquerda;
    return no;
}

static NoRN* rn_maximo(ArvoreRN* arvore, NoRN* no) {
    if (no == NULL || no == arvore->nil) return arvore->nil;
    while (no->direita != arvore->nil)
        no = no->direita;
    return no;
}

static void rn_transplantar(ArvoreRN* arvore, NoRN* u, NoRN* v) {
    if (u->pai == arvore->nil)
        arvore->raiz = v;
    else if (u == u->pai->esquerda)
        u->pai->esquerda = v;
    else
        u->pai->direita = v;
    v->pai = u->pai;
}

static void rn_corrigirRemocao(ArvoreRN* arvore, NoRN* no) {
    while (no != arvore->raiz && no->cor == PRETO) {
        if (no == no->pai->esquerda) {
            NoRN* irmao = no->pai->direita;
            if (irmao->cor == VERMELHO) {
                irmao->cor = PRETO;
                no->pai->cor = VERMELHO;
                rn_rotacaoEsquerda(arvore, no->pai);
                irmao = no->pai->direita;
            }

            if (irmao->esquerda->cor == PRETO && irmao->direita->cor == PRETO) {
                irmao->cor = VERMELHO;
                no = no->pai;
            } else {
                if (irmao->direita->cor == PRETO) {
                    irmao->esquerda->cor = PRETO;
                    irmao->cor = VERMELHO;
                    rn_rotacaoDireita(arvore, irmao);
                    irmao = no->pai->direita;
                }

                irmao->cor = no->pai->cor;
                no->pai->cor = PRETO;
                irmao->direita->cor = PRETO;
                rn_rotacaoEsquerda(arvore, no->pai);
                no = arvore->raiz;
            }
        } else {
            NoRN* irmao = no->pai->esquerda;
            if (irmao->cor == VERMELHO) {
                irmao->cor = PRETO;
                no->pai->cor = VERMELHO;
                rn_rotacaoDireita(arvore, no->pai);
                irmao = no->pai->esquerda;
            }

            if (irmao->direita->cor == PRETO && irmao->esquerda->cor == PRETO) {
                irmao->cor = VERMELHO;
                no = no->pai;
            } else {
                if (irmao->esquerda->cor == PRETO) {
                    irmao->direita->cor = PRETO;
                    irmao->cor = VERMELHO;
                    rn_rotacaoEsquerda(arvore, irmao);
                    irmao = no->pai->esquerda;
                }

                irmao->cor = no->pai->cor;
                no->pai->cor = PRETO;
                irmao->esquerda->cor = PRETO;
                rn_rotacaoDireita(arvore, no->pai);
                no = arvore->raiz;
            }
        }
    }

    no->cor = PRETO;
}

static bool rn_removerNo(ArvoreRN* arvore, posint v) {
    if (!arvore || arvore->raiz == arvore->nil) return false;

    NoRN* z = rn_buscarNo(arvore, v);
    if (z == arvore->nil) return false;

    NoRN* y = z;
    NoRN* x = arvore->nil;
    Cor corOriginal = y->cor;

    if (z->esquerda == arvore->nil) {
        x = z->direita;
        rn_transplantar(arvore, z, z->direita);
    } else if (z->direita == arvore->nil) {
        x = z->esquerda;
        rn_transplantar(arvore, z, z->esquerda);
    } else {
        y = rn_minimo(arvore, z->direita);
        corOriginal = y->cor;
        x = y->direita;
        if (y->pai == z) {
            x->pai = y;
        } else {
            rn_transplantar(arvore, y, y->direita);
            y->direita = z->direita;
            y->direita->pai = y;
        }

        rn_transplantar(arvore, z, y);
        y->esquerda = z->esquerda;
        y->esquerda->pai = y;
        y->cor = z->cor;
    }

    free(z);

    if (corOriginal == PRETO)
        rn_corrigirRemocao(arvore, x);

    return true;
}

static void rn_preOrdemNo(ArvoreRN* arvore, NoRN* no) {
    if (no == NULL || no == arvore->nil) return;
    printf("%u ", no->id);
    rn_preOrdemNo(arvore, no->esquerda);
    rn_preOrdemNo(arvore, no->direita);
}

static void rn_preOrdem(ArvoreRN* arvore) {
    if (rn_arvoreVazia(arvore)) return;
    rn_preOrdemNo(arvore, arvore->raiz);
}

static void rn_emOrdemNo(ArvoreRN* arvore, NoRN* no) {
    if (no == NULL || no == arvore->nil) return;
    rn_emOrdemNo(arvore, no->esquerda);
    printf("%u ", no->id);
    rn_emOrdemNo(arvore, no->direita);
}

static void rn_emOrdem(ArvoreRN* arvore) {
    if (rn_arvoreVazia(arvore)) return;
    rn_emOrdemNo(arvore, arvore->raiz);
}

static void rn_posOrdemNo(ArvoreRN* arvore, NoRN* no) {
    if (no == NULL || no == arvore->nil) return;
    rn_posOrdemNo(arvore, no->esquerda);
    rn_posOrdemNo(arvore, no->direita);
    printf("%u ", no->id);
}

static void rn_posOrdem(ArvoreRN* arvore) {
    if (rn_arvoreVazia(arvore)) return;
    rn_posOrdemNo(arvore, arvore->raiz);
}

static int rn_alturaNo(ArvoreRN* arvore, NoRN* no) {
    if (no == NULL || no == arvore->nil) return -1;
    int altEsq = rn_alturaNo(arvore, no->esquerda);
    int altDir = rn_alturaNo(arvore, no->direita);
    return 1 + (altEsq > altDir ? altEsq : altDir);
}

static int rn_altura(ArvoreRN* arvore) {
    if (!arvore) return -1;
    return rn_alturaNo(arvore, arvore->raiz);
}

static int rn_contarNosNo(ArvoreRN* arvore, NoRN* no) {
    if (no == NULL || no == arvore->nil) return 0;
    return 1 + rn_contarNosNo(arvore, no->esquerda) + rn_contarNosNo(arvore, no->direita);
}

static int rn_contarNos(ArvoreRN* arvore) {
    if (!arvore) return 0;
    return rn_contarNosNo(arvore, arvore->raiz);
}

static void rn_liberarNo(ArvoreRN* arvore, NoRN* no) {
    if (no == NULL || no == arvore->nil) return;
    rn_liberarNo(arvore, no->esquerda);
    rn_liberarNo(arvore, no->direita);
    free(no);
}

static void rn_liberarArvore(ArvoreRN* arvore) {
    if (!arvore) return;
    rn_liberarNo(arvore, arvore->raiz);
    free(arvore->nil);
    free(arvore);
}

static bool rn_lerIntOpcional(const char* s, int* out) {
    if (csvSoEspacos(s)) { *out = DADO_AUSENTE_INT; return true; }
    return csvParaInt(s, out);
}

static bool rn_lerFloatOpcional(const char* s, float* out) {
    if (csvSoEspacos(s)) { *out = DADO_AUSENTE_FLOAT; return true; }
    return csvParaFloat(s, out);
}

enum ColunaDatasetRN {
    COL_RN_SONG_ID, COL_RN_CHART_WEEK, COL_RN_POSITION, COL_RN_PEAK_POS, COL_RN_WKS_ON_CHART,
    COL_RN_TITLE, COL_RN_PERFORMER, COL_RN_GENRE, COL_RN_DANCEABILITY, COL_RN_ENERGY,
    COL_RN_TEMPO, COL_RN_VALENCE, COL_RN_POPULARITY,
    COL_RN_TOTAL
};

typedef struct CargaArvoreRN {
    ArvoreRN* arvore;
    posint proximoId;
    int truncados;
} CargaArvoreRN;

static bool rn_dadosDeCampos(char* campos[], int n, int numLinha,
                             Dados* d, int* truncados) {
    if (n != COL_RN_TOTAL) {
        fprintf(stderr, "Linha %d: esperadas %d colunas, ignorada\n",
                numLinha, COL_RN_TOTAL);
        return false;
    }

    memset(d, 0, sizeof *d);

    const char* invalido = NULL;
    if      (csvSoEspacos(campos[COL_RN_CHART_WEEK])) invalido = "chart_week";
    else if (csvSoEspacos(campos[COL_RN_TITLE]))     invalido = "title";
    else if (csvSoEspacos(campos[COL_RN_PERFORMER])) invalido = "performer";
    else if (!csvParaInt(campos[COL_RN_POSITION], &d->position)) invalido = "position";
    else if (!csvParaInt(campos[COL_RN_PEAK_POS], &d->peak_pos)) invalido = "peak_pos";
    else if (!csvParaInt(campos[COL_RN_WKS_ON_CHART], &d->wks_on_chart)) invalido = "wks_on_chart";
    else if (!rn_lerFloatOpcional(campos[COL_RN_DANCEABILITY], &d->danceability)) invalido = "danceability";
    else if (!rn_lerFloatOpcional(campos[COL_RN_ENERGY], &d->energy)) invalido = "energy";
    else if (!rn_lerFloatOpcional(campos[COL_RN_TEMPO], &d->tempo)) invalido = "tempo";
    else if (!rn_lerFloatOpcional(campos[COL_RN_VALENCE], &d->valence)) invalido = "valence";
    else if (!rn_lerIntOpcional(campos[COL_RN_POPULARITY], &d->popularity)) invalido = "popularity";

    if (invalido) {
        fprintf(stderr, "Linha %d: campo '%s' ausente ou inválido, ignorada\n",
                numLinha, invalido);
        return false;
    }

    if (!csvCopiarTexto(d->chart_week, sizeof d->chart_week, campos[COL_RN_CHART_WEEK])) (*truncados)++;
    if (!csvCopiarTexto(d->title, sizeof d->title, campos[COL_RN_TITLE])) (*truncados)++;
    if (!csvCopiarTexto(d->performer, sizeof d->performer, campos[COL_RN_PERFORMER])) (*truncados)++;
    if (!csvCopiarTexto(d->genre, sizeof d->genre, campos[COL_RN_GENRE])) (*truncados)++;
    return true;
}

static bool rn_linhaCsv(char* campos[], int n, int numLinha, void* ctx) {
    CargaArvoreRN* carga = (CargaArvoreRN*)ctx;
    Dados d;
    if (!rn_dadosDeCampos(campos, n, numLinha, &d, &carga->truncados))
        return false;

    rn_inserirNo(carga->arvore, carga->proximoId, &d);
    carga->proximoId++;
    return true;
}

static int carregarArvoreRubroCsv(const char* caminho, ArvoreRN** arvore, CsvResultado* res) {
    if (!arvore) return CSV_ERRO_CONFIG;
    if (!*arvore) *arvore = rn_criarArvore();

    CsvConfig cfg = { ',', true, COL_RN_TOTAL };
    CargaArvoreRN carga = { *arvore, 1, 0 };
    if (!rn_arvoreVazia(*arvore))
        carga.proximoId = rn_maximo(*arvore, (*arvore)->raiz)->id + 1;

    int status = csvCarregar(caminho, &cfg, rn_linhaCsv, &carga, res);
    *arvore = carga.arvore;

    if (carga.truncados > 0) {
        fprintf(stderr, "Aviso: %d texto(s) truncado(s); aumente os buffers dos campos de texto.\n",
                carga.truncados);
    }

    return status;
}

static void rn_imprimirNo(ArvoreRN* arvore, NoRN* no, int nivel) {
    if (no == NULL || no == arvore->nil) return;
    rn_imprimirNo(arvore, no->direita, nivel + 1);
    for (int i = 0; i < nivel; i++) printf("    ");
    printf("%u (%s)\n", no->id, no->cor == VERMELHO ? "V" : "P");
    rn_imprimirNo(arvore, no->esquerda, nivel + 1);
}

static void rn_imprimir(ArvoreRN* arvore, int nivel) {
    if (!arvore) return;
    rn_imprimirNo(arvore, arvore->raiz, nivel);
}

static int rn_validarBST(ArvoreRN* arvore, NoRN* no, int temMin, int min, int temMax, int max) {
    if (no == arvore->nil) return 1;
    if ((temMin && no->id <= (posint)min) || (temMax && no->id >= (posint)max))
        return 0;
    return rn_validarBST(arvore, no->esquerda, temMin, min, 1, no->id) &&
           rn_validarBST(arvore, no->direita, 1, no->id, temMax, max);
}

static int rn_semVermelhoConsecutivo(ArvoreRN* arvore, NoRN* no) {
    if (no == arvore->nil) return 1;
    if (no->cor == VERMELHO &&
        (no->esquerda->cor == VERMELHO || no->direita->cor == VERMELHO))
        return 0;
    return rn_semVermelhoConsecutivo(arvore, no->esquerda) &&
           rn_semVermelhoConsecutivo(arvore, no->direita);
}

static int rn_alturaPreta(ArvoreRN* arvore, NoRN* no, int* valido) {
    if (no == arvore->nil) return 1;
    int esquerda = rn_alturaPreta(arvore, no->esquerda, valido);
    int direita = rn_alturaPreta(arvore, no->direita, valido);
    if (esquerda != direita) *valido = 0;
    return (no->cor == PRETO ? 1 : 0) + esquerda;
}

static int rn_validar(ArvoreRN* arvore) {
    if (!arvore || arvore->raiz == arvore->nil) return 1;
    if (arvore->raiz->cor != PRETO) return 0;
    if (arvore->nil->cor != PRETO) return 0;
    if (!rn_validarBST(arvore, arvore->raiz, 0, 0, 0, 0)) return 0;
    if (!rn_semVermelhoConsecutivo(arvore, arvore->raiz)) return 0;

    int valido = 1;
    rn_alturaPreta(arvore, arvore->raiz, &valido);
    return valido;
}

/* ------------------------------------------------------------------ */
/* Serialização                                                       */
/* ------------------------------------------------------------------ */

/*
 * Formato (texto UTF-8, uma linha por item, campos separados por vírgula como
 * no dataset):
 *   linha 1: RN,<versão>,<quantidade de nós>
 *   linha 2: nomes das colunas
 *   demais : percurso em PRÉ-ORDEM. Cada nó vira uma linha
 *              id,cor,chart_week,position,peak_pos,wks_on_chart,title,
 *              performer,genre,danceability,energy,tempo,valence,popularity
 *            (cor = V para vermelho, P para preto) e cada subárvore vazia
 *            (sentinela nil) vira uma linha contendo só "-".
 * Os marcadores "-" e as cores guardam a forma exata da árvore. Valores
 * ausentes saem como campo vazio, igual ao dataset original. Textos com
 * vírgula, aspas ou quebra de linha saem entre aspas (aspas duplicadas).
 */
#define RN_SERIAL_VERSAO 1

static void rn_serialTexto(FILE* f, const char* s) {
    if (!strpbrk(s, ",\"\r\n")) { fputs(s, f); return; }
    fputc('"', f);
    for (; *s; s++) {
        if (*s == '"') fputc('"', f);
        fputc(*s, f);
    }
    fputc('"', f);
}

/* Sempre com '.', mesmo que o programa use um locale com decimal em ','. */
static void rn_serialFloat(FILE* f, float v) {
    if (v == DADO_AUSENTE_FLOAT) return;
    char buf[32];
    snprintf(buf, sizeof buf, "%.6g", v);
    if (strtof(buf, NULL) != v)            /* 6 dígitos não bastaram: usa 9 */
        snprintf(buf, sizeof buf, "%.9g", v);
    char ponto = localeconv()->decimal_point[0];
    for (char* p = buf; *p; p++)
        if (*p == ponto) *p = '.';
    fputs(buf, f);
}

static void rn_serialInt(FILE* f, int v) {
    if (v == DADO_AUSENTE_INT) return;
    fprintf(f, "%d", v);
}

static void rn_serializarNo(FILE* f, const ArvoreRN* arvore, const NoRN* no) {
    if (no == NULL || no == arvore->nil) { fputs("-\n", f); return; }

    const Dados* d = &no->dados;
    fprintf(f, "%u,%c,", no->id, no->cor == VERMELHO ? 'V' : 'P');
    rn_serialTexto(f, d->chart_week);   fputc(',', f);
    rn_serialInt(f, d->position);       fputc(',', f);
    rn_serialInt(f, d->peak_pos);       fputc(',', f);
    rn_serialInt(f, d->wks_on_chart);   fputc(',', f);
    rn_serialTexto(f, d->title);        fputc(',', f);
    rn_serialTexto(f, d->performer);    fputc(',', f);
    rn_serialTexto(f, d->genre);        fputc(',', f);
    rn_serialFloat(f, d->danceability); fputc(',', f);
    rn_serialFloat(f, d->energy);       fputc(',', f);
    rn_serialFloat(f, d->tempo);        fputc(',', f);
    rn_serialFloat(f, d->valence);      fputc(',', f);
    rn_serialInt(f, d->popularity);
    fputc('\n', f);

    rn_serializarNo(f, arvore, no->esquerda);
    rn_serializarNo(f, arvore, no->direita);
}

/*
 * Grava a árvore em 'caminho' no formato descrito acima (árvore vazia gera
 * só o cabeçalho). Devolve a quantidade de nós gravados ou -1 se houver erro.
 */
static int rn_serializarArvore(ArvoreRN* arvore, const char* caminho) {
    if (!arvore || !caminho) return -1;

    FILE* f = fopen(caminho, "w");
    if (!f) { perror(caminho); return -1; }

    int nos = rn_contarNos(arvore);
    fprintf(f, "RN,%d,%d\n", RN_SERIAL_VERSAO, nos);
    fputs("id,cor,chart_week,position,peak_pos,wks_on_chart,title,performer,"
          "genre,danceability,energy,tempo,valence,popularity\n", f);
    rn_serializarNo(f, arvore, arvore->raiz);

    bool erro = ferror(f) != 0;
    if (fclose(f) != 0) erro = true;
    return erro ? -1 : nos;
}

static int iniciarArvoreRN(ArvoreRN* arvore) {
    ArvoreRN* rn_criarArvore(void);
    void rn_liberarArvore(ArvoreRN*);
    bool rn_arvoreVazia(const ArvoreRN*);
    void rn_inserirNo(ArvoreRN*, posint, const Dados*);
    bool rn_buscar(ArvoreRN*, posint);
    bool rn_removerNo(ArvoreRN*, posint);
    NoRN* rn_minimo(ArvoreRN*, NoRN*);
    NoRN* rn_maximo(ArvoreRN*, NoRN*);
    void rn_preOrdem(ArvoreRN*);
    void rn_emOrdem(ArvoreRN*);
    void rn_posOrdem(ArvoreRN*);
    int rn_altura(ArvoreRN*);
    int rn_contarNos(ArvoreRN*);
    int carregarArvoreRubroCsv(const char*, ArvoreRN**, CsvResultado*);
    void rn_imprimir(ArvoreRN*, int);
    int rn_validar(ArvoreRN*);
    int rn_serializarArvore(ArvoreRN*, const char*);
    (void)arvore;
    return 0;
}

#endif
