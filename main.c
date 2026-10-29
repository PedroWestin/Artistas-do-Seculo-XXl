#include "scripts/arvore_avl.h"
#include "scripts/arvore_rubro_negra.h"
#include "scripts/leitor_dataset.h"
#include <locale.h>
#include <time.h>

#define CAMINHO_DATASET  "dataset/billboard_hot100_50k_dataset.txt"
#define CAMINHO_SERIAL_AVL   "dataset/arvore_avl_serializada.txt"
#define CAMINHO_SERIAL_RN "dataset/arvore_rubro_negra_serializada.txt"
#define LIMITE_PERCURSO  40

static void mostrarRegistro(const char* rotulo, const No* n) {
    if (!n) {
        printf("%s: registro ausente\n", rotulo);
        return;
    }
    printf("%s: id %u | %s - %s | semana %s | pos %d\n", rotulo, n->id,
           n->dados.performer, n->dados.title, n->dados.chart_week,
           n->dados.position);
}

static void mostrarPercursos(No* raiz) {
    int total = contarNos(raiz);
    if (total > LIMITE_PERCURSO) {
        printf("Percursos omitidos: árvore com %d nós (limite de impressão: %d).\n",
               total, LIMITE_PERCURSO);
        return;
    }
    printf("Pre-ordem: "); preOrdem(raiz); printf("\n");
    printf("Em-ordem: "); emOrdem(raiz); printf("\n");
    printf("Pos-ordem: "); posOrdem(raiz); printf("\n");
}

static void mostrarPercursosRN(ArvoreRN* arvore) {
    int total = rn_contarNos(arvore);
    if (total > LIMITE_PERCURSO) {
        printf("Percursos omitidos: árvore com %d nós (limite de impressão: %d).\n",
               total, LIMITE_PERCURSO);
        return;
    }
    printf("Pre-ordem: "); rn_preOrdem(arvore); printf("\n");
    printf("Em-ordem: "); rn_emOrdem(arvore); printf("\n");
    printf("Pos-ordem: "); rn_posOrdem(arvore); printf("\n");
}

static No* buscarPorChaveCompostaAvl(No* raiz, const char* titulo, const char* artista) {
    if (!raiz || !titulo || !artista) return NULL;

    if (strcmp(raiz->dados.title, titulo) == 0 && strcmp(raiz->dados.performer, artista) == 0)
        return raiz;

    No* esquerda = buscarPorChaveCompostaAvl(raiz->esquerda, titulo, artista);
    if (esquerda) return esquerda;
    return buscarPorChaveCompostaAvl(raiz->direita, titulo, artista);
}

static void executarAVL(const char* caminho) {
    No* raiz = NULL;
    CsvResultado res = {0, 0, 0};
    clock_t inicio = clock();
    int status = carregarArvoreCsv(caminho, &raiz, &res);
    double segundos = (double)(clock() - inicio) / CLOCKS_PER_SEC;

    printf("\n=== AVL ===\n");
    if (status != CSV_OK) {
        fprintf(stderr, "Falha ao carregar AVL '%s' (código %d)\n", caminho, status);
        liberarArvore(raiz);
        return;
    }

    printf("Arquivo: %s\n", caminho);
    printf("Linhas lidas: %d | aceitas: %d | ignoradas: %d\n",
           res.linhasLidas, res.aceitas, res.ignoradas);
    printf("Tempo de carga: %.2f s\n", segundos);

    if (arvoreVazia(raiz)) {
        fprintf(stderr, "Nenhum registro foi carregado para AVL.\n");
        return;
    }

    printf("Total de nós: %d\n", contarNos(raiz));
    printf("Altura da árvore: %d\n", altura(raiz));
    mostrarRegistro("Menor id", minimo(raiz));
    mostrarRegistro("Maior id", maximo(raiz));

    if (raiz && raiz->dados.title[0] != '\0') {
        No* encontrado = buscarPorChaveCompostaAvl(raiz, raiz->dados.title, raiz->dados.performer);
        printf("Busca composta AVL: %s\n", encontrado ? "encontrado" : "não encontrado");
    }

    int gravados = serializarArvore(raiz, CAMINHO_SERIAL_AVL);
    if (gravados >= 0)
        printf("Árvore AVL serializada: %d nós gravados em '%s'\n", gravados, CAMINHO_SERIAL_AVL);

    posint idBusca = minimo(raiz)->id + (posint)(contarNos(raiz) / 2);
    posint idInexistente = maximo(raiz)->id + 1;
    printf("Id %u %s.\n", idBusca,
           buscar(raiz, idBusca) ? "encontrado" : "NÃO encontrado");
    printf("Id %u %s.\n", idInexistente,
           buscar(raiz, idInexistente) ? "encontrado" : "NÃO encontrado");

    mostrarPercursos(raiz);
    posint idRemocao = raiz->id;
    raiz = removerNo(raiz, idRemocao);
    printf("Remoção do id %u concluída. Ainda existe? %s\n", idRemocao,
           buscar(raiz, idRemocao) ? "sim" : "não");
    printf("Total de nós após remoção: %d\n", contarNos(raiz));
    printf("Altura após remoção: %d\n", altura(raiz));
    printf("Fator de balanceamento da raiz após remoção: %d\n", fatorBalanceamento(raiz));

    liberarArvore(raiz);
}

static void executarRubroNegra(const char* caminho) {
    ArvoreRN* arvore = rn_criarArvore();
    CsvResultado res = {0, 0, 0};
    clock_t inicio = clock();
    int status = carregarArvoreRubroCsv(caminho, &arvore, &res);
    double segundos = (double)(clock() - inicio) / CLOCKS_PER_SEC;

    printf("\n=== Árvore Rubro-Negra ===\n");
    if (status != CSV_OK) {
        fprintf(stderr, "Falha ao carregar Rubro-Negra '%s' (código %d)\n", caminho, status);
        rn_liberarArvore(arvore);
        return;
    }

    printf("Arquivo: %s\n", caminho);
    printf("Linhas lidas: %d | aceitas: %d | ignoradas: %d\n",
           res.linhasLidas, res.aceitas, res.ignoradas);
    printf("Tempo de carga: %.2f s\n", segundos);

    if (rn_arvoreVazia(arvore)) {
        fprintf(stderr, "Nenhum registro foi carregado para Rubro-Negra.\n");
        rn_liberarArvore(arvore);
        return;
    }

    printf("Total de nós: %d\n", rn_contarNos(arvore));
    printf("Altura da árvore: %d\n", rn_altura(arvore));

    if (arvore->raiz->dados.title[0] != '\0') {
        NoRN* encontrado = rn_buscarPorChaveComposta(arvore,
            arvore->raiz->dados.title,
            arvore->raiz->dados.performer);
        printf("Busca composta Rubro-Negra: %s\n",
               encontrado != arvore->nil ? "encontrado" : "não encontrado");
    }

    int gravados = rn_serializarArvore(arvore, CAMINHO_SERIAL_RN);
    if (gravados >= 0)
        printf("Árvore Rubro-Negra serializada: %d nós gravados em '%s'\n",
               gravados, CAMINHO_SERIAL_RN);

    posint idBusca = rn_minimo(arvore, arvore->raiz)->id;
    printf("Busca por id %u: %s\n", idBusca,
           rn_buscar(arvore, idBusca) ? "encontrado" : "não encontrado");

    mostrarPercursosRN(arvore);

    rn_liberarArvore(arvore);
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, ".UTF-8");
    const char* caminho = (argc > 1) ? argv[1] : CAMINHO_DATASET;

    executarAVL(caminho);
    executarRubroNegra(caminho);
    return 0;
}