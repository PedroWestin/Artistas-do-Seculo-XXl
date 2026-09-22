# Artistas-do-Seculo-XXl

Entrega 1 de trabalho da disciplina Estruturas de Dados II — UNICID Prof. Cid Rodrigues de Andrade

---

## 👥 Integrantes do Grupo
<table>
  <tr>
    <th>Nome Completo</th>
    <th>RA</th>
  </tr>
  <tr>
    <td>Pedro Henrique Westin</td>
    <td>42570808</td>
  </tr>
  <tr>
    <td>Ágatha Ribeiro</td>
    <td>43343431</td>
  </tr>
  <tr>
    <td>Yasmim Bueno Miranda da Silva</td>
    <td>43598455</td>
  </tr>
  <tr>
    <td>Rayssa dos Santos</td>
    <td>44036990</td>
  </tr>
  
  <tr>
    <td>Gabrielly Ohane dos Santos</td>
    <td>37280431</td>
  </tr>
</table>

---

## 1. Dataset

### 1.1 Descrição
* **Origem/Domínio:** [Descrever a origem e o assunto dos dados, ex: Transações financeiras, registros de saúde, catálogo de produtos e-commerce]
* **Formato:** [Ex: CSV / JSON / Parquet]
* **Volume:** [Ex: 150.000 registros, contendo dados complexos/compostos]

### 1.2 Fonte
* **Referência:** [Link para o Kaggle, API, repositório ou menção de que foram gerados sinteticamente pelo grupo]

### 1.3 Estrutura dos dados
Atributos relevantes utilizados como chave de inserção, busca e comparação:
* `id` (`int`) – Identificador único atribuído internamente pelo grupo a cada registro, utilizado como chave principal nas estruturas de dados.
* `chart_week` (`string`) – Data da semana correspondente ao registro na parada.
* `position` (`int`) – Posição ocupada pela música na parada naquela semana.
* `peak_pos` (`int`) – Melhor posição alcançada pela música na parada.
* `wks_on_chart` (`int`) – Quantidade de semanas em que a música permaneceu na parada.
* `title` (`string`) – Título da música.

### 1.4 Justificativa da escolha
O dataset foi escolhido por apresentar um volume expressivo de 50.000 registros, permitindo trabalhar com uma quantidade significativa de dados e avaliar o comportamento das estruturas de dados implementadas.

Além do volume, os registros apresentam diferentes tipos de dados, incluindo inteiros, strings e valores de ponto flutuante, possibilitando o armazenamento e manipulação de informações variadas.

A quantidade de registros e a diversidade dos atributos tornam o dataset adequado para a implementação e análise das árvores AVL e Rubro-Negra, escolhidas pelo grupo para trabalhar com uma grande quantidade de dados mantendo a estrutura balanceada durante as operações.

## 2. Estrutura(s) de Árvore Escolhida(s)

### 2.1 Estrutura(s)
* [Ex: Árvore AVL / Árvore Rubro-Negra / B-Tree]

### 2.2 Justificativa técnica
A estrutura foi escolhida devido à necessidade de garantir **balanceamento automático**, evitando a degradação para o pior caso ($O(N)$) comum em BSTs simples. É ideal para cenários com grande volume de operações de busca e inserção.

### 2.3 Operações implementadas (Para Entrega 2)
* [ ] Inserção
* [ ] Remoção
* [ ] Busca
* [ ] Percursos (pré-ordem, em ordem, pós-ordem)
* [ ] Balanceamento (se aplicável)
* [ ] Outra: ______

### 2.4 Complexidade
Complexidade assintótica (Big-O) teórica de cada operação implementada:

| Operação | Melhor caso | Caso médio | Pior caso |
| :--- | :--- | :--- | :--- |
| **Inserção** | $O(\log N)$ | $O(\log N)$ | $O(\log N)$ |
| **Busca** | $O(\log N)$ | $O(\log N)$ | $O(\log N)$ |
| **Remoção** | $O(\log N)$ | $O(\log N)$ | $O(\log N)$ |

---

## 3. Plano de Testes

### 3.1 Objetivo dos testes
Validar a **corretude** das operações lógicas, a estabilidade do **balanceamento** sob carga e o **desempenho** temporal/espacial com grandes massas de dados.

### 3.2 Cenários de teste

| # | Cenário | Entrada | Resultado esperado | Status |
| :-: | :--- | :--- | :--- | :---: |
| 1 | Inserção básica | Chaves aleatórias simples | Árvore balanceada e sem perdas | ☐ |
| 2 | Busca por elemento inexistente | Chave fora do escopo | Retorno nulo / não encontrado | ☐ |
| 3 | Remoção de nó com dois filhos | Chave intermediária | Reestruturação correta da árvore | ☐ |

### 3.3 Casos extremos (edge cases)
* Árvore vazia.
* Inserção de um único elemento.
* Inserção de dados duplicados.
* Dados inseridos em ordem estritamente crescente ou decrescente (pior caso para BST).
* Teste com o volume máximo do dataset.

### 3.4 Testes de desempenho (Para Entrega 2)
Descrever como o grupo mediu o tempo de execução e/ou uso de memória, utilizando tamanhos de entrada como $100$, $1.000$, $10.000$ e $100.000$ registros.

### 3.5 Resultados obtidos (Para Entrega 2)
[Resuma aqui os resultados por meio de tabelas, gráficos ou links para arquivos de saída na pasta `/resultados`, comparando-os com a complexidade assintótica (Big-O) teórica].

---

## 4. Como Executar

### 4.1 Pré-requisitos (Para Entrega 2)
* Linguagem, versão e dependências necessárias (ex: Python 3.10+, C++17, Java 17).

### 4.2 Instruções (Para Entrega 2)
bash
git clone <link-do-repositorio>
cd <pasta>
# comandos de compilação/execução

### 4.3 Estrutura do repositório (já com pastas para a Entrega 2)

Plaintext

/src         → código-fonte
/dataset     → dataset utilizado
/testes      → scripts e casos de teste
/resultados  → saídas e relatórios de desempenho
README.md    → documentação do projeto

## 5 Referências:
