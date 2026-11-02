#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#include "../scripts/arvore_avl.h"
#include "../scripts/arvore_rubro_negra.h"

/* ============================================================
   ESTRUTURA PARA ARMAZENAR O DATASET
   ============================================================ */

typedef struct {
    Dados* dados;
    int quantidade;
    int capacidade;
} Dataset;

/* ============================================================
   LEITURA DOS REGISTROS DO DATASET
   ============================================================ */

static bool carregarRegistro(char* campos[], int n,
                             int linha, void* contexto) {

    Dataset* ds = (Dataset*)contexto;

    Dados d;
    int truncados = 0;

    if (!dadosDeCampos(campos, n, linha, &d, &truncados))
        return false;

    if (ds->quantidade == ds->capacidade) {

        int novaCapacidade = ds->capacidade == 0
                             ? 1024 : ds->capacidade * 2;

        Dados* novo = realloc(
            ds->dados,
            (size_t)novaCapacidade * sizeof(Dados)
        );

        if (!novo)
            return false;

        ds->dados = novo;
        ds->capacidade = novaCapacidade;
    }

    ds->dados[ds->quantidade++] = d;

    return true;
}

/* ============================================================
   VALIDAÇÃO DA ÁRVORE AVL
   ============================================================ */

static int validarAVLRecursivo(No* no,
                               unsigned minimo,
                               unsigned maximo,
                               int* valido) {

    if (!no)
        return -1;

    if (no->id <= minimo || no->id >= maximo)
        *valido = 0;

    int esquerda = validarAVLRecursivo(
        no->esquerda, minimo, no->id, valido
    );

    int direita = validarAVLRecursivo(
        no->direita, no->id, maximo, valido
    );

    int alturaCalculada = 1 +
        (esquerda > direita ? esquerda : direita);

    if (no->altura != alturaCalculada)
        *valido = 0;

    int fb = esquerda - direita;

    if (fb < -1 || fb > 1)
        *valido = 0;

    return alturaCalculada;
}

static bool validarAVL(No* raiz) {

    int valido = 1;

    validarAVLRecursivo(raiz, 0, UINT_MAX, &valido);

    return valido != 0;
}

/* ============================================================
   FUNCOES AUXILIARES
   ============================================================ */

static Dados criarDados(int id) {

    Dados d;

    memset(&d, 0, sizeof(Dados));

    snprintf(d.chart_week, sizeof(d.chart_week),
             "2026-01-%02d", (id % 28) + 1);

    d.position = id;
    d.peak_pos = id;
    d.wks_on_chart = 1;

    snprintf(d.title, sizeof(d.title),
             "Musica %d", id);

    snprintf(d.performer, sizeof(d.performer),
             "Artista %d", id);

    return d;
}

static void resultado(const char* nome, bool aprovado) {

    printf("[%s] %s\n",
           aprovado ? "APROVADO" : "FALHOU",
           nome);
}

static bool validarDuasArvores(No* avl, ArvoreRN* rn) {

    return validarAVL(avl) &&
           rn_validar(rn) &&
           contarNos(avl) == rn_contarNos(rn);
}

/* ============================================================
   TESTE 1 - INSERCAO BASICA
   ============================================================ */

static void testeInsercaoBasica(void) {

    No* avl = NULL;
    ArvoreRN* rn = rn_criarArvore();

    int ids[] = {50, 20, 80, 10, 30, 60, 90, 40, 70};

    int total = sizeof(ids) / sizeof(ids[0]);

    for (int i = 0; i < total; i++) {

        Dados d = criarDados(ids[i]);

        avl = inserirNo(avl, ids[i], &d);

        rn_inserirNo(rn, ids[i], &d);
    }

    bool ok =
        contarNos(avl) == total &&
        rn_contarNos(rn) == total &&
        validarDuasArvores(avl, rn);

    resultado("Insercao basica", ok);

    liberarArvore(avl);
    rn_liberarArvore(rn);
}

/* ============================================================
   TESTE 2 - BUSCA DE ELEMENTO INEXISTENTE
   ============================================================ */

static void testeBuscaInexistente(void) {

    No* avl = NULL;
    ArvoreRN* rn = rn_criarArvore();

    for (unsigned i = 1; i <= 10; i++) {

        Dados d = criarDados(i);

        avl = inserirNo(avl, i, &d);

        rn_inserirNo(rn, i, &d);
    }

    bool ok =
        !buscar(avl, 999) &&
        !rn_buscar(rn, 999) &&
        buscar(avl, 5) &&
        rn_buscar(rn, 5);

    resultado("Busca de elemento inexistente", ok);

    liberarArvore(avl);
    rn_liberarArvore(rn);
}

/* ============================================================
   TESTE 3 - REMOCAO DE NO COM DOIS FILHOS
   ============================================================ */

static void testeRemocaoDoisFilhos(void) {

    No* avl = NULL;
    ArvoreRN* rn = rn_criarArvore();

    int ids[] = {20, 10, 30, 5, 15, 25, 35};

    int total = sizeof(ids) / sizeof(ids[0]);

    for (int i = 0; i < total; i++) {

        Dados d = criarDados(ids[i]);

        avl = inserirNo(avl, ids[i], &d);

        rn_inserirNo(rn, ids[i], &d);
    }

    bool tinhaDoisFilhosAVL =
        avl && avl->esquerda && avl->direita;

    bool tinhaDoisFilhosRN =
        rn->raiz->esquerda != rn->nil &&
        rn->raiz->direita != rn->nil;

    avl = removerNo(avl, 20);

    bool removidoRN = rn_removerNo(rn, 20);

    bool ok =
        tinhaDoisFilhosAVL &&
        tinhaDoisFilhosRN &&
        removidoRN &&
        !buscar(avl, 20) &&
        !rn_buscar(rn, 20) &&
        contarNos(avl) == total - 1 &&
        rn_contarNos(rn) == total - 1 &&
        validarDuasArvores(avl, rn);

    resultado("Remocao de no com dois filhos", ok);

    liberarArvore(avl);
    rn_liberarArvore(rn);
}

/* ============================================================
   TESTE 4 - ARVORE VAZIA
   ============================================================ */

static void testeArvoreVazia(void) {

    No* avl = NULL;
    ArvoreRN* rn = rn_criarArvore();

    bool ok =
        arvoreVazia(avl) &&
        rn_arvoreVazia(rn) &&
        contarNos(avl) == 0 &&
        rn_contarNos(rn) == 0 &&
        altura(avl) == -1 &&
        rn_altura(rn) == -1 &&
        !buscar(avl, 1) &&
        !rn_buscar(rn, 1) &&
        removerNo(avl, 1) == NULL &&
        !rn_removerNo(rn, 1) &&
        validarDuasArvores(avl, rn);

    resultado("Arvore vazia", ok);

    liberarArvore(avl);
    rn_liberarArvore(rn);
}

/* ============================================================
   TESTE 5 - INSERCAO DE UM UNICO ELEMENTO
   ============================================================ */

static void testeElementoUnico(void) {

    No* avl = NULL;
    ArvoreRN* rn = rn_criarArvore();

    Dados d = criarDados(1);

    avl = inserirNo(avl, 1, &d);

    rn_inserirNo(rn, 1, &d);

    bool ok =
        contarNos(avl) == 1 &&
        rn_contarNos(rn) == 1 &&
        buscar(avl, 1) &&
        rn_buscar(rn, 1) &&
        altura(avl) == 0 &&
        rn_altura(rn) == 0 &&
        validarDuasArvores(avl, rn);

    resultado("Insercao de elemento unico", ok);

    liberarArvore(avl);
    rn_liberarArvore(rn);
}

/* ============================================================
   TESTE 6 - INSERCAO DE DADOS DUPLICADOS
   ============================================================ */

static void testeDuplicidade(void) {

    No* avl = NULL;
    ArvoreRN* rn = rn_criarArvore();

    Dados d1 = criarDados(10);
    Dados d2 = criarDados(20);

    avl = inserirNo(avl, 10, &d1);

    rn_inserirNo(rn, 10, &d1);

    avl = inserirNo(avl, 10, &d2);

    rn_inserirNo(rn, 10, &d2);

    bool ok =
        contarNos(avl) == 1 &&
        rn_contarNos(rn) == 1 &&
        buscar(avl, 10) &&
        rn_buscar(rn, 10) &&
        validarDuasArvores(avl, rn);

    resultado("Insercao de IDs duplicados", ok);

    liberarArvore(avl);
    rn_liberarArvore(rn);
}

/* ============================================================
   TESTE 7 - ORDEM CRESCENTE
   ============================================================ */

static void testeOrdemCrescente(void) {

    No* avl = NULL;
    ArvoreRN* rn = rn_criarArvore();

    const int total = 1000;

    for (int i = 1; i <= total; i++) {

        Dados d = criarDados(i);

        avl = inserirNo(avl, i, &d);

        rn_inserirNo(rn, i, &d);
    }

    bool ok =
        contarNos(avl) == total &&
        rn_contarNos(rn) == total &&
        validarDuasArvores(avl, rn);

    resultado("Insercao em ordem crescente", ok);

    printf("  Altura AVL: %d | Altura RN: %d\n",
           altura(avl), rn_altura(rn));

    liberarArvore(avl);
    rn_liberarArvore(rn);
}

/* ============================================================
   TESTE 8 - ORDEM DECRESCENTE
   ============================================================ */

static void testeOrdemDecrescente(void) {

    No* avl = NULL;
    ArvoreRN* rn = rn_criarArvore();

    const int total = 1000;

    for (int i = total; i >= 1; i--) {

        Dados d = criarDados(i);

        avl = inserirNo(avl, i, &d);

        rn_inserirNo(rn, i, &d);
    }

    bool ok =
        contarNos(avl) == total &&
        rn_contarNos(rn) == total &&
        validarDuasArvores(avl, rn);

    resultado("Insercao em ordem decrescente", ok);

    printf("  Altura AVL: %d | Altura RN: %d\n",
           altura(avl), rn_altura(rn));

    liberarArvore(avl);
    rn_liberarArvore(rn);
}

/* ============================================================
   TESTE 9 - VOLUME MAXIMO DO DATASET
   ============================================================ */

static void testeVolumeDataset(Dataset* ds) {

    No* avl = NULL;
    ArvoreRN* rn = rn_criarArvore();

    clock_t inicioAVL = clock();

    for (int i = 0; i < ds->quantidade; i++) {

        unsigned id = (unsigned)i + 1;

        avl = inserirNo(avl, id, &ds->dados[i]);
    }

    clock_t fimAVL = clock();

    clock_t inicioRN = clock();

    for (int i = 0; i < ds->quantidade; i++) {

        unsigned id = (unsigned)i + 1;

        rn_inserirNo(rn, id, &ds->dados[i]);
    }

    clock_t fimRN = clock();

    double tempoAVL =
        (double)(fimAVL - inicioAVL) / CLOCKS_PER_SEC;

    double tempoRN =
        (double)(fimRN - inicioRN) / CLOCKS_PER_SEC;

    bool ok =
        contarNos(avl) == ds->quantidade &&
        rn_contarNos(rn) == ds->quantidade &&
        validarDuasArvores(avl, rn);

    resultado("Volume maximo do dataset", ok);

    printf("  Registros: %d\n", ds->quantidade);
    printf("  Tempo AVL: %.6f segundos\n", tempoAVL);
    printf("  Tempo RN:  %.6f segundos\n", tempoRN);

    printf("  Altura AVL: %d\n", altura(avl));
    printf("  Altura RN: %d\n", rn_altura(rn));

    liberarArvore(avl);
    rn_liberarArvore(rn);
}

/* ============================================================
   TESTE 10 - DESEMPENHO COM DIFERENTES VOLUMES
   ============================================================ */

static void testeDesempenho(Dataset* ds) {

    int tamanhos[] = {100, 1000, 10000, 50000};

    int quantidade =
        sizeof(tamanhos) / sizeof(tamanhos[0]);

    printf("\n========================================\n");
    printf("       TESTE DE DESEMPENHO\n");
    printf("========================================\n");

    printf("%-12s %-15s %-15s\n",
           "Registros", "AVL (s)", "RN (s)");

    for (int t = 0; t < quantidade; t++) {

        int n = tamanhos[t];

        if (n > ds->quantidade)
            continue;

        No* avl = NULL;
        ArvoreRN* rn = rn_criarArvore();

        /* INSERCAO AVL */

        clock_t inicio = clock();

        for (int i = 0; i < n; i++) {

            avl = inserirNo(
                avl, (unsigned)i + 1, &ds->dados[i]
            );
        }

        clock_t fim = clock();

        double tempoAVL =
            (double)(fim - inicio) / CLOCKS_PER_SEC;

        /* INSERCAO RUBRO-NEGRA */

        inicio = clock();

        for (int i = 0; i < n; i++) {

            rn_inserirNo(
                rn, (unsigned)i + 1, &ds->dados[i]
            );
        }

        fim = clock();

        double tempoRN =
            (double)(fim - inicio) / CLOCKS_PER_SEC;

        bool ok =
            contarNos(avl) == n &&
            rn_contarNos(rn) == n &&
            validarDuasArvores(avl, rn);

        printf("%-12d %-15.6f %-15.6f %s\n",
               n, tempoAVL, tempoRN,
               ok ? "OK" : "FALHA");

        liberarArvore(avl);
        rn_liberarArvore(rn);
    }
}

/* ============================================================
   TESTE 11 - VOLUME SINTETICO PERSONALIZADO
   ============================================================ */

static void testeVolumeSintetico(unsigned int total) {

    if (total == 0) {

        printf("\nTeste sintetico ignorado pelo usuario.\n");

        return;
    }

    No* avl = NULL;
    ArvoreRN* rn = rn_criarArvore();

    printf("\n========================================\n");
    printf("       TESTE DE VOLUME SINTETICO\n");
    printf("========================================\n");

    printf("Quantidade solicitada: %u registros\n", total);

    /* INSERCAO AVL */

    clock_t inicioAVL = clock();

    for (unsigned int i = 1; i <= total; i++) {

        Dados d = criarDados((int)i);

        avl = inserirNo(avl, i, &d);
    }

    clock_t fimAVL = clock();

    /* INSERCAO RUBRO-NEGRA */

    clock_t inicioRN = clock();

    for (unsigned int i = 1; i <= total; i++) {

        Dados d = criarDados((int)i);

        rn_inserirNo(rn, i, &d);
    }

    clock_t fimRN = clock();

    /* CALCULO DO TEMPO */

    double tempoAVL =
        (double)(fimAVL - inicioAVL) / CLOCKS_PER_SEC;

    double tempoRN =
        (double)(fimRN - inicioRN) / CLOCKS_PER_SEC;

    /* VALIDACAO */

    bool ok =
        contarNos(avl) == (int)total &&
        rn_contarNos(rn) == (int)total &&
        validarDuasArvores(avl, rn);

    resultado("Volume sintetico personalizado", ok);

    printf("\nRESULTADOS:\n");

    printf("Registros inseridos: %u\n", total);

    printf("Tempo AVL: %.6f segundos\n", tempoAVL);

    printf("Tempo RN: %.6f segundos\n", tempoRN);

    printf("Altura AVL: %d\n", altura(avl));

    printf("Altura RN: %d\n", rn_altura(rn));

    liberarArvore(avl);
    rn_liberarArvore(rn);
}

/* ============================================================
   LOCALIZACAO AUTOMATICA DO DATASET
   ============================================================ */

static const char* localizarDataset(void) {

    static const char* caminhos[] = {

        "dataset/billboard_hot100_50k_dataset.txt",

        "../dataset/billboard_hot100_50k_dataset.txt",

        "../../dataset/billboard_hot100_50k_dataset.txt",

        "../../../dataset/billboard_hot100_50k_dataset.txt"
    };

    int quantidade =
        sizeof(caminhos) / sizeof(caminhos[0]);

    for (int i = 0; i < quantidade; i++) {

        FILE* arquivo = fopen(caminhos[i], "r");

        if (arquivo != NULL) {

            fclose(arquivo);

            return caminhos[i];
        }
    }

    return NULL;
}

/* ============================================================
   MAIN - EXECUCAO DOS TESTES
   ============================================================ */

int main(int argc, char* argv[]) {

    printf("\n========================================\n");
    printf("       TESTES DAS ARVORES AVL E RN\n");
    printf("========================================\n");

    /* TESTES LOGICOS */

    testeArvoreVazia();

    testeElementoUnico();

    testeInsercaoBasica();

    testeBuscaInexistente();

    testeRemocaoDoisFilhos();

    testeDuplicidade();

    testeOrdemCrescente();

    testeOrdemDecrescente();

    /* LOCALIZACAO DO DATASET */

    const char* caminhoDataset = NULL;

    if (argc >= 2) {

        caminhoDataset = argv[1];

    } else {

        caminhoDataset = localizarDataset();
    }

    if (caminhoDataset == NULL) {

        printf("\nERRO: Dataset nao encontrado!\n");

        printf("Verifique se a pasta dataset esta no projeto.\n");

        free(NULL);

        return 1;
    }

    printf("\n========================================\n");
    printf("       LEITURA DO DATASET\n");
    printf("========================================\n");

    printf("Arquivo: %s\n", caminhoDataset);

    Dataset ds = {0};

    CsvConfig cfg = { ',', true, COL_TOTAL };

    CsvResultado res;

    int status = csvCarregar(
        caminhoDataset,
        &cfg,
        carregarRegistro,
        &ds,
        &res
    );

    if (status != CSV_OK) {

        printf("\nERRO: Nao foi possivel carregar o dataset.\n");

        free(ds.dados);

        return 1;
    }

    printf("\nLinhas lidas: %d\n", res.linhasLidas);

    printf("Registros aceitos: %d\n", ds.quantidade);

    printf("Registros ignorados: %d\n", res.ignoradas);

    /* TESTES COM DATASET REAL */

    testeVolumeDataset(&ds);

    testeDesempenho(&ds);

    /* ESCOLHA DO VOLUME SINTETICO */

    printf("\n========================================\n");
    printf("       VOLUME PERSONALIZADO\n");
    printf("========================================\n");

    printf("Quantos registros sinteticos deseja testar?\n");

    printf("Exemplos: 100000, 200000, 500000, 1000000\n");

    printf("Digite 0 para ignorar este teste.\n");

    printf("\nQuantidade: ");

    char entrada[64];

    unsigned int quantidadeSintetica = 0;

    if (fgets(entrada, sizeof(entrada), stdin) == NULL) {

        printf("\nEntrada nao recebida. Teste sintetico ignorado.\n");

    } else {

        char* fim;

        unsigned long valor = strtoul(entrada, &fim, 10);

        while (*fim == ' ' || *fim == '\t' ||
               *fim == '\n' || *fim == '\r') {

            fim++;
        }

        if (fim == entrada || *fim != '\0' ||
            valor > INT_MAX) {

            printf("\nQuantidade invalida. Teste sintetico ignorado.\n");

        } else {

            quantidadeSintetica = (unsigned int)valor;
        }
    }

    /* EXECUCAO DO TESTE SINTETICO */

    testeVolumeSintetico(quantidadeSintetica);

    /* LIBERACAO DO DATASET */

    free(ds.dados);

    printf("\n========================================\n");
    printf("       FIM DOS TESTES\n");
    printf("========================================\n");

    return 0;
}
 