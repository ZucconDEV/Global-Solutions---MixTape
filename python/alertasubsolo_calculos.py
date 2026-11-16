# ALERTASUBSOLO
# Sistema de Monitoramento de Alagamentos (cálculos da maquete)
# Mesmo estilo do menu original. Números iguais aos do código do Arduino.

import math

# ---------- constantes da maquete ----------
altura_reservatorio = 11.5      # cm, altura da parede do subsolo
limite_alerta = 1.5             # cm de água
limite_critico = 2.5            # cm de água
altura_sensor = 22.0            # cm, do sensor ao piso
area_piso = 22.5 * 15.5         # cm², cada 1 cm de água = 348,75 mL
vazao_bomba = 14.0              # mL por segundo
velocidade_som = 0.034          # cm por µs (usada no Arduino)
medicoes = []


def classificar(nivel):
    if nivel >= limite_critico:
        return "CRÍTICO"
    elif nivel >= limite_alerta:
        return "ALERTA"
    else:
        return "NORMAL"


def ler_numero(pergunta):
    try:
        valor = float(input(pergunta))
    except ValueError:
        print("Digite um número válido!")
        return None
    if not math.isfinite(valor):
        print("Valor inválido!")
        return None
    return valor


while True:
    print("""
================================
         ALERTASUBSOLO
   Monitoramento de Alagamentos
================================
[1] Registrar nível da água
[2] Consultar histórico
[3] Ver estatísticas
[4] Consultar status de risco
[5] Calcular nível pelo eco do sensor
[6] Tempo de drenagem da bomba
[7] Força da água na parede
[8] Configurar reservatório
[9] Sair
================================
""")

    opcao = input("Escolha uma opção: ")

    match opcao:

        case "1":
            print("===== REGISTRAR NÍVEL DA ÁGUA =====")

            nivel_agua = ler_numero("Qual o nível da água em cm? ")
            if nivel_agua is None:
                continue

            if nivel_agua < 0 or nivel_agua > altura_reservatorio:
                print("Medição inválida!")
                continue

            medicoes.append(nivel_agua)
            print(f"Medição registrada: {nivel_agua} cm")
            print(f"Volume no piso: {nivel_agua * area_piso:.0f} mL")
            print(f"Situação: {classificar(nivel_agua)}")

        case "2":
            print("===== HISTÓRICO DE MEDIÇÕES =====")

            if len(medicoes) == 0:
                print("Nenhuma medição registrada!")
            else:
                for agua in medicoes:
                    print(f"{agua} cm ({agua * area_piso:.0f} mL) - {classificar(agua)}")

        case "3":
            print("===== ESTATÍSTICAS =====")

            if len(medicoes) == 0:
                print("Nenhuma medição registrada!")
            else:
                print(f"Total de medições: {len(medicoes)}")
                print(f"Menor nível: {min(medicoes)} cm")
                print(f"Maior nível: {max(medicoes)} cm")
                print(f"Média das medições: {sum(medicoes) / len(medicoes):.2f} cm")

        case "4":
            print("===== CONSULTAR STATUS DE RISCO =====")

            if len(medicoes) == 0:
                print("Nenhuma medição registrada!")
            else:
                nivel_agua = medicoes[len(medicoes) - 1]
                status = classificar(nivel_agua)

                print(f"Última medição: {nivel_agua} cm")
                print(f"STATUS: {status}")

                if status == "CRÍTICO":
                    print("ATENÇÃO: Risco de alagamento!")

        case "5":
            print("===== NÍVEL PELO ECO DO SENSOR =====")

            eco = ler_numero("Tempo de eco em µs: ")
            if eco is None or eco <= 0:
                print("Eco inválido!")
                continue

            distancia = eco * velocidade_som / 2
            nivel_agua = altura_sensor - distancia

            if distancia < 2 or distancia > 50:
                print(f"Distância {distancia:.2f} cm fora da faixa do sensor (2 a 50 cm).")
                continue

            if nivel_agua < 0:
                nivel_agua = 0

            print(f"Distância lida: {distancia:.2f} cm")
            print(f"Nível da água: {nivel_agua:.2f} cm")
            print(f"Situação: {classificar(nivel_agua)}")

            guardar = input("Registrar esta medição? (s/n): ")
            if guardar == "s" and nivel_agua <= altura_reservatorio:
                medicoes.append(round(nivel_agua, 2))
                print("Medição registrada!")

        case "6":
            print("===== TEMPO DE DRENAGEM =====")

            nivel_agua = ler_numero("Altura da água em cm: ")
            if nivel_agua is None or nivel_agua < 0:
                print("Altura inválida!")
                continue

            entrada = ler_numero("Água entrando em mL/s (0 se não entra): ")
            if entrada is None or entrada < 0:
                print("Valor inválido!")
                continue

            pulso = input("Bomba em pulso 5 s / 5 s? (s/n): ")

            saida = vazao_bomba
            if pulso == "s":
                saida = vazao_bomba / 2

            liquida = saida - entrada
            volume = nivel_agua * area_piso

            if liquida <= 0:
                print("A bomba não dá conta: o nível não baixa.")
            else:
                print(f"Volume a tirar: {volume:.0f} mL")
                print(f"Tempo para drenar: {volume / liquida:.0f} s")

        case "7":
            print("===== FORÇA DA ÁGUA NA PAREDE =====")

            altura = ler_numero("Altura da água em cm: ")
            largura = ler_numero("Largura da parede em cm (22.5 ou 15.5): ")
            if altura is None or largura is None or altura < 0 or largura <= 0:
                print("Valores inválidos!")
                continue

            forca = 0.5 * 1000 * 9.81 * (largura / 100) * (altura / 100) ** 2
            print(f"Força na parede: {forca:.2f} N ({forca / 9.81:.2f} kgf)")

        case "8":
            print("===== CONFIGURAR RESERVATÓRIO =====")
            print(f"Altura atual: {altura_reservatorio} cm")

            try:
                nova_altura = float(
                    input("Digite a nova altura do reservatório (cm): ")
                )

                if not math.isfinite(nova_altura) or nova_altura <= 0:
                    print("Altura inválida! Digite um número positivo.")
                else:
                    altura_reservatorio = nova_altura
                    print(
                        f"Reservatório configurado para "
                        f"{altura_reservatorio} cm!"
                    )
                    print("As medições anteriores foram mantidas.")

            except ValueError:
                print("Erro! Digite um número válido.")

        case "9":
            print("================================")
            print("Encerrando o AlertaSubsolo...")
            print("Obrigado por utilizar nosso sistema!")
            print("================================")
            break

        case _:
            print("Opção inválida! Tente novamente.")
