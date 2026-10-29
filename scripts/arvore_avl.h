#ifndef ARVORE_AVL_H
#define  ARVORE_AVL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <locale.h>
#include "leitor_dataset.h"

#ifndef DADOS_ESTRUTURA_DEFINIDA
#define DADOS_ESTRUTURA_DEFINIDA

typedef unsigned int posint;

/*
 * Colunas do dataset:
 *   song_id,chart_week,position,peak_pos,wks_on_chart,title,performer,genre,
 *   danceability,energy,tempo,valence,popularity
 *
 * O song_id NÃO é guardado: ele se repete (a mesma música aparece em várias
 * semanas) e é sempre "title__performer". A chave primária da árvore é o campo
 * 'id' do nó, gerado sequencialmente durante a carga (veja carregarArvoreCsv).
 */

/* Tamanhos dos textos, com folga sobre o maior valor do dataset (em bytes,
 * contando o '\0'): title 68, performer 114, genre 231. */
#ifndef TAM_TITULO
#define TAM_TITULO   80
#endif
#ifndef TAM_ARTISTA
#define TAM_ARTISTA  120
#endif
#ifndef TAM_GENERO
#define TAM_GENERO   240
#endif

/* O dataset deixa vazios genre, danceability, energy, tempo, valence e
 * popularity em muitas linhas. Esses registros são mantidos; o campo numérico
 * ausente recebe os valores abaixo (fora da faixa real) e o genre fica "". */
#define DADO_AUSENTE_INT    (-1)
#define DADO_AUSENTE_FLOAT  (-1.0f)

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

typedef struct No {
    posint id;          /* chave primária (única), gerada na carga */
    Dados dados;
    int altura;
    struct No* esquerda;
    struct No* direita;
} No;

No* novoNo(posint v, const Dados* dados) {
    No* n = (No*)malloc(sizeof(No));
    if (!n) {
        printf("Falha ao alocar nó");
        exit(EXIT_FAILURE);
    }
    n->id = v;
    if (dados) n->dados = *dados;
    else       memset(&n->dados, 0, sizeof n->dados);
    n->altura = 0;
    n->esquerda = NULL;
    n->direita = NULL;
    return n;
}

void liberarArvore(No* r) {
    if (r != NULL) {
        liberarArvore(r->direita);
        liberarArvore(r->esquerda);
        free(r);
    }
}

bool buscar(No* r, posint v) {
    if (r == NULL) return false;
    if (v == r->id) return true;
    if (v < r->id) return buscar(r->esquerda, v);
    return buscar(r->direita, v);
}

No* minimo(No* r) {
    if (r == NULL) return NULL;
    while (r->esquerda != NULL) {
        r = r->esquerda;
    }
    return r;
}

No* maximo(No* r) {
    if (r == NULL) return NULL;
    while (r->direita != NULL) {
        r = r->direita;
    }
    return r;
}

void preOrdem(No* r) {
    if (r == NULL) return;
    printf("%u ", r->id);
    preOrdem(r->esquerda);
    preOrdem(r->direita);
}

void emOrdem(No* r) {
    if (r == NULL) return;
    emOrdem(r->esquerda);
    printf("%u ", r->id);
    emOrdem(r->direita);
}

void posOrdem(No* r) {
    if (r == NULL) return;
    posOrdem(r->esquerda);
    posOrdem(r->direita);
    printf("%u ", r->id);
}

int altura(No* r) {
    if (r == NULL) return -1;
    return r->altura;   /* mantido por atualizarAltura(); evita percorrer a subárvore */
}

int contarNos(No* r) {
    if (r == NULL) return 0;
    return 1 + contarNos(r->esquerda) + contarNos(r->direita);
}

bool arvoreVazia(No* r) {
    return r == NULL;
}

void atualizarAltura(No* r) {
    if (r == NULL) return;
    int alturaEsquerda = altura(r->esquerda);
    int alturaDireita = altura(r->direita);
    r->altura = 1 + (alturaEsquerda > alturaDireita ? alturaEsquerda :
    alturaDireita);
}

int fatorBalanceamento(No* r) {
    if (r == NULL) return 0;
    return altura(r->esquerda) - altura(r->direita);
}

No* rotacaoDireita(No* y) {
    No* x = y->esquerda;
    No* T2 = x->direita;
    x->direita = y;
    y->esquerda = T2;
    atualizarAltura(y);
    atualizarAltura(x);
    return x;
}

No* rotacaoEsquerda(No* x) {
    No* y = x->direita;
    No* T2 = y->esquerda;
    y->esquerda = x;
    x->direita = T2;
    atualizarAltura(x);
    atualizarAltura(y);
    return y;
}

No* balancear(No* r) {
    atualizarAltura(r);
    int fb = fatorBalanceamento(r);
    if (fb > 1) {
        if (fatorBalanceamento(r->esquerda) < 0) {
            r->esquerda = rotacaoEsquerda(r->esquerda);
        }
        return rotacaoDireita(r);
    }
    if (fb < -1) {
        if (fatorBalanceamento(r->direita) > 0) {
            r->direita = rotacaoDireita(r->direita);
        }
        return rotacaoEsquerda(r);
    }
    return r;
}

No* removerNo(No* r, posint v) {
    if (r == NULL) return NULL;
    if (v < r->id) {
        r->esquerda = removerNo(r->esquerda, v);
    } else if (v > r->id) {
        r->direita = removerNo(r->direita, v);
    } else {
        // Achou o nó a remover
        if (r->esquerda == NULL && r->direita == NULL) {
            // Caso 1: folha
            free(r);
            return NULL;
        } else if (r->esquerda == NULL) {
            // Caso 2a: só filho direito
            No* temp = r->direita;
            free(r);
            return temp;
        } else if (r->direita == NULL) {
            // Caso 2b: só filho esquerdo
            No* temp = r->esquerda;
            free(r);
            return temp;
        } else {
            // Caso 3: dois filhos -> usa sucessor em-ordem (menor da direita)
            No* suc = minimo(r->direita);
            r->id = suc->id;
            r->dados = suc->dados;
            r->direita = removerNo(r->direita, suc->id);
        }
    }
    return balancear(r);
}

No* inserirNo(No* r, posint v, const Dados* dados) {
    if (r == NULL) return novoNo(v, dados);
    if (v < r->id) {
        r->esquerda = inserirNo(r->esquerda, v, dados);
    } else if (v > r->id) {
        r->direita = inserirNo(r->direita, v, dados);
    } else {
        return r;
    }
    return balancear(r);
}

/* ------------------------------------------------------------------ */
/* Integração com leitor_dataset.h                                    */
/* ------------------------------------------------------------------ */

/* Posição de cada coluna no arquivo do dataset. */
enum ColunaDataset {
    COL_SONG_ID, COL_CHART_WEEK, COL_POSITION, COL_PEAK_POS, COL_WKS_ON_CHART,
    COL_TITLE, COL_PERFORMER, COL_GENRE, COL_DANCEABILITY, COL_ENERGY,
    COL_TEMPO, COL_VALENCE, COL_POPULARITY,
    COL_TOTAL                      /* quantidade de colunas */
};

/* Estado repassado ao callback do leitor (campo 'ctx' de csvCarregar). */
typedef struct CargaAvl {
    No*    raiz;        /* árvore em construção                          */
    posint proximoId;   /* próximo id (chave primária) a ser atribuído   */
    int    truncados;   /* textos que não couberam nos buffers de Dados  */
} CargaAvl;

/* Campos opcionais: vazio vira "ausente"; só é erro se houver texto inválido. */
static bool lerIntOpcional(const char* s, int* out) {
    if (csvSoEspacos(s)) { *out = DADO_AUSENTE_INT; return true; }
    return csvParaInt(s, out);
}

static bool lerFloatOpcional(const char* s, float* out) {
    if (csvSoEspacos(s)) { *out = DADO_AUSENTE_FLOAT; return true; }
    return csvParaFloat(s, out);
}

/*
 * Converte as colunas de uma linha do dataset em um registro Dados. Não
 * conhece a árvore, então o mesmo código serve a qualquer outra estrutura.
 * Obrigatórios: chart_week, position, peak_pos, wks_on_chart, title, performer.
 * Opcionais:    genre, danceability, energy, tempo, valence, popularity.
 * Devolve false (e explica o motivo em stderr) se a linha deve ser rejeitada.
 */
static bool dadosDeCampos(char* campos[], int n, int numLinha,
                          Dados* d, int* truncados) {
    if (n != COL_TOTAL) {
        fprintf(stderr, "Linha %d: esperadas %d colunas, ignorada\n",
                numLinha, COL_TOTAL);
        return false;
    }
    memset(d, 0, sizeof *d);

    const char* invalido = NULL;
    if      (csvSoEspacos(campos[COL_CHART_WEEK]))  invalido = "chart_week";
    else if (csvSoEspacos(campos[COL_TITLE]))       invalido = "title";
    else if (csvSoEspacos(campos[COL_PERFORMER]))   invalido = "performer";
    else if (!csvParaInt(campos[COL_POSITION], &d->position))         invalido = "position";
    else if (!csvParaInt(campos[COL_PEAK_POS], &d->peak_pos))         invalido = "peak_pos";
    else if (!csvParaInt(campos[COL_WKS_ON_CHART], &d->wks_on_chart)) invalido = "wks_on_chart";
    else if (!lerFloatOpcional(campos[COL_DANCEABILITY], &d->danceability)) invalido = "danceability";
    else if (!lerFloatOpcional(campos[COL_ENERGY], &d->energy))       invalido = "energy";
    else if (!lerFloatOpcional(campos[COL_TEMPO], &d->tempo))         invalido = "tempo";
    else if (!lerFloatOpcional(campos[COL_VALENCE], &d->valence))     invalido = "valence";
    else if (!lerIntOpcional(campos[COL_POPULARITY], &d->popularity)) invalido = "popularity";

    if (invalido) {
        fprintf(stderr, "Linha %d: campo '%s' ausente ou inválido, ignorada\n",
                numLinha, invalido);
        return false;
    }

    if (!csvCopiarTexto(d->chart_week, sizeof d->chart_week, campos[COL_CHART_WEEK])) (*truncados)++;
    if (!csvCopiarTexto(d->title,      sizeof d->title,      campos[COL_TITLE]))      (*truncados)++;
    if (!csvCopiarTexto(d->performer,  sizeof d->performer,  campos[COL_PERFORMER]))  (*truncados)++;
    if (!csvCopiarTexto(d->genre,      sizeof d->genre,      campos[COL_GENRE]))      (*truncados)++;
    return true;
}

/* Callback do leitor: uma linha do arquivo vira um nó com id novo. */
static bool avlLinhaCsv(char* campos[], int n, int numLinha, void* ctx) {
    CargaAvl* c = (CargaAvl*)ctx;
    Dados d;
    if (!dadosDeCampos(campos, n, numLinha, &d, &c->truncados)) return false;
    c->raiz = inserirNo(c->raiz, c->proximoId, &d);
    c->proximoId++;
    return true;
}

/*
 * Lê o arquivo do dataset (csvCarregar) e insere cada linha aceita na árvore.
 * O id de cada registro é gerado aqui: sequencial, na ordem do arquivo. Se a
 * árvore já tiver nós, a numeração continua depois do maior id existente.
 * 'res' pode ser NULL. Devolve CSV_OK ou um código negativo de CsvStatus
 * (em caso de erro durante a leitura, a árvore fica com o que foi lido).
 */
int carregarArvoreCsv(const char* caminho, No** raiz, CsvResultado* res) {
    if (!raiz) return CSV_ERRO_CONFIG;

    CsvConfig cfg = { ',', true, COL_TOTAL };
    CargaAvl carga = { *raiz, *raiz ? maximo(*raiz)->id + 1 : 1, 0 };

    int status = csvCarregar(caminho, &cfg, avlLinhaCsv, &carga, res);
    *raiz = carga.raiz;

    if (carga.truncados > 0)
        fprintf(stderr, "Aviso: %d texto(s) truncado(s); aumente TAM_TITULO/TAM_ARTISTA/TAM_GENERO\n",
                carga.truncados);
    return status;
}

/* ------------------------------------------------------------------ */
/* Serialização                                                       */
/* ------------------------------------------------------------------ */

/*
 * Formato (texto UTF-8, uma linha por item, campos separados por vírgula como
 * no dataset):
 *   linha 1: AVL,<versão>,<quantidade de nós>
 *   linha 2: nomes das colunas
 *   demais : percurso em PRÉ-ORDEM. Cada nó vira uma linha
 *              id,altura,chart_week,position,peak_pos,wks_on_chart,title,
 *              performer,genre,danceability,energy,tempo,valence,popularity
 *            e cada subárvore vazia vira uma linha contendo só "-".
 * Os marcadores "-" guardam a forma exata da árvore (pré-ordem sozinha não
 * basta: reinserir em ordem de pré-ordem numa AVL dispara rotações). Valores
 * ausentes saem como campo vazio, igual ao dataset original. Textos com
 * vírgula, aspas ou quebra de linha saem entre aspas (aspas duplicadas).
 */
#define AVL_SERIAL_VERSAO 1

static void serialTexto(FILE* f, const char* s) {
    if (!strpbrk(s, ",\"\r\n")) { fputs(s, f); return; }
    fputc('"', f);
    for (; *s; s++) {
        if (*s == '"') fputc('"', f);
        fputc(*s, f);
    }
    fputc('"', f);
}

/* Sempre com '.', mesmo que o programa use um locale com decimal em ','. */
static void serialFloat(FILE* f, float v) {
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

static void serialInt(FILE* f, int v) {
    if (v == DADO_AUSENTE_INT) return;
    fprintf(f, "%d", v);
}

static void serializarNo(FILE* f, const No* r) {
    if (r == NULL) { fputs("-\n", f); return; }

    const Dados* d = &r->dados;
    fprintf(f, "%u,%d,", r->id, r->altura);
    serialTexto(f, d->chart_week);  fputc(',', f);
    serialInt(f, d->position);      fputc(',', f);
    serialInt(f, d->peak_pos);      fputc(',', f);
    serialInt(f, d->wks_on_chart);  fputc(',', f);
    serialTexto(f, d->title);       fputc(',', f);
    serialTexto(f, d->performer);   fputc(',', f);
    serialTexto(f, d->genre);       fputc(',', f);
    serialFloat(f, d->danceability); fputc(',', f);
    serialFloat(f, d->energy);       fputc(',', f);
    serialFloat(f, d->tempo);        fputc(',', f);
    serialFloat(f, d->valence);      fputc(',', f);
    serialInt(f, d->popularity);
    fputc('\n', f);

    serializarNo(f, r->esquerda);
    serializarNo(f, r->direita);
}

/*
 * Grava a árvore em 'caminho' no formato descrito acima (árvore vazia gera
 * só o cabeçalho). Devolve a quantidade de nós gravados ou -1 se houver erro.
 */
int serializarArvore(No* raiz, const char* caminho) {
    if (!caminho) return -1;

    FILE* f = fopen(caminho, "w");
    if (!f) { perror(caminho); return -1; }

    int nos = contarNos(raiz);
    fprintf(f, "AVL,%d,%d\n", AVL_SERIAL_VERSAO, nos);
    fputs("id,altura,chart_week,position,peak_pos,wks_on_chart,title,performer,"
          "genre,danceability,energy,tempo,valence,popularity\n", f);
    serializarNo(f, raiz);

    bool erro = ferror(f) != 0;
    if (fclose(f) != 0) erro = true;
    return erro ? -1 : nos;
}

/* ------------------------------------------------------------------ */
/* Busca por chave composta (música + artista)                        */
/* ------------------------------------------------------------------ */

/*
 * A chave principal da árvore é o 'id'. A chave composta (title, performer)
 * NÃO é única: a mesma música aparece uma vez por semana na parada (até 112
 * registros por música no dataset). Por isso a busca usa um índice secundário:
 * outra AVL, ordenada por (title, performer), em que cada nó representa uma
 * música e guarda os ids de todos os registros dela.
 *
 *   NoChave* indice = construirIndiceChave(raiz);              // após a carga
 *   const No* achados[10];
 *   int n = buscarPorChaveComposta(raiz, indice, "Lose Control", "Teddy Swims",
 *                                  achados, 10);
 *   ...
 *   liberarIndiceChave(indice);
 *
 * A comparação ignora maiúsculas/minúsculas (só ASCII): o dataset grafa o
 * mesmo artista de formas diferentes ("Charli XCX" / "Charli xcx").
 * Custos, com K = músicas distintas, N = registros e m = registros da música:
 * construir O(N log K); buscar O(log K + m log N).
 *
 * O índice guarda ids, não ponteiros. Se um registro for removido da árvore
 * principal, a busca simplesmente o ignora. Registros inseridos depois de
 * construir o índice só aparecem após indexarRegistro() ou nova construção.
 */
typedef struct NoChave {
    char     titulo[TAM_TITULO];    /* grafia do primeiro registro indexado */
    char     artista[TAM_ARTISTA];
    posint*  ids;                   /* ids dos registros com esta chave     */
    int      qtd;                   /* quantos ids há em 'ids'              */
    int      cap;                   /* capacidade alocada de 'ids'          */
    int      altura;
    struct NoChave* esquerda;
    struct NoChave* direita;
} NoChave;

/* Comparação ASCII sem diferenciar caixa; 0 se iguais. */
static int compararTexto(const char* a, const char* b) {
    for (;; a++, b++) {
        int ca = (unsigned char)*a, cb = (unsigned char)*b;
        if (ca >= 'A' && ca <= 'Z') ca += 'a' - 'A';
        if (cb >= 'A' && cb <= 'Z') cb += 'a' - 'A';
        if (ca != cb) return ca - cb;
        if (ca == '\0') return 0;
    }
}

/* Ordem da chave composta: primeiro a música, depois o artista. */
static int compararChave(const char* titulo1, const char* artista1,
                         const char* titulo2, const char* artista2) {
    int c = compararTexto(titulo1, titulo2);
    return c != 0 ? c : compararTexto(artista1, artista2);
}

/* Balanceamento do índice (mesma lógica da árvore principal). */
static int alturaChave(const NoChave* r) { return r ? r->altura : -1; }

static int fatorChave(const NoChave* r) {
    return r ? alturaChave(r->esquerda) - alturaChave(r->direita) : 0;
}

static void atualizarAlturaChave(NoChave* r) {
    int he = alturaChave(r->esquerda), hd = alturaChave(r->direita);
    r->altura = 1 + (he > hd ? he : hd);
}

static NoChave* rotacaoDireitaChave(NoChave* y) {
    NoChave* x = y->esquerda;
    NoChave* T2 = x->direita;
    x->direita = y;
    y->esquerda = T2;
    atualizarAlturaChave(y);
    atualizarAlturaChave(x);
    return x;
}

static NoChave* rotacaoEsquerdaChave(NoChave* x) {
    NoChave* y = x->direita;
    NoChave* T2 = y->esquerda;
    y->esquerda = x;
    x->direita = T2;
    atualizarAlturaChave(x);
    atualizarAlturaChave(y);
    return y;
}

static NoChave* balancearChave(NoChave* r) {
    atualizarAlturaChave(r);
    int fb = fatorChave(r);
    if (fb > 1) {
        if (fatorChave(r->esquerda) < 0) r->esquerda = rotacaoEsquerdaChave(r->esquerda);
        return rotacaoDireitaChave(r);
    }
    if (fb < -1) {
        if (fatorChave(r->direita) > 0) r->direita = rotacaoDireitaChave(r->direita);
        return rotacaoEsquerdaChave(r);
    }
    return r;
}

static void adicionarId(NoChave* n, posint id) {
    if (n->qtd == n->cap) {
        int nova = n->cap ? n->cap * 2 : 4;
        posint* p = (posint*)realloc(n->ids, (size_t)nova * sizeof(posint));
        if (!p) {
            printf("Falha ao alocar índice");
            exit(EXIT_FAILURE);
        }
        n->ids = p;
        n->cap = nova;
    }
    n->ids[n->qtd++] = id;
}

static NoChave* novoNoChave(const char* titulo, const char* artista) {
    NoChave* n = (NoChave*)calloc(1, sizeof(NoChave));
    if (!n) {
        printf("Falha ao alocar nó do índice");
        exit(EXIT_FAILURE);
    }
    snprintf(n->titulo, sizeof n->titulo, "%s", titulo);
    snprintf(n->artista, sizeof n->artista, "%s", artista);
    return n;
}

/* Chave que já existe só ganha mais um id; chave nova cria um nó. */
static NoChave* inserirChave(NoChave* r, const char* titulo, const char* artista, posint id) {
    if (r == NULL) {
        NoChave* n = novoNoChave(titulo, artista);
        adicionarId(n, id);
        return n;
    }
    int c = compararChave(titulo, artista, r->titulo, r->artista);
    if (c < 0) {
        r->esquerda = inserirChave(r->esquerda, titulo, artista, id);
    } else if (c > 0) {
        r->direita = inserirChave(r->direita, titulo, artista, id);
    } else {
        adicionarId(r, id);
        return r;
    }
    return balancearChave(r);
}

/* Acrescenta um registro ao índice; devolve a (possivelmente nova) raiz. */
NoChave* indexarRegistro(NoChave* indice, const No* registro) {
    return inserirChave(indice, registro->dados.title, registro->dados.performer,
                        registro->id);
}

static NoChave* indexarSubarvore(NoChave* indice, const No* r) {
    if (r == NULL) return indice;
    indice = indexarSubarvore(indice, r->esquerda);
    indice = indexarRegistro(indice, r);
    return indexarSubarvore(indice, r->direita);
}

/* Cria o índice com todos os registros da árvore principal (NULL se vazia). */
NoChave* construirIndiceChave(const No* raiz) {
    return indexarSubarvore(NULL, raiz);
}

/* Quantidade de músicas distintas (chaves) no índice. */
int contarChaves(const NoChave* r) {
    if (r == NULL) return 0;
    return 1 + contarChaves(r->esquerda) + contarChaves(r->direita);
}

void liberarIndiceChave(NoChave* r) {
    if (r == NULL) return;
    liberarIndiceChave(r->esquerda);
    liberarIndiceChave(r->direita);
    free(r->ids);
    free(r);
}

/* Nó do índice da música (ou NULL). Ignora maiúsculas/minúsculas. */
NoChave* buscarChave(NoChave* r, const char* titulo, const char* artista) {
    if (!titulo || !artista) return NULL;
    while (r) {
        int c = compararChave(titulo, artista, r->titulo, r->artista);
        if (c == 0) return r;
        r = c < 0 ? r->esquerda : r->direita;
    }
    return NULL;
}

static const No* registroPorId(const No* r, posint id) {
    while (r && r->id != id) r = id < r->id ? r->esquerda : r->direita;
    return r;
}

/*
 * Busca os registros de uma música pela chave composta (título + artista).
 * Devolve quantos registros existem na árvore principal com essa chave (0 se
 * não houver) e preenche 'saida' com até 'max' deles, em ordem crescente de id.
 * 'saida' pode ser NULL para só contar.
 */
int buscarPorChaveComposta(const No* raiz, NoChave* indice, const char* titulo,
                           const char* artista, const No** saida, int max) {
    NoChave* chave = buscarChave(indice, titulo, artista);
    if (!chave) return 0;

    int total = 0;
    for (int i = 0; i < chave->qtd; i++) {
        const No* reg = registroPorId(raiz, chave->ids[i]);
        /* removido (ou id reaproveitado por outra música) depois da indexação */
        if (!reg || compararChave(reg->dados.title, reg->dados.performer,
                                  titulo, artista) != 0) continue;
        if (saida && total < max) saida[total] = reg;
        total++;
    }
    return total;
}

int iniciarArvore(No* raiz) {
    No* novoNo(posint, const Dados*);
    No* inserirNo(No*, posint, const Dados*);
    int carregarArvoreCsv(const char*, No**, CsvResultado*);
    int serializarArvore(No*, const char*);
    NoChave* indexarRegistro(NoChave*, const No*);
    NoChave* construirIndiceChave(const No*);
    int contarChaves(const NoChave*);
    void liberarIndiceChave(NoChave*);
    NoChave* buscarChave(NoChave*, const char*, const char*);
    int buscarPorChaveComposta(const No*, NoChave*, const char*, const char*, const No**, int);
    void liberarArvore(No*);
    bool buscar(No*, posint);
    No* removerNo(No*, posint);
    No* minimo(No*);
    No* maximo(No*);
    void preOrdem(No*);
    void emOrdem(No*);
    void posOrdem(No*);
    int altura(No*);
    int contarNos(No*);
    bool arvoreVazia(No*);
    (void)raiz;
    return 0;
}

#endif