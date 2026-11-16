         AlertaSubsolo

**Global Solution 2026.2 — FIAP**  
**Curso:** Engenharia de Software  
**Disciplina:** Computational Thinking with Python

-Sobre o projeto

O AlertaSubsolo é um sistema de monitoramento do nível da água, desenvolvido para ajudar na prevenção de alagamentos em garagens, estacionamentos e áreas subterrâneas.

O projeto utiliza um Arduino Uno R3 com sensores para a parte física e uma aplicação em Python para registrar medições, consultar dados e identificar riscos.

Nesta versão, as medições são inseridas manualmente no programa Python.

-Funcionalidades

O sistema possui seis opções:

1. **Registrar nível da água:** recebe uma medição em centímetros, verifica se é válida e classifica o risco.
2. **Consultar histórico:** mostra todas as medições registradas.
3. **Ver estatísticas:** apresenta a quantidade de registros, o menor e o maior nível e a média das medições.
4. **Consultar status de risco:** verifica a última medição e informa a situação atual.
5. **Configurar reservatório:** permite alterar a altura máxima do reservatório.
6. **Sair:** encerra o programa.

- Classificação dos níveis

| Nível da água         | Situação |
| --------------------- | -------- |
| Menor que 6 cm        | Normal   |
| De 6 a menos de 12 cm | Alerta   |
| 12 cm ou mais         | Crítico  |

Os valores são utilizados para os testes do protótipo e podem ser ajustados conforme as características do reservatório.

- Tecnologias utilizadas

- Python 3
- Visual Studio Code
- Arduino Uno R3 (protótipo físico)

No Python, foram utilizados conceitos como variáveis, listas, `if/elif/else`, `match/case`, `while`, `for`, `input()`, `float()`, `append()` e funções para cálculos estatísticos.

- Estrutura dos arquivos

```text
AlertaSubsolo/
└── python/
    ├── main.py
    ├── README.md
    └── prompts.md
```

-Como executar

É necessário ter Python 3.10 ou superior instalado.

Abra o terminal na pasta `python` e execute:

```bash
python3 main.py
```

O programa exibirá um menu com seis opções. Basta digitar o número correspondente à funcionalidade desejada.

-Limitações

As medições são armazenadas temporariamente e perdidas quando o programa é encerrado.

A aplicação ainda não recebe automaticamente os dados do Arduino. Os limites de risco também são fixos, independentemente da altura configurada para o reservatório.

-Integrantes

- Rafael De Oliveira Santos Zuccon — 575995
- Gabriel Souza Bore De Carvalho — 576428

**FIAP — Global Solution 2026.2**
