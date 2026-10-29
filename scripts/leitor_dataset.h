#ifndef LEITOR_DATASET_H
#define LEITOR_DATASET_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <ctype.h>
#include <locale.h>

#ifndef CSV_MAX_LINHA
#define CSV_MAX_LINHA 1024   /* tamanho máximo de uma linha, em bytes */
#endif

#ifndef CSV_MAX_CAMPOS
#define CSV_MAX_CAMPOS 32    /* número máximo de colunas por linha */
#endif

#define CSV_SEP_AUTO '\0'    /* detecta o separador automaticamente */

/* ------------------------------------------------------------------ */
/* Tipos                                                              */
/* ------------------------------------------------------------------ */

typedef enum CsvStatus {
    CSV_OK           =  0,
    CSV_ERRO_ABRIR   = -1,   /* arquivo não existe / sem permissão    */
    CSV_ERRO_UTF16   = -2,   /* arquivo em UTF-16 (converta p/ UTF-8) */
    CSV_ERRO_CONFIG  = -3,   /* parâmetros inválidos                  */
    CSV_ERRO_LEITURA = -4    /* erro de I/O durante a leitura         */
} CsvStatus;

typedef struct CsvConfig {
    char sep;        /* ';' ',' '\t' '|'... ou CSV_SEP_AUTO                 */
    bool cabecalho;  /* true: a primeira linha não vazia é cabeçalho        */
    int  numCampos;  /* colunas esperadas por linha (0 = não validar)       */
} CsvConfig;

/* Detecta o separador, pula o cabeçalho e não valida o nº de colunas. */
#define CSV_CONFIG_PADRAO { CSV_SEP_AUTO, true, 0 }

typedef struct CsvResultado {
    int linhasLidas;   /* linhas de dados (sem cabeçalho e sem linhas em branco) */
    int aceitas;       /* callback devolveu true                                  */
    int ignoradas;     /* linha longa, colunas erradas ou callback devolveu false */
} CsvResultado;

/*
 * Callback chamado uma vez por linha de dados.
 *   campos   : vetor com as colunas já separadas e sem aspas externas.
 *              Os ponteiros só valem DURANTE a chamada: copie o que precisar
 *              (veja csvCopiarTexto). Os textos podem ser alterados no lugar.
 *   n        : quantidade de colunas encontradas
 *   numLinha : número da linha no arquivo (começa em 1), para mensagens
 *   ctx      : ponteiro livre passado em csvCarregar (sua árvore, contador...)
 * Devolva true se a linha foi aceita e false se foi rejeitada. O motivo da
 * rejeição deve ser impresso pelo próprio callback, que é quem o conhece.
 */
typedef bool (*CsvLinhaFn)(char* campos[], int n, int numLinha, void* ctx);

/* ------------------------------------------------------------------ */
/* Conversores de campo (usar dentro do callback)                     */
/* Devolvem false se o texto for vazio, inválido ou estourar o tipo.   */
/* Espaços nas pontas são aceitos.                                    */
/* ------------------------------------------------------------------ */

static inline bool csvSoEspacos(const char* s) {
    while (isspace((unsigned char)*s)) s++;
    return *s == '\0';
}

/* Remove espaços das duas pontas, no lugar. Devolve o novo início. */
static inline char* csvAparar(char* s) {
    while (isspace((unsigned char)*s)) s++;
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) s[--n] = '\0';
    return s;
}

static inline bool csvParaUint(const char* s, unsigned int* out) {
    while (isspace((unsigned char)*s)) s++;
    if (*s == '-') return false;            /* strtoul aceitaria e daria a volta */
    char* fim;
    errno = 0;
    unsigned long v = strtoul(s, &fim, 10);
    if (errno || fim == s || !csvSoEspacos(fim) || v > UINT_MAX) return false;
    *out = (unsigned int)v;
    return true;
}

static inline bool csvParaInt(const char* s, int* out) {
    char* fim;
    errno = 0;
    long v = strtol(s, &fim, 10);
    if (errno || fim == s || !csvSoEspacos(fim) || v < INT_MIN || v > INT_MAX)
        return false;
    *out = (int)v;
    return true;
}

/*
 * Aceita "8.5" e "8,5". O separador decimal é ajustado para o do locale
 * atual, então funciona mesmo que o programa tenha chamado
 * setlocale(LC_ALL, "pt_BR..."). Rejeita "nan" e "inf".
 */
static inline bool csvParaDouble(const char* s, double* out) {
    char buf[64];
    size_t n = strlen(s);
    if (n == 0 || n >= sizeof buf) return false;
    char ponto = localeconv()->decimal_point[0];
    for (size_t i = 0; i <= n; i++)         /* i <= n copia também o '\0' */
        buf[i] = (s[i] == ',' || s[i] == '.') ? ponto : s[i];
    char* fim;
    errno = 0;
    double v = strtod(buf, &fim);
    if (errno || fim == buf || !csvSoEspacos(fim) || !isfinite(v)) return false;
    *out = v;
    return true;
}

static inline bool csvParaFloat(const char* s, float* out) {
    double v;
    if (!csvParaDouble(s, &v) || v > FLT_MAX || v < -FLT_MAX) return false;
    *out = (float)v;
    return true;
}

/*
 * Copia texto para um buffer fixo sem estourar e sem cortar um caractere
 * UTF-8 ao meio. Devolve false se o texto foi truncado.
 */
static inline bool csvCopiarTexto(char* dest, size_t tam, const char* orig) {
    if (tam == 0) return false;
    size_t len = strlen(orig);
    bool coube = len < tam;
    size_t n = coube ? len : tam - 1;
    if (!coube)
        while (n > 0 && ((unsigned char)orig[n] & 0xC0) == 0x80) n--;
    memcpy(dest, orig, n);
    dest[n] = '\0';
    return coube;
}

/* ------------------------------------------------------------------ */
/* Internas                                                           */
/* ------------------------------------------------------------------ */

/* Escolhe o candidato que mais aparece fora de aspas na linha dada. */
static inline char csvDetectarSep(const char* linha) {
    static const char cand[] = { ';', ',', '\t', '|' };
    int melhor = 0, maior = 0;
    for (int i = 0; i < 4; i++) {
        int cont = 0;
        bool emAspas = false;
        for (const char* p = linha; *p; p++) {
            if (*p == '"') emAspas = !emAspas;
            else if (*p == cand[i] && !emAspas) cont++;
        }
        if (cont > maior) { maior = cont; melhor = i; }
    }
    return maior ? cand[melhor] : ';';
}

/*
 * Divide a linha no lugar, preservando campos vazios (strtok os pularia).
 * Entende "campo, com separador" e "" como aspas literal dentro de aspas.
 * A escrita nunca ultrapassa a leitura, então não precisa de buffer extra.
 * Devolve a quantidade de campos (no máximo 'max').
 */
static inline int csvDividir(char* linha, char sep, char* campos[], int max) {
    int n = 0;
    char* r = linha;   /* leitura  */
    char* w = linha;   /* escrita  */

    while (n < max) {
        campos[n++] = w;

        if (*r == '"') {
            r++;
            while (*r) {
                if (*r == '"') {
                    if (r[1] == '"') { *w++ = '"'; r += 2; }
                    else { r++; break; }          /* aspas de fechamento */
                } else {
                    *w++ = *r++;
                }
            }
            while (*r && *r != sep) r++;          /* ignora lixo após o fecho */
        } else {
            while (*r && *r != sep) *w++ = *r++;
        }

        if (*r == sep) { *w++ = '\0'; r++; }
        else           { *w = '\0'; break; }
    }
    return n;
}

/* ------------------------------------------------------------------ */
/* Função principal                                                   */
/* ------------------------------------------------------------------ */

/*
 * Lê 'caminho' (a extensão não importa: .csv, .txt...) e chama 'aoLer' para
 * cada linha de dados. 'cfg' pode ser NULL (usa CSV_CONFIG_PADRAO). 'res'
 * pode ser NULL. Devolve CSV_OK ou um código negativo de CsvStatus.
 *
 * Tratado automaticamente: \r\n do Windows, BOM UTF-8, linhas em branco,
 * linhas maiores que CSV_MAX_LINHA (ignoradas, sem desalinhar as seguintes),
 * última linha sem \n e UTF-16 (recusado com mensagem).
 */
static inline int csvCarregar(const char* caminho, const CsvConfig* cfg,
                              CsvLinhaFn aoLer, void* ctx, CsvResultado* res) {
    CsvConfig padrao = CSV_CONFIG_PADRAO;
    CsvResultado r = { 0, 0, 0 };
    if (res) *res = r;
    if (!cfg) cfg = &padrao;

    if (!caminho || !aoLer ||
        cfg->numCampos < 0 || cfg->numCampos >= CSV_MAX_CAMPOS ||
        cfg->sep == '"' || cfg->sep == '\n' || cfg->sep == '\r') {
        fprintf(stderr, "csvCarregar: parâmetros inválidos\n");
        return CSV_ERRO_CONFIG;
    }

    FILE* f = fopen(caminho, "rb");
    if (!f) { perror(caminho); return CSV_ERRO_ABRIR; }

    /* BOM: UTF-8 é pulado; UTF-16 é recusado */
    unsigned char bom[3];
    size_t lidos = fread(bom, 1, 3, f);
    if (lidos >= 2 && ((bom[0] == 0xFF && bom[1] == 0xFE) ||
                       (bom[0] == 0xFE && bom[1] == 0xFF))) {
        fprintf(stderr, "%s: arquivo em UTF-16. Salve como UTF-8 e tente de novo.\n",
                caminho);
        fclose(f);
        return CSV_ERRO_UTF16;
    }
    if (lidos != 3 || memcmp(bom, "\xEF\xBB\xBF", 3) != 0) rewind(f);

    char  sep = cfg->sep;
    int   maxCampos = cfg->numCampos > 0 ? cfg->numCampos + 1 : CSV_MAX_CAMPOS;
    bool  primeira = true;
    int   numLinha = 0;
    char  linha[CSV_MAX_LINHA];
    char* campos[CSV_MAX_CAMPOS];

    while (fgets(linha, sizeof linha, f)) {
        numLinha++;

        /* Sem \n no fim: ou é a última linha do arquivo, ou não coube no buffer */
        bool longa = false;
        size_t len = strlen(linha);
        if (len > 0 && linha[len - 1] != '\n') {
            int c = fgetc(f);
            if (c != EOF && c != '\n') {
                longa = true;
                while (c != '\n' && c != EOF) c = fgetc(f);
            }
        }

        linha[strcspn(linha, "\r\n")] = '\0';
        if (linha[0] == '\0') continue;               /* linha em branco */

        if (primeira) {
            primeira = false;
            if (sep == CSV_SEP_AUTO) sep = csvDetectarSep(linha);
            if (cfg->cabecalho) continue;
        }

        r.linhasLidas++;

        if (longa) {
            fprintf(stderr, "Linha %d: maior que %d bytes, ignorada\n",
                    numLinha, CSV_MAX_LINHA - 1);
            r.ignoradas++;
            continue;
        }

        int n = csvDividir(linha, sep, campos, maxCampos);
        if (cfg->numCampos > 0 && n != cfg->numCampos) {
            fprintf(stderr, "Linha %d: esperadas %d colunas, ignorada\n",
                    numLinha, cfg->numCampos);
            r.ignoradas++;
            continue;
        }

        if (aoLer(campos, n, numLinha, ctx)) r.aceitas++;
        else                                 r.ignoradas++;
    }

    bool erro = ferror(f) != 0;
    fclose(f);
    if (res) *res = r;
    return erro ? CSV_ERRO_LEITURA : CSV_OK;
}


#endif
