-- ================================================================
-- ALERTASUBSOLO - TABELA "leituras" (Supabase)
-- Onde colar: Supabase > SQL Editor > New query > cole tudo > Run
-- Pode rodar mais de uma vez sem problema (nao apaga nada).
-- ================================================================

-- 1) A TABELA ------------------------------------------------------
-- Cada linha = uma leitura enviada pelo Arduino (ou pela pagina de teste).
-- Os nomes das colunas sao IGUAIS aos nomes do JSON do Arduino.
create table if not exists public.leituras (
    id               bigint generated always as identity primary key,
    criado_em        timestamptz not null default now(),   -- o Supabase preenche sozinho

    data_rtc         text,       -- data do relogio RTC, ex: 15/11/26
    hora_rtc         text,       -- hora do relogio RTC, ex: 13:24:10
    status_sistema   text,       -- NORMAL, ALERTA ou CRITICO
    previsao_tempo   text,       -- texto da previsao
    trava_segundos   integer,    -- trava da bomba (5, 10 ou 20)

    erro_dht         boolean,    -- true = sensor com defeito
    erro_ultra1      boolean,
    erro_ultra2      boolean,
    erro_rtc         boolean,

    nivel_garrafa_cm numeric(5,1),   -- cm de agua na garrafa
    nivel_subsolo_cm numeric(5,1),   -- cm de agua no subsolo
    temperatura_ar   numeric(5,1),
    umidade_ar       numeric(5,1),
    umidade_solo     integer,        -- porcentagem
    chovendo         boolean,
    rele_dreno       boolean,        -- bomba 1 ligada?
    rele_encher      boolean,        -- bomba 2 ligada?
    guia_led         boolean,        -- fita LED acesa?
    luz_media        integer
);

-- Deixa a busca "ultimas leituras" rapida
create index if not exists leituras_criado_em_idx
    on public.leituras (criado_em desc);

-- 2) SEGURANCA (RLS) -----------------------------------------------
-- RLS = "quem pode fazer o que". Ligado, ninguem entra sem permissao.
alter table public.leituras enable row level security;

-- Permissao 1: qualquer um com a chave publica pode GRAVAR (o Arduino)
drop policy if exists "anon grava leituras" on public.leituras;
create policy "anon grava leituras"
    on public.leituras for insert
    to anon
    with check (true);

-- Permissao 2: qualquer um com a chave publica pode LER (o site)
drop policy if exists "anon le leituras" on public.leituras;
create policy "anon le leituras"
    on public.leituras for select
    to anon
    using (true);

-- Nao criamos permissao de apagar/alterar: pela chave publica
-- ninguem consegue apagar o historico.

-- 3) PERMISSOES DO ROLE "anon" -------------------------------------
-- Em projetos novos o Supabase pode nao liberar a tabela sozinho.
grant usage on schema public to anon;
grant select, insert on public.leituras to anon;
grant usage, select on all sequences in schema public to anon;

-- ================================================================
-- COMANDOS UTEIS (rode so quando precisar, um de cada vez):
--
--   Ver as ultimas 10 linhas:
--   select * from public.leituras order by criado_em desc limit 10;
--
--   Apagar as linhas criadas pela pagina de teste:
--   delete from public.leituras where status_sistema = 'TESTE';
--
--   Apagar TUDO antes da apresentacao (zera o historico):
--   truncate table public.leituras restart identity;
-- ================================================================
