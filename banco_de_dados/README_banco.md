# 🗄️ AlertaSubsolo — Banco de dados (Supabase)

Banco do **AlertaSubsolo** (Global Solution 2026.2 · FIAP · Grupo MIXTAPE).
Cada leitura enviada pelo Arduino vira uma linha na tabela `leituras`, e o site lê de lá.

## Arquivos

| Arquivo | O que faz |
|---|---|
| `01_criar_tabela.sql` | Cria a tabela `leituras`, o índice e as permissões (RLS). |
| `02_permitir_apagar.sql` | (Opcional) libera apagar registros pela chave pública. |

## Como configurar (uma vez só)

1. No projeto do Supabase, abra **SQL Editor → New query**.
2. Cole o conteúdo de `01_criar_tabela.sql` e clique em **Run**.
3. (Opcional) faça o mesmo com `02_permitir_apagar.sql`.

Pode rodar mais de uma vez sem problema — não apaga nada.

## A tabela `leituras`

Os nomes das colunas são **iguais** aos do JSON que o Arduino envia.

| Coluna | Tipo | Descrição |
|---|---|---|
| `id` | bigint | identificador (automático) |
| `criado_em` | timestamptz | preenchido sozinho pelo Supabase |
| `data_rtc` / `hora_rtc` | text | data e hora do relógio RTC |
| `status_sistema` | text | NORMAL, ALERTA ou CRITICO |
| `previsao_tempo` | text | texto da previsão |
| `trava_segundos` | integer | trava da bomba (5, 10 ou 20) |
| `erro_dht` / `erro_ultra1` / `erro_ultra2` / `erro_rtc` | boolean | sensor com defeito? |
| `nivel_garrafa_cm` / `nivel_subsolo_cm` | numeric | cm de água |
| `temperatura_ar` / `umidade_ar` | numeric | ambiente |
| `umidade_solo` / `luz_media` | integer | porcentagem |
| `chovendo` / `rele_dreno` / `rele_encher` / `guia_led` | boolean | estados |

## Segurança (RLS)

A proteção fica em **Row Level Security**: com a chave **pública (anon)**, qualquer um pode **gravar** e **ler**, mas **não** apagar nem alterar — o histórico fica protegido. Por isso a chave pública pode ir para o GitHub; a chave *secret/service_role* **nunca**.

## Comandos úteis (no SQL Editor)

```sql
-- ver as últimas 10 leituras
select * from public.leituras order by criado_em desc limit 10;

-- apagar as linhas criadas pela página de teste
delete from public.leituras where status_sistema = 'TESTE';

-- zerar o histórico antes da apresentação
truncate table public.leituras restart identity;
```
