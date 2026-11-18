-- ================================================================
-- ALERTASUBSOLO - PERMISSAO DE APAGAR (para os botoes do site)
-- Onde colar: Supabase > SQL Editor > New query > cole tudo > Run
-- Rode DEPOIS do 01_criar_tabela.sql.
-- ================================================================

-- Sem isto, os botoes "Apagar leituras de TESTE" e "Apagar TODOS
-- os registros" da pagina de teste nao apagam nada.

grant delete on public.leituras to anon;

drop policy if exists "anon apaga leituras" on public.leituras;
create policy "anon apaga leituras"
    on public.leituras for delete
    to anon
    using (true);

-- Observacao honesta para a documentacao: em um produto de verdade,
-- apagar historico jamais ficaria liberado para a chave publica.
-- Em um projeto escolar tudo bem, mas significa que qualquer pessoa
-- com o link do site consegue zerar a tabela.
