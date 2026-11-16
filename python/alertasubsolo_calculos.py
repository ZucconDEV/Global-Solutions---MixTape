# ALERTASUBSOLO
# Sistema de Monitoramento de Alagamentos (cálculos da maquete)
# Números iguais aos do código do Arduino.

import math

# ---------- constantes da maquete ----------
altura_reservatorio = 11.5      # cm, altura da parede do subsolo
limite_alerta = 1.5             # cm de água
limite_critico = 2.5            # cm de água
altura_sensor = 22.0            # cm, do sensor ao piso
area_piso = 22.5 * 15.5         # cm², cada 1 cm de água = 348,75 mL
vazao_bomba = 14.0              # mL por segundo 
velocidade_som = 0.034          # cm por µs (usada no Arduino para calcular a distância do sensor ultrasonico ao piso )
medicoes = []

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

            try:
                nivel_agua = float(input("Qual o nível da água em cm? "))
            except ValueError:
                print("Digite um número válido!")
                continue

            if not math.isfinite(nivel_agua):
                print("Medição inválida!")
                continue

            if nivel_agua < 0 or nivel_agua > altura_reservatorio:
                print("Medição inválida!")
                continue

            medicoes.append(nivel_agua)
            print(f"Medição registrada: {nivel_agua} cm")
            print(f"Volume no piso: {nivel_agua * area_piso:.0f} mL")

            if nivel_agua < limite_alerta:
                print("Situação: NORMAL")
            elif nivel_agua < limite_critico:
                print("Situação: ALERTA")
            else:
                print("Situação: CRÍTICO")

        case "2":
            print("===== HISTÓRICO DE MEDIÇÕES =====")

            if len(medicoes) == 0:
                print("Nenhuma medição registrada!")
            else:
                for agua in medicoes:
                    if agua < limite_alerta:
                        situacao = "NORMAL"
                    elif agua < limite_critico:
                        situacao = "ALERTA"
                    else:
                        situacao = "CRÍTICO"
                    print(f"{agua} cm ({agua * area_piso:.0f} mL) - {situacao}")

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
                posicao = len(medicoes) - 1
                nivel_agua = medicoes[posicao]

                print(f"Última medição: {nivel_agua} cm")

                if nivel_agua < limite_alerta:
                    print("STATUS: NORMAL")
                elif nivel_agua < limite_critico:
                    print("STATUS: ALERTA")
                else:
                    print("STATUS: CRÍTICO")
                    print("ATENÇÃO: Risco de alagamento!")

        case "5":
            print("===== NÍVEL PELO ECO DO SENSOR =====")

            try:
                eco = float(input("Tempo de eco em µs: "))
            except ValueError:
                print("Digite um número válido!")
                continue

            if not math.isfinite(eco) or eco <= 0:
                print("Eco inválido!")
                continue

            distancia = eco * velocidade_som / 2

            if distancia < 2 or distancia > 50:
                print(f"Distância {distancia:.2f} cm fora da faixa do sensor (2 a 50 cm).")
                continue

            nivel_agua = altura_sensor - distancia
            if nivel_agua < 0:
                nivel_agua = 0

            print(f"Distância lida: {distancia:.2f} cm")
            print(f"Nível da água: {nivel_agua:.2f} cm")

            if nivel_agua < limite_alerta:
                print("Situação: NORMAL")
            elif nivel_agua < limite_critico:
                print("Situação: ALERTA")
            else:
                print("Situação: CRÍTICO")

            guardar = input("Registrar esta medição? (s/n): ")
            if guardar == "s" and nivel_agua <= altura_reservatorio:
                medicoes.append(round(nivel_agua, 2))
                print("Medição registrada!")

        case "6":
            print("===== TEMPO DE DRENAGEM =====")

            try:
                nivel_agua = float(input("Altura da água em cm: "))
                entrada = float(input("Água entrando em mL/s (0 se não entra): "))
            except ValueError:
                print("Digite um número válido!")
                continue

            if not math.isfinite(nivel_agua) or not math.isfinite(entrada):
                print("Valor inválido!")
                continue

            if nivel_agua < 0 or entrada < 0:
                print("Os valores não podem ser negativos!")
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

            try:
                altura = float(input("Altura da água em cm: "))
                largura = float(input("Largura da parede em cm (22.5 ou 15.5): "))
            except ValueError:
                print("Digite um número válido!")
                continue

            if not math.isfinite(altura) or not math.isfinite(largura):
                print("Valor inválido!")
                continue

            if altura < 0 or largura <= 0:
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