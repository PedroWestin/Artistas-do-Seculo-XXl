# Hot-100-Seculo-XXl

Trabalho da disciplina Estruturas de Dados II — UNICID Prof. Cid Rodrigues de Andrade

---

## 👥 Integrantes do Grupo
<table>   <tr>     <th>Nome Completo</th>     <th>RA</th>   </tr>   <tr>     <td>Pedro Henrique Westin</td>     <td>42570808</td>   </tr>   <tr>     <td>Ágatha Ribeiro</td>     <td>43343431</td>   </tr>   <tr>     <td>Yasmim Bueno Miranda da Silva</td>     <td>43598455</td>   </tr>   <tr>     <td>Rayssa dos Santos</td>     <td>44036990</td>   </tr>   <tr>     <td>Gabrielly Ohane dos Santos</td>     <td>37280431</td>   </tr> </table>

---

## 1. Dataset
### 1.1 Descrição
* **Origem/Domínio:** Registros semanais de músicas presentes na parada musical Billboard Hot 100, abrangendo o período de 2000 a 2026.
* **Formato:** TXT, estruturado em campos separados por vírgulas.
* **Volume:** 50.000 registros, contendo informações sobre músicas, artistas, posições na parada, permanência no ranking e características musicais, com dados dos tipos inteiro, string e ponto flutuante.

### 1.2 Fonte
* **Referência:** Dataset da Billboard Hot 100 disponibilizado para utilização no projeto, contendo registros semanais de músicas entre 2000 e 2026.

### 1.3 Estrutura dos dados
A chave de inserção, busca, comparação e remoção nas árvores é composta por dois campos:

chart_week (string, formato AAAA-MM-DD) – semana da parada.
position (int, de 1 a 100) – posição ocupada na parada naquela semana.
Em uma mesma semana cada posição é ocupada por uma única música, então o par (chart_week, position) identifica um registro de forma única. Isso foi verificado nos 50.000 registros do dataset: nenhum par se repete. O song_id original não serve como chave porque a mesma música aparece em várias semanas (são cerca de 9.500 músicas distintas nos 50.000 registros).
Comparador customizado (comparaChave(a, b)): devolve valor negativo, zero ou positivo.
Compara chart_week com strcmp. Como o formato é AAAA-MM-DD, a ordem alfabética coincide com a ordem cronológica.
Se as semanas forem iguais, compara position numericamente.
Se as duas partes forem iguais, as chaves são iguais.
O mesmo comparador é usado na AVL e na Rubro-Negra, para que a comparação entre as duas árvores seja justa.
Campos do registro
chart_week (string) – Data da semana correspondente ao registro na parada. (parte 1 da chave)
position (int) – Posição ocupada pela música na parada naquela semana. (parte 2 da chave)
peak_pos (int) – Melhor posição alcançada pela música na parada.
wks_on_chart (int) – Quantidade de semanas em que a música permaneceu na parada.
title (string) – Título da música.
performer (string) – Artista ou intérprete associado à música.
genre (string) – Gênero ou gêneros musicais associados à música.
danceability (float) – Indicador numérico relacionado à dançabilidade da música.
energy (float) – Indicador numérico relacionado à energia da música.
tempo (float) – Andamento da música.
valence (float) – Indicador numérico relacionado à característica musical positiva ou negativa.
popularity (int) – Indicador numérico de popularidade.
O song_id do arquivo original não é guardado nos nós, pois se repete entre as semanas. O campo id gerado sequencialmente no carregamento deixa de ser a chave e passa a ser apenas o número de ordem do registro no arquivo, sem participar das comparações.
Campos sem valor: em parte dos registros (cerca de 25%) as características de áudio (danceability, energy, tempo, valence, popularity) vêm vazias, e em cerca de 22% o genre também. Esses campos não fazem parte da chave, então o registro continua sendo carregado normalmente.

### 1.4 Justificativa da escolha
O dataset foi escolhido por apresentar um volume expressivo de 50.000 registros, permitindo trabalhar com uma quantidade significativa de dados e avaliar o comportamento das estruturas de dados implementadas. Os registros também reúnem inteiros, strings e valores de ponto flutuante, o que permite armazenar e manipular informações variadas.

Além do volume e da variedade de tipos, três características do dataset são relevantes para comparar a AVL e a Rubro-Negra:
Chaves únicas. O par (chart_week, position) não se repete em nenhum dos 50.000 registros, então a carga completa não gera chaves duplicadas e todos os registros chegam a ocupar um nó.

Dados praticamente ordenados. No arquivo, os registros estão quase todos em ordem crescente da chave composta: dentro de cada semana as posições crescem, e as semanas avançam cronologicamente, com apenas 7 quebras de ordem no arquivo inteiro. Inserir os registros na ordem do arquivo equivale, portanto, a inserir chaves em ordem quase estritamente crescente. Esse é o pior caso de uma árvore de busca binária comum, que viraria uma lista encadeada de altura próxima de 50.000, e é exatamente a situação em que o balanceamento das duas árvores é posto à prova.

Muitas repetições de música, mas não de chave. A mesma música aparece em várias semanas (uma única chega a 48 registros), mas cada aparição tem chave diferente. Isso mostra por que o song_id não pode ser a chave e justifica a chave composta.

Essas características permitem comparar dois cenários com o mesmo conjunto de dados: inserção na ordem natural do arquivo (quase ordenada) e inserção em ordem embaralhada (aleatória). A diferença entre as duas árvores aparece principalmente no primeiro cenário.

## 2. Estrutura(s) de Árvore Escolhida(s)
### 2.1 Estrutura(s)
Árvore AVL | Árvore Rubro-Negra

### 2.2 Justificativa técnica
As estruturas foram escolhidas pois tanto a Árvore AVL quanto a Árvore Rubro-Negra são estruturas de dados auto-balanceáveis projetadas para resolver o maior problema das Árvores de Busca Binária (BST) comuns: o risco de desbalanceamento e a degradação do tempo de operação de $O(\log n)$ para $O(n)$. No entanto, elas abordam a eficiência por caminhos inversos:

AVL – balanceamento rígido. Mantém o fator de balanceamento de cada nó entre −1 e 1, o que limita a altura a cerca de $1{,}44 \log_2 n$. Isso deixa a árvore mais baixa e a busca mais rápida, mas exige mais rotações durante inserções e remoções (na remoção, pode haver rotações em vários níveis do caminho até a raiz).

Rubro-Negra – balanceamento relaxado. Usa cores dos nós e permite altura de até $2 \log_2 (n+1)$. A árvore pode ser mais alta, o que torna a busca um pouco mais lenta, mas as inserções e remoções fazem poucas rotações (no máximo 2 na inserção e 3 na remoção), com o restante do ajuste feito por recoloração.

Relação com o dataset. Como o arquivo está quase ordenado pela chave composta (seção 1.4), a inserção na ordem do arquivo força as duas árvores a se reorganizarem a cada nova chave. Com 50.000 registros, os limites teóricos de altura são de aproximadamente 22 níveis para a AVL e 31 para a Rubro-Negra, contra um pior caso de 50.000 para uma BST comum.

O que se espera observar na comparação:
Altura final: a AVL deve ficar igual ou menor que a Rubro-Negra, nos dois tipos de inserção (ordenada e aleatória).
Rotações: a Rubro-Negra deve fazer menos rotações que a AVL, principalmente nas remoções.
Tempo de inserção e remoção: a Rubro-Negra tende a ser igual ou mais rápida, por causa das menos rotações.
Tempo de busca: a AVL tende a ser igual ou mais rápida, por causa da árvore mais baixa.
Crescimento com o tamanho: o tempo total deve crescer de forma compatível com $O(n \log n)$ para $n$ inserções, e o tempo de cada operação de forma compatível com $O(\log n)$.
Esses pontos são hipóteses a serem confirmadas ou refutadas nos testes de desempenho (seções 3.4 e 3.5).

### 2.3 Operações implementadas (Para Entrega 2)
[ ] Inserção
[ ] Remoção
[ ] Busca
[ ] Percursos (pré-ordem, em ordem, pós-ordem)
[ ] Balanceamento (se aplicável)
[ ] Outra: ______

### 2.4 Complexidade
Complexidade assintótica (Big-O) teórica de cada operação implementada:
Árvore AVL
Operação	Melhor caso	Caso médio	Pior caso
Inserção	$O(1)$	$O(\log N)$	$O(\log N)$
Busca	$O(1)$ ou $O(\log N)$	$O(\log N)$	$O(\log N)$
Remoção	$O(1)$ ou $O(\log N)$	$O(\log N)$	$O(\log N)$
Árvore Rubro-Negra
Operação	Melhor caso	Caso médio	Pior caso
Inserção	$O(1)$	$O(\log N)$	$O(\log N)$
Busca	$O(1)$	$O(\log N)$	$O(\log N)$
Remoção	$O(1)$	$O(\log N)$	$O(\log N)$

## 3. Plano de Testes
3.1 Objetivo dos testes
Validar a corretude das operações lógicas, a estabilidade do balanceamento sob carga e o desempenho temporal com grandes massas de dados, comparando a Árvore AVL com a Árvore Rubro-Negra sobre o mesmo dataset de 50.000 registros e a mesma chave composta (chart_week, position).

### 3.2 Cenários de teste
Cenários de corretude
#	Cenário	Entrada	Operação	Resultado esperado	Status
1	Inserção básica	1.000 registros do dataset, em ordem embaralhada (semente fixa)	Inserir todos	Árvore com 1.000 nós, sem perdas; percurso em ordem devolve as chaves em ordem crescente	☐
2	Busca por elemento inexistente	Chave fora do escopo do dataset (ex.: chart_week = 1999-01-01,
position = 1)	Buscar	Retorno nulo / não encontrado	☐
3	Remoção de nó com dois filhos	Chave intermediária de uma árvore já carregada	Remover	Nó removido, demais chaves preservadas, reestruturação correta e árvore ainda balanceada	☐
4	Verificação de invariantes	Árvores carregadas com os 50.000 registros e depois submetidas a remoções	Verificar automaticamente após inserções e remoções	AVL: fator de balanceamento de todo nó entre −1 e 1. 
Rubro-Negra: raiz preta; nenhum nó vermelho com filho vermelho; mesma altura negra em todos os caminhos da raiz às folhas	☐
Cenários de desempenho e comparação AVL × Rubro-Negra
#	Cenário	Entrada	Operação	Medição / resultado esperado	Status
5	Inserção em ordem do arquivo (quase crescente)	Os primeiros N registros do arquivo, com N = 10.000, 25.000 e 50.000	Inserir todos, na ordem do arquivo	Tempo total, altura final, número de rotações e de comparações. Esperado: altura da AVL menor ou igual à da Rubro-Negra, e menos rotações na Rubro-Negra	☐
6	Inserção em ordem aleatória	Os mesmos N registros do cenário 5, embaralhados com semente fixa	Inserir todos	Mesmas métricas do cenário 5, para comparar o efeito da ordem de entrada	☐
7	Busca em árvore cheia	Árvore com N registros; 10.000 chaves existentes sorteadas com semente fixa	Buscar cada chave	Tempo total e número de comparações por busca. Esperado: crescimento compatível com $O(\log n)$	☐
8	Remoção em massa	Árvore com 50.000 registros; remoção de 10.000 chaves sorteadas com semente fixa	Remover cada chave	Tempo total, número de rotações, altura final e invariantes ainda válidos. Esperado: menos rotações na Rubro-Negra	☐

### 3.3 Casos extremos (edge cases)
Árvore vazia.
Inserção de um único elemento.
Inserção de dados duplicados.
Dados inseridos em ordem estritamente crescente ou decrescente (pior caso para BST).
Teste com o volume máximo do dataset.

### 3.4 Testes de desempenho (Para Entrega 2)
Tamanhos de entrada: 10.000, 25.000 e 50.000 registros, todos dentro do volume existente no dataset (50.000). Cada tamanho usa os primeiros N registros do arquivo, o que permite traçar tempo × tamanho e comparar a curva com $O(\log n)$ por operação.
Métricas medidas em cada cenário:
Tempo de execução: com relógio de alta resolução (por exemplo, clock_gettime com CLOCK_MONOTONIC em C), medindo apenas a operação testada, sem a leitura do arquivo.
Altura final da árvore.
Número de rotações (contador incrementado a cada rotação simples ou dupla).
Número de comparações de chaves feitas pelo comparador.
Repetições: cada cenário será executado 10 vezes. O resultado reportado será a mediana dos tempos, para reduzir o efeito de oscilações do sistema, junto com o mínimo e o máximo. Altura, rotações e comparações são determinísticas para uma mesma ordem de entrada e serão conferidas entre as repetições.
Ordens de entrada: os cenários com inserção aleatória usam sempre a mesma semente do gerador de números aleatórios, para que a AVL e a Rubro-Negra recebam exatamente a mesma sequência.
Condições de medição: todos os testes serão executados no mesmo computador, logo após ligá-lo e sem outros programas em execução, com o código compilado com as mesmas opções para as duas árvores.

### 3.5 Resultados obtidos (Para Entrega 2)
[Resuma aqui os resultados por meio de tabelas, gráficos ou links para arquivos de saída na pasta /resultados, comparando-os com a complexidade assintótica (Big-O) teórica].

## 4. Como Executar
### 4.1 Pré-requisitos (Para Entrega 2)
Linguagem, versão e dependências necessárias (ex: Python 3.10+, C++17, Java 17).

### 4.2 Instruções (Para Entrega 2)
git clone <link-do-repositorio>
cd <pasta>
# comandos de compilação/execução

### 4.3 Estrutura do repositório (já com pastas para a Entrega 2)
/src         → código-fonte
/dataset     → dataset utilizado
/testes      → scripts e casos de teste
/resultados  → saídas e relatórios de desempenho
README.md    → documentação do projeto

### 5 Referências:
Billboard Hot 100 weekly chart data — repositório utdata/rwd-billboard-data (GitHub).
Características de áudio (Spotify) — arquivo "Hot 100 Audio Features" fornecido pelo grupo.
 
