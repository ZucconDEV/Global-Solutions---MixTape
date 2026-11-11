
# ALERTASUBSOLO
# Sistema de Monitoramento de Alagamentos

import math

altura_reservatorio = 20.0
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
[5] Configurar reservatório
[6] Sair
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

            if nivel_agua < 6:
                print("Nível de água normal")
            elif nivel_agua < 12:
                print("Nível de água em alerta")
            else:
                print("Nível de água crítico")

        case "2":
            print("===== HISTÓRICO DE MEDIÇÕES =====")

            if len(medicoes) == 0:
                print("Nenhuma medição registrada!")
            else:
                for agua in medicoes:
                    print(f"Nível de água igual a {agua} cm")

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

                if nivel_agua < 6:
                    print("STATUS: NORMAL")
                elif nivel_agua < 12:
                    print("STATUS: ALERTA")
                else:
                    print("STATUS: CRÍTICO")
                    print("ATENÇÃO: Risco de alagamento!")

        case "5":
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

        case "6":
            print("================================")
            print("Encerrando o AlertaSubsolo...")
            print("Obrigado por utilizar nosso sistema!")
            print("================================")
            break

        case _:
            print("Opção inválida! Tente novamente.")
