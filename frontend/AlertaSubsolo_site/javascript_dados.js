// ================================================================
// ALERTASUBSOLO - DADOS AO VIVO (parte do Gabriel)
// Le o banco de dados (Supabase) e preenche a pagina:
//   - cartoes com a ultima leitura
//   - tabela com o historico das ultimas 24 horas
//   - botao "Sincronizar com o banco"
// Nao usa nenhuma biblioteca, so o fetch do navegador.
// Nao altera nada do que o Rafael ja fez.
// ================================================================

(function () {
  "use strict";

  // Usa o config.js do site se existir; senao, o projeto "alertasubsolo".
  var CFG = (window.CONFIG) || {
    SUPABASE_URL: "https://bsksglxttgeqclmqgqex.supabase.co",
    SUPABASE_KEY: "sb_publishable_ys8S6VaMgueWXTrxlX9rYQ_w11BbfON",
    TABELA: "leituras"
  };

  // Atualiza sozinho a cada 20 s (alem do botao manual).
  var INTERVALO_MS = 20000;

  // ---------- helpers ----------
  function endereco() {
    return CFG.SUPABASE_URL.replace(/\/+$/, "") + "/rest/v1/" + CFG.TABELA;
  }
  function cabecalhos() {
    var h = { "apikey": CFG.SUPABASE_KEY };
    if (CFG.SUPABASE_KEY.indexOf("eyJ") === 0) {
      h["Authorization"] = "Bearer " + CFG.SUPABASE_KEY;
    }
    return h;
  }
  function umaCasa(v) { return (v === null || v === undefined) ? "--" : Number(v).toFixed(1); }
  function simNao(v) {
    if (v === null || v === undefined) return "--";
    return v ? "sim" : "não";
  }
  function limparStatus(t) {
    return String(t || "").normalize("NFD").replace(/[̀-ͯ]/g, "").toUpperCase();
  }
  function quando(iso) {
    var d = new Date(iso);
    return d.toLocaleDateString("pt-BR") + " " +
      d.toLocaleTimeString("pt-BR", { hour: "2-digit", minute: "2-digit", second: "2-digit" });
  }
  function el(id) { return document.getElementById(id); }

  // ---------- estado do botao/mensagem ----------
  function aviso(txt, tipo) {
    var m = el("dadosMensagem");
    if (!m) return;
    m.textContent = txt;
    m.className = "dados-msg " + (tipo || "");
  }

  // ---------- preencher os cartoes com a ultima leitura ----------
  function preencherAoVivo(ultima) {
    if (!ultima) return;
    var st = limparStatus(ultima.status_sistema) || "--";

    if (el("dGarrafa")) el("dGarrafa").textContent = umaCasa(ultima.nivel_garrafa_cm) + " cm";
    if (el("dSubsolo")) el("dSubsolo").textContent = umaCasa(ultima.nivel_subsolo_cm) + " cm";
    if (el("dTemp"))    el("dTemp").textContent = umaCasa(ultima.temperatura_ar) + " °C";

    var campo = el("dStatus");
    if (campo) {
      campo.textContent = ultima.status_sistema || "--";
      // reaproveita as classes do Rafael: .alerta (ambar) e .critico (vermelho)
      campo.className = "valor";
      if (st === "ALERTA") campo.className = "valor alerta";
      else if (st === "CRITICO") campo.className = "valor critico";
      else if (st === "NORMAL") campo.className = "valor normal-ok";
    }

    if (el("dChuva"))  el("dChuva").textContent = simNao(ultima.chovendo);
    if (el("dSolo"))   el("dSolo").textContent = (ultima.umidade_solo == null ? "--" : ultima.umidade_solo + "%");
    if (el("dQuando")) el("dQuando").textContent = "última leitura: " + quando(ultima.criado_em);
  }

  // ---------- desenhar a tabela de historico (24 h) ----------
  function desenharHistorico(dados) {
    var corpo = el("dadosCorpo");
    if (!corpo) return;
    corpo.innerHTML = "";

    if (!dados.length) {
      var tr = document.createElement("tr");
      var td = document.createElement("td");
      td.colSpan = 8;
      td.textContent = "Nenhuma leitura nas últimas 24 horas.";
      td.className = "dados-vazio";
      tr.appendChild(td);
      corpo.appendChild(tr);
      return;
    }

    dados.forEach(function (l) {
      var s = limparStatus(l.status_sistema);
      var tr = document.createElement("tr");
      function cel(txt, cls) {
        var td = document.createElement("td");
        td.textContent = txt;
        if (cls) td.className = cls;
        tr.appendChild(td);
      }
      cel(quando(l.criado_em));
      cel(l.status_sistema || "--", "st-" + s);
      cel(umaCasa(l.nivel_garrafa_cm));
      cel(umaCasa(l.nivel_subsolo_cm));
      cel(umaCasa(l.temperatura_ar));
      cel(simNao(l.chovendo));
      cel(simNao(l.rele_dreno));
      cel(simNao(l.rele_encher));
      corpo.appendChild(tr);
    });
  }

  // ---------- buscar no banco ----------
  async function sincronizar() {
    aviso("Sincronizando com o banco…", "");
    var desde = new Date(Date.now() - 24 * 60 * 60 * 1000).toISOString();
    var url = endereco() + "?select=*&criado_em=gte." + desde +
      "&order=criado_em.desc&limit=1000";
    try {
      var r = await fetch(url, { headers: cabecalhos() });
      if (!r.ok) {
        aviso("O banco respondeu com erro " + r.status + ". Confira a chave/URL no config.js.", "erro");
        return;
      }
      var dados = await r.json();
      desenharHistorico(dados);
      if (dados.length) preencherAoVivo(dados[0]);
      var total = el("dTotal");
      if (total) total.textContent = dados.length;
      aviso("Sincronizado às " + new Date().toLocaleTimeString("pt-BR") +
        " · " + dados.length + " registros nas últimas 24 h.", "ok");
    } catch (e) {
      aviso("Não consegui falar com o banco. Confira a internet e a URL no config.js.", "erro");
    }
  }

  // ---------- ligar tudo quando a pagina abrir ----------
  function iniciar() {
    var botao = el("btnSincronizar");
    if (botao) {
      botao.addEventListener("click", function () {
        botao.disabled = true;
        sincronizar().then(function () { botao.disabled = false; });
      });
    }
    sincronizar();
    setInterval(sincronizar, INTERVALO_MS);
  }

  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", iniciar);
  } else {
    iniciar();
  }
})();
